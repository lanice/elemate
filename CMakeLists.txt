cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

# PROJECT CONFIG

set(META_PROJECT_NAME "elemate")

set(META_VERSION_MAJOR "0")
set(META_VERSION_MINOR "0")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(PROJECT_NAME ${META_PROJECT_NAME})  
project(${PROJECT_NAME} C CXX)


option(OPTION_LIMIT_CONFIGS  "Generate limited configs (Release;Debug;RelWithDebInfo)" ON)
option(OPTION_LOCAL_INSTALL "Install to local directory instead of system" OFF)


if(OPTION_LIMIT_CONFIGS)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo" CACHE STRING "Limited Configs" FORCE)
endif()


include(cmake/Custom.cmake)


# PLATFORM AND ARCHITECTURE

# Architecture (32/64 bit)
set(X64 OFF)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(X64 ON)
endif()

# Check for linux
if(UNIX AND NOT APPLE)
    set(LINUX 1)
endif()

# Setup platform specifics (compile flags, etc., ...)
if(MSVC)
    message(STATUS "Configuring for platform Windows/MSVC.")
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/PlatformWindowsMSVC.cmake)
elseif(LINUX AND CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "Configuring for platform Linux/GCC.")
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/PlatformLinuxGCC.cmake)
else()
    # Unsupported system/compiler
    message(WARNING "Unsupported platform/compiler combination")
endif()


# SOURCES AND TARGET CONFIGURATION

set(ELEMATE_SOURCE_GROUP "Source Files")
set(ELEMATE_SHADER_GROUP "Shader Files")

set(SOURCES
    src/main.cpp
    src/elements.h
    src/elements.cpp
    src/pxcompilerfix.h
    src/physicswrapper.cpp
    src/physicswrapper.h
    src/game.cpp
    src/game.h
    src/world.cpp
    src/world.h
    src/hand.cpp
    src/hand.h
    src/imagereader.h
    src/imagereader.cpp
    src/particlegroup.cpp
    src/particlegroup.h
    src/particlescriptaccess.cpp
    src/particlescriptaccess.h
    src/hpicgs/CyclicTime.cpp
    src/hpicgs/CyclicTime.h
    src/hpicgs/Timer.cpp
    src/hpicgs/Timer.h
    src/hpicgs/MathMacros.h
    src/hpicgs/ChronoTimer.cpp
    src/hpicgs/ChronoTimer.h
    src/simpleconfigreader.cpp
    src/simpleconfigreader.h
    src/soundmanager.cpp
    src/soundmanager.h
    src/lua/luawrapper.cpp
    src/lua/luawrapper.h
    src/lua/luawrapperfunction.cpp
    src/lua/luawrapperfunction.h
    src/ui/eventhandler.cpp
    src/ui/eventhandler.h
    src/ui/navigation.cpp
    src/ui/navigation.h
    src/ui/manipulator.cpp
    src/ui/manipulator.h
    src/cameraex.h
    src/cameraex.cpp
    src/terrain/terrain.h
    src/terrain/terrain.cpp
    src/terrain/terrainshadows.cpp
    src/terrain/terrainsettings.h
    src/terrain/terrainsettings.cpp
    src/terrain/terraininteractor.h
    src/terrain/terraininteractor.cpp
    src/terrain/terraintile.h
    src/terrain/terraintile.cpp
    src/terrain/basetile.h
    src/terrain/basetile.cpp
    src/terrain/watertile.h
    src/terrain/watertile.cpp
    src/terrain/terraingenerator.h
    src/terrain/terraingenerator.cpp
    src/rendering/drawable.h
    src/rendering/drawable.cpp
    src/rendering/renderer.h
    src/rendering/renderer.cpp
    src/rendering/renderingstep.h
    src/rendering/renderingstep.cpp
    src/rendering/particledrawable.h
    src/rendering/particledrawable.cpp
    src/rendering/particlestep.h
    src/rendering/particlestep.cpp
    src/rendering/shadowmappingstep.cpp
    src/rendering/shadowmappingstep.h
    src/rendering/string_rendering/CharacterDrawable.h
    src/rendering/string_rendering/CharacterDrawable.cpp
    src/rendering/string_rendering/RawFile.h
    src/rendering/string_rendering/RawFile.cpp
    src/rendering/string_rendering/StringComposer.h
    src/rendering/string_rendering/StringComposer.cpp
    src/rendering/string_rendering/StringDrawer.h
    src/rendering/string_rendering/StringDrawer.cpp
    src/ui/userinterface.cpp
    src/ui/userinterface.h
    src/ui/menupage.cpp
    src/ui/menupage.h
)

