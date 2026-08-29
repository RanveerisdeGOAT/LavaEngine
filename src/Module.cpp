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
