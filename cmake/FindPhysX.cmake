SET(PHYSX_INCLUDE_PATH_DESCRIPTION "The Base directory containing the PhysX include files. E.g /usr/include/PhysX/2.8.0 or C:/Program\ Files/Ageia\ Technologies/SDK/2.8.0")

SET(PhysX_DIR_SEARCH $ENV{PHYSX_ROOT}/Include)

SET(VERSION_SUFFIX_FOR_PATH
   v3.2.1
   v3.2.4
   3.2.1
   3.2.4
)

# Add in some path suffixes. These will have to be updated whenever a new Boost version comes out.

SET(PhysX_DIR_SEARCH
  ${PhysX_DIR_SEARCH}
  ${DELTA3D_EXT_DIR}/inc
  /usr/include/PhysX
  "C:/Program Files/NVIDIA Corporation/NVIDIA PhysX SDK"
  "C:/Program Files/AGEIA Technologies/AGEIA PhysX SDK"
  "C:/Program Files/AGEIA Technologies/SDK"
)

FIND_PATH(PHYSX_INCLUDES PxPhysicsAPI.h PATH_SUFFIXES ${VERSION_SUFFIX_FOR_PATH} PATHS

  # Look in other places.
  ${PhysX_DIR_SEARCH}

  # Help the user find it if we cannot.
  DOC "The ${PHYSX_INCLUDE_PATH_DESCRIPTION}"
)


SET(PHYSX_INCLUDE_DIR ${PHYSX_INCLUDES})

SET(PHYSX_LIB_DIR_SEARCH $ENV{PHYSX_ROOT}/Lib/win64)

SET(PHYSX_LIB_DIR_SEARCH
   ${PHYSX_LIB_DIR_SEARCH}
   ${DELTA_DIR}/ext/lib
   /usr/lib/PhysX
)

if (NOT WIN32)
   FIND_FILE(PHYSX_LIBRARY_DIR NAMES ${VERSION_SUFFIX_FOR_PATH} PATHS

      # Look in other places.
      ${PHYSX_LIB_DIR_SEARCH}

      # Help the user find it if we cannot.
      DOC "Set PHYSX_LIB_DIR_SEARCH to set the base path to search for PhysX versions."
   )
else (NOT WIN32)
   SET(PHYSX_LIBRARY_DIR ${PHYSX_LIB_DIR_SEARCH})
endif (NOT WIN32)


MACRO(FIND_PHYSX_LIBRARY MYLIBRARY MYLIBRARYNAME)

    FIND_LIBRARY(${MYLIBRARY}
        NAMES ${MYLIBRARYNAME}
        PATHS
        ${PHYSX_LIBRARY_DIR}
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/lib
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        /usr/freeware/lib64
    )
	
ENDMACRO(FIND_PHYSX_LIBRARY MYLIBRARY MYLIBRARYNAME)

if (WIN32)
    SET(PHYSX_LIB PhysX3_x64)
    SET(PHYSX_LIB_COMMON PhysX3Common_x64)
    SET(PHYSX_LIB_EXTENSIONS PhysX3Extensions)
    SET(PHYSX_LIB_COOKING PhysX3Cooking_x64)
else()
    SET(PHYSX_LIB PhysX3)
    SET(PHYSX_LIB_COMMON PhysX3Common)
    SET(PHYSX_LIB_EXTENSIONS PhysX3Extensions)
    SET(PHYSX_LIB_COOKING PhysX3Cooking)
endif()

FIND_PHYSX_LIBRARY(PHYSX_LIBRARY "${PHYSX_LIB}")
FIND_PHYSX_LIBRARY(PHYSX_COMMON_LIBRARY "${PHYSX_LIB_COMMON}")
FIND_PHYSX_LIBRARY(PHYSX_EXTENSION_LIBRARY "${PHYSX_LIB_EXTENSIONS}")
FIND_PHYSX_LIBRARY(PHYSX_COOKING_LIBRARY "${PHYSX_LIB_COOKING}")

SET(PHYSX_LIBRARY ${PHYSX_LIBRARY} ${PHYSX_COMMON_LIBRARY} ${PHYSX_EXTENSION_LIBRARY} ${PHYSX_COOKING_LIBRARY})

IF (${CMAKE_SYSTEM} MATCHES "Linux")
    ADD_DEFINITIONS(-DLINUX -DNX_DISABLE_FLUIDS)
ENDIF (${CMAKE_SYSTEM} MATCHES "Linux")

set (PHYSX_FOUND "NO")
if (PHYSX_INCLUDE_DIR AND PHYSX_LIBRARY)
   set (PHYSX_FOUND "YES")
endif (PHYSX_INCLUDE_DIR AND PHYSX_LIBRARY)
