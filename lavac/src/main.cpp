#include <dlfcn.h>
#include <iostream>
#include <thread>

#include "LavaEngine/Application.hpp"

using EngineFunction = void (*)(LavaEngine::Application&);

class UserGame
{
public:
    UserGame(
        LavaEngine::Application& engine,
        const std::string& gameLibName
    )
        : m_engine(engine),
          m_libraryPath("./" + gameLibName)
    {
        loadLibrary();
    }

    ~UserGame()
    {
        unload();
    }

    void loadLibrary()
    {
        namespace fs = std::filesystem;

        if (!fs::exists(m_libraryPath))
        {
            throw std::runtime_error(
                "Game library does not exist: " + m_libraryPath.string()
            );
        }

        if (!waitForStableFile(m_libraryPath))
        {
            throw std::runtime_error(
                "Timed out waiting for game library to finish writing: "
                + m_libraryPath.string()
            );
        }

        fs::path runtimePath =
            "./.lavac_game_" + std::to_string(m_generation++) + ".so";

        fs::copy_file(m_libraryPath, runtimePath, fs::copy_options::overwrite_existing);

        m_runtimePath = runtimePath;

        m_library = dlopen(
            m_runtimePath.c_str(),
            RTLD_NOW | RTLD_LOCAL
        );

        if (!m_library)
        {
            throw std::runtime_error(dlerror());
        }

        m_main =
            reinterpret_cast<EngineFunction>(
                dlsym(
                    m_library,
                    "lavaEngineMain"
                )
            );

        if (!m_main)
        {
            dlclose(m_library);
            m_library = nullptr;

            throw std::runtime_error(
                "Failed to find game function"
            );
        }

        m_lastWriteTime =
            fs::last_write_time(m_libraryPath);
    }

    void load()
    {
        m_main(m_engine);
    }

    void reload()
    {
        std::cerr << "[LAVAC] Reloading...\n";

        m_engine.unloadGame();

        dlclose(m_library);

        m_library = nullptr;
        m_main = nullptr;

        loadLibrary();

        load();

        m_lastWriteTime =
            std::filesystem::last_write_time(m_libraryPath);

        std::cerr << "[LAVAC] Reload complete.\n";
    }

    static bool waitForStableFile(
        const std::filesystem::path& path,
        std::chrono::milliseconds timeout = std::chrono::seconds(3)
    )
    {
        namespace fs = std::filesystem;
        std::error_code ec;

        const auto deadline = std::chrono::steady_clock::now() + timeout;

        // Wait for the file to exist at all first.
        while (!fs::exists(path, ec))
        {
            if (std::chrono::steady_clock::now() >= deadline)
                return false;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        auto lastSize = fs::file_size(path, ec);
        if (ec) lastSize = static_cast<std::uintmax_t>(-1); // force at least one more comparison

        while (std::chrono::steady_clock::now() < deadline)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            const auto size = fs::file_size(path, ec);
            if (ec)
            {
                lastSize = static_cast<std::uintmax_t>(-1);
                continue;
            } // file vanished/relocked — keep waiting

            if (size == lastSize && size > 0)
                return true;

            lastSize = size;
        }

        return false;
    }

    bool changed() const
    {
        std::error_code ec;

        const auto current =
            std::filesystem::last_write_time(
                m_libraryPath,
                ec
            );

        if (ec)
        {
            // Library is currently being rebuilt.
            return false;
        }

        return current != m_lastWriteTime;
    }

    void run()
    {
        while (!m_engine.step())
        {
            if (changed())
            {
                std::cerr << "[LAVAC] Changes detected.\n";
                if (!waitForStableFile(m_libraryPath))   // let any remaining build steps finish
                {
                    continue;
                }
                try { reload(); }
                catch (const std::exception& e) { std::cerr << "[LAVAC] Reload failed: " << e.what() << '\n'; }
            }
        }
    }

    void unload()
    {
        if (!m_library)
            return;

        m_engine.unloadGame();

        dlclose(m_library);

        m_library = nullptr;
        m_main = nullptr;
    }

private:
    LavaEngine::Application& m_engine;

    void* m_library = nullptr;

    EngineFunction m_main = nullptr;

    std::filesystem::path m_libraryPath;
    std::filesystem::path m_runtimePath;

    std::filesystem::file_time_type m_lastWriteTime;

    uint64_t m_generation = 0;
};

int main(int argc, char* argv[])
{
    auto engine = LavaEngine::Application();
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

    auto game = UserGame(engine, gameLibName);
    game.load();
    game.run();
    game.unload();
}
