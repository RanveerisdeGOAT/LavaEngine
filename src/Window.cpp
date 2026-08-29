#include "LavaEngine/Window.hpp"

#include <stdexcept>

namespace LavaEngine
{
    Window::Window(
        int width,
        int height,
        const std::string& name
    )
    {
        if (!glfwInit())
            throw std::runtime_error(
                "Failed to initialize GLFW"
            );

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_window = glfwCreateWindow(
            width,
            height,
            name.c_str(),
            nullptr,
            nullptr
        );

        if (!m_window)
        {
            glfwTerminate();

            throw std::runtime_error(
                "Failed to create GLFW window"
            );
        }
    }

    Window::~Window()
    {
        if (m_window)
            glfwDestroyWindow(m_window);

        glfwTerminate();
    }

    GLFWwindow* Window::getGlfwWindow() const
    {
        return m_window;
    }

    std::vector<const char*>
    Window::getRequiredInstanceExtensions()
    {
        uint32_t count = 0;

        const char** extensions =
            glfwGetRequiredInstanceExtensions(&count);

        if (!extensions)
            throw std::runtime_error(
                "Failed to get GLFW Vulkan extensions"
            );

        return {
            extensions,
            extensions + count
        };
    }
}