#include "LavaEngine/LavaEngine.hpp"

#include <stdexcept>

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
                          &surface) != VK_SUCCESS)
                  {
                      throw std::runtime_error(
                          "Failed to create Vulkan surface"
                      );
                  }

                  return surface;
              })
    {
    }

    std::string LavaEngine::getName() const
    {
        return glfwGetWindowTitle(
            m_window.getGlfwWindow()
        );
    }
}