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

    void Container::expose(const std::string& name, float* variable)
    {
        m_variables.push_back(Variable{name, static_cast<void*>(variable), VarType::Float});
    }

    void Container::expose(const std::string& name, int* variable)
    {
        m_variables.push_back(Variable{name, static_cast<void*>(variable), VarType::Int});
    }

    void Container::expose(const std::string& name, bool* variable)
    {
        m_variables.push_back(Variable{name, static_cast<void*>(variable), VarType::Bool});
    }

    void Container::expose(const std::string& name, std::string* variable)
    {
        m_variables.push_back(Variable{name, static_cast<void*>(variable), VarType::String});
    }
}
