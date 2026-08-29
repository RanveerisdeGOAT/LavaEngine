#pragma once

#include <string>

#include "LavaVK/LavaVK.hpp"

#include "Window.hpp"
#include "Container.hpp"
#include "Module.hpp"
#include "Resource.hpp"
#include "Modules.hpp"


namespace LavaEngine
{
    using namespace LavaVK;

    class LavaEngine
    {
    public:
        LavaEngine(
            int width,
            int height,
            const std::string& name
        );

        ~LavaEngine() = default;

        [[nodiscard]] std::string getName() const;

        Window* getWindow()
        {
            return &m_window;
        }

        Instance* getInstance()
        {
            return &m_instance;
        }

        Surface* getSurface()
        {
            return &m_surface;
        }

    private:
        Window m_window;
        Instance m_instance;
        Surface m_surface;
    };
}