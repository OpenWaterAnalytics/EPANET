::
::  run_nrtest.cmd - Runs numerical regression test
::
::  Date Created: 1/8/2018
::
::  Author: Michael E. Tryby
::          US EPA - ORD/NRMRL
::
::  Arguments:
::    1 - (REF build identifier)
::    2 - (SUT build identifier)
::    3 - (test suite path)
::

@echo off
setlocal


:: Check existence and apply default arguments
IF [%1]==[] ( echo "ERROR: REF_BUILD_ID must be defined" & exit /B 1
) ELSE ( set "REF_BUILD_ID=%~1" )

IF [%2]==[] ( set "SUT_BUILD_ID=local"
) ELSE ( set "SUT_BUILD_ID=%~2" )

IF [%3]==[] ( set "TEST_SUITE_PATH=nrtestsuite"
) ELSE ( set "TEST_SUITE_PATH=%~3" )


:: determine location of python Scripts folder
FOR /F "tokens=*" %%G IN ('where python') DO (
  set PYTHON_DIR=%%~dpG
)
set "NRTEST_SCRIPT_PATH=%PYTHON_DIR%Scripts"


set NRTEST_EXECUTE_CMD=python %NRTEST_SCRIPT_PATH%\nrtest execute
set TEST_APP_PATH=apps\epanet-%SUT_BUILD_ID%.json
set TESTS=tests\examples tests\exeter tests\large tests\network_one tests\press_depend tests\small tests\tanks tests\valves
set TEST_OUTPUT_PATH=benchmark\epanet-%SUT_BUILD_ID%

set NRTEST_COMPARE_CMD=python %NRTEST_SCRIPT_PATH%\nrtest compare
set REF_OUTPUT_PATH=benchmark\epanet-%REF_BUILD_ID%
set RTOL_VALUE=0.01
set ATOL_VALUE=0.0

:: change current directory to test suite
cd %TEST_SUITE_PATH%

:: if present clean test benchmark results
if exist %TEST_OUTPUT_PATH% (
  rmdir /s /q %TEST_OUTPUT_PATH%
)

echo INFO: Creating SUT %SUT_BUILD_ID% artifacts
set NRTEST_COMMAND=%NRTEST_EXECUTE_CMD% %TEST_APP_PATH% %TESTS% -o %TEST_OUTPUT_PATH%
:: if there is an error exit the script with error value 1
%NRTEST_COMMAND% || exit /B 1

echo.

echo INFO: Comparing SUT artifacts to REF %REF_BUILD_ID%
set NRTEST_COMMAND=%NRTEST_COMPARE_CMD% %TEST_OUTPUT_PATH% %REF_OUTPUT_PATH% --rtol %RTOL_VALUE% --atol %ATOL_VALUE% -o benchmark\receipt.json
%NRTEST_COMMAND%
