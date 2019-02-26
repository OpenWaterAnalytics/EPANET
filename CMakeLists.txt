# CMakeLists.txt - CMake configuration file for EPANET 2.0
#
# CMake is a cross-platform build tool. CMake generates platform native
# build systems that can be used with your compiler of choice. CMake uses a
# generator concept to represent different build tooling. CMake automatically
# detects the platform it is running on and generates the appropriate makefiles
# for the platform default compiler. Different generators can also be specified.
#
# Note: CMake requires that your platform build system and compiler are
# properly installed. Build using Visual Studio requires msbuild shell.
#
# Build Options:
#   BUILD_TESTS = ON/OFF
#   BUILD_PY_LIB = ON/OFF
#
# Generic Invocation:
#   cmake -E make_directory buildprod
#   cd build
#   cmake -G GENERATOR -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON ..
#   cmake --build . --target SOME_TARGET --config Release
#
# More information:
#   cmake --help
#
#   CMake is available at https://cmake.org/download/
#

cmake_minimum_required (VERSION 2.8.8)

project(EPANET)

# Append local dir to module search path
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)


option(BUILD_TESTS "Build unit tests (requires Boost test)" OFF)
option(BUILD_PY_LIB "Build library for Python wrapper" OFF)
option(BUILD_COVERAGE "Build library for coverage" OFF)


IF (NOT BUILD_PY_LIB)
  add_subdirectory(run)
ENDIF (NOT BUILD_PY_LIB)
add_subdirectory(tools/epanet-output)

IF (BUILD_TESTS)
  add_subdirectory(tests)
ENDIF (BUILD_TESTS)


# Sets for output directory for executables and libraries.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


# Sets the position independent code property for all targets
SET(CMAKE_POSITION_INDEPENDENT_CODE ON)


IF (APPLE)
  set(INSTALL_NAME_DIR @executable_path/../lib)
  set(CMAKE_MACOSX_RPATH 1)
ENDIF (APPLE)

IF (MSVC)
  set(CMAKE_C_FLAGS_RELEASE "/GL")
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
ENDIF (MSVC)


# configure file groups
file(GLOB EPANET_SOURCES RELATIVE ${PROJECT_SOURCE_DIR} src/*.c)
file(GLOB EPANET_LIB_ALL RELATIVE ${PROJECT_SOURCE_DIR} src/*)
# exclude epanet python API from the default build
list(REMOVE_ITEM EPANET_LIB_ALL "src/epanet_py.c")
source_group("Library" FILES ${EPANET_LIB_ALL})


# create build target for epanet library with python API
IF (BUILD_PY_LIB)

  # exclude legacy epanet 2.0 API and include epanet py API
  list(REMOVE_ITEM EPANET_LIB_ALL "src/epanet2.c")
  add_library(epanet_py SHARED ${EPANET_LIB_ALL} src/epanet_py.c src/util/errormanager.c)
  target_include_directories(epanet_py PUBLIC ${PROJECT_SOURCE_DIR}/include)

  include(GenerateExportHeader)
  GENERATE_EXPORT_HEADER(epanet_py
    BASE_NAME epanet_py
    EXPORT_MACRO_NAME EXPORT_PY_API
    EXPORT_FILE_NAME epanet_py_export.h
    STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC)

  file(COPY ${CMAKE_CURRENT_BINARY_DIR}/epanet_py_export.h
    DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/include)


# create build target for code coverage
ELSEIF (BUILD_COVERAGE)

include(CodeCoverage)
set(CMAKE_BUILD_TYPE "Debug")
APPEND_COVERAGE_COMPILER_FLAGS()
set(CMAKE_C_FLAGS_DEBUG "-O0")

add_library(epanet2 SHARED ${EPANET_LIB_ALL})
target_include_directories(epanet2 PUBLIC ${PROJECT_SOURCE_DIR}/include)


# create build target for default epanet library with 2.0 and 2.2 API
ELSE (BUILD_PY_LIB)

  # the shared library
  IF("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)" OR NOT MSVC)
    add_library(epanet2 SHARED ${EPANET_LIB_ALL})
  ELSE(TRUE)
    add_library(epanet2 SHARED ${EPANET_LIB_ALL} ${PROJECT_SOURCE_DIR}/include/epanet2.def)
    set_source_files_properties(${PROJECT_SOURCE_DIR}/include/epanet2.def PROPERTIES_HEADER_FILE_ONLY TRUE)
  ENDIF("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)" OR NOT MSVC)

  target_include_directories(epanet2 PUBLIC ${PROJECT_SOURCE_DIR}/include)

ENDIF (BUILD_PY_LIB)
