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

cmake_minimum_required (VERSION 3.5.2)

project(EPANET)

# Append local dir to module search path
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
option(BUILD_TESTS "Build tests (requires Boost)" OFF)
option(BUILD_PY_LIB "Build library for Python wrapper" OFF)
option(BUILD_COVERAGE "Build library for coverage" OFF)

IF (NOT BUILD_PY_LIB)
  add_subdirectory(run)
ENDIF (NOT BUILD_PY_LIB)
add_subdirectory(src/outfile)

IF (BUILD_TESTS)
    #Prep ourselves for compiling with boost
    IF(WIN32)
        set(Boost_USE_STATIC_LIBS       ON)
    ELSE(TRUE)
        set(Boost_USE_STATIC_LIBS       OFF)
        add_definitions(-DBOOST_ALL_DYN_LINK)
    ENDIF(WIN32)

    find_package(Boost COMPONENTS unit_test_framework system thread filesystem)
    include_directories (${Boost_INCLUDE_DIRS})

    enable_testing()
    add_subdirectory(tests)
    add_subdirectory(tests/outfile)
    add_subdirectory(tests/util)
ENDIF (BUILD_TESTS)


# Sets the output directory for executables and libraries.
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
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE -MT)
ENDIF (MSVC)

# configure file groups
file(GLOB EPANET_SOURCES RELATIVE ${PROJECT_SOURCE_DIR} src/*.c src/util/*.c)
file(GLOB EPANET_LIB_ALL RELATIVE ${PROJECT_SOURCE_DIR} src/* src/util/*)
# exclude epanet python API from the default build
list(REMOVE_ITEM EPANET_LIB_ALL "src/epanet_py.c")
source_group("Library" FILES ${EPANET_LIB_ALL})

# the shared library
IF(MSVC AND "${CMAKE_VS_PLATFORM_NAME}" MATCHES "(Win32)")
  message("   ************************************")
  message("   Configuring with epanet2.def mapping")
  message("   ************************************")
  add_library(epanet2 SHARED ${EPANET_LIB_ALL} ${PROJECT_SOURCE_DIR}/include/epanet2.def)
  set_source_files_properties(${PROJECT_SOURCE_DIR}/include/epanet2.def PROPERTIES_HEADER_FILE_ONLY TRUE)
ELSE(TRUE)
  add_library(epanet2 ${EPANET_LIB_ALL})
ENDIF(MSVC AND "${CMAKE_VS_PLATFORM_NAME}" MATCHES "(Win32)")

target_include_directories(epanet2 PUBLIC ${PROJECT_SOURCE_DIR}/include)

install(TARGETS epanet2 DESTINATION lib)
install(FILES ./include/epanet2.h DESTINATION include)
install(FILES ./include/epanet2_2.h DESTINATION include)
install(FILES ./include/epanet2_enums.h DESTINATION include)
install(DIRECTORY ./src/ DESTINATION include FILES_MATCHING PATTERN "*.h")


