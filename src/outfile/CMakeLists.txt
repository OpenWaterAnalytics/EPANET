#
# CMakeLists.txt - CMake configuration file for epanet-output library
#
# Created: March 9, 2018
# Author: Michael E. Tryby
#         US EPA ORD/NRMRL
#


# Sets for output directory for executables and libraries.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


# configure file groups
set(EPANET_OUT_SOURCES  src/epanet_output.c
                       ../util/errormanager.c
                       ../util/filemanager.c
                       ../util/cstr_helper.c)


# the binary output file API
add_library(epanet-output SHARED ${EPANET_OUT_SOURCES})
target_include_directories(epanet-output PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                                                ${CMAKE_CURRENT_SOURCE_DIR}/../)

include(GenerateExportHeader)
generate_export_header(epanet-output
  BASE_NAME epanet_output
  EXPORT_MACRO_NAME EXPORT_OUT_API
  EXPORT_FILE_NAME epanet_output_export.h
  STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC)

file(COPY ${CMAKE_CURRENT_BINARY_DIR}/epanet_output_export.h DESTINATION
  ${CMAKE_CURRENT_SOURCE_DIR}/include)
