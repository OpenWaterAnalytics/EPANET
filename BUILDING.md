The most straightforward way to build the EPANET files is by using `CMake` ([https://cmake.org/](https://cmake.org/)). `CMake` is a cross-platform build tool. `CMake` generates platform native build systems that can be used with your compiler of choice. `CMake` uses a generator concept to represent different build tooling. `CMake` automatically detects the platform it is running on and generates the appropriate makefiles for the platform default compiler. Different generators can also be specified.

The project's `CMake` file (`CMakeLists.txt`) is located in its root directory and supports builds for Linux, Mac OS and Windows. To build the EPANET library and its command line executable using `CMake`, first open a console window and navigate to the project's root directory. Then enter the following commands:
```
mkdir build
cd build
cmake ..
cmake --build . --config Release
```
Note: Use `cmake -G "Visual Studio 15 2017 Win64" ..` as the third command for a 64-bit build on Windows.

Under Windows the resulting EPANET toolkit library `epanet2.dll` and its command line executable `runepanet.exe` will be in the `build\bin\Release` directory. The `build\lib\Release` directory contains an `epanet2.lib` file which is needed to build C/C++ applications using the Windows version of the library. For Linux and Mac OS the EPANET toolkit shared library `libepanet2.so` appears in the `build/lib` directory and the command line executable `runepanet` is in the `build/bin` directory. 

In addition, two Windows one-click-build scripts are included in the `win_build` directory:
1. `Makefile2.bat`: this script uses the `CMake` file and requires the build tools for Visual Studio available from [https://visualstudio.microsoft.com/downloads/](https://visualstudio.microsoft.com/downloads/). The Community version will work just fine. This script was tested with Visual Studio 2017 and 2019.
2. `Makefile.bat`: this is the legacy build script compatible with Visual Studio 2010 which conforms with the C89 Standard which was the standard EPANET supported from earlier versions. This script requires the installation of Microsoft Windows SDK 7.1 ([https://www.microsoft.com/en-us/download/details.aspx?id=8279](https://www.microsoft.com/en-us/download/details.aspx?id=8279)).
 
Both scripts will build the EPANET library and the command line executable for the 32 and 64 bit Windows platforms, placing them in the `win_build\32bit` and `win_build\64bit` directories, respectively.