source_group_by_path(${CMAKE_CURRENT_SOURCE_DIR}/src "\\\\.cpp$|\\\\.c$|\\\\.h$|\\\\.hpp$|\\\\.ui$|\\\\.inl$" 
    ${ELEMATE_SOURCE_GROUP} ${SOURCES})


set(SHADER
    shader/flush.vert
    shader/flush.frag
    shader/passthrough.frag
    shader/terrain_base.vert
    shader/terrain_base.geo
    shader/terrain_base.frag
    shader/hand.frag
    shader/hand.vert
    shader/particle/material.frag
    shader/particle/particle.vert
    shader/particle/particle.geo
    shader/particle/particle.frag
    shader/particle/particle_normals.frag
    shader/particle/particle_depthblurring_h.frag
    shader/particle/particle_depthblurring_v.frag
    shader/phongLighting.frag
    shader/shadows/depthmap_terrain.vert
    shader/shadows/depthmap_terrain.geo
    shader/shadows/depthmap_hand.vert
    shader/shadows/depthmapLinearized.frag
    shader/shadows/shadowmapping_terrain.vert
    shader/shadows/shadowmapping_hand.vert
    shader/shadows/shadowmapping.frag
    shader/depth_util.frag
    shader/string_drawer.vert
    shader/string_drawer.frag
    shader/ui/preview.vert
    shader/ui/preview.frag
    shader/ui/screen.vert
    shader/ui/screen.frag
)

source_group_by_path(${CMAKE_CURRENT_SOURCE_DIR}/shader "\\\\.frag$|\\\\.vert$|\\\\.geo$"
    ${ELEMATE_SHADER_GROUP} ${SHADER})

set( GLOW_DIR $ENV{GLOW_DIR} )
if (GLOW_DIR)
    STRING(REGEX REPLACE "\\\\" "/" GLOW_DIR ${GLOW_DIR})
endif()
    
if (WIN32)
    include(findPackageHandleStandardArgs)
endif()

find_package( OpenGL REQUIRED )
find_package( GLM REQUIRED )
find_package( GLEW REQUIRED )
find_package( GLFW REQUIRED )
find_package( GLOW REQUIRED )
find_package( PhysX REQUIRED )
find_package( FMOD REQUIRED )
find_package( ASSIMP REQUIRED )
find_package( LUA REQUIRED )

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    SYSTEM ${GLOW_INCLUDE_DIR}
    SYSTEM ${GLM_INCLUDE_DIR}
    SYSTEM ${GLFW_INCLUDE_DIR}
    SYSTEM ${PHYSX_INCLUDE_DIR}
    SYSTEM ${LUA_INCLUDE_DIR}
)

list(APPEND SOURCES ${SHADER})

add_executable( ${PROJECT_NAME} ${SOURCES})


# set c++ compiler options globally
set ( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${DEFAULT_COMPILE_FLAGS} ${DEFAULT_COMPILE_FLAGS_RELEASE}" )
set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${DEFAULT_COMPILE_FLAGS} ${DEFAULT_COMPILE_FLAGS_DEBUG}" )
set ( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${DEFAULT_COMPILE_FLAGS} ${DEFAULT_COMPILE_FLAGS_RELWITHDEBINFO}" )

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    LINKER_LANGUAGE CXX
    COMPILE_DEFINITIONS_DEBUG   "${DEFAULT_COMPILE_DEFS_DEBUG}"
    COMPILE_DEFINITIONS_RELEASE "${DEFAULT_COMPILE_DEFS_RELEASE}"
    COMPILE_DEFINITIONS_RELWITHDEBINFO "${DEFAULT_COMPILE_DEFS_RELWITHDEBINFO}"
    #COMPILE_FLAGS               "${DEFAULT_COMPILE_FLAGS}"
    LINK_FLAGS_DEBUG            "${DEFAULT_LINKER_FLAGS_DEBUG}"
    LINK_FLAGS_RELEASE          "${DEFAULT_LINKER_FLAGS_RELEASE}"
    LINK_FLAGS_RELWITHDEBINFO   "${DEFAULT_LINKER_FLAGS_RELWITHDEBINFO}"
)

config_project( ${PROJECT_NAME} OPENGL 0 )
config_project( ${PROJECT_NAME} GLEW 0 )
config_project( ${PROJECT_NAME} GLFW 0 )
config_project( ${PROJECT_NAME} GLOW 0 )
config_project( ${PROJECT_NAME} GLOW_UTILS 0 )
config_project( ${PROJECT_NAME} PHYSX 0 )
config_project( ${PROJECT_NAME} FMOD 0 )
config_project( ${PROJECT_NAME} ASSIMP 0 )
config_project( ${PROJECT_NAME} LUA 0 )

if(LINUX)
    target_link_libraries( ${PROJECT_NAME} dl)
endif()
