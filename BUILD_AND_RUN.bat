@echo off
setlocal EnableExtensions DisableDelayedExpansion
pushd "%~dp0"
if errorlevel 1 exit /b 1

set "CMAKE_GENERATOR_INSTANCE="
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto missing_visual_studio

set "VSROOT="
for /f "usebackq delims=" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSROOT=%%I"
if not defined VSROOT goto missing_visual_studio
if not exist "%VSROOT%\Common7\Tools\VsDevCmd.bat" goto missing_visual_studio

call "%VSROOT%\Common7\Tools\VsDevCmd.bat" -no_logo -arch=amd64 -host_arch=amd64
if errorlevel 1 goto developer_environment_failed
cd /d "%~dp0"
set "CMAKE_GENERATOR_INSTANCE="

set "CMAKE_EXE=%VSROOT%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
if exist "%CMAKE_EXE%" goto cmake_ready
set "CMAKE_EXE="
for %%I in (cmake.exe) do set "CMAKE_EXE=%%~$PATH:I"
if not defined CMAKE_EXE goto missing_cmake

:cmake_ready
for %%I in ("%CMAKE_EXE%") do set "CTEST_EXE=%%~dpIctest.exe"
if not exist "%CTEST_EXE%" goto missing_cmake
set "NINJA_EXE=%VSROOT%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"
if exist "%NINJA_EXE%" goto ninja_ready
set "NINJA_EXE="
for %%I in (ninja.exe) do set "NINJA_EXE=%%~$PATH:I"
if not defined NINJA_EXE goto missing_cmake

:ninja_ready
echo Visual Studio: %VSROOT%
echo Configuring BlockBlast3D...
"%CMAKE_EXE%" -S . -B build-ninja -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_MAKE_PROGRAM="%NINJA_EXE%" -DBUILD_TESTING=ON
if errorlevel 1 goto configure_failed

echo Building BlockBlast3D...
"%CMAKE_EXE%" --build build-ninja --config Release --parallel
if errorlevel 1 goto build_failed

echo Checking game logic...
"%CTEST_EXE%" --test-dir build-ninja -C Release --output-on-failure
if errorlevel 1 goto tests_failed

echo Starting BlockBlast3D...
pushd build-ninja
BlockBlast3D.exe
set "RESULT=%ERRORLEVEL%"
popd
if not "%RESULT%"=="0" goto launch_failed
popd
endlocal
exit /b 0

:missing_visual_studio
echo ERROR: Visual Studio with C++ build tools was not found.
echo Open Visual Studio Installer, select Modify, and install Desktop development with C++.
goto failure

:missing_cmake
echo ERROR: CMake, CTest, or Ninja was not found.
echo In Visual Studio Installer, install C++ CMake tools for Windows.
goto failure

:developer_environment_failed
echo ERROR: Visual Studio could not initialize the x64 C++ environment.
goto failure

:configure_failed
echo ERROR: CMake configuration failed. See the message above.
goto failure

:build_failed
echo ERROR: Compilation failed. See the first compiler error above.
goto failure

:tests_failed
echo ERROR: Game logic checks failed. See the test output above.
goto failure

:launch_failed
echo ERROR: BlockBlast3D exited with code %RESULT%. See the message above.
goto failure

:failure
echo.
echo Copy this error output or take a screenshot for troubleshooting.
pause
popd
endlocal
exit /b 1
