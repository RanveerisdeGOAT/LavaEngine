add_executable(LavaEngineTests tests.cpp)

target_link_libraries(LavaEngineTests
        PRIVATE
        LavaEngine::LavaEngine
)

add_test(NAME LavaEngineTests COMMAND LavaEngineTests)