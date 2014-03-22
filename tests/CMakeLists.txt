
set(TARGET_NAME elemate_tests)

set(ELEMATE_TEST_GROUP "elemate Tests")
set(gtestroot ${CMAKE_CURRENT_SOURCE_DIR}/gtest-1.7.0)

enable_testing()

add_subdirectory(${gtestroot})

set_target_properties(gtest
    PROPERTIES
    FOLDER ${ELEMATE_TEST_GROUP})
set_target_properties(gtest_main
    PROPERTIES
    FOLDER ${ELEMATE_TEST_GROUP})

set_cxx_target_properties(gtest)
set_cxx_target_properties(gtest_main)
    
# VS2012 sets the maximum number of arguments in its simulation of variadic templates to 5, but google test needs more
if(MSVC)
    target_compile_definitions(gtest PUBLIC _VARIADIC_MAX=10)
    target_compile_definitions(gtest_main PUBLIC _VARIADIC_MAX=10)
endif()

include_directories(
    ${ELEMATE_INCLUDE_DIR}
    ${gtestroot}/include
)

set( TEST_SOURCES
    test.cpp
    units/game_test.cpp
)

add_executable(${TARGET_NAME} ${TEST_SOURCES} )

set_cxx_target_properties(${TARGET_NAME})

target_link_libraries(${TARGET_NAME}
    gtest
    gtest_main
    libelemate
)

add_test(
    NAME ${TARGET_NAME}
    COMMAND ${TARGET_NAME})
    
set_target_properties(${TARGET_NAME}
    PROPERTIES
    FOLDER ${ELEMATE_TEST_GROUP})
