cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

set(TARGET_NAME elemate)
    
add_executable( ${PROJECT_NAME} main.cpp )
target_link_libraries( ${PROJECT_NAME} libelemate )

set_target_properties(${TARGET_NAME}
    PROPERTIES
    LINKER_LANGUAGE CXX
    COMPILE_DEFINITIONS_DEBUG   "${DEFAULT_COMPILE_DEFS_DEBUG}"
    COMPILE_DEFINITIONS_RELEASE "${DEFAULT_COMPILE_DEFS_RELEASE}"
    COMPILE_DEFINITIONS_RELWITHDEBINFO "${DEFAULT_COMPILE_DEFS_RELWITHDEBINFO}"
    #COMPILE_FLAGS               "${DEFAULT_COMPILE_FLAGS}"
    LINK_FLAGS_DEBUG            "${DEFAULT_LINKER_FLAGS_DEBUG}"
    LINK_FLAGS_RELEASE          "${DEFAULT_LINKER_FLAGS_RELEASE}"
    LINK_FLAGS_RELWITHDEBINFO   "${DEFAULT_LINKER_FLAGS_RELWITHDEBINFO}"
)

config_project( ${TARGET_NAME} OPENGL )
config_project( ${TARGET_NAME} GLEW )
config_project( ${TARGET_NAME} GLFW )
config_project( ${TARGET_NAME} GLOW )
config_project( ${TARGET_NAME} GLOW_UTILS )
config_project( ${TARGET_NAME} PHYSX )
config_project( ${TARGET_NAME} FMOD )
config_project( ${TARGET_NAME} ASSIMP )
config_project( ${TARGET_NAME} LUA )

if(LINUX)
    target_link_libraries( ${TARGET_NAME} dl)
endif()
