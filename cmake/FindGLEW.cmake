# - Find the OpenGL Extension Wrangler Library (GLEW)
# This module defines the following variables:
#  GLEW_INCLUDE_DIRS - include directories for GLEW
#  GLEW_LIBRARIES - libraries to link against GLEW
#  GLEW_FOUND - true if GLEW has been found and can be used

#=============================================================================
# Copyright 2012 Benjamin Eikel
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

find_path(GLEW_INCLUDE_DIR GL/glew.h)
find_library(GLEW_LIBRARY NAMES GLEW glew32 glew glew32s PATH_SUFFIXES lib64)

find_path( GLEW_INCLUDE_DIR 
    NAMES
        GL/glew.h
    PATHS
        ${GLEW_HOME}/include
        $ENV{GLEW_HOME}/include
        $ENV{PROGRAMFILES}/GLEW/include
        ${OPENGL_INCLUDE_DIR}
        /opt/graphics/OpenGL/include
        /opt/graphics/OpenGL/contrib/libglew
        /usr/local/include
        /usr/include/GL
        /usr/include
    DOC 
        "The directory where GL/glew.h resides"
)

find_library( GLEW_LIBRARY
    NAMES 
        GLEW 
        glew32 
        glew
        glew32s
    PATHS
        ${GLEW_HOME}/lib
        ${GLEW_HOME}/lib/Release/x64
        $ENV{GLEW_HOME}/lib
        $ENV{GLEW_HOME}/lib/Release/x64
        ${OPENGL_LIBRARY_DIR}
    DOC 
        "The GLEW library"
)

set(GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIR})
set(GLEW_LIBRARIES ${GLEW_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLEW
                                  REQUIRED_VARS GLEW_INCLUDE_DIR GLEW_LIBRARY)

mark_as_advanced(GLEW_INCLUDE_DIR GLEW_LIBRARY)
