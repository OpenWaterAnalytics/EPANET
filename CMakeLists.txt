# CMakeLists.txt - CMake configuration file for EPANET 2.0 
# 
# CMake is a cross-platform build tool. CMake generates platform native 
# makefiles that can be used with your compiler of choice. CMake uses a 
# generator concept to represent different build tooling. CMake automatically 
# detects the platform it is running on and generates the appropriate makefiles
# for the platform default compiler. Different generators can also be specified. 
# 
# Note: CMake requires that your platform build system and compiler are 
# properly installed. Build using Visual Studio requires msbuild shell.  
#  
# Example Usage: 
#   cd build/cmake
#   mkdir buildproducts
#   cd buildproducts
#   cmake ..
#   make 
#    
# Building MSYS on Windows:
#   ...
#   cmake -G "MSYS Makefiles" ..
#   make 
# 
# Building Visual Studio on Windows: 
#   ...
#   cmake -G "Visual Studio 10 2010" ..
#   msbuild /p:Configuration=Release ALL_BUILD.vcxproj
#
# Generic Invocation:
#   cmake -E make_directory buildprod
#   cd build
#   cmake -G GENERATOR -DCMAKE_BUILD_TYPE=Release ..
#   cmake --build . --target SOME_TARGET --config Release
#
# More information:
#   cmake --help
# 
#   CMake is available at https://cmake.org/download/
# 

cmake_minimum_required (VERSION 2.8.8)

project(EPANET)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_POSITION_INDEPENDENT_CODE ON)

IF (APPLE)
  SET(CMAKE_INSTALL_NAME_DIR @executable_path)
  SET(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
ENDIF (APPLE)

IF (MSVC)
  set(CMAKE_C_FLAGS_RELEASE "/GL")
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
ENDIF (MSVC)

set(EPANET_API_HEADER "include/epanet2.h")

# create object library for reuse in other targets
include_directories(include; src)
file(GLOB EPANET_SOURCES src/*.c)
add_library(epanet_objs OBJECT ${EPANET_SOURCES})

# the shared library
add_library(epanet SHARED  $<TARGET_OBJECTS:epanet_objs> ${EPANET_API_HEADER})

# the standalone executable
add_executable(runepanet run/main.c $<TARGET_OBJECTS:epanet_objs>)
if (NOT MSVC)
    target_link_libraries (runepanet m)    
endif (NOT MSVC)
