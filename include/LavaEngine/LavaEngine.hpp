#pragma once

#include <string>

#include "LavaVK/LavaVK.hpp"
#include "../external/glm/glm/glm.hpp"

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

        // Non-copyable and non-movable
        LavaEngine(const LavaEngine&) = delete;
        LavaEngine& operator=(const LavaEngine&) = delete;
        LavaEngine(LavaEngine&&) = delete;
        LavaEngine& operator=(LavaEngine&&) = delete;

        [[nodiscard]]
        std::string getName() const;
        void run();

        template <typename T, typename... Args>
        T& createContainer(Args&&... args)
        {
            auto container =
                std::make_unique<T>(
                    std::forward<Args>(args)...
                );

            T& ref = *container;

            m_containers.push_back(
                std::move(container)
            );

            return ref;
        }

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

        std::vector<std::unique_ptr<Container>> m_containers;
    };
};
