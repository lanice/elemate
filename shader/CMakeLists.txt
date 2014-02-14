
file(GLOB SHADER
    "*.vert"
    "*.geo"
    "*.frag"
    "*/*.vert"
    "*/*.geo"
    "*/*.frag"
)

source_group_by_path(${CMAKE_CURRENT_SOURCE_DIR} "\\\\.frag$|\\\\.vert$|\\\\.geo$" ${SHADER})

add_custom_target(elemateShader SOURCES ${SHADER})
