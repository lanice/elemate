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
    texturemanager.h
    texturemanager.cpp
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
    particles/emittergroup.h
    particles/emittergroup.cpp
    particles/downgroup.h
    particles/downgroup.cpp
    particles/particlescriptaccess.cpp
    particles/particlescriptaccess.h
    particles/particlecollision.h
    particles/particlecollision.cpp
    rendering/debugstep.h
    rendering/debugstep.cpp
    rendering/drawable.h
    rendering/drawable.cpp
    rendering/shadowingdrawable.h
    rendering/shadowingdrawable.cpp
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
    terrain/terraininteraction.h
    terrain/terraininteraction.cpp
    terrain/terraintile.h
    terrain/terraintile.cpp
    terrain/physicaltile.h
    terrain/physicaltile.cpp
    terrain/basetile.h
    terrain/basetile.cpp
    terrain/liquidtile.h
    terrain/liquidtile.cpp
    terrain/temperaturetile.h
    terrain/temperaturetile.cpp
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
    ui/achievement.cpp
    ui/achievement.h
    ui/achievementmanager.cpp
    ui/achievementmanager.h
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

set_cxx_target_properties(${TARGET_NAME})

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
