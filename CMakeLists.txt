cmake_minimum_required(VERSION 3.16)
project(EdgeLab CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Add test_program executable
add_executable(test_program test/test.cpp)

# Add verify_undirected executable
add_executable(verify_undirected test/verify_undirected.cpp)

# Optionally, set warnings
foreach(target test_program verify_undirected)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
endforeach() 