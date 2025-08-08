#!/usr/bin/env python3

"""
Micro Benchmark Runner for EdgeLab
This script runs micro benchmarks for various graph operations:
- Graph generation times
- Graph loading times  
- Graph saving times
- Graph iteration performance
- Memory usage
"""

import os
import sys
import time
import subprocess
import shutil
import statistics
from pathlib import Path
from typing import List, Tuple, Optional
import argparse


class Colors:
    """ANSI color codes for terminal output"""
    RED = '\033[0;31m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    NC = '\033[0m'  # No Color


def log_info(message: str):
    """Print info message with blue color"""
    print(f"{Colors.BLUE}[INFO]{Colors.NC} {message}")


def log_success(message: str):
    """Print success message with green color"""
    print(f"{Colors.GREEN}[SUCCESS]{Colors.NC} {message}")


def log_warning(message: str):
    """Print warning message with yellow color"""
    print(f"{Colors.YELLOW}[WARNING]{Colors.NC} {message}")


def log_error(message: str):
    """Print error message with red color"""
    print(f"{Colors.RED}[ERROR]{Colors.NC} {message}")


class MicroBenchmarkRunner:
    def __init__(self, build_dir: str = "../build", results_dir: str = "micro_benchmark_results", num_runs: int = 5):
        self.build_dir = Path(build_dir)
        self.results_dir = Path(results_dir)
        self.num_runs = num_runs
        
        # Create results directory
        self.results_dir.mkdir(exist_ok=True)
        
        # Change to build directory
        if not self.build_dir.exists():
            raise FileNotFoundError(f"Build directory {self.build_dir} does not exist")
        
        self.original_cwd = os.getcwd()
        os.chdir(self.build_dir)
        
    def __enter__(self):
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        os.chdir(self.original_cwd)
    
    def run_command(self, cmd: List[str], capture_output: bool = True) -> Tuple[bool, str, float]:
        """Run a command and measure execution time"""
        start_time = time.time()
        
        try:
            if capture_output:
                result = subprocess.run(cmd, capture_output=True, text=True, check=True)
                output = result.stdout + result.stderr
            else:
                result = subprocess.run(cmd, check=True)
                output = ""
            
            execution_time = time.time() - start_time
            return True, output, execution_time
            
        except subprocess.CalledProcessError as e:
            execution_time = time.time() - start_time
            output = e.stdout + e.stderr if e.stdout or e.stderr else str(e)
            return False, output, execution_time
    
    def run_benchmark(self, name: str, cmd: List[str]) -> None:
        """Run a benchmark multiple times and record results"""
        output_file = self.results_dir / f"{name}.txt"
        
        log_info(f"Running benchmark: {name}")
        
        with open(output_file, 'w') as f:
            f.write(f"Benchmark: {name}\n")
            f.write(f"Command: {' '.join(cmd)}\n")
            f.write(f"Runs: {self.num_runs}\n")
            f.write("Results:\n")
        
        times = []
        success_count = 0
        
        for i in range(1, self.num_runs + 1):
            log_info(f"  Run {i}/{self.num_runs}")
            
            success, output, execution_time = self.run_command(cmd)
            
            with open(output_file, 'a') as f:
                if success:
                    times.append(execution_time)
                    success_count += 1
                    f.write(f"  Run {i}: {execution_time:.3f}s\n")
                    log_success(f"  Run {i}: {execution_time:.3f}s")
                else:
                    f.write(f"  Run {i}: FAILED\n")
                    log_error(f"  Run {i}: FAILED")
                    if output:
                        print(output)
        
        with open(output_file, 'a') as f:
            if success_count > 0:
                avg_time = statistics.mean(times)
                f.write(f"Average time: {avg_time:.3f}s\n")
                f.write(f"Success rate: {success_count}/{self.num_runs}\n")
                log_success(f"Average time: {avg_time:.3f}s ({success_count}/{self.num_runs} successful)")
            else:
                f.write("Average time: FAILED\n")
                log_error("All runs failed")
            
            f.write("\n")
    
    def run_memory_benchmark(self, name: str, cmd: List[str]) -> None:
        """Run a memory benchmark using system time command"""
        output_file = self.results_dir / f"{name}_memory.txt"
        
        log_info(f"Running memory benchmark: {name}")
        
        with open(output_file, 'w') as f:
            f.write(f"Memory Benchmark: {name}\n")
            f.write(f"Command: {' '.join(cmd)}\n")
            f.write("Results:\n")
        
        # Try to use /usr/bin/time for memory measurement
        time_cmd = shutil.which('time')
        if time_cmd and os.path.exists('/usr/bin/time'):
            # Use GNU time for detailed memory info
            time_cmd = ['/usr/bin/time', '-v'] + cmd
        elif time_cmd:
            # Use system time command
            time_cmd = [time_cmd] + cmd
        else:
            log_warning("Could not find time command, running without memory measurement")
            time_cmd = cmd
        
        success, output, execution_time = self.run_command(time_cmd, capture_output=True)
        
        with open(output_file, 'a') as f:
            f.write(output)
            if not success:
                log_error(f"Memory benchmark failed: {output}")
    
    def cleanup_temp_files(self) -> None:
        """Clean up temporary test files"""
        log_info("Cleaning up temporary files...")
        
        temp_patterns = [
            "/tmp/test_*.el",
            "/tmp/test_*.wel", 
            "/tmp/test_*.cg"
        ]
        
        for pattern in temp_patterns:
            try:
                for file_path in Path("/tmp").glob(pattern.split("/")[-1]):
                    file_path.unlink()
            except Exception as e:
                log_warning(f"Could not clean up {pattern}: {e}")
    
    def generate_summary(self) -> None:
        """Generate a summary of all benchmark results"""
        log_info("Generating benchmark summary...")
        
        summary_file = self.results_dir / "summary.txt"
        
        with open(summary_file, 'w') as f:
            f.write("=== EdgeLab Micro Benchmark Summary ===\n")
            f.write(f"Date: {time.strftime('%Y-%m-%d %H:%M:%S')}\n")
            f.write(f"Build directory: {self.build_dir}\n")
            f.write(f"Number of runs per benchmark: {self.num_runs}\n")
            f.write("\n")
            
            # Read all result files
            for result_file in self.results_dir.glob("*.txt"):
                if result_file.name != "summary.txt":
                    f.write(f"=== {result_file.stem} ===\n")
                    f.write(result_file.read_text())
                    f.write("\n")
        
        log_success(f"Micro benchmarks completed! Results saved in {self.results_dir}/")
        log_info(f"Summary available at: {summary_file}")


def main():
    parser = argparse.ArgumentParser(description="Run EdgeLab micro benchmarks")
    parser.add_argument("--build-dir", default="../build", help="Build directory path")
    parser.add_argument("--results-dir", default="micro_benchmark_results", help="Results directory path")
    parser.add_argument("--num-runs", type=int, default=5, help="Number of runs per benchmark")
    
    args = parser.parse_args()
    
    try:
        with MicroBenchmarkRunner(args.build_dir, args.results_dir, args.num_runs) as runner:
            log_info("Starting micro benchmarks...")
            
            # Graph Generation Benchmarks
            log_info("=== Graph Generation Benchmarks ===")
            
            # Small graphs (scale 8)
            runner.run_benchmark("gen_small_sparse", 
                ["./generate_and_print.exe", "--scale", "8", "--degree", "2", "--gen-type", "erdos_renyi", "--save-file", "/tmp/test_small_sparse.el"])
            runner.run_benchmark("gen_small_medium", 
                ["./generate_and_print.exe", "--scale", "8", "--degree", "4", "--gen-type", "erdos_renyi", "--save-file", "/tmp/test_small_medium.el"])
            runner.run_benchmark("gen_small_dense", 
                ["./generate_and_print.exe", "--scale", "8", "--degree", "8", "--gen-type", "erdos_renyi", "--save-file", "/tmp/test_small_dense.el"])
            
            # Medium graphs (scale 12)
            runner.run_benchmark("gen_medium_sparse", 
                ["./generate_and_print.exe", "--scale", "12", "--degree", "2", "--gen-type", "erdos_renyi", "--save-file", "/tmp/test_medium_sparse.el"])
            runner.run_benchmark("gen_medium_medium", 
                ["./generate_and_print.exe", "--scale", "12", "--degree", "4", "--gen-type", "erdos_renyi", "--save-file", "/tmp/test_medium_medium.el"])
            runner.run_benchmark("gen_medium_dense", 
                ["./generate_and_print.exe", "--scale", "12", "--degree", "8", "--gen-type", "erdos_renyi", "--save-file", "/tmp/test_medium_dense.el"])
            
            # Large graphs (scale 16)
            runner.run_benchmark("gen_large_sparse", 
                ["./generate_and_print.exe", "--scale", "16", "--degree", "2", "--gen-type", "erdos_renyi", "--save-file", "/tmp/test_large_sparse.el"])
            runner.run_benchmark("gen_large_medium", 
                ["./generate_and_print.exe", "--scale", "16", "--degree", "4", "--gen-type", "erdos_renyi", "--save-file", "/tmp/test_large_medium.el"])
            runner.run_benchmark("gen_large_dense", 
                ["./generate_and_print.exe", "--scale", "16", "--degree", "8", "--gen-type", "erdos_renyi", "--save-file", "/tmp/test_large_dense.el"])
            
            # High degree tests (scale 16, high degree)
            runner.run_benchmark("gen_high_degree_32", 
                ["./generate_and_print.exe", "--scale", "16", "--degree", "32", "--gen-type", "erdos_renyi", "--save-file", "/tmp/test_high_degree_32.el"])
            runner.run_benchmark("gen_high_degree_64", 
                ["./generate_and_print.exe", "--scale", "16", "--degree", "64", "--gen-type", "erdos_renyi", "--save-file", "/tmp/test_high_degree_64.el"])
            runner.run_benchmark("gen_high_degree_128", 
                ["./generate_and_print.exe", "--scale", "16", "--degree", "128", "--gen-type", "erdos_renyi", "--save-file", "/tmp/test_high_degree_128.el"])
            runner.run_benchmark("gen_high_degree_256", 
                ["./generate_and_print.exe", "--scale", "16", "--degree", "256", "--gen-type", "erdos_renyi", "--save-file", "/tmp/test_high_degree_256.el"])
            
            # Graph Loading Benchmarks
            log_info("=== Graph Loading Benchmarks ===")
            
            # Load different formats
            runner.run_benchmark("load_small_el", ["./print.exe", "--load-file", "/tmp/test_small_sparse.el"])
            runner.run_benchmark("load_medium_el", ["./print.exe", "--load-file", "/tmp/test_medium_sparse.el"])
            runner.run_benchmark("load_large_el", ["./print.exe", "--load-file", "/tmp/test_large_sparse.el"])
            
            # Convert to different formats and test loading
            runner.run_benchmark("convert_to_wel", 
                ["./convert.exe", "--load-file", "/tmp/test_medium_sparse.el", "--save-file", "/tmp/test_medium_sparse.wel"])
            runner.run_benchmark("load_wel", ["./print.exe", "--load-file", "/tmp/test_medium_sparse.wel"])
            
            runner.run_benchmark("convert_to_cg", 
                ["./convert.exe", "--load-file", "/tmp/test_medium_sparse.el", "--save-file", "/tmp/test_medium_sparse.cg"])
            runner.run_benchmark("load_cg", ["./print.exe", "--load-file", "/tmp/test_medium_sparse.cg"])
            
            # Graph Saving Benchmarks
            log_info("=== Graph Saving Benchmarks ===")
            
            runner.run_benchmark("save_el", 
                ["./convert.exe", "--load-file", "/tmp/test_medium_sparse.el", "--save-file", "/tmp/test_save.el"])
            runner.run_benchmark("save_wel", 
                ["./convert.exe", "--load-file", "/tmp/test_medium_sparse.el", "--save-file", "/tmp/test_save.wel"])
            runner.run_benchmark("save_cg", 
                ["./convert.exe", "--load-file", "/tmp/test_medium_sparse.el", "--save-file", "/tmp/test_save.cg"])
            
            # Graph Iteration Benchmarks
            log_info("=== Graph Iteration Benchmarks ===")
            
            # Create a test graph for iteration benchmarks
            runner.run_command(["./generate_and_print.exe", "--scale", "10", "--degree", "4", "--gen-type", "erdos_renyi", "--save-file", "/tmp/iteration_test.el"], capture_output=False)
            
            # Test different iteration patterns
            runner.run_benchmark("iteration_forward", ["./iteration.exe", "--load-file", "/tmp/iteration_test.el", "--direction", "forward"])
            runner.run_benchmark("iteration_backward", ["./iteration.exe", "--load-file", "/tmp/iteration_test.el", "--direction", "backward"])
            runner.run_benchmark("iteration_random", ["./iteration.exe", "--load-file", "/tmp/iteration_test.el", "--direction", "random"])
            
            # Memory Usage Benchmarks
            log_info("=== Memory Usage Benchmarks ===")
            
            runner.run_memory_benchmark("memory_large_graph", ["./print.exe", "--load-file", "/tmp/test_large_sparse.el"])
            
            # Cleanup and generate summary
            runner.cleanup_temp_files()
            runner.generate_summary()
            
    except Exception as e:
        log_error(f"Benchmark failed: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main() 