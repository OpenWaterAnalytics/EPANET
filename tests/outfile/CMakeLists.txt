


enable_testing()

# Sets for output directory for executables and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


if (MSVC)
    set (Boost_USE_STATIC_LIBS OFF)
    add_definitions(-DBOOST_ALL_DYN_LINK)
endif(MSVC)

find_package(Boost COMPONENTS unit_test_framework)
include_directories (${Boost_INCLUDE_DIRS} ../../src/outfile/include)


add_executable(test_output test_output.cpp)

target_link_libraries(test_output ${Boost_LIBRARIES} epanet-output)

add_test(NAME test_output
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_output
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data)
