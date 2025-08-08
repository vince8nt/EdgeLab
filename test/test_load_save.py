#!/usr/bin/env python3
"""
Load/Save Test Script for EdgeLab

This script tests graph save/load functionality for all combinations of:
- Graph types: directed/undirected
- Vertex types: weighted/unweighted  
- Edge types: weighted/unweighted
- File formats: EL (text) and CG (binary)

Replaces both test_load_save.ps1 and test_load_save.sh
"""

import os
import sys
import subprocess
import tempfile
import shutil
from pathlib import Path


def main():
    # Get script directory and project root
    script_dir = Path(__file__).parent.absolute()
    project_root = script_dir.parent
    build_dir = project_root / "build"
    temp_dir = script_dir / "load_save_temp"
    
    print(f"Running load/save tests from: {script_dir}")
    print(f"Build directory: {build_dir}")
    print(f"Temp directory: {temp_dir}")
    
    # Check if build directory exists
    if not build_dir.exists():
        print(f"Error: Build directory not found: {build_dir}")
        print("Please build the project first")
        sys.exit(1)
    
    # Check if test_load_save executable exists (handle platform-specific extensions)
    executable_path = build_dir / "test_load_save"
    if not executable_path.exists():
        # Try with .exe extension (Windows)
        executable_path = build_dir / "test_load_save.exe"
        if not executable_path.exists():
            print(f"Error: test_load_save executable not found in build directory")
            print("Please build the project first")
            sys.exit(1)
    
    print(f"Found executable: {executable_path}")
    
    # Check if temp directory already exists - if so, fail
    if temp_dir.exists():
        print(f"Error: Temp directory already exists: {temp_dir}")
        print("Please remove it manually to run the test")
        sys.exit(1)
    
    # Create temp directory
    print(f"Creating temp directory: {temp_dir}")
    temp_dir.mkdir(parents=True, exist_ok=True)
    
    # Define test configurations
    graph_types = ["u", "d"]
    vertex_types = ["uw", "w", "uw", "w"]
    edge_types = ["uw", "uw", "w", "w"]
    el_files = ["temp.el", "temp.vel", "temp.wel", "temp.vwel"]
    cg_file = "temp.cg"
    
    # Counter for tests (each configuration runs 2 tests: EL and CG)
    test_count = 0
    passed_count = 0
    
    print("Starting load/save tests...")
    
    try:
        # Test all combinations of graph type, vertex type, edge type, and save file
        for graph_type in graph_types:
            for i, (vertex_type, edge_type, el_file) in enumerate(zip(vertex_types, edge_types, el_files)):
                print(f"Test {test_count}: Graph={graph_type}, Vertex={vertex_type}, Edge={edge_type}")
                
                # Test with EL format
                test_count += 1
                print("  Testing EL format...")
                el_path = temp_dir / el_file
                
                cmd = [
                    str(executable_path),
                    "--graph-type", graph_type,
                    "--vertex-type", vertex_type,
                    "--edge-type", edge_type,
                    "--scale", "8",
                    "--degree", "8",
                    "--gen-type", "er",
                    "--save-file", str(el_path)
                ]
                
                print(f"  Running: {' '.join(cmd)}")
                
                result = subprocess.run(cmd, capture_output=True, text=True)
                if result.returncode == 0:
                    print("  EL format passed")
                    passed_count += 1
                else:
                    print(f"  EL format failed with exit code {result.returncode}")
                    if result.stdout:
                        print(f"  stdout: {result.stdout}")
                    if result.stderr:
                        print(f"  stderr: {result.stderr}")
                    sys.exit(1)
                
                # Test with CG format
                test_count += 1
                print("  Testing CG format...")
                cg_path = temp_dir / cg_file
                
                cmd = [
                    str(executable_path),
                    "--graph-type", graph_type,
                    "--vertex-type", vertex_type,
                    "--edge-type", edge_type,
                    "--scale", "8",
                    "--degree", "8",
                    "--gen-type", "er",
                    "--save-file", str(cg_path)
                ]
                
                print(f"  Running: {' '.join(cmd)}")
                
                result = subprocess.run(cmd, capture_output=True, text=True)
                if result.returncode == 0:
                    print("  CG format passed")
                    passed_count += 1
                else:
                    print(f"  CG format failed with exit code {result.returncode}")
                    if result.stdout:
                        print(f"  stdout: {result.stdout}")
                    if result.stderr:
                        print(f"  stderr: {result.stderr}")
                    sys.exit(1)
                
                print()
    
    finally:
        # Clean up temp directory
        print("Cleaning up...")
        if temp_dir.exists():
            shutil.rmtree(temp_dir)
            print(f"Removed temp directory: {temp_dir}")
    
    print("=" * 42)
    print("Load/Save Test Results:")
    print(f"Total tests: {test_count}")
    print(f"Passed: {passed_count}")
    print(f"Failed: {test_count - passed_count}")
    print("=" * 42)
    
    if passed_count == test_count:
        print("All load/save tests passed!")
        sys.exit(0)
    else:
        print("Some load/save tests failed!")
        sys.exit(1)


if __name__ == "__main__":
    main() 