#Top/Root/Relentless

cmake_minimum_required(VERSION 4.1.0)

include(ProcessorCount)
ProcessorCount(N)
if (NOT DEFINED CMAKE_BUILD_PARALLEL_LEVEL)
    if (N GREATER 2)
        math(EXPR N "${N} - 2")
    endif()
    set(CMAKE_BUILD_PARALLEL_LEVEL ${N})
endif()

if(${CMAKE_VERSION} VERSION_LESS 4.1.0)
	message(FATAL_ERROR "Fatal error: CMake minimum version required is 4.1.0. Update CMake at cmake.org to continue. Exiting...")
endif()

if (NOT WIN32)
	message(FATAL_ERROR "Fatal error: Relentless currently only supports Windows platform. Exiting...")
endif()

project("Relentless")

include(FetchContent)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.17.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(spdlog)


set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

set(CMAKE_SYSTEM_VERSION "10.0.26100.0" CACHE STRING "Target Windows SDK version")

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define the macro globally to resolve mutex constructor issue
add_compile_definitions(_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR)

if(MSVC)
    add_compile_options(/wd4201)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ox -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O2")

add_subdirectory("Relentless")
add_subdirectory("Relentless-Editor")

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Relentless-Editor)
