::
::  before-test.cmd - Prepares AppVeyor CI worker to run epanet regression tests
::
::  Date Created: 4/3/2018
::
::  Author: Michael E. Tryby
::          US EPA - ORD/NRMRL
::
::  Arguments:
::    1 - relative path regression test file staging location
::    2 - absolute path to location of software under test
::    3 - build identifier for software under test
::
::  Note:
::    Tests and benchmark files are stored in the epanet-example-networks repo.
::    This script retreives them using a stable URL associated with a release on
::    GitHub and stages the files for nrtest to run. The script assumes that
::    before-test.cmd and gen-config.cmd are located together in the same folder.
::

@echo off
setlocal

set SCRIPT_HOME=%~dp0
set TEST_HOME=%~1


set EXAMPLES_VER=1.0.2-dev.5
set BENCHMARK_VER=220dev5


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
%SCRIPT_HOME%\gen-config.cmd %~2 > apps\epanet-%~3.json
