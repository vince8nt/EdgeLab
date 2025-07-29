# EdgeLab Benchmarking Infrastructure

This directory contains the benchmarking infrastructure for EdgeLab, designed to test performance across different graph algorithms, implementations, and operations.

## Overview

The benchmarking system consists of two main components:

1. **Algorithm Benchmarks** - Python script for testing graph algorithms
2. **Micro Benchmarks** - Shell script and C++ programs for testing specific operations

## Algorithm Benchmarks

### Usage

```bash
# Run with default settings (uses existing test graphs)
python3 run_algorithm_benchmarks.py

# Generate test graphs and run benchmarks
python3 run_algorithm_benchmarks.py --generate-graphs

# Run with specific graph files
python3 run_algorithm_benchmarks.py --graph-files ../graphs/mini.el ../graphs/test_1000_0.01.el

# Run with more iterations
python3 run_algorithm_benchmarks.py --num-runs 10

# Specify build directory
python3 run_algorithm_benchmarks.py --build-dir ../build
```

### What it tests

- **BFS (Breadth-First Search)**
  - Sequential implementation
  - Multithreaded implementation (2, 4, 8, 16 threads)
  - OpenCL implementation

- **Triangle Counting**
  - Sequential implementation
  - Multithreaded implementation (2, 4, 8, 16 threads)
  - OpenCL implementation

### Output

Results are saved to `algorithm_benchmark_results.json` with detailed timing information including:
- Mean execution time
- Standard deviation
- Min/max times
- Success rate

## Micro Benchmarks

### Usage

```bash
# Run all micro benchmarks
./run_micro_benchmarks.sh

# Make executable first (if needed)
chmod +x run_micro_benchmarks.sh
```

### What it tests

1. **Graph Generation**
   - Small/medium/large graphs
   - Sparse/dense graphs
   - Different edge densities

2. **Graph Loading**
   - Different file formats (EL, WEL, ELAB)
   - Different graph sizes
   - Format conversion times

3. **Graph Saving**
   - Saving to different formats
   - Performance comparison

4. **Graph Iteration**
   - Forward/backward/random iteration
   - Iterator vs indexing performance
   - Memory access patterns

5. **Memory Usage**
   - Memory consumption for large graphs

### Individual C++ Benchmarks

#### Iteration Benchmark

```bash
# Compile
g++ -std=c++20 -O3 -I../src iteration_benchmark.cpp -o iteration_benchmark

# Run
./iteration_benchmark ../graphs/test_1000_0.01.el forward 1000
./iteration_benchmark ../graphs/test_1000_0.01.el backward 1000
./iteration_benchmark ../graphs/test_1000_0.01.el random 1000
```

#### Generation Benchmark

```bash
# Compile
g++ -std=c++20 -O3 -I../src generation_benchmark.cpp -o generation_benchmark

# Run different benchmark types
./generation_benchmark scaling
./generation_benchmark types
./generation_benchmark memory
./generation_benchmark all
```

## Test Graph Generation

The benchmarking system can generate test graphs of various sizes and densities:

- **Small graphs**: 100 vertices, 500-4950 edges
- **Medium graphs**: 1000 vertices, 5000-499500 edges  
- **Large graphs**: 10000 vertices, 50000-49995000 edges
- **Densities**: 0.1%, 0.5%, 1.0% of possible edges

## Results Analysis

### Algorithm Benchmark Results

The JSON output can be analyzed to:
- Compare performance across different implementations
- Identify optimal thread counts for multithreaded algorithms
- Measure scaling behavior with graph size
- Compare sequential vs parallel vs GPU implementations

### Micro Benchmark Results

Results are saved in `micro_benchmark_results/` with:
- Individual benchmark results in separate files
- Summary file with all results
- Timing statistics (mean, min, max, success rate)

## Performance Considerations

### System Requirements

- **CPU**: Multi-core processor for threaded benchmarks
- **Memory**: At least 8GB RAM for large graph tests
- **GPU**: OpenCL-capable GPU for GPU benchmarks
- **Storage**: Sufficient space for test graphs and results

### Optimization Tips

1. **Build with optimizations**: Use `-O3` for maximum performance
2. **Warm up**: Run benchmarks multiple times to account for cache effects
3. **System load**: Ensure minimal background processes during benchmarking
4. **Memory**: Close other applications to ensure consistent memory availability

## Extending the Benchmarks

### Adding New Algorithms

1. Create the algorithm implementation in `examples/` or `examples_threaded/`
2. Add the algorithm to the Python benchmark runner
3. Update the shell script if needed

### Adding New Micro Benchmarks

1. Create a new C++ benchmark file
2. Add the benchmark to the shell script
3. Update the README with usage instructions

### Custom Test Graphs

You can create custom test graphs using the existing tools:

```bash
# Generate a specific graph
./generate_and_print.exe --num-vertices 5000 --num-edges 25000 --save-file custom_graph.el

# Use in benchmarks
python3 run_algorithm_benchmarks.py --graph-files custom_graph.el
```

## Troubleshooting

### Common Issues

1. **Build errors**: Ensure all dependencies are installed and CMake is configured correctly
2. **Memory errors**: Reduce graph sizes for systems with limited RAM
3. **Timeout errors**: Increase timeout values in the Python script for large graphs
4. **Permission errors**: Make shell scripts executable with `chmod +x`

### Performance Debugging

1. **Check system load**: Use `top` or `htop` to monitor CPU and memory usage
2. **Profile with tools**: Use `perf`, `gprof`, or `valgrind` for detailed analysis
3. **Compare results**: Run benchmarks multiple times to ensure consistency
4. **Check hardware**: Ensure CPU frequency scaling is disabled for consistent results

## Contributing

When adding new benchmarks:

1. Follow the existing naming conventions
2. Include proper error handling
3. Add documentation for new features
4. Test on different systems if possible
5. Update this README with new usage instructions 