#
# CMakeLists.txt - CMake configuration file for epanet/tests
#
# Created: February 13, 2018
# Author:  Constantin Savtchenko
# Ref: http://neyasystems.com/an-engineers-guide-to-unit-testing-cmake-and-boost-unit-tests/
#
# Modified by: Michael E. Tryby
#              US EPA ORD/NRMRL
#

#Setup CMake to run tests
enable_testing()

# Sets for output directory for executables and libraries.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(UNIX)
    set(CMAKE_CXX_FLAGS "-std=c++11")
endif(UNIX)

#Prep ourselves for compiling boost
IF(MSVC)
    set(Boost_DEBUG                 OFF)
    set(Boost_DETAILED_FAILURE_MSG  OFF)
    set(Boost_USE_STATIC_LIBS       OFF)
    add_definitions(-DBOOST_ALL_DYN_LINK)
ENDIF(MSVC)

set(Boost_THREAD_FOUND              OFF)
find_package(Boost COMPONENTS unit_test_framework system thread filesystem)
include_directories (${Boost_INCLUDE_DIRS})


set(toolkit_test_srcs
     test_toolkit.cpp
     test_project.cpp
     test_hydraulics.cpp
     test_quality.cpp
     test_report.cpp
     test_analysis.cpp
     test_node.cpp
     test_demand.cpp
     test_link.cpp
#     test_pump.cpp
     test_pattern.cpp
#     test_curve.cpp
     test_control.cpp
     test_net_builder.cpp)

add_executable(test_toolkit ${toolkit_test_srcs})

target_link_libraries(test_toolkit ${Boost_LIBRARIES} epanet2)

add_test(NAME test_toolkit
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_toolkit
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data)



add_executable(test_reent test_reent.cpp)

IF(MSVC)
    target_link_libraries(test_reent ${Boost_LIBRARIES} epanet2)
ELSE(TRUE)
    target_link_libraries(test_reent ${Boost_LIBRARIES} pthread epanet2)
ENDIF(MSVC)

add_test(NAME test_reent
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_reent
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data)
