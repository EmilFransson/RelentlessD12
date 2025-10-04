#Top/Root/Relentless

cmake_minimum_required(VERSION "3.22.0")

if(${CMAKE_VERSION} VERSION_LESS "3.22.0")
	message(FATAL_ERROR "Fatal error: CMake minimum version required is 3.22.0. Update CMake at cmake.org to continue. Exiting...")
endif()

if (NOT WIN32)
	message(FATAL_ERROR "Fatal error: Relentless currently only supports Windows platform. Exiting...")
endif()

project("Relentless")
set(CMAKE_SYSTEM_VERSION "10.0.26100.0" CACHE STRING "Target Windows SDK version")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define the macro globally to resolve mutex constructor issue
add_compile_definitions(_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR)

if(MSVC)
    add_compile_options(/wd4201)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ox -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O2")

add_subdirectory("Relentless")
add_subdirectory("Relentless-Editor")

