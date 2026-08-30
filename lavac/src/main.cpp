#include <dlfcn.h>
#include <iostream>

#include "LavaEngine/LavaEngine.hpp"

using LoadFunction =
    void (*)(LavaEngine::LavaEngine&);

int main(int argc, char* argv[])
{
    LavaEngine::LavaEngine engine(
        1980,
        1080,
        "LavaEngine Compiler"
    );

    std::string gameLibName;
    if (argc > 1)
    {
        gameLibName = argv[1];
    }
    else
    {
        std::cerr << "No game passed." << std::endl;
        return 1;
    }

    void* library = dlopen(
        ("./"+gameLibName).c_str(),
        RTLD_NOW
    );

    if (!library)
    {
        std::cerr << dlerror() << '\n';
        return 1;
    }

    auto load =
        reinterpret_cast<LoadFunction>(
            dlsym(library, "lavaEngineMain")
        );

    if (!load)
    {
        std::cerr << dlerror() << '\n';
        dlclose(library);
        return 1;
    }

    // Give the user's code access to the engine.
    load(engine);

    // Inspector controls the engine.
    engine.run();

    dlclose(library);

    return 0;
}