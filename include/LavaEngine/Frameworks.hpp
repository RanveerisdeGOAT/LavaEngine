#pragma once

#include <functional>
#include <stdexcept>
#include <string>

#include <GLFW/glfw3.h>

#include "Framework.hpp"
#include "LavaVK/LavaVK.hpp"
#include "Window.hpp"

namespace LavaEngine
{
    class Application;


    class VulkanRenderer : public Framework
    {
    public:
        using OverlayCallback = std::function<void(CommandBuffer&)>;

        explicit VulkanRenderer(
            Application& engine,
            int width,
            int height,
            const std::string& name
        )
            : m_engine(engine),

              m_window(
                  width,
                  height,
                  name
              ),

              m_instance({
                  .applicationName = name,
                  .extensions =
                  Window::getRequiredInstanceExtensions()
              }),

              m_surface(
                  m_instance,
                  [this](VkInstance vkInstance) -> VkSurfaceKHR
                  {
                      VkSurfaceKHR surface =
                          VK_NULL_HANDLE;

                      if (
                          glfwCreateWindowSurface(
                              vkInstance,
                              m_window.getGlfwWindow(),
                              nullptr,
                              &surface
                          ) != VK_SUCCESS
                      )
                      {
                          throw std::runtime_error(
                              "Failed to create Vulkan surface"
                          );
                      }

                      return surface;
                  }
              )
        {
            const auto& selectedGPU =
                GPUHardware::selectOptimalGPU(
                    m_instance,
                    m_surface
                );

            m_device = Device(
                selectedGPU,
                {
                    QueueType::GRAPHICS,
                    QueueType::PRESENT
                },
                &m_surface
            );

            m_renderPass = RenderPass(
                &m_device,

                Format(
                    ChannelOrder::BGRA,
                    BitDepth::B8,
                    NumericType::Srgb
                ),

                Format(
                    ChannelOrder::D,
                    BitDepth::B32,
                    NumericType::Float
                )
            );

            m_swapChain = SwapChain(
                m_device,
                m_surface,
                m_renderPass,
                m_renderPass.getColorFormat(),
                m_renderPass.getDepthFormat(),
                VkExtent2D(width, height)
            );

            m_commandPool = &m_device.getCommandPool(QueueType::GRAPHICS);

            m_commandPool->allocate(
                MAX_FRAMES_IN_FLIGHT
            );
        }

        Result acquire()
        {
            Result result =
                m_swapChain.acquireImage(m_imageIndex);

            if (!result)
            {
                recreate();
                return result;
            }

            m_frameIndex =
                m_swapChain.currentFrame();

            return result;
        }

        void record(const std::function<void(CommandBuffer&)>& cmd)
        {
            CommandBuffer& cmdBuffer = getCommandBuffer();

            const bool offscreen =
                m_offscreenFramebuffer && m_offscreenRenderPass;

            cmdBuffer.reset();
            cmdBuffer.begin();

            if (offscreen)
            {
                cmdBuffer.beginRenderPass(
                    *m_offscreenRenderPass,
                    *m_offscreenFramebuffer,
                    m_offscreenExtent
                );

                cmd(cmdBuffer);

                cmdBuffer.endRenderPass();
            }
            else
            {
                cmdBuffer.beginRenderPass(
                    m_renderPass,
                    m_swapChain.framebuffer(m_imageIndex),
                    m_swapChain.extent()
                );

                cmd(cmdBuffer);

                cmdBuffer.endRenderPass();
            }

            // The overlay (ImGui) is only ever installed together with an
            // offscreen target (the Inspector sets both as a pair), so
            // whenever it runs, the swapchain image hasn't been touched
            // yet this frame — it's safe, and correct, for this pass to
            // clear it before the overlay draws on top.
            if (m_overlayCallback)
            {
                cmdBuffer.beginRenderPass(
                    m_renderPass,
                    m_swapChain.framebuffer(m_imageIndex),
                    m_swapChain.extent()
                );

                m_overlayCallback(cmdBuffer);

                cmdBuffer.endRenderPass();
            }

            cmdBuffer.end();
        }

        void submit(
            const std::vector<std::reference_wrapper<const Semaphore>>& waitSemaphores = {},
            const std::vector<PipelineStage>& waitStages = {},
            const std::vector<std::reference_wrapper<const Semaphore>>& signalSemaphores = {},
            const Fence* fence = {}
        )
        {
            m_device.submit(
                QueueType::GRAPHICS,
                m_swapChain.currentFrame(),
                (!waitSemaphores.empty())
                    ? waitSemaphores
                    : std::vector<std::reference_wrapper<const Semaphore>>
                    {
                        m_swapChain.imageAvailableSemaphore()
                    },
                (!waitStages.empty())
                    ? waitStages
                    : std::vector<PipelineStage>{
                        PipelineStage::ColorAttachmentOutput
                    },
                (!signalSemaphores.empty())
                    ? signalSemaphores
                    : std::vector<std::reference_wrapper<const Semaphore>>{
                        m_swapChain.renderFinishedSemaphore(m_imageIndex)
                    },
                &m_swapChain.inFlightFence()
            );
        }

        void shutdown() override
        {
            device().waitIdle();
        }

        Application& application() const
        {
            return m_engine;
        }

        Window& window()
        {
            return m_window;
        }

        Instance& instance()
        {
            return m_instance;
        }

        Surface& surface()
        {
            return m_surface;
        }

        Device& device()
        {
            return m_device;
        }


        const Window& window() const
        {
            return m_window;
        }

        const Instance& instance() const
        {
            return m_instance;
        }

        const Surface& surface() const
        {
            return m_surface;
        }

        const Device& device() const
        {
            return m_device;
        }

        [[nodiscard]] CommandBuffer& getCommandBuffer()
        {
            return m_device.getCommandPool(QueueType::GRAPHICS)
                           .retrieve(m_frameIndex);
        }

        void present()
        {
            m_swapChain.present(m_imageIndex);
        }

        void recreate()
        {
            m_swapChain.recreate();
        }

        [[nodiscard]] SwapChain& swapChain()
        {
            return m_swapChain;
        }

        [[nodiscard]] RenderPass& renderPass()
        {
            return m_renderPass;
        }


        std::string getName() const
        {
            return glfwGetWindowTitle(
                m_window.getGlfwWindow()
            );
        }

        // --- Overlay (ImGui) ---

        void setOverlayCallback(OverlayCallback callback)
        {
            m_overlayCallback = std::move(callback);
        }

        void clearOverlayCallback()
        {
            m_overlayCallback = nullptr;
        }

        void setOffscreenTarget(
            Framebuffer* framebuffer,
            RenderPass* renderPass,
            VkExtent2D extent
        )
        {
            m_offscreenFramebuffer = framebuffer;
            m_offscreenRenderPass = renderPass;
            m_offscreenExtent = extent;
        }

        void clearOffscreenTarget()
        {
            m_offscreenFramebuffer = nullptr;
            m_offscreenRenderPass = nullptr;
        }

    private:
        Application& m_engine;

        Window m_window;

        Instance m_instance;
        Surface m_surface;
        Device m_device;
        RenderPass m_renderPass;
        SwapChain m_swapChain;
        CommandPool* m_commandPool = nullptr;

        uint32_t m_imageIndex = 0;
        size_t m_frameIndex = 0;

        OverlayCallback m_overlayCallback;

        Framebuffer* m_offscreenFramebuffer = nullptr;
        RenderPass* m_offscreenRenderPass = nullptr;
        VkExtent2D m_offscreenExtent{};
    };
}