cmake_minimum_required(VERSION 3.22)
project(Allocator)

set(CMAKE_CXX_STANDARD 17)
set(ALLOCATOR_TARGET Allocator_target)

add_library(${ALLOCATOR_TARGET} STATIC Allocator.cpp Allocator.hpp)

