#pragma once
#include <string>

#include "Module.hpp"
#include "Resource.hpp"
#include "LavaVK/LavaVK.hpp"

namespace LavaEngine
{
    class Container
    {
    public:
        Container() = default;

        explicit Container(std::string name);

        ~Container() = default;

        template <typename T, typename... Args>
        T& addModule(Args&&... args)
        {
            return m_modules.add<T>(
                std::forward<Args>(args)...
            );
        }


        template <typename T>
        T* getModule()
        {
            return m_modules.get<T>();
        }


        template <typename T>
        bool hasModule() const
        {
            return m_modules.has<T>();
        }


        template <typename T>
        void removeModule()
        {
            m_modules.remove<T>();
        }

        template <typename T, typename... Args>
        ResourceHandle createResource(
            Args&&... args
        )
        {
            return m_resources.create<T>(
                std::forward<Args>(args)...
            );
        }


        template <typename T>
        T* getResource(
            ResourceHandle handle
        )
        {
            return m_resources.get<T>(handle);
        }


        template <typename T>
        const T* getResource(
            ResourceHandle handle
        ) const
        {
            return m_resources.get<T>(handle);
        }


        template <typename T>
        void exportResource(ResourceHandle handle)
        {
            T* resource = m_resources.get<T>(handle);

            if (!resource)
                return;

            resource->exportResource();
        }


        template <typename T>
        T* importResource(
            Container& exporter,
            ResourceHandle handle
        )
        {
            T* resource = exporter.m_resources.get<T>(handle);

            if (!resource)
                return nullptr;

            if (!resource->isExported())
                return nullptr;

            return resource;
        }

        [[nodiscard]]
        const std::string& name() const;

    private:
        std::string m_name;

        ModuleRegistry m_modules;
        ResourceRegistry m_resources;
    };
}
