## cmake .
## make

cmake_minimum_required (VERSION 2.6)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

project (EPANET)

# the library
include_directories(../../include)
file(GLOB EPANET_SOURCES ../../src/*.c)
add_library(epanet STATIC  ${EPANET_SOURCES})

# the standalone executable
include_directories(../../src)
add_executable(runepanet ../../run/main.c)
target_link_libraries (runepanet LINK_PUBLIC epanet m)

# the binary hydraulics file API
include_directories(../../tools/outputapi)
add_library(epanet_bin_out STATIC ../../tools/outputapi/outputapi.c)
