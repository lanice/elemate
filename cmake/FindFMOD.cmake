
# FMOD_FOUND
# FMOD_INCLUDE_DIR
# FMOD_LIBRARY
# FMOD_BINARY

find_path(FMOD_INCLUDE_DIR fmod.hpp
    $ENV{FMOD_HOME}/inc
    /usr/include/fmod
    /usr/local/include/fmod
    /sw/include/fmod
    /opt/local/include/fmod
    DOC "The directory where FMOD resides")

if (WIN32)

    find_file(FMOD_BINARY
        NAMES fmodex64.dll
        PATHS
        $ENV{FMOD_HOME}
        DOC "The FMOD binary")

    find_library(
        FMOD_LIBRARY
        NAMES fmodex64_vc.lib
        PATHS
        $ENV{FMOD_HOME}/lib
        DOC "The FMOD library")

else()

    find_library(FMOD_LIBRARY
        NAMES fmodex
        PATHS
        /usr/lib64
        /usr/local/lib64
        /sw/lib64
        /opt/local/lib64
        /usr/lib
        /usr/local/lib
        /sw/lib
        /opt/local/lib
        DOC "The FMOD library")

endif()

    
if(FMOD_INCLUDE_DIR AND FMOD_LIBRARY)
    set(FMOD_FOUND 1 CACHE STRING "Set to 1 if FMOD is found, 0 otherwise")
else()
    set(FMOD_FOUND 0 CACHE STRING "Set to 1 if FMOD is found, 0 otherwise")
    message(WARNING "Note: an envvar like FMOD_HOME assists this script to locate fmod.")
endif()

mark_as_advanced( FMOD_FOUND )
