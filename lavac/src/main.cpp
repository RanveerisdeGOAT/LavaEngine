#include "LavaEngine/Application.hpp"
#include "../inspector/src/Inspector.h"
#include "lavac.h"


int main(int argc, char* argv[])
{
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

    auto game = UserGame(gameLibName);
    game.load();

    if (argc > 2)
    {
        if (strcmp(argv[2], "inspect") == 0)
        {
            std::cerr
                << "Inspecting "
                << gameLibName
                << '\n';

            LavaEngine::Inspector inspector(
                game
            );

            inspector.inspect();
        }
        else if (strcmp(argv[2], "run") == 0)
        {
            game.run();
        }
        else
        {
            std::cerr
                << "Unknown mode: "
                << argv[2]
                << '\n';

            game.unload();
            return 1;
        }
    }
    else
    {
        game.run();
    }

    game.unload();
}
