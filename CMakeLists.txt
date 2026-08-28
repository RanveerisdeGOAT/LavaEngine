cmake_minimum_required(VERSION 3.25)

project(LavaEngine
        VERSION 0.1
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Dependencies
find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

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

        LavaVK/include/LavaVK/Surface.hpp
        LavaVK/include/LavaVK/Core.hpp
)

target_include_directories(LavaVK
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/LavaVK/include>
        $<INSTALL_INTERFACE:include>

        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/LavaVK/external
)

target_link_libraries(LavaVK
        PUBLIC
        Vulkan::Vulkan
        glfw
        glm::glm
)

# LavaEngine

add_library(LavaEngine STATIC
        include/LavaEngine/LavaEngine.h
        src/LavaEngine.cpp
)

target_include_directories(LavaEngine
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(LavaEngine
        PUBLIC
        LavaVK
)

# Alias

add_library(LavaEngine::LavaEngine ALIAS LavaEngine)

# Shader compiler

find_program(GLSLC_EXECUTABLE glslc REQUIRED)