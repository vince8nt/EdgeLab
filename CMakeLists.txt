cmake_minimum_required(VERSION 3.16)
project(EdgeLab CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Add example executables
add_executable(generate_and_print examples/generate_and_print.cpp)
add_executable(tc examples/triangle_counting.cpp)
add_executable(bfs examples/breadth_first_search.cpp)

# Add test executables
add_executable(verify_undirected test/verify_undirected.cpp)
add_executable(iteration test/iteration.cpp)

# Optionally, set warnings
foreach(target generate_and_print verify_undirected iteration)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
endforeach() 