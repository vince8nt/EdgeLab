cmake_minimum_required(VERSION 3.16)
project(EdgeLab CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find pthreads
find_package(Threads REQUIRED)

# Add include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Add example executables
add_executable(generate_and_print examples/generate_and_print.cpp)
add_executable(tc examples/triangle_counting.cpp)
add_executable(bfs examples/breadth_first_search.cpp)

# Add threaded example executables
add_executable(bfs_threaded examples_threaded/breadth_first_search.cpp)
target_link_libraries(bfs_threaded Threads::Threads)

# Add test executables
add_executable(verify_undirected test/verify_undirected.cpp)
add_executable(iteration test/iteration.cpp)
add_executable(packing test/vertex_edge_packing.cpp)

# Optionally, set warnings
foreach(target generate_and_print verify_undirected iteration bfs_threaded)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
endforeach() 