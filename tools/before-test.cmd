::
::  before-test.cmd - Prepares AppVeyor CI worker to run epanet regression tests
::
::  Date Created: 4/3/2018
::
::  Author: Michael E. Tryby
::          US EPA - ORD/NRMRL
::
::  Arguments:
::    1 - (platform)
::    2 - (build identifier for reference)
::    3 - (build identifier for software under test)
::    4 - (version identifier for software under test)
::    5 - (relative path regression test file staging location)
::
::  Note:
::    Tests and benchmark files are stored in the epanet-example-networks repo.
::    This script retrieves them using a stable URL associated with a GitHub
::    release and stages the files for nrtest to run. The script assumes that
::    before-test.cmd and gen-config.cmd are located together in the same folder.
::

@echo off
setlocal EnableExtensions


IF [%1]==[] ( set PLATFORM=
) ELSE ( set "PLATFORM=%~1" )

IF [%2]==[] ( echo "ERROR: REF_BUILD_ID must be defined" & exit /B 1
) ELSE (set "REF_BUILD_ID=%~2" )

IF [%3]==[] ( set "SUT_BUILD_ID=local"
) ELSE ( set "SUT_BUILD_ID=%~3" )

IF [%4]==[] (set SUT_VERSION=
) ELSE ( set "SUT_VERSION=%~4" )

IF [%5]==[] ( set "TEST_HOME=nrtestsuite"
) ELSE ( set "TEST_HOME=%~5" )


echo INFO: Staging files for regression testing


:: determine SUT executable path
set "SCRIPT_HOME=%~dp0"
:: TODO: This may fail when there is more than one cmake buildprod folder
FOR /D /R "%SCRIPT_HOME%..\" %%a IN (*) DO IF /i "%%~nxa"=="bin" set "BUILD_HOME=%%a"
set "SUT_PATH=%BUILD_HOME%\Release"


:: determine platform from CMakeCache.txt
IF NOT DEFINED PLATFORM (
  FOR /F "tokens=*" %%p IN ( 'findstr CMAKE_SHARED_LINKER_FLAGS:STRING %BUILD_HOME%\..\CmakeCache.txt' ) DO ( set "FLAG=%%p" )
  FOR /F "delims=: tokens=3" %%m IN ( 'echo %FLAG%' ) DO IF "%%m"=="x64" ( set "PLATFORM=win64" ) ELSE ( set "PLATFORM=win32" )
)

:: hack to determine latest tag in epanet-example-networks repo
:: TODO: use GitHub api instead
set "LATEST_URL=https://github.com/OpenWaterAnalytics/epanet-example-networks/releases/latest"
FOR /F delims^=^"^ tokens^=2 %%g IN ('curl --silent %LATEST_URL%') DO ( set "LATEST_TAG=%%~nxg" )

set "TESTFILES_URL=https://github.com/OpenWaterAnalytics/epanet-example-networks/archive/%LATEST_TAG%.zip"
set "BENCHFILES_URL=https://github.com/OpenWaterAnalytics/epanet-example-networks/releases/download/%LATEST_TAG%/benchmark-%PLATFORM%-%REF_BUILD_ID%.zip"


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
mklink /D .\tests .\epanet-example-networks-%LATEST_TAG:~1%\epanet-tests > nul


:: generate json configuration file for software under test
mkdir apps
%SCRIPT_HOME%\gen-config.cmd %SUT_PATH% %PLATFORM% %SUT_BUILD_ID% %SUT_VERSION% > apps\epanet-%SUT_BUILD_ID%.json
