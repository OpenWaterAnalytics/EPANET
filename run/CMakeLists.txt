# EPANET COMMAND LINE EXECUTABLE
cmake_minimum_required (VERSION 2.8.8)


# Sets for output directory for executables and libraries.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Sets the position independent code property for all targets.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


# Set up file groups for exe target
set(EPANET_CLI_SOURCES main.c)
include_directories(include)

source_group("CLI" FILES ${EPANET_CLI_SOURCES})


# Creates the EPANET command line executable
add_executable(runepanet ${EPANET_CLI_SOURCES})
if(NOT WIN32)
    target_link_libraries(runepanet LINK_PUBLIC epanet2 m)
else(NOT WIN32)
    target_link_libraries(runepanet LINK_PUBLIC epanet2)
endif(NOT WIN32)
