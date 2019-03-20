

cmake_minimum_required(VERSION 3.12)

enable_testing()

set (Boost_USE_STATIC_LIBS OFF)
find_package(Boost COMPONENTS unit_test_framework)
include_directories (${Boost_INCLUDE_DIRS} ../../src/)

set (test_source
./test_errormanager.cpp
../../src/util/errormanager.c
)

add_executable(test_errormanager ${test_source})

target_link_libraries(test_errormanager ${Boost_LIBRARIES})
