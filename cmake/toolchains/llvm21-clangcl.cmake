# Discover clang-cl. Search order:
#   1. A value the user explicitly passed (-DCLANG_CL_EXECUTABLE=...)
#   2. PATH (covers package-manager installs / "add to PATH" installs)
#   3. LLVM_HOME / LLVM_DIR environment variable (custom install locations)
#   4. The standard LLVM installer location
#   5. Visual Studio's bundled Clang (last resort — may be too old)

find_program(CLANG_CL_EXECUTABLE
    NAMES clang-cl.exe clang-cl
    HINTS
        "$ENV{LLVM_HOME}/bin"
        "$ENV{LLVM_DIR}/bin"
    PATHS
        "C:/Program Files/LLVM/bin"
        "C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/Llvm/x64/bin"
        "C:/Program Files/Microsoft Visual Studio/18/Professional/VC/Tools/Llvm/x64/bin"
        "C:/Program Files/Microsoft Visual Studio/18/Enterprise/VC/Tools/Llvm/x64/bin"
    DOC "Path to clang-cl executable (override with -DCLANG_CL_EXECUTABLE=... if not found)"
)

if(NOT CLANG_CL_EXECUTABLE)
    message(FATAL_ERROR
        "clang-cl not found.\n"
        "Relentless requires standalone LLVM/Clang 21 or newer.\n"
        "  1. Install from https://github.com/llvm/llvm-project/releases\n"
        "  2. Either install to the default C:/Program Files/LLVM,\n"
        "     OR set the LLVM_HOME environment variable to your install location,\n"
        "     OR pass -DCLANG_CL_EXECUTABLE=<path-to-clang-cl.exe> to CMake.")
endif()

message(STATUS "Relentless: Using clang-cl at ${CLANG_CL_EXECUTABLE}")
set(CMAKE_C_COMPILER   "${CLANG_CL_EXECUTABLE}" CACHE FILEPATH "" FORCE)
set(CMAKE_CXX_COMPILER "${CLANG_CL_EXECUTABLE}" CACHE FILEPATH "" FORCE)
set(CMAKE_C_COMPILER_TARGET   "x86_64-pc-windows-msvc" CACHE STRING "" FORCE)
set(CMAKE_CXX_COMPILER_TARGET "x86_64-pc-windows-msvc" CACHE STRING "" FORCE)