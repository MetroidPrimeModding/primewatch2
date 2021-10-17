cmake_minimum_required(VERSION 3.20)
project(primewatch2)

set(CMAKE_CXX_STANDARD 20)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(extern/glfw)

set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")
add_subdirectory(extern/json)

add_subdirectory(extern/fmt)

add_subdirectory(extern/glm)

find_package(OpenGL REQUIRED)

include_directories(include)
include_directories(src)
include_directories(extern/imgui)
include_directories(extern/imgui/backends)
include_directories(extern/implot)
include_directories(extern/ImGuiFileDialog)
include_directories(extern/imgui_club/imgui_memory_editor)

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

        extern/implot/implot.h
        extern/implot/implot.cpp
        extern/implot/implot_demo.cpp
        extern/implot/implot_items.cpp

        extern/ImGuiFileDialog/ImGuiFileDialog.cpp
        extern/ImGuiFileDialog/ImGuiFileDialog.h
        extern/ImGuiFileDialog/ImGuiFileDialogConfig.h

        extern/imgui_club/imgui_memory_editor/imgui_memory_editor.h

        src/glad.c

        src/main.cpp
        src/PrimeWatch.cpp
        src/PrimeWatch.hpp
        src/MemoryAccess.hpp
        src/MemoryAccess.cpp

        include/json_optional.hpp

        src/defs/GameDefinitions.cpp
        src/defs/GameDefinitions.hpp

        src/GameMemory.h
        src/GameMemory.cpp
        src/defs/GameObjectRenderers.hpp
        src/defs/GameObjectRenderers.cpp
        src/world/CollisionMesh.hpp
        src/world/CollisionMesh.cpp
        src/world/WorldRenderer.cpp
        src/world/WorldRenderer.hpp src/defs/GameOffsets.hpp src/utils/AreaUtils.cpp src/utils/AreaUtils.hpp src/gl/OpenGLMesh.cpp src/gl/OpenGLMesh.hpp src/gl/OpenGLShader.cpp src/gl/OpenGLShader.hpp src/PrimeWatchInput.cpp src/PrimeWatchInput.hpp src/gl/ShapeGenerator.cpp src/gl/ShapeGenerator.hpp src/utils/GameObjectUtils.cpp src/utils/GameObjectUtils.hpp src/utils/MathUtils.cpp src/utils/MathUtils.hpp src/gl/ImmediateModeBuffer.cpp src/gl/ImmediateModeBuffer.hpp)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/prime1.json
        COMMAND node lib/index.js
        -i '${CMAKE_SOURCE_DIR}/prime_defs/**/*.bs'
        -o ${CMAKE_CURRENT_BINARY_DIR}/prime1.json
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/extern/bstruct"
)
add_custom_target(
        def_gen ALL
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/prime1.json
)


add_executable(primewatch2 ${SOURCE_FILES})
target_link_libraries(primewatch2
        OpenGL::GL
        glfw
        nlohmann_json::nlohmann_json
        fmt::fmt
        glm)
add_dependencies(primewatch2 def_gen)
