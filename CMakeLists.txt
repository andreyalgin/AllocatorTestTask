cmake_minimum_required(VERSION 3.22)
project(AllocatorTest)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(googletest)
add_subdirectory(allocator)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable( AllocatorTest
                main.cpp

                tests/AllocatorTest.cpp)

target_include_directories(AllocatorTest PUBLIC allocator_SOURCE_DIR)
target_include_directories(AllocatorTest PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(AllocatorTest PUBLIC gtest gtest_main ${ALLOCATOR_TARGET})
