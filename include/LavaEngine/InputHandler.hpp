#pragma once

#include <array>

#include "LavaVK/LavaVK.hpp"
#include <GLFW/glfw3.h>

#include "../external/glm/glm/glm.hpp"

namespace LavaEngine
{
    class Window;
    /**
     * @brief Frame-based keyboard/mouse state for a single GLFW window.
     * @detail Polls GLFW during update() and records current/previous key
     * and mouse state so pressed/released edges can be detected.
     * @note Ownership: Owned by `Window` as a value member. It borrows the
     * native `GLFWwindow` pointer from its owning Window; that pointer is
     * valid only while the owning Window owns a live GLFW window.
     * @example
     * @code
     * if (input.isKeyPressed(GLFW_KEY_SPACE)) jump();
     * glm::vec2 delta = input.mouseDelta();
     * @endcode
     */
    class InputHandler
    {
    public:
        explicit InputHandler(Window& window);

        // Not copyable: holds frame-to-frame state tied to one window.
        InputHandler(const InputHandler&) = delete;
        InputHandler& operator=(const InputHandler&) = delete;

        // Movable: m_window is a non-owning pointer borrowed from a
        // Window, so moving just transfers it (and the current state
        // snapshot) and leaves the source pointing at nothing.
        InputHandler(InputHandler&& other) noexcept;
        InputHandler& operator=(InputHandler&& other) noexcept;

        void update();

        [[nodiscard]] bool isKeyDown(int key) const;
        [[nodiscard]] bool isKeyPressed(int key) const;
        [[nodiscard]] bool isKeyReleased(int key) const;

        [[nodiscard]] bool isMouseButtonDown(int button) const;
        [[nodiscard]] bool isMouseButtonPressed(int button) const;
        [[nodiscard]] bool isMouseButtonReleased(int button) const;

        [[nodiscard]] glm::vec2 mousePosition() const { return m_mousePosition; }
        [[nodiscard]] glm::vec2 mouseDelta() const { return m_mouseDelta; }

        void setCursorMode(int mode) const;

        [[nodiscard]] bool isFocused() const { return m_focused; }

        void setFocused(const bool focused) { m_focused = focused; }

        void focus()
        {
            if (m_focused) return;
            if (!m_window) return;
            m_focused = true;

            glfwSetInputMode(
                m_window,
                GLFW_CURSOR,
                GLFW_CURSOR_NORMAL
            );

            m_firstUpdate = true;
        }

        void unfocus()
        {
            if (!m_focused) return;
            if (!m_window) return;
            m_focused = false;

            glfwSetInputMode(
                m_window,
                GLFW_CURSOR,
                GLFW_CURSOR_NORMAL
            );

            m_mouseDelta = {0.0f, 0.0f};
            m_firstUpdate = true;
        }

    private:
        static constexpr int KeyCount = GLFW_KEY_LAST + 1;
        static constexpr int MouseButtonCount = GLFW_MOUSE_BUTTON_LAST + 1;

        GLFWwindow* m_window = nullptr;

        std::array<bool, KeyCount> m_currentKeys{};
        std::array<bool, KeyCount> m_previousKeys{};

        std::array<bool, MouseButtonCount> m_currentMouseButtons{};
        std::array<bool, MouseButtonCount> m_previousMouseButtons{};

        glm::vec2 m_mousePosition{0.0f};
        glm::vec2 m_previousMousePosition{0.0f};
        glm::vec2 m_mouseDelta{0.0f};

        bool m_firstUpdate = true;

        bool m_focused = true;
    };
}