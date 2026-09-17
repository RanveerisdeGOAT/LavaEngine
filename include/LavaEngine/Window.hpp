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

    /**
     * @brief Owns a native GLFW window and its input state.
     * @detail Creates a Vulkan-capable GLFW window and an InputHandler bound
     * to it. Non-copyable, movable.
     * @note Ownership: Owned by `VulkanRenderer` as a value member. Window
     * owns the native `GLFWwindow` (destroyed in ~Window) and its
     * `InputHandler`. A moved-from Window holds a null GLFW handle and must
     * not be used. GLFW initialization/termination is process-wide; the
     * destructor calls glfwTerminate(), so no more than one Window should
     * be alive at a time.
     * @example
     * @code
     * Window window(1280, 720, "Game");
     * while (!window.shouldClose()) { window.getInputHandler().update(); }
     * @endcode
     */
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