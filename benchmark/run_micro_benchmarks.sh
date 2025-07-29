#!/bin/bash

# Micro Benchmark Runner for EdgeLab
# This script runs micro benchmarks for various graph operations:
# - Graph generation times
# - Graph loading times  
# - Graph saving times
# - Graph iteration performance
# - Memory usage

set -e

# Configuration
BUILD_DIR="../build"
RESULTS_DIR="micro_benchmark_results"
NUM_RUNS=5

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Helper functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Create results directory
mkdir -p "$RESULTS_DIR"

# Function to run a command and measure time
run_benchmark() {
    local name="$1"
    local cmd="$2"
    local output_file="$RESULTS_DIR/${name}.txt"
    
    log_info "Running benchmark: $name"
    echo "Benchmark: $name" > "$output_file"
    echo "Command: $cmd" >> "$output_file"
    echo "Runs: $NUM_RUNS" >> "$output_file"
    echo "Results:" >> "$output_file"
    
    local total_time=0
    local success_count=0
    
    for ((i=1; i<=NUM_RUNS; i++)); do
        log_info "  Run $i/$NUM_RUNS"
        
        # Run command and measure time
        start_time=$(date +%s.%N)
        if eval "$cmd" > /tmp/benchmark_output 2>&1; then
            end_time=$(date +%s.%N)
            execution_time=$(echo "$end_time - $start_time" | bc -l)
            total_time=$(echo "$total_time + $execution_time" | bc -l)
            success_count=$((success_count + 1))
            echo "  Run $i: ${execution_time}s" >> "$output_file"
            log_success "  Run $i: ${execution_time}s"
        else
            echo "  Run $i: FAILED" >> "$output_file"
            log_error "  Run $i: FAILED"
            cat /tmp/benchmark_output
        fi
    done
    
    if [ $success_count -gt 0 ]; then
        avg_time=$(echo "scale=3; $total_time / $success_count" | bc -l)
        echo "Average time: ${avg_time}s" >> "$output_file"
        echo "Success rate: ${success_count}/${NUM_RUNS}" >> "$output_file"
        log_success "Average time: ${avg_time}s (${success_count}/${NUM_RUNS} successful)"
    else
        echo "Average time: FAILED" >> "$output_file"
        log_error "All runs failed"
    fi
    
    echo "" >> "$output_file"
}

# Function to run memory benchmark
run_memory_benchmark() {
    local name="$1"
    local cmd="$2"
    local output_file="$RESULTS_DIR/${name}_memory.txt"
    
    log_info "Running memory benchmark: $name"
    echo "Memory Benchmark: $name" > "$output_file"
    echo "Command: $cmd" >> "$output_file"
    echo "Results:" >> "$output_file"
    
    # Use /usr/bin/time to measure memory usage
    if command -v /usr/bin/time >/dev/null 2>&1; then
        /usr/bin/time -v eval "$cmd" 2>&1 | tee -a "$output_file"
    else
        log_warning "Could not find /usr/bin/time, skipping memory measurement"
        eval "$cmd" | tee -a "$output_file"
    fi
}

# Change to build directory
cd "$BUILD_DIR"

log_info "Starting micro benchmarks..."

# Graph Generation Benchmarks
log_info "=== Graph Generation Benchmarks ==="

# Small graphs (scale 8)
run_benchmark "gen_small_sparse" "./generate_and_print.exe --scale 8 --degree 2 --gen-type erdos_renyi --save-file /tmp/test_small_sparse.el"
run_benchmark "gen_small_medium" "./generate_and_print.exe --scale 8 --degree 4 --gen-type erdos_renyi --save-file /tmp/test_small_medium.el"
run_benchmark "gen_small_dense" "./generate_and_print.exe --scale 8 --degree 8 --gen-type erdos_renyi --save-file /tmp/test_small_dense.el"

# Medium graphs (scale 12)
run_benchmark "gen_medium_sparse" "./generate_and_print.exe --scale 12 --degree 2 --gen-type erdos_renyi --save-file /tmp/test_medium_sparse.el"
run_benchmark "gen_medium_medium" "./generate_and_print.exe --scale 12 --degree 4 --gen-type erdos_renyi --save-file /tmp/test_medium_medium.el"
run_benchmark "gen_medium_dense" "./generate_and_print.exe --scale 12 --degree 8 --gen-type erdos_renyi --save-file /tmp/test_medium_dense.el"

