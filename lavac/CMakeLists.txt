cmake_minimum_required(VERSION 3.25)

project(lavac
        VERSION 0.0.1
        LANGUAGES CXX
)

add_executable(lavac
        src/main.cpp
)

target_link_libraries(lavac PRIVATE
        LavaEngine
)