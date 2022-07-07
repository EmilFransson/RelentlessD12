#Top/Root/Relentless
cmake_minimum_required(VERSION "3.22.0")

if(${CMAKE_VERSION} VERSION_LESS "3.22.0")
	message(FATAL_ERROR "Fatal error: CMake minimum version required is 3.22.0. Update CMake at cmake.org to continue. Exiting...")
endif()

if (NOT WIN32)
	message(FATAL_ERROR "Fatal error: Relentless currently only supports Windows platform. Exiting...")
endif()

#if (NOT CMAKE_GENERATOR MATCHES "Visual Studio")
#	message(FATAL_ERROR "Fatal error: Relentless currently only supports Visual Studio as generator. Exiting...")
#endif()

project("Relentless")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory("Relentless")
add_subdirectory("Relentless-Editor")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Relentless-Editor)