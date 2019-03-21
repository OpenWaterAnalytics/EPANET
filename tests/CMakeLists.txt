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
ENDIF(MSVC)

set(Boost_THREAD_FOUND              OFF)
find_package(Boost COMPONENTS unit_test_framework filesystem)
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
     test_control.cpp)

add_executable(test_toolkit ${toolkit_test_srcs})

target_link_libraries(test_toolkit ${Boost_LIBRARIES} epanet2)



#I like to keep test files in a separate source directory called test
#file(GLOB TEST_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} test_*.cpp)

#add_library(shared_test OBJECT shared_test.cpp)

#Run through each source
#foreach(testSrc ${TEST_SRCS})
#    #Extract the filename without an extension (NAME_WE)
#    get_filename_component(testName ${testSrc} NAME_WE)
#
#    #Add compile target
#    IF(${testName} MATCHES "test_reent")
#        add_executable(${testName} ${testSrc})
#    ELSE (TRUE)
#        add_executable(${testName} ${testSrc} $<TARGET_OBJECTS:shared_test>)
#    ENDIF(${testName} MATCHES "test_reent")
#
#    #link to Boost libraries AND your targets and dependencies
#    IF(MSVC)
#        target_link_libraries(${testName} ${Boost_LIBRARIES} epanet2 epanet-output)
#    ELSE(TRUE)
#        target_link_libraries(${testName} ${Boost_LIBRARIES} pthread epanet2 epanet-output)
#    ENDIF(MSVC)
#
#    #Finally add it to test execution
#    #Notice the WORKING_DIRECTORY and COMMAND
#    add_test(NAME ${testName}
#        COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${testName}
#        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data)
#endforeach(testSrc)
