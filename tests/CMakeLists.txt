#
# CMakeLists.txt - CMake configuration file for epanet/tests
#
# Created: February 13, 2018
#
# Modified by: Michael E. Tryby
#              US EPA ORD/NRMRL
#


# Sets for output directory for executables and libraries.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(UNIX)
    set(CMAKE_CXX_FLAGS "-std=c++11")
endif(UNIX)


add_executable(test_net_builder test_net_builder.cpp)
target_link_libraries(test_net_builder ${Boost_LIBRARIES} epanet2)
add_test(NAME test_net_builder
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_net_builder
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data)


set(toolkit_test_srcs
     test_toolkit.cpp
     test_project.cpp
     test_hydraulics.cpp
     test_hydraulic_characterization.cpp
     test_hydraulic_unit_equivalence.cpp
     test_quality.cpp
     test_report.cpp
     test_analysis.cpp
     test_node.cpp
     test_demand.cpp
     test_link.cpp
#     test_pump.cpp
     test_pattern.cpp
     test_curve.cpp
     test_control.cpp
     test_overflow.cpp
     test_pda.cpp
     test_valve.cpp
     test_units.cpp
     test_leakage.cpp
)

add_executable(test_toolkit ${toolkit_test_srcs})
target_link_libraries(test_toolkit ${Boost_LIBRARIES} epanet2)
target_compile_definitions(test_toolkit PRIVATE EPANET_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME test_toolkit
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_toolkit
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data)


# White-box hydraulic tests intentionally access internal solver structures and
# functions. Link them to a test-only static copy of the core so Windows tests
# do not depend on exporting private symbols from the epanet2 DLL.
file(GLOB epanet_internal_test_srcs
     ${CMAKE_SOURCE_DIR}/src/*.c
     ${CMAKE_SOURCE_DIR}/src/util/*.c)
# The white-box tests use the thread-safe EN_* API plus solver internals.
# src/epanet2.c only provides the legacy DLL wrapper API and, on Win32,
# carries __stdcall/import-export decorations that are inappropriate for this
# test-only static library.
list(REMOVE_ITEM epanet_internal_test_srcs ${CMAKE_SOURCE_DIR}/src/epanet2.c)
add_library(epanet2_internal_test STATIC ${epanet_internal_test_srcs})
target_include_directories(epanet2_internal_test PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(epanet2_internal_test PRIVATE epanet_py_EXPORTS)

add_custom_target(hydraulic_core_lint
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/hydraulic_core_lint.cmake
    VERBATIM)

set(hydraulic_internal_test_srcs
    test_toolkit.cpp
    test_hydraulic_performance_guardrails.cpp
    test_hydraulic_solver_scaling.cpp)
add_executable(test_hydraulic_internal ${hydraulic_internal_test_srcs})
target_link_libraries(test_hydraulic_internal ${Boost_LIBRARIES} epanet2_internal_test)
target_compile_definitions(test_hydraulic_internal PRIVATE
    EPANET_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
    epanet_py_EXPORTS)
add_dependencies(test_hydraulic_internal hydraulic_core_lint)
add_test(NAME test_hydraulic_internal
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_hydraulic_internal
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data)
set_tests_properties(test_toolkit test_hydraulic_internal PROPERTIES RUN_SERIAL TRUE)



add_executable(test_reent test_reent.cpp)

IF(MSVC)
    target_link_libraries(test_reent ${Boost_LIBRARIES} epanet2)
ELSE(TRUE)
    target_link_libraries(test_reent ${Boost_LIBRARIES} pthread epanet2)
ENDIF(MSVC)

add_test(NAME test_reent
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_reent
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data)


# ctest doesn't like tests added in subdirectories so adding them here
add_test(NAME test_errormanager
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_errormanager)

add_test(NAME test_filemanager
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_filemanager
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/outfile/data)

add_test(NAME test_output
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_output
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/outfile/data)
