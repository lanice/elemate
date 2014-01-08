
set(VERSION_SUFFIX_FOR_PATH
   v3.2.1
   v3.2.4
   3.2.1
   3.2.4
)

set(PHYSX_ROOT $ENV{PHYSX_ROOT} CACHE PATH
    "Base path to search for PhysX versions.")

set(PHYSX_INCLUDE_SEARCH_DIR
    ${PHYSX_ROOT}/Include
    "C:/Program Files/NVIDIA Corporation/NVIDIA PhysX SDK"
    "C:/Program Files/AGEIA Technologies/AGEIA PhysX SDK"
    "C:/Program Files/AGEIA Technologies/SDK"
    /usr/include/PhysX
    /usr/local/include/PhysX
)

find_path(PHYSX_INCLUDE_DIR PxPhysicsAPI.h PATH_SUFFIXES ${VERSION_SUFFIX_FOR_PATH}
    PATHS ${PHYSX_INCLUDE_SEARCH_DIR}
    DOC " The Base directory containing the PhysX include files."
)

set(PHYSX_LIB_SEARCH_DIR
    ${PHYSX_ROOT}/Lib/win64
    ${PHYSX_ROOT}/lib64
    ${PHYSX_ROOT}/lib
    ${PHYSX_ROOT}/Lib/linux64
    ${PHYSX_ROOT}/Bin/linux64
    /usr/lib64/PhysX
    /usr/local/lib64/Physx
    /usr/lib/PhysX
    /usr/local/lib/Physx
)

set(PHYSX_LIBRARY "")


set (PHYSX_LIBRARY_TYPE "CHECKED" CACHE STRING
    ".. [emptystring], DEBUG, CHECKED, PROFILE")

MACRO(ADD_PHYSX_LIBRARY LIBNAME)
    find_library( ${LIBNAME}_LIBRARY
        NAMES
            ${LIBNAME}${PHYSX_LIBRARY_TYPE}
            ${LIBNAME}${PHYSX_LIBRARY_TYPE}_x64
        PATHS ${PHYSX_LIB_SEARCH_DIR}
    )
    message (${LIBNAME} : ${${LIBNAME}_LIBRARY} )
    #ADD_LIBRARY(${LIBNAME} STATIC IMPORTED)
    #SET_TARGET_PROPERTIES( ${LIBNAME} PROPERTIES
    #    IMPORTED_LOCATION ${${LIBNAME}_LIBRARY} )
    set(PHYSX_LIBRARY ${PHYSX_LIBRARY} ${${LIBNAME}_LIBRARY})
ENDMACRO()

if (WIN32)
    ADD_PHYSX_LIBRARY( PhysX3 )
    ADD_PHYSX_LIBRARY( PhysX3Common )
    ADD_PHYSX_LIBRARY( PhysX3Extensions )
else()
    ADD_PHYSX_LIBRARY( PvdRuntime )
    ADD_PHYSX_LIBRARY( SimulationController )
    ADD_PHYSX_LIBRARY( SceneQuery )
    ADD_PHYSX_LIBRARY( LowLevel )
    ADD_PHYSX_LIBRARY( LowLevelCloth )
    ADD_PHYSX_LIBRARY( PhysX3Vehicle )
    ADD_PHYSX_LIBRARY( PhysX3Cooking )
    ADD_PHYSX_LIBRARY( PhysX3Extensions )
    ADD_PHYSX_LIBRARY( PhysX3CharacterKinematic )
    ADD_PHYSX_LIBRARY( PhysXProfileSDK )
    ADD_PHYSX_LIBRARY( PhysXVisualDebuggerSDK )
    ADD_PHYSX_LIBRARY( PxTask )
    ADD_PHYSX_LIBRARY( PhysX3 )
    ADD_PHYSX_LIBRARY( PhysX3Common )
endif()

set(PHYSX_LIBRARY ${PHYSX_LIBRARY}
    GL
    GLU
    glut
    X11
    rt
    pthread)

#IF (${CMAKE_SYSTEM} MATCHES "Linux")
    #ADD_DEFINITIONS(-DLINUX -DNX_DISABLE_FLUIDS)
#    ADD_DEFINITIONS(-DLINUX)
#ENDIF ()

set (PHYSX_FOUND "NO")
if (PHYSX_INCLUDE_DIR AND PHYSX_LIBRARY)
   set (PHYSX_FOUND "YES")
endif ()
