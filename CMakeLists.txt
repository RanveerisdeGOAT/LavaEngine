cmake_minimum_required(VERSION 3.25)

project(LavaEngine
        VERSION 0.5.0
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)

add_subdirectory(external/glfw-3.5.1)

# LavaVK

add_library(LavaVK STATIC
        LavaVK/src/Instance.cpp
        LavaVK/src/Device.cpp
        LavaVK/src/Queue.cpp
        LavaVK/src/Sync.cpp
        LavaVK/src/Pipeline.cpp
        LavaVK/src/SwapChain.cpp
        LavaVK/src/Buffer.cpp
        LavaVK/src/Command.cpp
        LavaVK/src/Shader.cpp
        LavaVK/src/Descriptor.cpp
        LavaVK/src/Texture.cpp
)

target_include_directories(LavaVK
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/LavaVK/include

        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/LavaVK/external
)

target_link_libraries(LavaVK
        PUBLIC
        Vulkan::Vulkan
        glfw
)

# LavaEngine

add_library(LavaEngine STATIC
        include/LavaEngine/Application.hpp

        include/LavaEngine/Window.hpp
        src/Window.cpp

        include/LavaEngine/Container.hpp
        src/Container.cpp

        include/LavaEngine/Module.hpp
        src/Module.cpp

        include/LavaEngine/Resource.hpp
        src/Resource.cpp

        include/LavaEngine/Modules.hpp
        src/Modules.cpp

        include/LavaEngine/Scheduler.hpp
        src/Scheduler.cpp

        include/LavaEngine/Framework.h

        include/LavaEngine/Frameworks.h
)

target_include_directories(LavaEngine
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(LavaEngine
        PUBLIC
        LavaVK
)

add_library(LavaEngine::LavaEngine ALIAS LavaEngine)

# lavac

add_subdirectory(lavac)

# Shader compiler

find_program(GLSLC_EXECUTABLE glslc REQUIRED)