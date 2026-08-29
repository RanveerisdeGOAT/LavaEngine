#include "../include/LavaEngine/Container.hpp"

namespace LavaEngine
{
    Container::Container(std::string name)
        : m_name(std::move(name))
    {}



    const std::string& Container::name() const
    {
        return m_name;
    }
}