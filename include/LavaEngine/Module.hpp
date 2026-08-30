#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <algorithm>


namespace LavaEngine
{
    class Container;

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

    private:
        Container* m_container = nullptr;
        std::unordered_map<TypeID, Module*> m_lookup;
        std::vector<std::unique_ptr<Module>> m_modules;
    };
}
