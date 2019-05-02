::
::  makefile.bat - build epanet and go do whatever
::
::  Date Created: 5/2/2019
::
::  Author: Michael E. Tryby
::          US EPA - ORD/NRMRL
::
:: Requires:
::    Build Tools for Visual Studio download:
::      https://visualstudio.microsoft.com/downloads/
::
::    CMake download:
::      https://cmake.org/download/
::
:: Note:
::    This script must be located at the root of the project folder
::    in order to work properly.
::

@echo off
echo INFO: Building epanet  ...

set GENERATOR=Visual Studio 15 2017

:: Determine project path and strip trailing \ from path
set "PROJECT_PATH=%~dp0"
IF %PROJECT_PATH:~-1%==\ set "PROJECT_PATH=%PROJECT_PATH:~0,-1%"

:: check for requirements
WHERE cmake
IF %ERRORLEVEL% NEQ 0 ECHO cmake not installed & EXIT /B 1

:: generate build system
IF exist buildprod_win32 ( cd buildprod_win32 ) ELSE ( mkdir buildprod_win32 & cd buildprod_win32 )
cmake -G"%GENERATOR%" -DBUILD_TESTS=OFF ..

cd ..
IF exist buildprod_win64 ( cd buildprod_win64 ) ELSE ( mkdir buildprod_win64 & cd buildprod_win64 )
cmake -G"%GENERATOR% Win64" -DBUILD_TESTS=OFF ..

:: perform build
cmake --build . --config Release
cd .. & cd buildprod_win32
cmake --build . --config Release

:: return to project root
cd %PROJECT_PATH%
