


# Sets for output directory for executables and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


set (test_source
./test_errormanager.cpp
../../src/util/errormanager.c
)

add_executable(test_errormanager ${test_source})
target_include_directories(test_errormanager PUBLIC ../../src/)
target_link_libraries(test_errormanager ${Boost_LIBRARIES})

add_test(NAME test_errormanager
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_errormanager)
