cmake_minimum_required(VERSION 3.20)
project(CosmoSim3D VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(COSMOSIM_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(CosmoSim3D
    src/main.cpp
    src/Application.cpp
    src/Shader.cpp
    src/Camera.cpp
    src/Mesh.cpp
    src/Geometry.cpp
    src/SolarSystem.cpp
    src/Renderer.cpp
    src/ParticleSystem.cpp
    src/Spacecraft.cpp
    src/Starfield.cpp
    src/UiOverlay.cpp
)

target_include_directories(CosmoSim3D PRIVATE src)
set_target_properties(CosmoSim3D PROPERTIES OUTPUT_NAME "CosmoSim3D")
target_link_libraries(CosmoSim3D PRIVATE OpenGL::GL GLEW::GLEW glfw glm::glm)

target_compile_definitions(CosmoSim3D PRIVATE
    GLFW_INCLUDE_NONE
    COSMOSIM_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
)

if(MSVC)
    target_compile_options(CosmoSim3D PRIVATE /W4 /permissive-)
    if(COSMOSIM_WARNINGS_AS_ERRORS)
        target_compile_options(CosmoSim3D PRIVATE /WX)
    endif()
else()
    target_compile_options(CosmoSim3D PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    if(COSMOSIM_WARNINGS_AS_ERRORS)
        target_compile_options(CosmoSim3D PRIVATE -Werror)
    endif()
endif()
