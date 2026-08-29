#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "LavaVK/LavaVK.hpp"
#include <GLFW/glfw3.h>

namespace LavaEngine
{
    using namespace LavaVK;

    class Window
    {
    public:
        Window(int width, int height, const std::string& name);
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        [[nodiscard]] GLFWwindow* getGlfwWindow() const;

        static std::vector<const char*> getRequiredInstanceExtensions() ;

    private:
        GLFWwindow* m_window = nullptr;
    };
}