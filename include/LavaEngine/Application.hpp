#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../../LavaVK/include/LavaVK/LavaVK.hpp"
#include "../../external/glm/glm/glm.hpp"

#include "Window.hpp"
#include "Container.hpp"
#include "Framework.hpp"
#include "Frameworks.hpp"
#include "Logger.hpp"
#include "Module.hpp"
#include "Resource.hpp"
#include "Modules.hpp"
#include "Scheduler.hpp"

namespace LavaEngine
{
    using namespace LavaVK;

    /**
     * @brief Root object and owner of a LavaEngine game session.
     * @detail Owns the Scheduler, Logger, framework (Framework), and every
     * Container created through createContainer(). Lifecycle is driven
     * through this object: create containers/frameworks, then run() or
     * step() the game loop.
     * @note Ownership: `Application` is the session root. It uniquely owns
     * the framework, the scheduler's jobs, the containers, and the logger.
     * References and pointers borrowed from it (e.g. getScheduler(),
     * getLogger(), findFramework<>(), findContainer<>()) are valid only
     * while the Application outlives them. unloadGame() teardown runs in fixed phases:
     * (1) container/module onUnload() hooks, (2) framework shutdown(),
     * (3) scheduler job destruction, (4) container destruction, (5) framework
     * destruction. A teardown hook at phase 1 is the last point at which
     * non-owning references (e.g. the renderer's overlay callback) may be
     * released while the borrowed-from objects are still alive.
     * @example
     * @code
     * LavaEngine::Application app;
     * auto& world = app.createContainer<Container>("world");
     * app.setFramework<VulkanRenderer>(app, 1280, 720, "Game");
     * app.run();
     * @endcode
     */
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

        Logger* getLogger()
        {
            return &m_logger;
        }

        void unloadGame()
        {
            // Phase 1 - teardown hooks: give containers/modules a chance
            // to release borrowed references while the framework is alive.
            for (auto& container : m_containers)
                container->onUnload();

            // Phase 2 - framework detach: release the framework's
            // references into container-owned objects (e.g. overlay
            // callback, offscreen target).
            if (m_framework) m_framework->shutdown();

            // Phase 3 - scheduler: destroy jobs before the objects they
            // may borrow from.
            m_scheduler.clear();

            // Phase 4 - containers: destroy modules, then resources.
            m_containers.clear();

            // Phase 5 - framework (shutdown() already ran; resetting
            // prevents a second shutdown() call).
            if (m_framework) m_framework.reset();
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
        Logger m_logger;

        std::vector<std::unique_ptr<Container>> m_containers;
        std::unique_ptr<Framework> m_framework;

        bool m_shutdown = false;
    };
}