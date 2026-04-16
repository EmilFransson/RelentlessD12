#Top/Root/Relentless

cmake_minimum_required(VERSION "3.22.0")

include(ProcessorCount)
ProcessorCount(N)
if (NOT DEFINED CMAKE_BUILD_PARALLEL_LEVEL)
    if (N GREATER 2)
        math(EXPR N "${N} - 2")
    endif()
    set(CMAKE_BUILD_PARALLEL_LEVEL ${N})
endif()

if(${CMAKE_VERSION} VERSION_LESS "3.22.0")
	message(FATAL_ERROR "Fatal error: CMake minimum version required is 3.22.0. Update CMake at cmake.org to continue. Exiting...")
endif()

if (NOT WIN32)
	message(FATAL_ERROR "Fatal error: Relentless currently only supports Windows platform. Exiting...")
endif()

add_compile_definitions(FMT_USE_CONSTEXPR=0)

project("Relentless")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

set(CMAKE_SYSTEM_VERSION "10.0.26100.0" CACHE STRING "Target Windows SDK version")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND MSVC)
	message(STATUS "Relentless: Clang detected → enabling -std=c++latest")
	add_compile_options(/clang:-std=c++2c)
endif()

# Define the macro globally to resolve mutex constructor issue
add_compile_definitions(
	_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR
	FMT_CONSTEVAL=
)

if(MSVC)
    add_compile_options(/wd4201)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ox -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O2")

add_subdirectory("Relentless")
add_subdirectory("Relentless-Editor")

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Relentless-Editor)

