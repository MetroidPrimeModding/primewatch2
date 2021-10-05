cmake_minimum_required(VERSION 3.20)
project(primewatch2)

set(CMAKE_CXX_STANDARD 17)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(extern/glfw)

set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")
add_subdirectory(extern/json)

add_subdirectory(extern/fmt)

find_package(OpenGL REQUIRED)

include_directories(include)
include_directories(src)
include_directories(extern/imgui)
include_directories(extern/imgui/backends)
include_directories(extern/implot)

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

        src/glad.c

        src/main.cpp
        src/PrimeWatch.cpp
        src/PrimeWatch.hpp
        src/MemoryAccess.hpp
        src/MemoryAccess.cpp

        include/json_optional.hpp

        src/defs/GameDefinitions.cpp
        src/defs/GameDefinitions.hpp

        src/game_value.h
        src/GameMemory.h
        src/GameMemory.cpp
        src/prime1/CAABB.hpp
        src/prime1/CCameraManager.hpp
        src/prime1/CGameGlobalObjects.hpp
        src/prime1/CMatrix4f.hpp
        src/prime1/CPair.hpp
        src/prime1/CQuaternion.hpp
        src/prime1/CReservedVector.hpp
        src/prime1/CStateManager.hpp
        src/prime1/CVector3f.hpp
        src/prime1/SObjectTag.hpp
        src/prime1/CAABBPrimitive.hpp
        src/prime1/CFirstPersonCamera.hpp
        src/prime1/CGameState.hpp
        src/prime1/CObjectList.hpp
        src/prime1/CPlayerCameraBob.hpp
        src/prime1/CRBTree.hpp
        src/prime1/CSimplePool.hpp
        src/prime1/CTransform.hpp
        src/prime1/CWorld.hpp
        src/prime1/CBallCamera.hpp
        src/prime1/CGameAllocator.hpp
        src/prime1/CHealthInfo.hpp
        src/prime1/CObjectReference.hpp
        src/prime1/CPlayerState.hpp
        src/prime1/CRandom16.hpp
        src/prime1/CSpherePrimitive.hpp
        src/prime1/actors/CActor.hpp
        src/prime1/actors/CEntity.hpp
        src/prime1/actors/CGameCamera.hpp
        src/prime1/actors/CMorphBall.hpp
        src/prime1/actors/CPhysicsActor.hpp
        src/prime1/actors/CPlayer.hpp
        src/prime1/actors/CPlayerGun.hpp
        src/prime1/rstl/rstl.hpp
        src/prime1/CGameArea.hpp
        src/defs/GameObjectRenderers.hpp src/defs/GameObjectRenderers.cpp)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/prime1.json
        COMMAND npm i && node lib/index.js
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
        fmt::fmt)
add_dependencies(primewatch2 def_gen)
