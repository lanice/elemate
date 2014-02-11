
# Group source files in folders (e.g. for MSVC solutions)
# Example: source_group_by_path("${CMAKE_CURRENT_SOURCE_DIR}/src" 
#   "\\\\.h$|\\\\.hpp$|\\\\.cpp$|\\\\.c$|\\\\.ui$|\\\\.qrc$" "Source Files" ${sources})

macro(source_subgroup_by_path PARENT_PATH REGEX GROUP)

    set(args ${ARGV})

    list(REMOVE_AT args 0)
    list(REMOVE_AT args 0)
    list(REMOVE_AT args 0)

    foreach(FILENAME ${args})

        get_filename_component(FILEPATH "${FILENAME}" REALPATH)
        file(RELATIVE_PATH FILEPATH ${PARENT_PATH} ${FILEPATH})
        get_filename_component(FILEPATH "${FILEPATH}" PATH)

        string(REPLACE "/" "\\" FILEPATH "${FILEPATH}")

        if(${FILENAME} MATCHES "${REGEX}")
            source_group("${GROUP}\\${FILEPATH}" FILES ${FILENAME})  
        endif()

    endforeach()

endmacro()

macro(source_group_by_path PARENT_PATH REGEX)

    set(args ${ARGV})

    list(REMOVE_AT args 0)
    list(REMOVE_AT args 0)

    foreach(FILENAME ${args})

        get_filename_component(FILEPATH "${FILENAME}" REALPATH)
        file(RELATIVE_PATH FILEPATH ${PARENT_PATH} ${FILEPATH})
        get_filename_component(FILEPATH "${FILEPATH}" PATH)

        string(REPLACE "/" "\\" FILEPATH "${FILEPATH}")

        if(${FILENAME} MATCHES "${REGEX}")
            source_group("${GROUP}\\${FILEPATH}" FILES ${FILENAME})  
        endif()

    endforeach()

endmacro()


macro(list_extract OUTPUT REGEX)

    set(args ${ARGV})

    list(REMOVE_AT args 0)
    list(REMOVE_AT args 0)

    foreach(FILENAME ${args})

        if(${FILENAME} MATCHES "${REGEX}")
            list(APPEND ${OUTPUT} ${FILENAME})
        endif()

    endforeach()

endmacro()

macro( config_project PROJNAME LIBNAME )
    include_directories( ${${LIBNAME}_INCLUDE_DIR} )
    
    if( ${LIBNAME}_LIBRARY_DEBUG )
        message(STATUS "adding library: " ${LIBNAME} " (optimized/debug)")
        target_link_libraries( ${PROJNAME}
            optimized ${${LIBNAME}_LIBRARY}
            debug   ${${LIBNAME}_LIBRARY_DEBUG})
    else()
        message(STATUS "adding library: " ${LIBNAME} " (optimized only)")
        target_link_libraries( ${PROJNAME} ${${LIBNAME}_LIBRARY})
    endif()
endmacro()

macro( set_cxx_target_properties TARGET_NAME )

    set_target_properties(${TARGET_NAME}
        PROPERTIES
        LINKER_LANGUAGE CXX
        COMPILE_DEFINITIONS_DEBUG   "${DEFAULT_COMPILE_DEFS_DEBUG}"
        COMPILE_DEFINITIONS_RELEASE "${DEFAULT_COMPILE_DEFS_RELEASE}"
        COMPILE_DEFINITIONS_RELWITHDEBINFO "${DEFAULT_COMPILE_DEFS_RELWITHDEBINFO}"
        LINK_FLAGS_DEBUG            "${DEFAULT_LINKER_FLAGS_DEBUG}"
        LINK_FLAGS_RELEASE          "${DEFAULT_LINKER_FLAGS_RELEASE}"
        LINK_FLAGS_RELWITHDEBINFO   "${DEFAULT_LINKER_FLAGS_RELWITHDEBINFO}"
    )

endmacro()
