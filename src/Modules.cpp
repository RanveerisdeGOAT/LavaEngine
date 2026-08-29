#include "../include/LavaEngine/Modules.hpp"

namespace LavaEngine
{
    Renderer::Renderer(
        Device& device,
        Surface& surface,
        uint32_t width,
        uint32_t height
    )
        : m_device(device)
        , m_surface(surface)
        , m_renderPass(
            &device,

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
        )
        , m_swapChain(
            device,
            surface,
            m_renderPass,
            m_renderPass.getColorFormat(),
            m_renderPass.getDepthFormat(),
            {width, height}
        )
        , m_commandPool(
            device.getCommandPool(
                QueueType::GRAPHICS
            )
        )
    {
        m_commandPool.allocate(
            MAX_FRAMES_IN_FLIGHT
        );
    }

    Result Renderer::acquire()
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

    void Renderer::record(const std::function<void(CommandBuffer&)>& cmd)
    {
        CommandBuffer& cmdBuffer = getCommandBuffer();
        cmdBuffer.record(
            m_renderPass,
            m_swapChain.framebuffer(m_imageIndex),
            m_swapChain.extent(),
            cmd
        );
    }

    void Renderer::submit(const std::vector<std::reference_wrapper<const Semaphore>>& waitSemaphores, const std::vector<PipelineStage>& waitStages, const std::vector<std::reference_wrapper<const Semaphore>>& signalSemaphores, const Fence* fence) const
    {
        m_device.submit(
            QueueType::GRAPHICS,
            m_swapChain.currentFrame(),
            (!waitSemaphores.empty())? waitSemaphores:std::vector<std::reference_wrapper<const Semaphore>>
            {
                m_swapChain.imageAvailableSemaphore()
            },
            (!waitStages.empty())?waitStages:std::vector<PipelineStage>{
                PipelineStage::ColorAttachmentOutput
            },
            (!signalSemaphores.empty())?signalSemaphores:std::vector<std::reference_wrapper<const Semaphore>>{
                m_swapChain.renderFinishedSemaphore(m_imageIndex)
            },
            &m_swapChain.inFlightFence()
        );
    }
}
