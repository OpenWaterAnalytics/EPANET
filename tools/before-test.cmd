::
::  before-test.cmd - Prepares AppVeyor CI worker to run epanet regression tests
::
::  Date Created: 4/3/2018
::
::  Author: Michael E. Tryby
::          US EPA - ORD/NRMRL
::
::  Arguments:
::    1 - build identifier for software under test
::    2 - (relative path regression test file staging location)
::
::  Note:
::    Tests and benchmark files are stored in the epanet-example-networks repo.
::    This script retreives them using a stable URL associated with a GitHub
::    release and stages the files for nrtest to run. The script assumes that
::    before-test.cmd and gen-config.cmd are located together in the same folder.
::

@echo off
setlocal


set EXAMPLES_VER=1.0.2-dev.5
set BENCHMARK_VER=220dev5

set "SCRIPT_HOME=%~dp0"
set "EXE_HOME=buildprod\bin\Release"

:: Determine SUT executable path
for %%a in ("%SCRIPT_HOME:~0,-1%") do set "SUT_PATH=%%~dpa"
set SUT_PATH=%SUT_PATH%%EXE_HOME%

:: Check existence and apply default arguments
IF NOT [%1]==[] ( set "SUT_VER=%~1"
) ELSE ( set "SUT_VER=vXXX" )

IF NOT [%2]==[] ( set "TEST_HOME=%~2"
) ELSE ( set "TEST_HOME=nrtestsuite" )


set TESTFILES_URL=https://github.com/OpenWaterAnalytics/epanet-example-networks/archive/v%EXAMPLES_VER%.zip
set BENCHFILES_URL=https://github.com/OpenWaterAnalytics/epanet-example-networks/releases/download/v%EXAMPLES_VER%/epanet-benchmark-%BENCHMARK_VER%.zip


echo INFO: Staging files for regression testing

:: create a clean directory for staging regression tests
if exist %TEST_HOME% (
  rmdir /s /q %TEST_HOME%
)
mkdir %TEST_HOME%
cd %TEST_HOME%

:: retrieve epanet-examples for regression testing
curl -fsSL -o examples.zip %TESTFILES_URL%

:: retrieve epanet benchmark results
curl -fsSL -o benchmark.zip %BENCHFILES_URL%


:: extract tests and benchmarks
7z x examples.zip *\epanet-tests\* > nul
7z x benchmark.zip -obenchmark\ > nul

:: set up symlink for tests directory
mklink /D .\tests .\epanet-example-networks-%EXAMPLES_VER%\epanet-tests


:: generate json configuration file for software under test
mkdir apps
%SCRIPT_HOME%\gen-config.cmd %SUT_PATH% > apps\epanet-%SUT_VER%.json