# Large graphs (scale 16)
run_benchmark "gen_large_sparse" "./generate_and_print.exe --scale 16 --degree 2 --gen-type erdos_renyi --save-file /tmp/test_large_sparse.el"
run_benchmark "gen_large_medium" "./generate_and_print.exe --scale 16 --degree 4 --gen-type erdos_renyi --save-file /tmp/test_large_medium.el"
run_benchmark "gen_large_dense" "./generate_and_print.exe --scale 16 --degree 8 --gen-type erdos_renyi --save-file /tmp/test_large_dense.el"

# High degree tests (scale 16, high degree)
run_benchmark "gen_high_degree_32" "./generate_and_print.exe --scale 16 --degree 32 --gen-type erdos_renyi --save-file /tmp/test_high_degree_32.el"
run_benchmark "gen_high_degree_64" "./generate_and_print.exe --scale 16 --degree 64 --gen-type erdos_renyi --save-file /tmp/test_high_degree_64.el"
run_benchmark "gen_high_degree_128" "./generate_and_print.exe --scale 16 --degree 128 --gen-type erdos_renyi --save-file /tmp/test_high_degree_128.el"
run_benchmark "gen_high_degree_256" "./generate_and_print.exe --scale 16 --degree 256 --gen-type erdos_renyi --save-file /tmp/test_high_degree_256.el"

# Graph Loading Benchmarks
log_info "=== Graph Loading Benchmarks ==="

# Load different formats
run_benchmark "load_small_el" "./print.exe --load-file /tmp/test_small_sparse.el"
run_benchmark "load_medium_el" "./print.exe --load-file /tmp/test_medium_sparse.el"
run_benchmark "load_large_el" "./print.exe --load-file /tmp/test_large_sparse.el"

# Convert to different formats and test loading
run_benchmark "convert_to_wel" "./convert.exe --load-file /tmp/test_medium_sparse.el --save-file /tmp/test_medium_sparse.wel"
run_benchmark "load_wel" "./print.exe --load-file /tmp/test_medium_sparse.wel"

run_benchmark "convert_to_elab" "./convert.exe --load-file /tmp/test_medium_sparse.el --save-file /tmp/test_medium_sparse.elab"
run_benchmark "load_elab" "./print.exe --load-file /tmp/test_medium_sparse.elab"

# Graph Saving Benchmarks
log_info "=== Graph Saving Benchmarks ==="

run_benchmark "save_el" "./convert.exe --load-file /tmp/test_medium_sparse.el --save-file /tmp/test_save.el"
run_benchmark "save_wel" "./convert.exe --load-file /tmp/test_medium_sparse.el --save-file /tmp/test_save.wel"
run_benchmark "save_elab" "./convert.exe --load-file /tmp/test_medium_sparse.el --save-file /tmp/test_save.elab"

# Graph Iteration Benchmarks
log_info "=== Graph Iteration Benchmarks ==="

# Create a test graph for iteration benchmarks
./generate_and_print.exe --scale 10 --degree 4 --gen-type erdos_renyi --save-file /tmp/iteration_test.el > /dev/null

# Test different iteration patterns
run_benchmark "iteration_forward" "./iteration.exe --load-file /tmp/iteration_test.el --direction forward"
run_benchmark "iteration_backward" "./iteration.exe --load-file /tmp/iteration_test.el --direction backward"
run_benchmark "iteration_random" "./iteration.exe --load-file /tmp/iteration_test.el --direction random"

# Memory Usage Benchmarks
log_info "=== Memory Usage Benchmarks ==="

run_memory_benchmark "memory_large_graph" "./print.exe --load-file /tmp/test_large_sparse.el"

# Cleanup
log_info "Cleaning up temporary files..."
rm -f /tmp/test_*.el /tmp/test_*.wel /tmp/test_*.elab

# Generate summary
log_info "Generating benchmark summary..."
{
    echo "=== EdgeLab Micro Benchmark Summary ==="
    echo "Date: $(date)"
    echo "Build directory: $BUILD_DIR"
    echo "Number of runs per benchmark: $NUM_RUNS"
    echo ""
    
    for result_file in "$RESULTS_DIR"/*.txt; do
        if [ -f "$result_file" ]; then
            echo "=== $(basename "$result_file" .txt) ==="
            cat "$result_file"
            echo ""
        fi
    done
} > "$RESULTS_DIR/summary.txt"

log_success "Micro benchmarks completed! Results saved in $RESULTS_DIR/"
log_info "Summary available at: $RESULTS_DIR/summary.txt" 