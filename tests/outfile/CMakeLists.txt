


enable_testing()

# Sets for output directory for executables and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


set (Boost_USE_STATIC_LIBS OFF)
find_package(Boost COMPONENTS unit_test_framework)
include_directories (${Boost_INCLUDE_DIRS} ../../src/outfile/include)


add_executable(test_errormanager test_output.cpp)

target_link_libraries(test_errormanager ${Boost_LIBRARIES} epanet-output)
