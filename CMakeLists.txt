cmake_minimum_required(VERSION 2.8.9 FATAL_ERROR)

# PROJECT CONFIG

set(META_PROJECT_NAME "elemate")

set(META_VERSION_MAJOR "0")
set(META_VERSION_MINOR "0")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(PROJECT_NAME ${META_PROJECT_NAME})  
project(${PROJECT_NAME} C CXX)

option(OPTION_LIMIT_CONFIGS "Generate limited configs (Release; Debug)" ON)
option(OPTION_LOCAL_INSTALL "Install to local directory instead of system" OFF)

find_package( OpenThreads )
find_package( osg )
find_package( osgDB )
find_package( osgUtil )
find_package( osgViewer )
find_package( osgGA )
find_package( PhysX REQUIRED)
# find_package( APEX )

macro( config_project PROJNAME LIBNAME )
    include_directories( ${${LIBNAME}_INCLUDE_DIR} )
    target_link_libraries( ${PROJNAME} ${${LIBNAME}_LIBRARY} )
endmacro()


set(SOURCES

    src/main.cpp
    src/worlddrawable.cpp
    src/worlddrawable.h
    src/physicswrapper.cpp
    src/physicswrapper.h
    src/game.cpp
    src/game.h
    src/hpicgs/CyclicTime.cpp
    src/hpicgs/CyclicTime.h
    src/hpicgs/Timer.cpp
    src/hpicgs/Timer.h
    src/hpicgs/MathMacros.h
    src/hpicgs/ChronoTimer.cpp
    src/hpicgs/ChronoTimer.h
)


add_executable( ${PROJECT_NAME} ${SOURCES} )

config_project( ${PROJECT_NAME} OPENTHREADS )
config_project( ${PROJECT_NAME} OSG )
config_project( ${PROJECT_NAME} OSGDB )
config_project( ${PROJECT_NAME} OSGUTIL )
config_project( ${PROJECT_NAME} OSGVIEWER )
config_project( ${PROJECT_NAME} OSGGA )
config_project( ${PROJECT_NAME} PHYSX )
# config_project( ${PROJECT_NAME} APEX )