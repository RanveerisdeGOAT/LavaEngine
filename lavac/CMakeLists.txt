cmake_minimum_required(VERSION 3.25)

project(lavac
        VERSION 0.0.1
        LANGUAGES CXX
)



add_executable(lavac
        src/main.cpp
        inspector/src/Inspector.h
        src/lavac.h
)

target_include_directories(lavac PRIVATE
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)

target_link_libraries(lavac PRIVATE
        LavaEngine
)