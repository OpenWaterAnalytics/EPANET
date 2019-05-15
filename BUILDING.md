Building the EPANET library and executable files
============================
The most straightforward way to build the EPANET files is by using CMake (https://cmake.org/). CMake is a cross-platform build tool. CMake generates platform native build systems that can be used with your compiler of choice. CMake uses a generator concept to represent different build tooling. CMake automatically detects the platform it is running on and generates the appropriate makefiles for the platform default compiler. Different generators can also be specified.

The project's CMake file (CMakeLists.txt) is located in the root directory and supports builds for Linux, Max and Windows.

In addition, two Windows one-click-build scripts are included in the win_build/WinSDK directory:

**Makefile2.bat:** this scripts uses the CMake file and requires the build tools for Visual Studio available from https://visualstudio.microsoft.com/downloads/. The Community version will work just fine. This script was tested with Visual Studio 2017 and 2019.

**Makefile.bat:** this is the legacy build script compatible with Visual Studio 2010 which conforms with the C89 Standard which was the standard EPANET supported from earlier versions. This script requires the installation of Microsoft Windows SDK 7.1 (https://www.microsoft.com/en-us/download/details.aspx?id=8279).
Both scripts will build the EPANET library (DLL) and the executable files for 32 and 64 bits Windows platforms.