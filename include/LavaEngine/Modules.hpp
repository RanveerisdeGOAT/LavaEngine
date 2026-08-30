#pragma once

#include "Module.hpp"
#include "LavaVK/LavaVK.hpp"

namespace LavaEngine
{
    using namespace LavaVK;

    class RenderingModule : public Module
    {
    public:

        RenderingModule(
            Device& device,
            Surface& surface,
            uint32_t width,
            uint32_t height
        );

        ~RenderingModule() override = default;

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

    class GraphicalPiplineModule : public Module
    {
    public:

        GraphicalPiplineModule(
            Device& device,
            PipelineLayout& layout,
            RenderPass& renderPass,
            const std::string& vertexShader,
            const std::string& fragmentShader,
            Topology topology = Topology::TRIANGLES,
            PolygonMode polygonMode = PolygonMode::FILL,
            CullMode cullMode = CullMode::NONE,
            FrontFace frontFace = FrontFace::COUNTER_CLOCKWISE,
            bool depthTest = true,
            bool depthWrite = true,
            bool blending = false
        );

        ~GraphicalPiplineModule() override = default;

        [[nodiscard]]
        GraphicsPipeline& pipeline()
        {
            return m_pipeline;
        }

        [[nodiscard]]
        const GraphicsPipeline& pipeline() const
        {
            return m_pipeline;
        }

        [[nodiscard]]
        const Shader& vertexShader() const
        {
            return m_vertexShader;
        }

        [[nodiscard]]
        const Shader& fragmentShader() const
        {
            return m_fragmentShader;
        }

    private:
        Shader m_vertexShader;
        Shader m_fragmentShader;
        GraphicsPipeline m_pipeline;
    };
}