include(CheckCXXCompilerFlag)

# Check for UBSAN support
set(CMAKE_REQUIRED_FLAGS "-fsanitize=undefined")
set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=undefined")
check_cxx_source_compiles("int main() { return 0; }" SANITIZERS_SUPPORTED)
unset(CMAKE_REQUIRED_FLAGS)
unset(CMAKE_REQUIRED_LINK_OPTIONS)

if(UNIX)
    set(CMAKE_CXX_FLAGS "-std=c++11")
endif(UNIX)

# Sets for output directory for executables and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


add_executable(test_cstrhelper ./test_cstrhelper.cpp
                               ../../src/util/cstr_helper.c)
target_include_directories(test_cstrhelper PUBLIC ../../src/)
target_link_libraries(test_cstrhelper ${Boost_LIBRARIES})
if(SANITIZERS_SUPPORTED AND SANITIZERS_ENABLED)
    target_compile_options(test_cstrhelper PUBLIC -fsanitize=undefined)
    target_link_options(test_cstrhelper PUBLIC -fsanitize=undefined)
endif()


add_executable(test_errormanager ./test_errormanager.cpp
                                 ../../src/util/errormanager.c)
target_include_directories(test_errormanager PUBLIC ../../src/)
target_link_libraries(test_errormanager ${Boost_LIBRARIES})
if(SANITIZERS_SUPPORTED AND SANITIZERS_ENABLED)
    target_compile_options(test_errormanager PUBLIC -fsanitize=undefined)
    target_link_options(test_errormanager PUBLIC -fsanitize=undefined)
endif()

add_executable(test_filemanager ./test_filemanager.cpp
                                ../../src/util/filemanager.c
                                ../../src/util/cstr_helper.c)
target_include_directories(test_filemanager PUBLIC ../../src/)
target_link_libraries(test_filemanager ${Boost_LIBRARIES})
if(SANITIZERS_SUPPORTED AND SANITIZERS_ENABLED)
    target_compile_options(test_filemanager PUBLIC -fsanitize=undefined)
    target_link_options(test_filemanager PUBLIC -fsanitize=undefined)
endif()
