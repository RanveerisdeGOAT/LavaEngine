#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <algorithm>


namespace LavaEngine
{
    class Container;

    /**
     * @brief Base class for all gameplay/engine logic hosted by a Container.
     * @detail Subclass Module and add it to a Container's ModuleRegistry via
     * Container::addModule<T>(). The registry owns the module and sets its
     * container back-pointer.
     * @note Ownership: Owned (as std::unique_ptr<Module>) by the owning
     * ModuleRegistry/Container. `m_container` is a borrowed, non-owning
     * back-pointer and is only valid while the module is owned by that
     * registry; a module must not be added to more than one registry.
     * @example
     * @code
     * struct Player : Module {
     *     void imgui() const override { ... }
     * };
     * auto& p = world.addModule<Player>();
     * p.getContainer(); // -> the owning Container
     * @endcode
     */
    class Module
    {
    public:
        virtual ~Module() = default;

        Container& getContainer()
        {
            return *m_container;
        }

        [[nodiscard]] const Container& getContainer() const
        {
            return *m_container;
        }

        // Teardown hook: called by the owning Container during
        // Application::unloadGame() while the framework is still alive,
        // so game code can release borrowed references before destruction.
        virtual void onUnload() {}

        virtual void imgui() const = 0;

    private:
        friend class Container;
        friend class ModuleRegistry;

        void setContainer(Container* container)
        {
            m_container = container;
        }

        Container* m_container = nullptr;
    };

    using TypeID = std::uint32_t;

    template <typename T>
    constexpr TypeID typeID()
    {
        return typeid(T).hash_code();
    }

    /**
     * @brief Owns the Modules of a Container, indexed by type.
     * @detail Storage is a vector of std::unique_ptr plus a type-to-pointer
     * lookup table. add<T>()/get<T>()/has<T>()/remove<T>() provide typed
     * access; one module of each type is allowed.
     * @note Ownership: Owned by its Container. It uniquely owns every Module
     * in `m_modules`; the lookup table holds borrowed raw pointers into that
     * vector (kept consistent by add/remove/clear). `m_container` is a
     * borrowed, non-owning back-pointer to the owning Container.
     * @example
     * @code
     * ModuleRegistry reg;
     * auto& m = reg.add<PlayerModule>();
     * PlayerModule* p = reg.get<PlayerModule>();
     * reg.remove<PlayerModule>();
     * @endcode
     */
    class ModuleRegistry
    {
    public:
        explicit ModuleRegistry() = default;

        explicit ModuleRegistry(Container* container)
        : m_container(container)
        {}

        ModuleRegistry(const ModuleRegistry&) = delete;
        ModuleRegistry& operator=(const ModuleRegistry&) = delete;

        ModuleRegistry(ModuleRegistry&& other) noexcept;
        ModuleRegistry& operator=(ModuleRegistry&& other) noexcept;


        template <typename T, typename... Args>
        T& add(Args&&... args)
        {
            const TypeID id = typeID<T>();

            // Don't allow two modules of the same type.
            if (m_lookup.contains(id))
            {
                throw std::runtime_error(
                    "Module already exists in ModuleRegistry"
                );
            }

            auto module =
                std::make_unique<T>(
                    std::forward<Args>(args)...
                );

            T* ptr = module.get();

            m_modules.push_back(std::move(module));
            m_lookup.emplace(id, ptr);

            return *ptr;
        }


        template <typename T>
        T* get()
        {
            const TypeID id = typeID<T>();

            auto it = m_lookup.find(id);

            if (it == m_lookup.end())
                return nullptr;

            return static_cast<T*>(it->second);
        }


        template <typename T>
        const T* get() const
        {
            const TypeID id = typeID<T>();

            auto it = m_lookup.find(id);

            if (it == m_lookup.end())
                return nullptr;

            return static_cast<const T*>(it->second);
        }


        template <typename T>
        bool has() const
        {
            return m_lookup.contains(typeID<T>());
        }


        template <typename T>
        void remove()
        {
            const TypeID id = typeID<T>();

            auto lookupIt = m_lookup.find(id);

            if (lookupIt == m_lookup.end())
                return;

            Module* module = lookupIt->second;

            // Remove from lookup table first.
            m_lookup.erase(lookupIt);

            // Remove the owning unique_ptr.
            for (auto it = m_modules.begin();
                 it != m_modules.end();
                 ++it)
            {
                if (it->get() == module)
                {
                    m_modules.erase(it);
                    return;
                }
            }
        }

        void clear();
        std::uint32_t size() const;
        bool empty() const;

        [[nodiscard]]
        const std::vector<std::unique_ptr<Module>>& all() const
        {
            return m_modules;
        }


    private:
        Container* m_container = nullptr;
        std::unordered_map<TypeID, Module*> m_lookup;
        std::vector<std::unique_ptr<Module>> m_modules;

        friend class Container;

        void setContainer(Container* container)
        {
            m_container = container;

            for (auto& module : m_modules)
                module->setContainer(container);
        }
    };
}
