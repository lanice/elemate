
file(GLOB SCRIPTS
    "*.lua"
    "*/*.lua")

source_group_by_path(${CMAKE_CURRENT_SOURCE_DIR} "\\\\.lua$" ${SCRIPTS})

add_custom_target(elemateScripts VERBATIM SOURCES ${SCRIPTS})
