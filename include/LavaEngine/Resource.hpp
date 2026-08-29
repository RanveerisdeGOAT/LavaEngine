#pragma once
#include <memory>
#include <unordered_map>

namespace LavaEngine
{
    class Resource
    {
    public:
        virtual ~Resource() = default;

        virtual void exportResource()
        {
            m_exported = true;
        }

        virtual void retractResource()
        {
            m_exported = false;
        }

        [[nodiscard]] bool isExported() const
        {
            return m_exported;
        }

    private:
        bool m_exported = false;
    };

    using ResourceID = uint32_t;

    class ResourceHandle
    {
    public:
        ResourceHandle() = default;

        [[nodiscard]]
        bool valid() const
        {
            return m_id != INVALID_ID;
        }

        [[nodiscard]]
        ResourceID id() const
        {
            return m_id;
        }

    private:
        static constexpr ResourceID INVALID_ID =
            static_cast<ResourceID>(-1);

        explicit ResourceHandle(ResourceID id)
            : m_id(id)
        {
        }

        ResourceID m_id = INVALID_ID;

        friend class ResourceRegistry;
        friend class Container;
    };

    class ResourceRegistry
    {
    public:
        template <typename T, typename... Args>
        ResourceHandle create(Args&&... args)
        {
            static_assert(
                std::is_base_of_v<Resource, T>,
                "T must derive from Resource"
            );

            const ResourceID id = m_nextID++;

            auto resource =
                std::make_unique<T>(
                    std::forward<Args>(args)...
                );

            m_resources.emplace(
                id,
                std::move(resource)
            );

            return ResourceHandle(id);
        }


        template <typename T>
        T* get(ResourceHandle handle)
        {
            if (!handle.valid())
                return nullptr;

            auto it = m_resources.find(handle.id());

            if (it == m_resources.end())
                return nullptr;

            return dynamic_cast<T*>(
                it->second.get()
            );
        }


        template <typename T>
        const T* get(ResourceHandle handle) const
        {
            if (!handle.valid())
                return nullptr;

            auto it = m_resources.find(handle.id());

            if (it == m_resources.end())
                return nullptr;

            return dynamic_cast<const T*>(
                it->second.get()
            );
        }

        bool contains(ResourceHandle handle) const;

        void remove(ResourceHandle handle);

        void clear();

        std::size_t size() const;

        bool empty() const;

    private:
        std::unordered_map<
            ResourceID,
            std::unique_ptr<Resource>
        > m_resources;

        ResourceID m_nextID = 1;
    };
}
