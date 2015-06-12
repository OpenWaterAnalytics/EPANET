## cmake .
## make

cmake_minimum_required (VERSION 2.6)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

project (EPANET)

# the library
include_directories(../../include)
add_library(epanet STATIC  ../../src/epanet.c ../../src/hydraul.c ../../src/hash.c ../../src/inpfile.c ../../src/input1.c ../../src/input2.c ../../src/input3.c ../../src/mempool.c ../../src/output.c ../../src/quality.c ../../src/report.c ../../src/rules.c ../../src/smatrix.c)

# the standalone executable
include_directories(../../src)
add_executable(runepanet ../../run/main.c)
target_link_libraries (runepanet LINK_PUBLIC epanet)
