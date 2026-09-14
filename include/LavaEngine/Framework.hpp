#pragma once
#include <string>

namespace LavaEngine
{
    class Framework
    {
    public:
        explicit Framework()= default;

        virtual ~Framework() = default;

        virtual void shutdown() {}

        Framework(const Framework&) = delete;
        Framework& operator=(const Framework&) = delete;
    };
}
