cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

set(TARGET_NAME libelemate)

set(SOURCES
    elements.cpp
    elements.h
    physicswrapper.cpp
    physicswrapper.h
    game.cpp
    game.h
    world.cpp
    world.h
    io/imagereader.h
    io/imagereader.cpp
    io/soundmanager.cpp
    io/soundmanager.h
    lua/luawrapper.cpp
    lua/luawrapper.h
    lua/luawrapperfunction.cpp
    lua/luawrapperfunction.h
    particles/particlegroup.cpp
    particles/particlegroup.h
    particles/particlescriptaccess.cpp
    particles/particlescriptaccess.h
    rendering/drawable.h
    rendering/drawable.cpp
    rendering/renderer.h
    rendering/renderer.cpp
    rendering/renderingstep.h
    rendering/renderingstep.cpp
    rendering/particledrawable.h
    rendering/particledrawable.cpp
    rendering/particlestep.h
    rendering/particlestep.cpp
    rendering/shadowmappingstep.cpp
    rendering/shadowmappingstep.h
    rendering/string_rendering/CharacterDrawable.h
    rendering/string_rendering/CharacterDrawable.cpp
    rendering/string_rendering/RawFile.h
    rendering/string_rendering/RawFile.cpp
    rendering/string_rendering/StringComposer.h
    rendering/string_rendering/StringComposer.cpp
    rendering/string_rendering/StringDrawer.h
    rendering/string_rendering/StringDrawer.cpp
    terrain/terrain.h
    terrain/terrain.cpp
    terrain/terrainshadows.cpp
    terrain/terrainsettings.h
    terrain/terrainsettings.cpp
    terrain/terraininteractor.h
    terrain/terraininteractor.cpp
    terrain/terraintile.h
    terrain/terraintile.cpp
    terrain/basetile.h
    terrain/basetile.cpp
    terrain/watertile.h
    terrain/watertile.cpp
    terrain/terraingenerator.h
    terrain/terraingenerator.cpp
    ui/eventhandler.cpp
    ui/eventhandler.h
    ui/navigation.cpp
    ui/navigation.h
    ui/manipulator.cpp
    ui/manipulator.h
    ui/hand.cpp
    ui/hand.h
    ui/userinterface.cpp
    ui/userinterface.h
    ui/menupage.cpp
    ui/menupage.h
    utils/cameraex.h
    utils/cameraex.cpp
    utils/pxcompilerfix.h
    utils/CyclicTime.cpp
    utils/CyclicTime.h
    utils/Timer.cpp
    utils/Timer.h
    utils/MathMacros.h
    utils/ChronoTimer.cpp
    utils/ChronoTimer.h
)

source_group_by_path(${CMAKE_CURRENT_SOURCE_DIR} "\\\\.cpp$|\\\\.c$|\\\\.h$|\\\\.hpp$|\\\\.ui$|\\\\.inl$" ${SOURCES})

add_library( ${TARGET_NAME} ${SOURCES})

set_target_properties(${TARGET_NAME}
    PROPERTIES
    LINKER_LANGUAGE CXX
    COMPILE_DEFINITIONS_DEBUG   "${DEFAULT_COMPILE_DEFS_DEBUG}"
    COMPILE_DEFINITIONS_RELEASE "${DEFAULT_COMPILE_DEFS_RELEASE}"
    COMPILE_DEFINITIONS_RELWITHDEBINFO "${DEFAULT_COMPILE_DEFS_RELWITHDEBINFO}"
    LINK_FLAGS_DEBUG            "${DEFAULT_LINKER_FLAGS_DEBUG}"
    LINK_FLAGS_RELEASE          "${DEFAULT_LINKER_FLAGS_RELEASE}"
    LINK_FLAGS_RELWITHDEBINFO   "${DEFAULT_LINKER_FLAGS_RELWITHDEBINFO}"
)

config_project( ${TARGET_NAME} OPENGL )
config_project( ${TARGET_NAME} GLEW )
config_project( ${TARGET_NAME} GLFW )
config_project( ${TARGET_NAME} GLOW )
config_project( ${TARGET_NAME} GLOW_UTILS )
config_project( ${TARGET_NAME} PHYSX )
config_project( ${TARGET_NAME} FMOD )
config_project( ${TARGET_NAME} ASSIMP )
config_project( ${TARGET_NAME} LUA )

if(LINUX)
    target_link_libraries( ${TARGET_NAME} dl)
endif()
