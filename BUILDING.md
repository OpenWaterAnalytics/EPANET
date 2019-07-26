### How to Build OWA-EPANET

The most straightforward way to build the OWA-EPANET project is by using `CMake` ([https://cmake.org/](https://cmake.org/)), a cross-platform build tool that generates platform native build systems that can be used with your compiler of choice. `CMake` uses a generator concept to represent different build tooling. It automatically detects the platform it is running on and generates the appropriate makefiles for the platform's default compiler. Different generators can also be specified.

The project's `CMake` configuration file (`CMakeLists.txt`) is located in its root directory and supports builds for Linux, Mac OS and Windows. To build the EPANET library and its command line executable using `CMake`, first open a command prompt console window and navigate to the project's root directory. Then enter the following commands:
```
mkdir build
cd build
cmake ..
cmake --build . --config Release
```
Note: Use `cmake -G "Visual Studio 15 2017 Win64" ..` as the third command for a 64-bit build on Windows.

Under Windows the resulting EPANET toolkit library `epanet2.dll` and its command line executable `runepanet.exe` will be in the `build\bin\Release` directory. The `build\lib\Release` directory contains an `epanet2.lib` file which is needed to build C/C++ applications using the Windows version of the library. For Linux and Mac OS the EPANET toolkit shared library `libepanet2.so` appears in the `build/lib` directory and the command line executable `runepanet` is in the `build/bin` directory. 

As an alternative to using `CMake` for a Windows build, two one-click-build scripts are included in the `win_build` directory:
1. `Makefile2.bat`: this script requires both `CMake` and the build tools for Visual Studio (available from [https://visualstudio.microsoft.com/downloads/](https://visualstudio.microsoft.com/downloads/) be installed on your machine. It can be executed from any command prompt window. The Community version of Visual Studio will work just fine. This script was tested with both Visual Studio 2017 and 2019.
2. `Makefile.bat`: this build script uses only Visual Studio commands and does not require `CMake`. It is  compatible with both Visual Studio 2010 and 2017 and should be executed from a Visual Studio x86 Native Tools command prompt window.
 
Both scripts will build the EPANET library and the command line executable for the 32 and 64 bit Windows platforms, placing them in the `win_build\32bit` and `win_build\64bit` directories, respectively.