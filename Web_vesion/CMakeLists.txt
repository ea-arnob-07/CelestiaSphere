cmake_minimum_required(VERSION 3.20)
project(CosmoSim3D VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(COSMOSIM_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

if(NOT EMSCRIPTEN)
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(glfw3 CONFIG REQUIRED)
    find_package(glm CONFIG REQUIRED)
else()
    include(FetchContent)
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(glm)
endif()

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
if(NOT EMSCRIPTEN)
    target_link_libraries(CosmoSim3D PRIVATE OpenGL::GL GLEW::GLEW glfw glm::glm)
else()
    target_link_libraries(CosmoSim3D PRIVATE glm::glm)
    target_link_options(CosmoSim3D PRIVATE "SHELL:-s USE_GLFW=3" "SHELL:-s USE_WEBGL2=1" "SHELL:-s FULL_ES3=1" "SHELL:-s WASM=1" "SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/shaders@/shaders" "SHELL:-s ALLOW_MEMORY_GROWTH=1")
    target_compile_options(CosmoSim3D PRIVATE -sUSE_GLFW=3)
endif()

if(NOT EMSCRIPTEN)
    target_compile_definitions(CosmoSim3D PRIVATE
        GLFW_INCLUDE_NONE
        COSMOSIM_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
    )
else()
    target_compile_definitions(CosmoSim3D PRIVATE
        GLFW_INCLUDE_NONE
        COSMOSIM_SHADER_DIR="/shaders"
    )
endif()

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
