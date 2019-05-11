


# Sets for output directory for executables and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


add_executable(test_cstrhelper ./test_cstrhelper.cpp
                               ../../src/util/cstr_helper.c)
target_include_directories(test_cstrhelper PUBLIC ../../src/)
target_link_libraries(test_cstrhelper ${Boost_LIBRARIES})


add_executable(test_errormanager ./test_errormanager.cpp
                                 ../../src/util/errormanager.c)
target_include_directories(test_errormanager PUBLIC ../../src/)
target_link_libraries(test_errormanager ${Boost_LIBRARIES})


add_executable(test_filemanager ./test_filemanager.cpp
                                ../../src/util/filemanager.c
                                ../../src/util/cstr_helper.c)
target_include_directories(test_filemanager PUBLIC ../../src/)
target_link_libraries(test_filemanager ${Boost_LIBRARIES})

