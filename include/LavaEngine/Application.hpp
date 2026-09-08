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

        [[nodiscard]]
        const std::vector<std::unique_ptr<Container>>& containers() const
        {
            return m_containers;
        }


        template <typename T, typename... Args>
        T& setFramework(Args&&... args)
        {
            auto framework =
                std::make_unique<T>(
                    *this,
                    std::forward<Args>(args)...
                );

            T& ref = *framework;

            m_framework = std::move(framework);

            return ref;
        }

        template <typename T>
        T* findFramework()
        {
            return dynamic_cast<T*>(m_framework.get());
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
            if (m_framework) m_framework->shutdown();
            m_scheduler.clear();
            m_containers.clear();
            if (m_framework) m_framework.reset();   // prevents a second shutdown() call
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
        std::unique_ptr<Framework> m_framework;

        bool m_shutdown = false;
    };
}