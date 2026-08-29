#include "../include/LavaEngine/Resource.hpp"

namespace LavaEngine
{
    bool ResourceRegistry::contains(ResourceHandle handle) const
    {
        return handle.valid()
            && m_resources.contains(handle.id());
    }


    void ResourceRegistry::remove(ResourceHandle handle)
    {
        if (!handle.valid())
            return;

        m_resources.erase(handle.id());
    }


    void ResourceRegistry::clear()
    {
        m_resources.clear();
    }


    [[nodiscard]]
    std::size_t ResourceRegistry::size() const
    {
        return m_resources.size();
    }


    [[nodiscard]]
    bool ResourceRegistry::empty() const
    {
        return m_resources.empty();
    }
}
