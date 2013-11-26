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


include(cmake/Custom.cmake)

set(ELEMATE_SOURCE_GROUP "Source Files")
set(ELEMATE_SHADER_GROUP "Shader Files")

set(SOURCES
    src/main.cpp
    src/worlddrawable.cpp
    src/worlddrawable.h
    src/physicswrapper.cpp
    src/physicswrapper.h
    src/game.cpp
    src/game.h
    src/makermanipulator.cpp
    src/makermanipulator.h
    src/objectscontainer.cpp
    src/objectscontainer.h
    src/hpicgs/CyclicTime.cpp
    src/hpicgs/CyclicTime.h
    src/hpicgs/Timer.cpp
    src/hpicgs/Timer.h
    src/hpicgs/MathMacros.h
    src/hpicgs/ChronoTimer.cpp
    src/hpicgs/ChronoTimer.h
    src/simpleconfigreader.cpp
    src/simpleconfigreader.h
    src/terraingenerator.cpp
    src/terraingenerator.h
)

source_group_by_path(${CMAKE_CURRENT_SOURCE_DIR}/src "\\\\.cpp$|\\\\.c$|\\\\.h$|\\\\.hpp$|\\\\.ui$|\\\\.inl$" 
    ${ELEMATE_SOURCE_GROUP} ${SOURCES})


set(SHADER
    data/world.frag
    data/world.vert
)

source_group_by_path(${CMAKE_CURRENT_SOURCE_DIR}/data "\\\\.frag$|\\\\.vert$|\\\\.geo$"
    ${ELEMATE_SHADER_GROUP} ${SHADER})
    

find_package( OpenThreads )
find_package( osg )
find_package( osgDB )
find_package( osgUtil )
find_package( osgViewer )
find_package( osgGA )
find_package( osgTerrain )
find_package( PhysX REQUIRED)
# find_package( APEX )

list(APPEND SOURCES ${SHADER})

add_executable( ${PROJECT_NAME} ${SOURCES})

config_project( ${PROJECT_NAME} OPENTHREADS 1 )
config_project( ${PROJECT_NAME} OSG 1 )
config_project( ${PROJECT_NAME} OSGDB 1 )
config_project( ${PROJECT_NAME} OSGUTIL 1 )
config_project( ${PROJECT_NAME} OSGVIEWER 1 )
config_project( ${PROJECT_NAME} OSGGA 1 )
config_project( ${PROJECT_NAME} OSGTERRAIN 1 )
config_project( ${PROJECT_NAME} PHYSX 0 )
# config_project( ${PROJECT_NAME} APEX )
