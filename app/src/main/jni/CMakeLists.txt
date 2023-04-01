cmake_minimum_required(VERSION 3.22.1)

project(modmenu)

include_directories(
        .
        includes
        cheat_library
        cheat_library/Dobby/include/
        cheat_library/imgui
        cheat_library/imgui/backends
)

add_library(native SHARED
        Main.cpp
        cheat_main/HookManager.cpp
        cheat_main/events.cpp
        cheat_main/MThreadRenderer.cpp
        cheat_library/SDK/Unity/API.cpp
        cheat_library/SDK/Renderer.cpp
        cheat_library/SDK/GameObject.cpp
        cheat_library/SDK/Object.cpp
        cheat_library/SDK/Screen.cpp
        cheat_library/SDK/Camera.cpp
        cheat_library/SDK/Component.cpp
        cheat_library/SDK/Transform.cpp
        cheat_library/SDK/GamePlay.cpp
        cheat_library/SDK/ActorCreators.cpp
        cheat_library/SDK/ActorLinker.cpp
        cheat_library/SDK/ActorHelper.cpp
        cheat_library/SDK/ValueLinkerComponent.cpp
        cheat_library/KittyMemory/KittyMemory.cpp
        cheat_library/KittyMemory/MemoryPatch.cpp
        cheat_library/KittyMemory/MemoryBackup.cpp
        cheat_library/KittyMemory/KittyUtils.cpp
        cheat_library/events/joins/abstracteventjoin.cpp
        cheat_library/events/joins/eventjoinwrapper.cpp
        cheat_library/imgui/imgui.cpp
        cheat_library/imgui/imgui_demo.cpp
        cheat_library/imgui/imgui_draw.cpp
        cheat_library/imgui/imgui_tables.cpp
        cheat_library/imgui/imgui_widgets.cpp
        cheat_library/imgui/backends/imgui_impl_android.cpp
        cheat_library/imgui/backends/imgui_impl_opengl3.cpp
        includes/Utils.cpp
        includes/MemUtil.cpp
)

target_compile_definitions(native PRIVATE IMGUI_IMPL_OPENGL_ES2)

if (NOT TARGET dobby_static)
    set(DOBBY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cheat_library/Dobby)
    macro(SET_OPTION option value)
        set(${option} ${value} CACHE INTERNAL "" FORCE)
    endmacro()
    SET_OPTION(DOBBY_DEBUG OFF)
    SET_OPTION(DOBBY_GENERATE_SHARED OFF)
    SET_OPTION(Plugin.Android.BionicLinkerUtil ON)
    add_subdirectory(${DOBBY_DIR} dobby_static)
    get_property(DOBBY_INCLUDE_DIRECTORIES
            TARGET dobby_static
            PROPERTY INCLUDE_DIRECTORIES)
    include_directories(
            .
            ${DOBBY_INCLUDE_DIRECTORIES}
            $<TARGET_PROPERTY:dobby_static,INCLUDE_DIRECTORIES>
    )
endif ()

target_link_libraries(native log dobby_static EGL GLESv2 android)