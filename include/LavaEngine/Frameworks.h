#pragma once

#include <stdexcept>
#include <string>

#include <GLFW/glfw3.h>

#include "Framework.h"
#include "LavaVK/LavaVK.hpp"
#include "Window.hpp"

namespace LavaEngine
{
    class Application;


    class Vulkan : public Framework
    {
    public:

        explicit Vulkan(
            Application& engine,
            int width,
            int height,
            const std::string& name
        )
            : Framework("Vulkan"),
              m_engine(engine),

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


        std::string getName() const
        {
            return glfwGetWindowTitle(
                m_window.getGlfwWindow()
            );
        }


    private:

        Application& m_engine;

        Window m_window;

        Instance m_instance;
        Surface m_surface;
        Device m_device;
    };
}
