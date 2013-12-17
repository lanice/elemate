SET(APEX_INCLUDE_PATH_DESCRIPTION "The directory containing NxApex.h.")

SET(APEX_DIR_SEARCH $ENV{APEX_ROOT})

SET(VERSION_SUFFIX_FOR_PATH
   v1.2.4
   v1.2.3
   1.2.3
   1.2.3
)

SET(APEX_DIR_SEARCH
  ${APEX_DIR_SEARCH}/module
  # ${DELTA3D_EXT_DIR}/inc
  # /usr/include/APEX
  # "C:/Program Files/NVIDIA Corporation/NVIDIA APEX SDK"
  # "C:/Program Files/AGEIA Technologies/AGEIA APEX SDK"
  # "C:/Program Files/AGEIA Technologies/APEXSDK-1.2.4-Build6-CL16501900-PhysX_3.2.4-WIN-VC10-BIN"
)

FIND_PATH(APEX_MODULES_INCLUDES NxModifier.h PATH_SUFFIXES ${VERSION_SUFFIX_FOR_PATH} PATHS

  # Look in other places.
  ${APEX_DIR_SEARCH}/iofx/public

  # Help the user find it if we cannot.
  DOC "The ${APEX_INCLUDE_PATH_DESCRIPTION}"
)
if(APEX_MODULES_INCLUDES)
	SET(APEX_MODULES_INCLUDE_DIR ${APEX_DIR_SEARCH})
endif (APEX_MODULES_INCLUDES)

IF (${CMAKE_SYSTEM} MATCHES "Linux")
    ADD_DEFINITIONS(-DLINUX -DNX_DISABLE_FLUIDS)
ENDIF (${CMAKE_SYSTEM} MATCHES "Linux")

set (APEX_MODULES_FOUND "NO")
if (APEX_MODULES_INCLUDE_DIR)
   set (APEX_MODULES_FOUND "YES")
endif (APEX_MODULES_INCLUDE_DIR)