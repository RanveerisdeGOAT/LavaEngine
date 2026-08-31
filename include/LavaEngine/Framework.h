#pragma once
#include <string>

namespace LavaEngine
{
    class Framework
    {
    public:
        explicit Framework(std::string name)
            : m_name(std::move(name))
        {
        }

        virtual ~Framework() = default;

        virtual void shutdown() {}

        Framework(const Framework&) = delete;
        Framework& operator=(const Framework&) = delete;

        [[nodiscard]] const std::string& name() const
        {
            return m_name;
        }



    private:
        std::string m_name;
    };
}
