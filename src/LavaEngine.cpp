#include "LavaEngine/LavaEngine.hpp"

#include <stdexcept>
#include <utility>

namespace LavaEngine
{
    LavaEngine::LavaEngine(
        int width,
        int height,
        const std::string& name
    )
        : m_window(width, height, name),
          m_instance({
              .applicationName = name,
              .extensions =
                  Window::getRequiredInstanceExtensions()
          }),
          m_surface(
              m_instance,
              [this](VkInstance vkInst) -> VkSurfaceKHR
              {
                  VkSurfaceKHR surface = VK_NULL_HANDLE;

                  if (glfwCreateWindowSurface(
                          vkInst,
                          m_window.getGlfwWindow(),
                          nullptr,
                          &surface
                      ) != VK_SUCCESS)
                  {
                      throw std::runtime_error(
                          "Failed to create Vulkan surface"
                      );
                  }

                  return surface;
              })
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
    }

    LavaEngine::LavaEngine(
        LavaEngine&& other
    ) noexcept
        : m_window(std::move(other.m_window)),
          m_instance(std::move(other.m_instance)),
          m_surface(std::move(other.m_surface)),
          m_device(std::move(other.m_device)),
          m_scheduler(std::move(other.m_scheduler))
    {
    }

    LavaEngine& LavaEngine::operator=(
        LavaEngine&& other
    ) noexcept
    {
        if (this == &other)
            return *this;

        m_device = std::move(other.m_device);
        m_surface = std::move(other.m_surface);
        m_instance = std::move(other.m_instance);
        m_window = std::move(other.m_window);
        m_scheduler = std::move(other.m_scheduler);

        return *this;
    }

    std::string LavaEngine::getName() const
    {
        return glfwGetWindowTitle(
            m_window.getGlfwWindow()
        );
    }

    void LavaEngine::run()
    {
        m_scheduler.execute();
        m_device.waitIdle();
    }
}