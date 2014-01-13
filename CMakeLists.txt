cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

# PROJECT CONFIG

set(META_PROJECT_NAME "elemate")

set(META_VERSION_MAJOR "0")
set(META_VERSION_MINOR "0")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(PROJECT_NAME ${META_PROJECT_NAME})  
project(${PROJECT_NAME} C CXX)


option(OPTION_LIMIT_CONFIGS "Generate limited configs (Release; Debug)" ON)
option(OPTION_LOCAL_INSTALL "Install to local directory instead of system" OFF)


if(OPTION_LIMIT_CONFIGS)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Limited Configs" FORCE)
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
    src/particleemitter.cpp
    src/particleemitter.h
    src/particledrawable.h
    src/particledrawable.cpp
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
    src/eventhandler.cpp
    src/eventhandler.h
    src/navigation.cpp
    src/navigation.h
    src/manipulator.cpp
    src/manipulator.h
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
    src/rendering/renderer.h
    src/rendering/renderer.cpp
    src/rendering/renderingstep.h
    src/rendering/renderingstep.cpp
    src/rendering/particlewaterstep.h
    src/rendering/particlewaterstep.cpp
    src/rendering/shadowmappingstep.cpp
    src/rendering/shadowmappingstep.h
)

source_group_by_path(${CMAKE_CURRENT_SOURCE_DIR}/src "\\\\.cpp$|\\\\.c$|\\\\.h$|\\\\.hpp$|\\\\.ui$|\\\\.inl$" 
    ${ELEMATE_SOURCE_GROUP} ${SOURCES})


set(SHADER
    shader/flush.vert
    shader/flush.frag
    shader/terrain_base.vert
    shader/terrain_base.geo
    shader/terrain_base.frag
    shader/terrain_water.vert
    shader/terrain_water.frag
    shader/hand.frag
    shader/hand.vert
    shader/particle_water.vert
    shader/particle_water.geo
    shader/particle_water.frag
    shader/particle_water_normals.frag
    shader/phongLighting.frag
    shader/shadows/lightmap_terrain.vert
    shader/shadows/shadow_terrain.vert
    shader/shadows/shadow_terrain.frag
    shader/shadows/lightmap_hand.vert
    shader/shadows/lightmap.frag
    shader/shadows/depth_util.frag
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

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    SYSTEM ${GLOW_INCLUDE_DIR}
    SYSTEM ${GLM_INCLUDE_DIR}
    SYSTEM ${GLFW_INCLUDE_DIR}
    SYSTEM ${PHYSX_INCLUDE_DIR}
)

list(APPEND SOURCES ${SHADER})

add_executable( ${PROJECT_NAME} ${SOURCES})


# set c++ compiler options globally
set ( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${DEFAULT_COMPILE_FLAGS} ${DEFAULT_COMPILE_FLAGS_RELEASE}" )
set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${DEFAULT_COMPILE_FLAGS} ${DEFAULT_COMPILE_FLAGS_DEBUG}" )

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    LINKER_LANGUAGE CXX
    COMPILE_DEFINITIONS_DEBUG   "${DEFAULT_COMPILE_DEFS_DEBUG}"
    COMPILE_DEFINITIONS_RELEASE "${DEFAULT_COMPILE_DEFS_RELEASE}"
    #COMPILE_FLAGS               "${DEFAULT_COMPILE_FLAGS}"
    LINK_FLAGS_DEBUG            "${DEFAULT_LINKER_FLAGS_DEBUG}"
    LINK_FLAGS_RELEASE          "${DEFAULT_LINKER_FLAGS_RELEASE}"
)

config_project( ${PROJECT_NAME} OPENGL 0 )
config_project( ${PROJECT_NAME} GLEW 0 )
config_project( ${PROJECT_NAME} GLFW 0 )
config_project( ${PROJECT_NAME} GLOW 0 )
config_project( ${PROJECT_NAME} GLOW_UTILS 0 )
config_project( ${PROJECT_NAME} PHYSX 0 )
config_project( ${PROJECT_NAME} FMOD 0 )
config_project( ${PROJECT_NAME} ASSIMP 0 )
