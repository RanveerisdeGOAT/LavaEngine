#pragma once

#include "Module.hpp"
#include "LavaVK/LavaVK.hpp"

namespace LavaEngine
{
    using namespace LavaVK;

    class Renderer : public Module
    {
    public:

        Renderer(
            Device& device,
            Surface& surface,
            uint32_t width,
            uint32_t height
        );

        ~Renderer() override = default;

        [[nodiscard]] CommandBuffer& getCommandBuffer()
        {
            return m_device.getCommandPool(QueueType::GRAPHICS)
                            .retrieve(m_frameIndex);
        }

        Result acquire();

        void record(const std::function<void(CommandBuffer &)> &cmd);

        void submit(
            const std::vector<std::reference_wrapper<const Semaphore> > &waitSemaphores = {},
            const std::vector<PipelineStage> &waitStages = {},
            const std::vector<std::reference_wrapper<const Semaphore> > &signalSemaphores = {},
            const Fence *fence = {}
        ) const;

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

    private:

        Device& m_device;
        Surface& m_surface;

        RenderPass m_renderPass;
        SwapChain m_swapChain;

        CommandPool& m_commandPool;

        uint32_t m_imageIndex = 0;
        size_t m_frameIndex = 0;
    };
}