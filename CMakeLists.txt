cmake_minimum_required(VERSION 3.20)
project(primewatch2)

set(CMAKE_CXX_STANDARD 17)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(extern/glfw)

find_package(OpenGL REQUIRED)

include_directories(include)
include_directories(extern/imgui)
include_directories(extern/imgui/backends)

set(SOURCE_FILES
        extern/imgui/imgui.cpp
        extern/imgui/imgui_draw.cpp
        extern/imgui/imgui_demo.cpp
        extern/imgui/imgui_tables.cpp
        extern/imgui/imgui_widgets.cpp
        extern/imgui/imgui.h

        extern/imgui/backends/imgui_impl_opengl3.cpp
        extern/imgui/backends/imgui_impl_opengl3.h
        extern/imgui/backends/imgui_impl_glfw.cpp
        extern/imgui/backends/imgui_impl_glfw.h

        src/glad.c

        src/main.cpp
        src/PrimeWatch.cpp
        src/PrimeWatch.hpp src/MemoryAccess.hpp src/MemoryAccess.cpp)

add_executable(primewatch2 ${SOURCE_FILES})
target_link_libraries(primewatch2 OpenGL::GL glfw)
