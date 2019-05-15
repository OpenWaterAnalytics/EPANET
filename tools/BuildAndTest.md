## Building OWA EPANET From Source on Windows
by Michael E. Tryby

Created on: March 13, 2018 


### Introduction

Building OWA's fork of EPANET from source is a basic skill that all developers 
interested in contributing to the project should know how to perform. This 
document describes the build process step-by-step. You will learn 1) how to 
configure your machine to build the project locally; 2) how to obtain the 
project files using git; 3) how to use cmake to generate build files and build 
the project; and 4) how to use ctest and nrtest to perform unit and regression 
testing on the build artifacts produced. Be advised, you will need local admin 
privileges on your machine to follow this tutorial. Let’s begin! 

### Dependencies

Before the project can be built the required tools must be installed. The OWA 
EPANET project adheres to a platform compiler policy - for each platform there 
is a designated compiler. The platform compiler for Windows is Visual 
Studio cl, for Linux gcc, and for Mac OS clang. These instructions describe how
to build EPANET on Windows. CMake is a cross platform build, testing, and packaging 
tool that is used to automate the EPANET build workflow. Boost is a free portable 
peer-reviewed C++ library. Unit tests are linked with Boost unit test libraries. 
Lastly, git is a free and open source distributed version control system. Git must 
be installed to obtain the project source code from the OWA EPANET repository 
found on GitHub. 

### Summary of Build Dependencies
  - Platform Compiler
    - Windows: Visual Studio 10.0 32-bit cl (version 16.00.40219.01 for 80x86)
  - CMake (version 3.0.0 or greater)
  - Boost Libraries (version 1.58 or greater)
  - git (version 2.6.0 or greater)

### Build Procedure
1. Install Dependencies
  * A. Install Visual Studio 2010 Express and SP1
    Our current benchmark platform and compiler is Windows 32-bit Visual Studio 10 
    2010. Older versions of Visual Studio are available for download here:  

    https://www.visualstudio.com/vs/older-downloads/ 

    A service pack for Visual Studio 10 2010 is available here: 

    https://www.microsoft.com/en-us/download/details.aspx?id=34677

  * B. Install Boost 
    Boost binaries for Windows offer a convenient installation solution. Be sure to 
    select for download the boost installer exe that corresponds to the version of Visual Studio you have installed. 

    https://sourceforge.net/projects/boost/files/boost-binaries/1.58.0/

    Although newer version of Boost are available, a link to Boost 1.58 is provided. This is the library version that the unit tests have been written against. Older versions of Boost may not work. The default install location for the Boost 
    Libraries is C:\local\boost_1_58_0

  * C. Install Chocolatey, CMake, and git
    Chocolatey is a Windows package manager that makes installing some of these 
    dependencies a little easier. When working with Chocolatey it is useful to have 
    local admin privileges. Chocolatey is available here:

    https://chocolatey.org/install

    Once Chocolately is installed, from a command prompt running with admin privileges 
    issue the following commands
    ```
    \>choco install -y cmake --installargs 'ADD_CMAKE_TO_PATH=User'
    \>choco install -y git --installargs /GitOnlyOnPath
    \>refreshenv
    ```

  * D. Common Problems
    Using chocolatey requires a command prompt with admin privileges. 
    Check to make sure installed applications are on the command path. 
    Make note of the Boost Library install location. 


2. Build The Project
  As administrator open a Visual Studio 2010 Command Prompt. Change directories to 
  the location where you wish to build the EPANET project. Now we will issue a series 
  of commands to create a parent directory for the project root and clone the project
  from OWA's GitHub repository.  

  * A. Clone the EPANET Repository
    ```
    \>mkdir OWA
    \>cd OWA
    \>git clone --branch=dev https://github.com/OpenWaterAnalytics/EPANET.git
    \>cd EPANET
    ```
  The present working directory is now the project root EPANET. The directory contains 
  the same files that are visibly present in the GitHub Repo by browsing to the URL
  https://github.com/OpenWaterAnalytics/EPANET/tree/dev. 

  Now we will create a build products directory and generate the platform build 
  file using cmake.  

  * B. Generate the build files
    ```
    \>mkdir buildprod
    \>cd buildprod
    \>set BOOST_ROOT=C:\local\boost_1_58_0
    \>cmake -G "Visual Studio 10 2010" -DBOOST_ROOT="%BOOST_ROOT%" -DBoost_USE_STATIC_LIBS="ON" ..
    ```
  
  Now that the dependencies have been installed and the build system has been 
  generated, building EPANET is a simple CMake command. 

  * C. Build EPANET
    \>cmake --build . --config Debug
    
  * D. Common Problems
    CMake may not be able to find the project CMakeLists.txt file or the Boost 
    library install location. 


3. Testing
  Unit Testing uses Boost Unit Test library and CMake ctest as the test runner. 
  Cmake has been configured to register tests with ctest as part of the build process. 

  * A. Unit Testing
    ```
    \>cd tests
    \>ctest -C Debug
    ```
  The unit tests run quietly. Ctest redirects stdout to a log file which can be 
  found in the "tests\Testing\Temporary" folder. This is useful when a test fails.  

  Regression testing is somewhat more complicated because it relies on Python 
  to execute EPANET for each test and compare the binary files and report files. 
  To run regression tests first python and any required packages must be installed. 
  If Python is already installed on your local machine the installation of 
  miniconda can be skipped.  

  * B. Installing Regression Testing Dependencies
    ```
    cd ..\..
    \>choco install -y miniconda --installargs '/AddToPath=1'
    \>choco install -y curl
    \>choco install -y 7zip 
    \>refreshenv
    \>pip install -r tools/requirements-appveyor.txt
    ```

  With Python and the necessary dependencies installed, regression testing can be run 
  using the before-test and run-nrtest helper scripts found in the tools folder. The script 
  before-test stages the test and benchmark files for nrtest. The script run-nrtest calls 
  nrtest execute and nrtest compare to perform the regression test. 

  To run the executable under test, nrtest needs the absolute path to it and a 
  unique identifier for it such as the version number. The project cmake build places build 
  artifacts in the buildprod\bin\ folder. On Windows the build configuration "Debug" or 
  "Release" must also be indicated. On Windows it is also necessary to specify the path to 
  the Python Scripts folder so the nrtest execute and compare commands can be found. You 
  need to substitute bracketed fields below like "<build identifier>" with the values for 
  your setup. 

  * C. Regression Testing
    ```
    \>tools\before-test.cmd <relative path to regression test location> <absolute path to exe under test> <build identifier>
    \>tools\run-nrtest.cmd <absolute path to python scripts> <relative path to regression test location> <build identifier>
    ```

  * D. Common Problems
  The batch file before-test.cmd needs to run with admin privileges. The nrtest script complains when it can't find manifest files. 

That concludes this tutorial on building OWA EPANET from source on Windows. 
You have learned how to configure your machine satisfying project dependencies 
and how to acquire, build, and test EPANET on your local machine. To be sure, 
there is a lot more to learn, but this is a good start! Learn more about project
build and testing dependencies by following the links provided below. 

### Further Reading
  * Visual Studio - https://msdn.microsoft.com/en-us/library/dd831853(v=vs.100).aspx
  * CMake - https://cmake.org/documentation/
  * Boost - http://www.boost.org/doc/
  * git - https://git-scm.com/doc
  * Miniconda - https://conda.io/docs/user-guide/index.html
  * curl - https://curl.haxx.se/
  * 7zip - https://www.7-zip.org/
  * nrtest - https://nrtest.readthedocs.io/en/latest/
