#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "LavaVK/LavaVK.hpp"
#include <GLFW/glfw3.h>

#include "InputHandler.hpp"

namespace LavaEngine
{
    using namespace LavaVK;

    class Window
    {
    public:
        Window(int width, int height, const std::string& name);

        ~Window();

        // Copying is not allowed.
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        // Moving is allowed.
        Window(Window&& other) noexcept;
        Window& operator=(Window&& other) noexcept;
        [[nodiscard]] int getWidth() const;
        [[nodiscard]] int getHeight() const;

        [[nodiscard]]
        GLFWwindow* getGlfwWindow() const;

        static std::vector<const char*> getRequiredInstanceExtensions();

        [[nodiscard]]
        InputHandler& getInputHandler()
        {
            return m_inputHandler;
        }

        [[nodiscard]]
        const InputHandler& getInputHandler() const
        {
            return m_inputHandler;
        }

        [[nodiscard]]
        bool shouldClose() const
        {
            return glfwWindowShouldClose(getGlfwWindow());
        }

        [[nodiscard]]
        float getTime() const
        {
            return static_cast<float>(glfwGetTime());
        }

        float tick()
        {
            const float dt = getTime() - m_lastFrameTime;
            m_lastFrameTime = getTime();
            return dt;
        }

    private:
        GLFWwindow* m_window = nullptr;
        InputHandler m_inputHandler;
        float m_lastFrameTime = 0;
    };
}