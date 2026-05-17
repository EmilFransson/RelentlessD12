@echo off
setlocal

set CONFIG=Release
if not "%~1"=="" set CONFIG=%~1

set BUILD_DIR=out\build\%CONFIG%

REM --- Initialize Visual Studio developer environment (Windows SDK, mt.exe, etc.) ---
REM Try VS 2026 Community first, fall back to other editions / years.
set VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist %VSWHERE% (
    echo [ERROR] Visual Studio not detected ^(vswhere.exe missing^).
    echo Install Visual Studio 2022 or 2026 with the "Desktop development with C++" workload.
    pause
    exit /b 1
)

for /f "usebackq tokens=*" %%i in (`%VSWHERE% -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set VS_INSTALL=%%i
)

if not defined VS_INSTALL (
    echo [ERROR] No Visual Studio installation found with C++ tools.
    pause
    exit /b 1
)

call "%VS_INSTALL%\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 (
    echo [ERROR] Failed to initialize Visual Studio developer environment.
    pause
    exit /b 1
)

REM --- Configure if needed ---
if not exist "%BUILD_DIR%\CMakeCache.txt" (
    echo Configuring CMake...
    cmake -S . -B "%BUILD_DIR%" -G Ninja ^
        -DCMAKE_BUILD_TYPE=%CONFIG% ^
        -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/llvm21-clangcl.cmake
    if errorlevel 1 (
        pause
        exit /b 1
    )
)

REM --- Build ---
cmake --build "%BUILD_DIR%"
if errorlevel 1 (
    pause
    exit /b 1
)

REM --- Launch ---
set EXE_DIR=%BUILD_DIR%\Relentless-Editor
set EXE=%EXE_DIR%\Relentless-Editor.exe

if not exist "%EXE%" (
    echo [ERROR] Build succeeded but executable not found at:
    echo   %EXE%
    pause
    exit /b 1
)

echo.
echo Launching %EXE% ...
start "" "%EXE%"

endlocal