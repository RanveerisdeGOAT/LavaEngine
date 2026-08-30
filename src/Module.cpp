#pragma once

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../include/LavaEngine/Module.hpp"

namespace LavaEngine
{
    class Module;

    ModuleRegistry::ModuleRegistry(
        ModuleRegistry&& other
    ) noexcept
        : m_container(other.m_container),
          m_lookup(std::move(other.m_lookup)),
          m_modules(std::move(other.m_modules))
    {
        for (auto& module : m_modules)
        {
            module->setContainer(m_container);
        }

        other.m_container = nullptr;
    }

    ModuleRegistry& ModuleRegistry::operator=(
        ModuleRegistry&& other
    ) noexcept
    {
        if (this == &other)
            return *this;

        m_lookup = std::move(other.m_lookup);
        m_modules = std::move(other.m_modules);

        m_container = other.m_container;

        for (auto& module : m_modules)
        {
            module->setContainer(m_container);
        }

        other.m_container = nullptr;

        return *this;
    }


    void ModuleRegistry::clear()
    {
        m_lookup.clear();
        m_modules.clear();
    }


    std::uint32_t ModuleRegistry::size() const
    {
        return m_modules.size();
    }


    bool ModuleRegistry::empty() const
    {
        return m_modules.empty();
    };
}
