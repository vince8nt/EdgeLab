#!/bin/bash

# Bash script for testing load/save functionality
# Exit on any error
set -e

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"
TEMP_DIR="$SCRIPT_DIR/load_save_temp"

echo "Running load/save tests from: $SCRIPT_DIR"
echo "Build directory: $BUILD_DIR"
echo "Temp directory: $TEMP_DIR"

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: Build directory not found: $BUILD_DIR"
    echo "Please build the project first"
    exit 1
fi

# Check if test_load_save executable exists
EXECUTABLE_PATH="$BUILD_DIR/test_load_save"
if [ ! -f "$EXECUTABLE_PATH" ]; then
    echo "Error: test_load_save executable not found in build directory"
    echo "Please build the project first"
    exit 1
fi

# Check if temp directory already exists - if so, fail
if [ -d "$TEMP_DIR" ]; then
    echo "Error: Temp directory already exists: $TEMP_DIR"
    echo "Please remove it manually to run the test"
    exit 1
fi

# Create temp directory
echo "Creating temp directory: $TEMP_DIR"
mkdir -p "$TEMP_DIR"

# Define test configurations
graph_types=("undirected" "directed")
vertex_types=("unweighted" "weighted" "unweighted" "weighted")
edge_types=("unweighted" "unweighted" "weighted" "weighted")
el_files=("temp.el" "temp.vel" "temp.wel" "temp.vwel")
elab_file="temp.elab"

# Counter for tests (each configuration runs 2 tests: EL and ELAB)
test_count=0
passed_count=0

echo "Starting load/save tests..."

# Function to cleanup on exit
cleanup() {
    echo "Cleaning up..."
    if [ -d "$TEMP_DIR" ]; then
        rm -rf "$TEMP_DIR"
        echo "Removed temp directory: $TEMP_DIR"
    fi
}

# Set trap to cleanup on exit
trap cleanup EXIT

# Test all combinations of graph type, vertex type, edge type, and save file
for graph_type in "${graph_types[@]}"; do
    for i in "${!vertex_types[@]}"; do
        vertex_type="${vertex_types[$i]}"
        edge_type="${edge_types[$i]}"
        el_file="${el_files[$i]}"
        
        echo "Test $test_count: Graph=$graph_type, Vertex=$vertex_type, Edge=$edge_type"
        
        # Test with EL format
        ((test_count++))
        echo "  Testing EL format..."
        el_path="$TEMP_DIR/$el_file"
        
        if "$EXECUTABLE_PATH" \
            --graph-type "$graph_type" \
            --vertex-type "$vertex_type" \
            --edge-type "$edge_type" \
            --scale 8 \
            --degree 8 \
            --gen-type erdos_renyi \
            --save-file "$el_path"; then
            echo "  EL format passed"
            ((passed_count++))
        else
            echo "  EL format failed"
            exit 1
        fi
        
        # Test with ELAB format
        ((test_count++))
        echo "  Testing ELAB format..."
        elab_path="$TEMP_DIR/$elab_file"
        
        if "$EXECUTABLE_PATH" \
            --graph-type "$graph_type" \
            --vertex-type "$vertex_type" \
            --edge-type "$edge_type" \
            --scale 8 \
            --degree 8 \
            --gen-type erdos_renyi \
            --save-file "$elab_path"; then
            echo "  ELAB format passed"
            ((passed_count++))
        else
            echo "  ELAB format failed"
            exit 1
        fi
        
        echo ""
    done
done

echo "=========================================="
echo "Load/Save Test Results:"
echo "Total tests: $test_count"
echo "Passed: $passed_count"
echo "Failed: $((test_count - passed_count))"
echo "=========================================="

if [ "$passed_count" -eq "$test_count" ]; then
    echo "All load/save tests passed!"
    exit 0
else
    echo "Some load/save tests failed!"
    exit 1
fi
    
        
