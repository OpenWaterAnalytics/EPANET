


# Sets for output directory for executables and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


add_executable(test_output test_output.cpp)
target_include_directories(test_output PUBLIC ../../src/outfile/include)
target_link_libraries(test_output ${Boost_LIBRARIES} epanet-output)
