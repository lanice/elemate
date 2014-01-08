
# Group source files in folders (e.g. for MSVC solutions)
# Example: source_group_by_path("${CMAKE_CURRENT_SOURCE_DIR}/src" 
#   "\\\\.h$|\\\\.hpp$|\\\\.cpp$|\\\\.c$|\\\\.ui$|\\\\.qrc$" "Source Files" ${sources})

macro(source_group_by_path PARENT_PATH REGEX GROUP)

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

macro( config_project PROJNAME LIBNAME ADDLIBPATHINCLUDE )
    include_directories( ${${LIBNAME}_INCLUDE_DIR} )
    if ( ${ADDLIBPATHINCLUDE} )
        get_filename_component( LIBPATH ${${LIBNAME}_LIBRARY} DIRECTORY )
        include_directories( ${LIBPATH}/../include )
    endif()
    
    if( ${LIBNAME}_LIBRARY_DEBUG )
        message(STATUS "adding library: " ${LIBNAME} " (optimized/debug)")
        target_link_libraries( ${PROJNAME}
            optimized ${${LIBNAME}_LIBRARY}
            debug   ${${LIBNAME}_LIBRARY_DEBUG})
    else()
        message(STATUS "adding library: " ${LIBNAME} " (optimized only)")
        target_link_libraries( ${PROJNAME} ${${LIBNAME}_LIBRARY})
        message("is: " ${${LIBNAME}_LIBRARY})
    endif()
endmacro()
