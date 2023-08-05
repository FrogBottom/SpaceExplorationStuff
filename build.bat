@echo off
REM C++ Build script. To use, make adjustments to the compiler debug, release, common, and linker flags.

REM Set the build file and output executable names, compile flags, and libraries here.

set output_exe_name=VecFinder.exe
set build_file_name=Build.cpp

set common_flags=/W3 /Gm- /EHsc /nologo
set debug_flags=/Od /Z7 /MTd /D DEBUG
set release_flags=/O2 /GL /MT /analyze- /D NDEBUG

set common_libs=
set debug_libs=
set release_libs=

REM Run the build tools, but only if they aren't set up already.

cl >nul 2>nul
if %errorlevel% neq 9009 goto :build
echo Running VS build tool setup.
echo Initializing MS build tools...
call scripts\setup_cl.bat
cl >nul 2>nul
if %errorlevel% neq 9009 goto :build
echo Unable to find build tools! Make sure that you have Microsoft Visual Studio 10 or above installed!
exit /b 1

REM Use the first command-line argument to set the build mode to debug or release (defaulting to debug).
REM If the build directory doesn't exist, create one.

:build
set mode=debug
if /i $%1 equ $release (set mode=release)
if %mode% equ debug (
set flags=%common_flags% %debug_flags%
set libs=%common_libs% %debug_libs%
) else (
set flags=%common_flags% %release_flags%
set libs=%common_libs% %release_libs%
)
echo Building in %mode% mode.
if not exist bin\%mode% mkdir bin\%mode%
pushd bin\%mode%

REM Perform the actual build.

echo.     -Compiling:
call cl %flags%  /Fe: %output_exe_name% ..\..\src\%build_file_name% /link /INCREMENTAL:no /NOLOGO %libs%
if %errorlevel% neq 0 (
popd
goto :fail
)
popd

REM If we made it here, the build was successful!

echo Build complete!
exit /b 0

REM Error state. Print failure message and exit.

:fail
echo Build failed!
exit /b %errorlevel%
