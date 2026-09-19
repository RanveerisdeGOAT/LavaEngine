cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(LavaEngineTests tests.cpp)

target_link_libraries(LavaEngineTests
        PRIVATE
        LavaEngine::LavaEngine
)

add_test(NAME LavaEngineTests COMMAND LavaEngineTests)