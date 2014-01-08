
# support for C++11 etc.

execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
	OUTPUT_VARIABLE GCC_VERSION)

if(NOT (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7))
    message(FATAL_ERROR "unsupported gcc version ${GCC_VERSION}, 4.7 or heigher is required.")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")


set(LINUX_COMPILE_DEFS
	LINUX	                  # Linux system
	PIC		                  # Position-independent code
	_REENTRANT                # Reentrant code
	PX_CHECKED
	PX_SUPPORT_VISUAL_DEBUGGER
)
set(DEFAULT_COMPILE_DEFS_DEBUG
    ${LINUX_COMPILE_DEFS}
    _DEBUG	                  # Debug build
)
set(DEFAULT_COMPILE_DEFS_RELEASE
    ${LINUX_COMPILE_DEFS}
    NDEBUG	                  # Release build
)

set(LINUX_COMPILE_FLAGS "-pthread -pipe -fPIC -Wreturn-type -Wall -Wextra -Wtrampolines -Wcast-qual -Wcast-align -fexceptions -MMD -m64 -march=core-avx-i -mfpmath=sse -ffast-math")
# pthread       -> use pthread library
# no-rtti       -> disable c++ rtti
# no-exceptions -> disable exception handling
# pipe          -> use pipes
# fPIC          -> use position independent code
# -Wreturn-type -Werror=return-type -> missing returns in functions and methods are handled as errors which stops the compilation

set(DEFAULT_COMPILE_FLAGS ${LINUX_COMPILE_FLAGS})
