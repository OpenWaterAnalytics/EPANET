# Building

The most straightforward way to build the EPANET files is by using `CMake` ([https://cmake.org/](https://cmake.org/)). `CMake` is a cross-platform build tool that generates platform native build systems that can be used with your compiler of choice. It uses a generator concept to represent different build tooling. `CMake` automatically detects the platform it is running on and generates the appropriate makefiles for the platform default compiler. Different generators can also be specified.

The project's `CMake` file (`CMakeLists.txt`) is located in its root directory and supports builds for Linux, Mac OS and Windows. To build the EPANET library and its command line executable using `CMake`, first open a console window and navigate to the project's root directory. Then enter the following commands:

```
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

Note: under Windows, the third command should be `cmake .. -A Win32` for a 32-bit build or `cmake .. -A x64` for a 64-bit build when Microsoft Visual Studio is the default compiler.

For Windows the resulting EPANET toolkit library `epanet2.dll` and its command line executable `runepanet.exe` are placed in the `build\bin\Release` directory. The `build\lib\Release` directory will contain an `epanet2.lib` file which is needed to build C/C++ applications using the Windows version of the library. For Linux and Mac OS the EPANET toolkit shared library `libepanet2.so` appears in the `build/lib` directory and the command line executable `runepanet` is in the `build/bin` directory.

In addition, two Windows one-click-build scripts are included in the `win_build` directory:

1. `Makefile2.bat`: this script uses the `CMake` file and requires the build tools for Visual Studio available from [https://visualstudio.microsoft.com/downloads/](https://visualstudio.microsoft.com/downloads/). The Community version will work just fine. This script was tested with Visual Studio 2017 and 2019.
2. `Makefile.bat`: this is the legacy build script compatible with Visual Studio 2010 which conforms with the C89 Standard which was the standard EPANET supported from earlier versions. This script requires the installation of Microsoft Windows SDK 7.1 ([https://www.microsoft.com/en-us/download/details.aspx?id=8279](https://www.microsoft.com/en-us/download/details.aspx?id=8279)) and will probably not run correctly on later versions of the SDK. `CMake` is not used in this script.

These two scripts build EPANET binaries for both the 32 and 64 bit Windows platforms, placing them in the `win_build\32bit` and `win_build\64bit` directories, respectively.

A tutorial on [building OWA EPANET from source on Windows](tools/BuildAndTest.md), including running unit tests and performing regression testing, is also avaiable.

## Alternative build with Conan
Conan is an increasingly popular C/C++ package management suite. To build EPANET using Conan, use the following commands as a starting point:

```
conan build . -s build_type=Release
conan export-pkg . -s build_type=Release
```


# Testing

Unit tests have been written using the Boost Unit Testing Framework and other Boost libraries. The tests are compiled into individual executables that automatically perform checks on the EPANET toolkit and output libraries.

The CMake build system has been configured with a build option for building tests. When enabled (`-DBUILD_TESTS=ON`) the test executables are built and registered with the CTest test runner, the default value for the test build option is off. The location of Boost can also be defined with `-DBOOST_ROOT="%BOOST_ROOT%"` if required.

To build the test executables for the EPANET library, first open a console window and navigate to the project's root directory. Then enter the following commands:

```
mkdir build
cd build
cmake -DBUILD_TESTS=ON ..
cmake --build . --config Release
cd tests
ctest -C Release --output-on-failure
```
