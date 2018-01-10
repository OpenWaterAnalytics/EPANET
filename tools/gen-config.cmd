::
::  gen-config.cmd - Generated nrtest app configuration file for test executable
::  
::  Date Created: 1/8/2018
::
::  Author: Michael E. Tryby
::          US EPA - ORD/NRMRL
::
::  Arguments: 
::    1 - absolute path to test executable (valid path seperator for nrtest is "/")
::

@echo off
setlocal

:: process path to remove quotes and convert backward to forward slash
set ABS_BUILD_PATH=%~1
set ABS_BUILD_PATH=%ABS_BUILD_PATH:\=/%

:: this is the target created by the cmake build script
set TEST_CMD=runepanet.exe

echo {
echo     "name" : "epanet",
echo     "version" : "",
echo     "description" : "",
echo     "setup_script" : "",
echo     "exe" : "%ABS_BUILD_PATH%/%TEST_CMD%"
echo }
