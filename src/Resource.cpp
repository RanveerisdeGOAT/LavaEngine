#include "../include/LavaEngine/Resource.hpp"

namespace LavaEngine
{
    bool ResourceRegistry::contains(ResourceHandle handle) const
    {
        return handle.valid()
            && m_storage
            && findEntry(handle) != nullptr;
    }


    void ResourceRegistry::remove(ResourceHandle handle)
    {
        if (!handle.valid() || !m_storage)
            return;

        auto it = m_storage->resources.find(handle.id());

        if (it == m_storage->resources.end())
            return;

        if (it->second.generation != handle.generation())
            return;

        m_storage->resources.erase(it);
    }


    void ResourceRegistry::clear()
    {
        if (!m_storage)
            return;

        m_storage->resources.clear();
    }


    [[nodiscard]]
    std::size_t ResourceRegistry::size() const
    {
        return m_storage
            ? m_storage->resources.size()
            : 0;
    }


    [[nodiscard]]
    bool ResourceRegistry::empty() const
    {
        return m_storage
            ? m_storage->resources.empty()
            : true;
    }
}