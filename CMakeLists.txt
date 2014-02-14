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

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

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

set(ELEMATE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libelemate)

include_directories(
    ${ELEMATE_INCLUDE_DIR}
    SYSTEM ${GLOW_INCLUDE_DIR}
    SYSTEM ${GLEW_INCLUDE_DIR}
    SYSTEM ${GLM_INCLUDE_DIR}
    SYSTEM ${GLFW_INCLUDE_DIR}
    SYSTEM ${PHYSX_INCLUDE_DIR}
    SYSTEM ${LUA_INCLUDE_DIR}
)

# set c++ compiler options globally
set ( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${DEFAULT_COMPILE_FLAGS} ${DEFAULT_COMPILE_FLAGS_RELEASE}" )
set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${DEFAULT_COMPILE_FLAGS} ${DEFAULT_COMPILE_FLAGS_DEBUG}" )
set ( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${DEFAULT_COMPILE_FLAGS} ${DEFAULT_COMPILE_FLAGS_RELWITHDEBINFO}" )


# SOURCES AND TARGET CONFIGURATION

add_subdirectory(shader)
add_subdirectory(scripts)
add_subdirectory(libelemate)
add_subdirectory(elemate)

option(ELEMATE_BUILD_TESTS "Set to ON to build elemate tests." ON)
if(ELEMATE_BUILD_TESTS)
	add_subdirectory(tests)
endif()
