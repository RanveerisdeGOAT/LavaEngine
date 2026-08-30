#include "../include/LavaEngine/Container.hpp"

namespace LavaEngine
{
    Container::Container(std::string name)
        : m_name(std::move(name))
    {}

    Container::Container(Container&& other) noexcept
        : m_name(std::move(other.m_name)),
          m_modules(std::move(other.m_modules)),
          m_resources(std::move(other.m_resources))
    {
    }


    Container& Container::operator=(Container&& other) noexcept
    {
        if (this == &other)
            return *this;

        m_name = std::move(other.m_name);
        m_modules = std::move(other.m_modules);
        m_resources = std::move(other.m_resources);

        return *this;
    }

    const std::string& Container::name() const
    {
        return m_name;
    }
}