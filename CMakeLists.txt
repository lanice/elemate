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

set(ELEMATE_SOURCE_GROUP "Source Files")
set(ELEMATE_SHADER_GROUP "Shader Files")

include_directories(${CMAKE_SOURCE_DIR}/src)

set(SOURCES
    src/main.cpp
    src/elements.h
    src/elements.cpp
    src/physicswrapper.cpp
    src/physicswrapper.h
    src/game.cpp
    src/game.h
    src/world.cpp
    src/world.h
    src/godmanipulator.cpp
    src/godmanipulator.h
    src/godnavigation.cpp
    src/godnavigation.h
    src/objectscontainer.cpp
    src/objectscontainer.h
    src/screenquad.cpp
    src/screenquad.h
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
    src/terrain/terraingenerator.cpp
    src/terrain/terraingenerator.h
    src/terrain/elemateterrain.h
    src/terrain/elemateterrain.cpp
    src/osg/sharedgeometrytechnique.h;
    src/osg/sharedgeometrytechnique.cpp;
    src/soundmanager.cpp
    src/soundmanager.h
)

source_group_by_path(${CMAKE_CURRENT_SOURCE_DIR}/src "\\\\.cpp$|\\\\.c$|\\\\.h$|\\\\.hpp$|\\\\.ui$|\\\\.inl$" 
    ${ELEMATE_SOURCE_GROUP} ${SOURCES})


set(SHADER
    shader/terrain_base.vert
    shader/terrain_water.vert
    shader/terrain_base.geom
    shader/terrain_base.frag
    shader/terrain_water.geom
    shader/terrain_water.frag
    shader/sphere.frag
    shader/sphere.vert
    shader/phongLighting.frag
)

source_group_by_path(${CMAKE_CURRENT_SOURCE_DIR}/shader "\\\\.frag$|\\\\.vert$|\\\\.geom$"
    ${ELEMATE_SHADER_GROUP} ${SHADER})
    

find_package( OpenThreads )
find_package( osg )
find_package( osgDB )
find_package( osgUtil )
find_package( osgViewer )
find_package( osgGA )
find_package( osgTerrain )
find_package( PhysX REQUIRED )
find_package( FMOD )

list(APPEND SOURCES ${SHADER})

add_executable( ${PROJECT_NAME} ${SOURCES})

# PLATFORM AND ARCHITECTURE

# Setup platform specifics (compile flags, etc., ...)
if(MSVC)
    message(STATUS "Configuring for platform Windows/MSVC.")
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/PlatformWindowsMSVC.cmake)
else()
    # Unsupported system/compiler
    message(WARNING "Unsupported platform/compiler combination")
endif()


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

config_project( ${PROJECT_NAME} OPENTHREADS 1 )
config_project( ${PROJECT_NAME} OSG 1 )
config_project( ${PROJECT_NAME} OSGDB 1 )
config_project( ${PROJECT_NAME} OSGUTIL 1 )
config_project( ${PROJECT_NAME} OSGVIEWER 1 )
config_project( ${PROJECT_NAME} OSGGA 1 )
config_project( ${PROJECT_NAME} OSGTERRAIN 1 )
config_project( ${PROJECT_NAME} PHYSX 0 )
config_project( ${PROJECT_NAME} FMOD 0 )
