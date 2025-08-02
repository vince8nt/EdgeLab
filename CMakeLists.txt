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

# Enable precompiled headers
set(CMAKE_PCH_ENABLED ON)

# Add example executables
add_executable(tc examples/triangle_counting.cpp)
target_precompile_headers(tc PRIVATE src/graph_pch.h)
add_executable(bfs examples/breadth_first_search.cpp)
target_precompile_headers(bfs PRIVATE src/graph_pch.h)

# Add threaded example executables
add_executable(tc_threaded examples_threaded/triangle_counting.cpp)
target_precompile_headers(tc_threaded PRIVATE src/graph_pch.h)
target_link_libraries(tc_threaded Threads::Threads)
add_executable(bfs_threaded examples_threaded/breadth_first_search.cpp)
target_precompile_headers(bfs_threaded PRIVATE src/graph_pch.h)
target_link_libraries(bfs_threaded Threads::Threads)

# Add executable for OpenCL examples (only if OpenCL is available)
# Allow ignored attributes for OpenCL headers
if(OpenCL_FOUND)
    add_executable(bfs_opencl examples_opencl/breadth_first_search.cpp)
    target_precompile_headers(bfs_opencl PRIVATE src/graph_pch.h)
    target_link_libraries(bfs_opencl ${OpenCL_LIBRARIES} Threads::Threads)
    
    add_executable(tc_opencl examples_opencl/triangle_counting.cpp)
    target_precompile_headers(tc_opencl PRIVATE src/graph_pch.h)
    target_link_libraries(tc_opencl ${OpenCL_LIBRARIES} Threads::Threads)
endif()

# Add test executables
add_executable(generate_and_print test/generate_and_print.cpp)
target_precompile_headers(generate_and_print PRIVATE src/graph_pch.h)
add_executable(convert src/converter.cpp)
target_precompile_headers(convert PRIVATE src/graph_pch.h)
add_executable(print test/print.cpp)
target_precompile_headers(print PRIVATE src/graph_pch.h)
add_executable(validate_graph test/validate_graph.cpp)
target_precompile_headers(validate_graph PRIVATE src/graph_pch.h)
add_executable(iteration test/iteration.cpp)
target_precompile_headers(iteration PRIVATE src/graph_pch.h)
add_executable(packing test/vertex_edge_packing.cpp)
target_precompile_headers(packing PRIVATE src/graph_pch.h)
add_executable(test_load_save test/test_load_save.cpp)
target_precompile_headers(test_load_save PRIVATE src/graph_pch.h)

add_executable(triangle_counting_test test/triangle_counting_test.cpp)
target_precompile_headers(triangle_counting_test PRIVATE src/graph_pch.h)
target_link_libraries(triangle_counting_test Threads::Threads)
if(OpenCL_FOUND)
    target_link_libraries(triangle_counting_test ${OpenCL_LIBRARIES})
endif()

add_executable(breadth_first_search_test test/breadth_first_search_test.cpp)
target_precompile_headers(breadth_first_search_test PRIVATE src/graph_pch.h)
target_link_libraries(breadth_first_search_test Threads::Threads)
if(OpenCL_FOUND)
    target_link_libraries(breadth_first_search_test ${OpenCL_LIBRARIES})
endif()

# Add benchmark executables
add_executable(iteration_benchmark benchmark/iteration_benchmark.cpp)
target_precompile_headers(iteration_benchmark PRIVATE src/graph_pch.h)
target_link_libraries(iteration_benchmark Threads::Threads)
add_executable(generation_benchmark benchmark/generation_benchmark.cpp)
target_precompile_headers(generation_benchmark PRIVATE src/graph_pch.h)
target_link_libraries(generation_benchmark Threads::Threads)

# Add threaded algorithm benchmark executables
add_executable(bfs_threaded_benchmark benchmark/bfs_threaded_benchmark.cpp)
target_precompile_headers(bfs_threaded_benchmark PRIVATE src/graph_pch.h)
target_link_libraries(bfs_threaded_benchmark Threads::Threads)
add_executable(tc_threaded_benchmark benchmark/tc_threaded_benchmark.cpp)
target_precompile_headers(tc_threaded_benchmark PRIVATE src/graph_pch.h)
target_link_libraries(tc_threaded_benchmark Threads::Threads)

# Optionally, set warnings
set(WARNING_TARGETS generate_and_print validate_graph iteration bfs_threaded iteration_benchmark generation_benchmark)
if(OpenCL_FOUND)
    list(APPEND WARNING_TARGETS bfs_opencl tc_opencl breadth_first_search_test triangle_counting_test)
endif()
foreach(target ${WARNING_TARGETS})
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wno-ignored-attributes)
endforeach() 