::
::  run_nrtest.cmd - Runs numerical regression test
::
::  Date Created: 1/8/2018
::
::  Author: Michael E. Tryby
::          US EPA - ORD/NRMRL
::
::  Arguments:
::    1 - version/build identifier
::    2 - (test suite path)
::

@echo off
setlocal


:: CHANGE THIS VARIABLES TO UPDATE BENCHMARK
set BENCHMARK_VER=220dev5


:: Determine location of python Scripts folder
FOR /F "tokens=*" %%G IN ('where python') DO (
  set PYTHON_DIR=%%~dpG
)
set "NRTEST_SCRIPT_PATH=%PYTHON_DIR%Scripts"

:: Check existence and apply default arguments
IF NOT [%1]==[] ( set "SUT_VER=%~1"
) ELSE ( set "SUT_VER=vXXX" )

IF NOT [%2]==[] ( set "TEST_SUITE_PATH=%~2"
) ELSE ( set "TEST_SUITE_PATH=nrtestsuite" )


set NRTEST_EXECUTE_CMD=python %NRTEST_SCRIPT_PATH%\nrtest execute
set TEST_APP_PATH=apps\epanet-%SUT_VER%.json
set TESTS=tests\examples tests\exeter tests\large tests\network_one tests\press_depend tests\small tests\tanks tests\valves
set TEST_OUTPUT_PATH=benchmark\epanet-%SUT_VER%

set NRTEST_COMPARE_CMD=python %NRTEST_SCRIPT_PATH%\nrtest compare
set REF_OUTPUT_PATH=benchmark\epanet-%BENCHMARK_VER%
set RTOL_VALUE=0.01
set ATOL_VALUE=0.0

:: change current directory to test suite
cd %TEST_SUITE_PATH%

:: if present clean test benchmark results
if exist %TEST_OUTPUT_PATH% (
  rmdir /s /q %TEST_OUTPUT_PATH%
)

echo INFO: Creating test benchmark
set NRTEST_COMMAND=%NRTEST_EXECUTE_CMD% %TEST_APP_PATH% %TESTS% -o %TEST_OUTPUT_PATH%
:: if there is an error exit the script with error value 1
%NRTEST_COMMAND% || exit /B 1

echo.

echo INFO: Comparing test and ref benchmark
set NRTEST_COMMAND=%NRTEST_COMPARE_CMD% %TEST_OUTPUT_PATH% %REF_OUTPUT_PATH% --rtol %RTOL_VALUE% --atol %ATOL_VALUE% -o receipt.json
%NRTEST_COMMAND%
