#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "LavaVK/LavaVK.hpp"
#include "../external/glm/glm/glm.hpp"

#include "Window.hpp"
#include "Container.hpp"
#include "Framework.h"
#include "Frameworks.h"
#include "Module.hpp"
#include "Resource.hpp"
#include "Modules.hpp"
#include "Scheduler.hpp"

namespace LavaEngine
{
    using namespace LavaVK;

    class Application
    {
    public:
        Application() = default;

        ~Application()
        {
            shutdown();
        }

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        Application(Application&&) = delete;
        Application& operator=(Application&&) = delete;

        void run()
        {
            while (
                m_scheduler.m_jobs_completed.size()
                <
                m_scheduler.m_jobs.size()
            )
            {
                m_scheduler.execute();
            }
        }

        int step()
        {
            if (m_scheduler.m_jobs_completed.size() >= m_scheduler.m_jobs.size()) return 1;
            m_scheduler.execute();
            return 0;
        }

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

        template <typename T, typename... Args>
        T& addFramework(Args&&... args)
        {
            auto framework =
                std::make_unique<T>(
                    *this,
                    std::forward<Args>(args)...
                );

            T& reference = *framework;

            m_frameworks.push_back(
                std::move(framework)
            );

            return reference;
        }

        template <typename T>
        T* findFramework()
        {
            for (auto& framework : m_frameworks)
            {
                if (auto* result = dynamic_cast<T*>(framework.get()))
                {
                    return result;
                }
            }

            return nullptr;
        }

        template <typename T>
        T* findContainer()
        {
            for (auto& container : m_containers)
            {
                if (auto* result = dynamic_cast<T*>(container.get()))
                {
                    return result;
                }
            }

            return nullptr;
        }

        Scheduler* getScheduler()
        {
            return &m_scheduler;
        }

        void unloadGame()
        {
            // Shutdown all frameworks before destroying containers.
            for (auto& framework : m_frameworks)
                framework->shutdown();

            m_scheduler.clear();
            m_containers.clear();

            m_frameworks.clear();
        }

    private:
        void shutdown()
        {
            if (m_shutdown)
                return;

            m_shutdown = true;

            unloadGame();


        }

        Scheduler m_scheduler;

        std::vector<std::unique_ptr<Container>> m_containers;
        std::vector<std::unique_ptr<Framework>> m_frameworks;

        bool m_shutdown = false;
    };
}