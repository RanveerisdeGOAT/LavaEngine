#pragma once
#include <string>

namespace LavaEngine
{
    /**
     * @brief Abstract base class for top-level engine subsystems driven by an Application.
     * @detail A framework is a high-level subsystem (e.g. the Vulkan
     * renderer) installed on an Application via setFramework<T>(). It is
     * owned and destroyed solely by that Application.
     * @note Ownership: Owned by `Application` (m_framework, std::unique_ptr).
     * Concrete frameworks own their internal members; they must not outlive
     * the owning Application. shutdown() is invoked by the Application
     * during teardown and must release any references the framework holds
     * into container/scene-owned objects.
     * @example
     * @code
     * struct MyFramework : Framework { void shutdown() override { ... } };
     * app.setFramework<MyFramework>(app);
     * @endcode
     */
    class Framework
    {
    public:
        explicit Framework()= default;

        virtual ~Framework() = default;

        virtual void shutdown() {}

        Framework(const Framework&) = delete;
        Framework& operator=(const Framework&) = delete;
    };
}
