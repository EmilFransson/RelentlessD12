# Force LLVM 21 clang-cl (installed standalone)
set(CMAKE_C_COMPILER  "C:/Program Files/LLVM/bin/clang-cl.exe" CACHE FILEPATH "" FORCE)
set(CMAKE_CXX_COMPILER "C:/Program Files/LLVM/bin/clang-cl.exe" CACHE FILEPATH "" FORCE)

# (Optional) Make sure we target MSVC ABI explicitly (clang-cl already does, but harmless)
set(CMAKE_C_COMPILER_TARGET  "x86_64-pc-windows-msvc" CACHE STRING "" FORCE)
set(CMAKE_CXX_COMPILER_TARGET "x86_64-pc-windows-msvc" CACHE STRING "" FORCE)