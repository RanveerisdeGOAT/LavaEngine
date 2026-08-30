#pragma once

#include <string>

#include "LavaVK/LavaVK.hpp"

#include "Window.hpp"
#include "Container.hpp"
#include "Module.hpp"
#include "Resource.hpp"
#include "Modules.hpp"
#include "Scheduler.hpp"

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

        // Non-copyable
        LavaEngine(const LavaEngine&) = delete;
        LavaEngine& operator=(const LavaEngine&) = delete;

        // Moveable
        LavaEngine(LavaEngine&& other) noexcept;
        LavaEngine& operator=(LavaEngine&& other) noexcept;

        [[nodiscard]]
        std::string getName() const;
        void run();

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

        Scheduler* getScheduler()
        {
            return &m_scheduler;
        }

        Device* getDevice()
        {
            return &m_device;
        }
    private:
        Window m_window;
        Instance m_instance;
        Surface m_surface;
        Device m_device;
        Scheduler m_scheduler;
    };
}