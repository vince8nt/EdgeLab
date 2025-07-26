cmake_minimum_required(VERSION 3.10)
project(EdgeLab)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCL (optional)
find_package(OpenCL QUIET)

# Add OpenCL include directories if found
if(OpenCL_FOUND)
    include_directories(${OpenCL_INCLUDE_DIRS})
    add_definitions(-DOPENCL_AVAILABLE)
endif()

# Find pthreads
find_package(Threads REQUIRED)

# Add include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Add example executables
add_executable(generate_and_print examples/generate_and_print.cpp src/util.cpp)
add_executable(tc examples/triangle_counting.cpp src/util.cpp)
add_executable(bfs examples/breadth_first_search.cpp src/util.cpp)

# Add threaded example executables
add_executable(tc_threaded examples_threaded/triangle_counting.cpp src/util.cpp)
target_link_libraries(tc_threaded Threads::Threads)
add_executable(bfs_threaded examples_threaded/breadth_first_search.cpp src/util.cpp)
target_link_libraries(bfs_threaded Threads::Threads)

# Add executable for OpenCL examples (only if OpenCL is available)
# Allow ignored attributes for OpenCL headers
if(OpenCL_FOUND)
    add_executable(bfs_opencl examples_opencl/breadth_first_search_opencl.cpp src/opencl_wrapper.cpp src/util.cpp)
    target_link_libraries(bfs_opencl ${OpenCL_LIBRARIES} Threads::Threads)
    
    add_executable(tc_opencl examples_opencl/triangle_counting_opencl.cpp src/opencl_wrapper.cpp src/util.cpp)
    target_link_libraries(tc_opencl ${OpenCL_LIBRARIES} Threads::Threads)
endif()

# Add test executables
add_executable(verify_undirected test/verify_undirected.cpp src/util.cpp)
add_executable(iteration test/iteration.cpp src/util.cpp)
add_executable(packing test/vertex_edge_packing.cpp src/util.cpp)

# Optionally, set warnings
set(WARNING_TARGETS generate_and_print verify_undirected iteration bfs_threaded)
if(OpenCL_FOUND)
    list(APPEND WARNING_TARGETS bfs_opencl tc_opencl)
endif()
foreach(target ${WARNING_TARGETS})
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wno-ignored-attributes)
endforeach() 