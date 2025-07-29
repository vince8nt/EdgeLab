#!/usr/bin/env python3
"""
Algorithm Benchmark Runner for EdgeLab

This script runs various graph algorithms with different configurations:
- Different graph sizes and types
- Different numbers of threads for multithreaded versions
- Different algorithms (BFS, Triangle Counting)
- Different implementations (sequential, threaded, OpenCL)

Test configurations use one of two tuple formats:
1. (scale, degree, genType, EdgeType, VertexType, GraphType) - for generated graphs
2. (filepath) - for existing graph files
"""

import subprocess
import time
import json
import os
import sys
import argparse
from pathlib import Path
from typing import Dict, List, Any, Union, Tuple
import statistics

# Get the script directory and project root
SCRIPT_DIR = Path(__file__).parent.absolute()
PROJECT_ROOT = SCRIPT_DIR.parent

# Test configurations
# Each test uses exactly one of these two tuple formats:
# 1. (scale, degree, genType, EdgeType, VertexType, GraphType) - for generated graphs
# 2. (filepath) - for existing graph files

DEFAULT_TEST_CONFIGS = [
    # Existing graph files - (filepath)
    str(PROJECT_ROOT / "graphs" / "mini.el"),
    str(PROJECT_ROOT / "graphs" / "mini_d.el"),
    
    # Generated graphs - (scale, degree, genType, EdgeType, VertexType, GraphType)
    # These will be generated in memory and run directly
    (8, 4, "erdos_renyi", "unweighted", "unweighted", "directed"),
    (12, 4, "erdos_renyi", "unweighted", "unweighted", "directed"),
    (16, 4, "erdos_renyi", "unweighted", "unweighted", "directed"),
    (18, 4, "erdos_renyi", "unweighted", "unweighted", "directed"),
    (20, 4, "erdos_renyi", "unweighted", "unweighted", "directed"),
]

# Placeholder for user-provided graph files
# Users can add their own graph files here for testing
USER_GRAPH_FILES = [
    # Add your custom graph files here, for example:
    # str(PROJECT_ROOT / "graphs" / "my_custom_graph.el"),
    # str(PROJECT_ROOT / "graphs" / "large_dataset.wel"),
]

class BenchmarkRunner:
    def __init__(self, build_dir: str = None):
        if build_dir is None:
            self.build_dir = PROJECT_ROOT / "build"
        else:
            self.build_dir = Path(build_dir)
        self.results = []
        
        # Ensure build directory exists
        if not self.build_dir.exists():
            raise FileNotFoundError(f"Build directory not found: {self.build_dir}")
        
        print(f"Using build directory: {self.build_dir}")
        
    def run_command(self, cmd: List[str], timeout: int = 300) -> Dict[str, Any]:
        """Run a command and return timing results"""
        start_time = time.time()
        try:
            result = subprocess.run(
                cmd, 
                cwd=self.build_dir,
                capture_output=True, 
                text=True, 
                timeout=timeout
            )
            end_time = time.time()
            
            return {
                "success": result.returncode == 0,
                "stdout": result.stdout,
                "stderr": result.stderr,
                "returncode": result.returncode,
                "execution_time": end_time - start_time,
                "command": " ".join(cmd)
            }
        except subprocess.TimeoutExpired:
            return {
                "success": False,
                "stdout": "",
                "stderr": "Timeout expired",
                "returncode": -1,
                "execution_time": timeout,
                "command": " ".join(cmd)
            }
        except Exception as e:
            return {
                "success": False,
                "stdout": "",
                "stderr": str(e),
                "returncode": -1,
                "execution_time": 0,
                "command": " ".join(cmd)
            }
    
    def generate_graph(self, scale: int, degree: int, gen_type: str, 
                      edge_type: str, vertex_type: str, graph_type: str) -> str:
        """Generate a graph and return the file path"""
        num_vertices = 2 ** scale
        num_edges = num_vertices * degree
        
        # Use absolute path for graph file
        graphs_dir = PROJECT_ROOT / "graphs"
        graphs_dir.mkdir(exist_ok=True)
        graph_file = graphs_dir / f"generated_scale{scale}_degree{degree}_{gen_type}.el"
        
        # Use relative path from build directory for the save-file parameter
        # Build directory is PROJECT_ROOT / "build"
        # Graph file is PROJECT_ROOT / "graphs" / filename
        # So relative path from build to graphs is "../graphs/filename"
        relative_graph_path = Path("..") / graph_file.relative_to(PROJECT_ROOT)
        
        # Use full path to executable
        executable_path = self.build_dir / "generate_and_print.exe"
        
        cmd = [str(executable_path), 
              "--scale", str(scale),
              "--degree", str(degree),
              "--gen-type", gen_type,
              "--edge-type", edge_type,
              "--vertex-type", vertex_type,
              "--graph-type", graph_type,
              "--save-file", str(relative_graph_path)]
        
        print(f"Generating: scale={scale} (2^{scale}={num_vertices} vertices), degree={degree} ({num_edges} edges)")
        print(f"  Type: {gen_type}, Edge: {edge_type}, Vertex: {vertex_type}, Graph: {graph_type}")
        
        result = self.run_command(cmd)
        if result["success"]:
            print(f"  Generated: {graph_file}")
            return str(graph_file)
        else:
            print(f"  Failed: {result['stderr']}")
            return None
    
    def get_graph_files(self, test_configs: List[Union[Tuple, str]]) -> List[str]:
        """Process test configurations and return list of graph file paths"""
        graph_files = []
        
        for config in test_configs:
            if isinstance(config, tuple) and len(config) == 6:
                # Generated graph configuration - skip for now, will handle separately
                continue
            elif isinstance(config, str):
                # Existing graph file
                graph_path = Path(config)
                if graph_path.exists():
                    graph_files.append(str(graph_path))
                else:
                    print(f"Warning: Graph file not found: {config}")
            else:
                print(f"Warning: Invalid test configuration: {config}")
        
        return graph_files
    
    def get_generated_graph_configs(self, test_configs: List[Union[Tuple, str]]) -> List[Tuple]:
        """Extract generated graph configurations from test configs"""
        generated_configs = []
        
        for config in test_configs:
            if isinstance(config, tuple) and len(config) == 6:
                generated_configs.append(config)
        
        return generated_configs
    
    def run_algorithm_on_generated_graph(self, algorithm: str, 
                                        scale: int, degree: int, gen_type: str,
                                        edge_type: str, vertex_type: str, graph_type: str,
                                        num_threads: int = None, num_runs: int = 3) -> Dict[str, Any]:
        """Run a single algorithm benchmark on a generated graph"""
        # For threaded algorithms, use the new benchmark executables that can sweep thread counts
        if algorithm.endswith("_threaded"):
            return self.run_threaded_algorithm_on_generated_graph(algorithm, scale, degree, gen_type, num_runs)
        
        # Use full path to executable
        executable_path = self.build_dir / f"{algorithm}.exe"
        
        cmd = [str(executable_path), 
              "--scale", str(scale),
              "--degree", str(degree),
              "--gen-type", gen_type,
              "--edge-type", edge_type,
              "--vertex-type", vertex_type,
              "--graph-type", graph_type]
        
        # Note: Threaded algorithms don't support --num-threads when generating graphs
        # They automatically use available threads
        if num_threads and not algorithm.endswith("_threaded"):
            cmd.extend(["--num-threads", str(num_threads)])
        
        # Create a descriptive name for the generated graph
        graph_name = f"generated_scale{scale}_degree{degree}_{gen_type}"
        
        print(f"Running: {' '.join(cmd)}")
        
        times = []
        for run in range(num_runs):
            result = self.run_command(cmd)
            if result["success"]:
                times.append(result["execution_time"])
                print(f"  Run {run + 1}: {result['execution_time']:.3f}s")
            else:
                print(f"  Run {run + 1}: FAILED - {result['stderr']}")
        
        if times:
            return {
                "algorithm": algorithm,
                "graph_name": graph_name,
                "graph_config": (scale, degree, gen_type, edge_type, vertex_type, graph_type),
                "num_threads": num_threads,
                "num_runs": num_runs,
                "times": times,
                "mean_time": statistics.mean(times),
                "std_time": statistics.stdev(times) if len(times) > 1 else 0,
                "min_time": min(times),
                "max_time": max(times)
            }
        else:
            return {
                "algorithm": algorithm,
                "graph_name": graph_name,
                "graph_config": (scale, degree, gen_type, edge_type, vertex_type, graph_type),
                "num_threads": num_threads,
                "num_runs": num_runs,
                "times": [],
                "mean_time": float('inf'),
                "std_time": 0,
                "min_time": float('inf'),
                "max_time": 0,
                "error": "All runs failed"
            }
    
    def run_threaded_algorithm_on_generated_graph(self, algorithm: str, 
                                                 scale: int, degree: int, gen_type: str,
                                                 num_runs: int = 3) -> Dict[str, Any]:
        """Run threaded algorithm benchmark on generated graph using the new benchmark executables"""
        # Map algorithm names to benchmark executable names
        benchmark_executable = {
            "bfs_threaded": "bfs_threaded_benchmark.exe",
            "tc_threaded": "tc_threaded_benchmark.exe"
        }
        
        if algorithm not in benchmark_executable:
            raise ValueError(f"No benchmark executable found for algorithm: {algorithm}")
        
        executable_path = self.build_dir / benchmark_executable[algorithm]
        
        # Create a descriptive name for the generated graph
        graph_name = f"generated_scale{scale}_degree{degree}_{gen_type}"
        
        cmd = [str(executable_path), 
              "generated", str(scale), str(degree), gen_type,
              "--runs", str(num_runs)]
        
        print(f"Running threaded benchmark: {' '.join(cmd)}")
        
        result = self.run_command(cmd)
        if result["success"]:
            # Parse the output to extract timing information
            # The benchmark executable prints detailed results, but for now we'll use the overall execution time
            # In a more sophisticated version, we could parse the detailed output
            return {
                "algorithm": algorithm,
                "graph_name": graph_name,
                "graph_config": (scale, degree, gen_type, "unweighted", "unweighted", "directed"),
                "num_threads": None,  # The benchmark tests multiple thread counts
                "num_runs": num_runs,
                "times": [result["execution_time"]],  # Overall execution time
                "mean_time": result["execution_time"],
                "std_time": 0,
                "min_time": result["execution_time"],
                "max_time": result["execution_time"],
                "note": "Threaded benchmark executed - see output for detailed thread scaling results"
            }
        else:
            return {
                "algorithm": algorithm,
                "graph_name": graph_name,
                "graph_config": (scale, degree, gen_type, "unweighted", "unweighted", "directed"),
                "num_threads": None,
                "num_runs": num_runs,
                "times": [],
                "mean_time": float('inf'),
                "std_time": 0,
                "min_time": float('inf'),
                "max_time": 0,
                "error": result["stderr"]
            }
    
    def run_algorithm_benchmark(self, algorithm: str, graph_file: str, 
                               num_threads: int = None, num_runs: int = 3) -> Dict[str, Any]:
        """Run a single algorithm benchmark"""
        # Convert absolute graph file path to relative path from build directory
        graph_path = Path(graph_file)
        if graph_path.is_absolute():
            # Calculate relative path from build directory to graph file
            try:
                # Build directory is PROJECT_ROOT / "build"
                # Graph file is PROJECT_ROOT / "graphs" / filename
                # So relative path from build to graphs is "../graphs/filename"
                relative_graph_path = Path("..") / graph_path.relative_to(PROJECT_ROOT)
            except ValueError:
                # If graph file is not under project root, use absolute path
                relative_graph_path = graph_path
        else:
            relative_graph_path = graph_path
        
        # Use full path to executable
        executable_path = self.build_dir / f"{algorithm}.exe"
        
        cmd = [str(executable_path), "--load-file", str(relative_graph_path)]
        
        if num_threads:
            cmd.extend(["--num-threads", str(num_threads)])
        
        print(f"Running: {' '.join(cmd)}")
        
        times = []
        for run in range(num_runs):
            result = self.run_command(cmd)
            if result["success"]:
                times.append(result["execution_time"])
                print(f"  Run {run + 1}: {result['execution_time']:.3f}s")
            else:
                print(f"  Run {run + 1}: FAILED - {result['stderr']}")
        
        if times:
            return {
                "algorithm": algorithm,
                "graph_file": graph_file,
                "num_threads": num_threads,
                "num_runs": num_runs,
                "times": times,
                "mean_time": statistics.mean(times),
                "std_time": statistics.stdev(times) if len(times) > 1 else 0,
                "min_time": min(times),
                "max_time": max(times)
            }
        else:
            return {
                "algorithm": algorithm,
                "graph_file": graph_file,
                "num_threads": num_threads,
                "num_runs": num_runs,
                "times": [],
                "mean_time": float('inf'),
                "std_time": 0,
                "min_time": float('inf'),
                "max_time": 0,
                "error": "All runs failed"
            }
    
    def run_bfs_benchmarks(self, graph_files: List[str], generated_configs: List[Tuple], num_runs: int = 3):
        """Run BFS benchmarks with different implementations"""
        print("\n=== BFS Benchmarks ===")
        
        # Sequential BFS on file-based graphs
        for graph_file in graph_files:
            result = self.run_algorithm_benchmark("bfs", graph_file, num_runs=num_runs)
            self.results.append(result)
        
        # Sequential BFS on generated graphs
        for config in generated_configs:
            scale, degree, gen_type, edge_type, vertex_type, graph_type = config
            result = self.run_algorithm_on_generated_graph("bfs", scale, degree, gen_type, 
                                                          edge_type, vertex_type, graph_type, 
                                                          num_runs=num_runs)
            self.results.append(result)
        
        # Multithreaded BFS with different thread counts on file-based graphs
        thread_counts = [2, 4, 8, 16]
        for graph_file in graph_files:
            for num_threads in thread_counts:
                result = self.run_algorithm_benchmark("bfs_threaded", graph_file, 
                                                    num_threads, num_runs)
                self.results.append(result)
        
        # Multithreaded BFS with different thread counts on generated graphs
        for config in generated_configs:
            scale, degree, gen_type, edge_type, vertex_type, graph_type = config
            for num_threads in thread_counts:
                result = self.run_algorithm_on_generated_graph("bfs_threaded", scale, degree, gen_type,
                                                              edge_type, vertex_type, graph_type,
                                                              num_threads, num_runs)
                self.results.append(result)
        
        # OpenCL BFS on file-based graphs
        for graph_file in graph_files:
            result = self.run_algorithm_benchmark("bfs_opencl", graph_file, num_runs=num_runs)
            self.results.append(result)
        
        # OpenCL BFS on generated graphs
        for config in generated_configs:
            scale, degree, gen_type, edge_type, vertex_type, graph_type = config
            result = self.run_algorithm_on_generated_graph("bfs_opencl", scale, degree, gen_type,
                                                          edge_type, vertex_type, graph_type,
                                                          num_runs=num_runs)
            self.results.append(result)
    
    def run_triangle_counting_benchmarks(self, graph_files: List[str], generated_configs: List[Tuple], num_runs: int = 3):
        """Run Triangle Counting benchmarks with different implementations"""
        print("\n=== Triangle Counting Benchmarks ===")
        
        # Sequential Triangle Counting on file-based graphs
        for graph_file in graph_files:
            result = self.run_algorithm_benchmark("tc", graph_file, num_runs=num_runs)
            self.results.append(result)
        
        # Sequential Triangle Counting on generated graphs
        for config in generated_configs:
            scale, degree, gen_type, edge_type, vertex_type, graph_type = config
            result = self.run_algorithm_on_generated_graph("tc", scale, degree, gen_type,
                                                          edge_type, vertex_type, graph_type,
                                                          num_runs=num_runs)
            self.results.append(result)
        
        # Multithreaded Triangle Counting with different thread counts on file-based graphs
        thread_counts = [2, 4, 8, 16]
        for graph_file in graph_files:
            for num_threads in thread_counts:
                result = self.run_algorithm_benchmark("tc_threaded", graph_file, 
                                                    num_threads, num_runs)
                self.results.append(result)
        
        # Multithreaded Triangle Counting with different thread counts on generated graphs
        for config in generated_configs:
            scale, degree, gen_type, edge_type, vertex_type, graph_type = config
            for num_threads in thread_counts:
                result = self.run_algorithm_on_generated_graph("tc_threaded", scale, degree, gen_type,
                                                              edge_type, vertex_type, graph_type,
                                                              num_threads, num_runs)
                self.results.append(result)
        
        # OpenCL Triangle Counting on file-based graphs
        for graph_file in graph_files:
            result = self.run_algorithm_benchmark("tc_opencl", graph_file, num_runs=num_runs)
            self.results.append(result)
        
        # OpenCL Triangle Counting on generated graphs
        for config in generated_configs:
            scale, degree, gen_type, edge_type, vertex_type, graph_type = config
            result = self.run_algorithm_on_generated_graph("tc_opencl", scale, degree, gen_type,
                                                          edge_type, vertex_type, graph_type,
                                                          num_runs=num_runs)
            self.results.append(result)
    
    def save_results(self, output_file: str = "algorithm_benchmark_results.json"):
        """Save benchmark results to JSON file"""
        output_path = SCRIPT_DIR / output_file
        with open(output_path, 'w') as f:
            json.dump(self.results, f, indent=2)
        print(f"\nResults saved to: {output_path}")
    
    def print_summary(self):
        """Print a summary of benchmark results"""
        print("\n=== Benchmark Summary ===")
        
        # Group results by algorithm
        by_algorithm = {}
        for result in self.results:
            alg = result["algorithm"]
            if alg not in by_algorithm:
                by_algorithm[alg] = []
            by_algorithm[alg].append(result)
        
        for alg, results in by_algorithm.items():
            print(f"\n{alg.upper()}:")
            for result in results:
                if result["mean_time"] != float('inf'):
                    threads = f" ({result['num_threads']} threads)" if result['num_threads'] else ""
                    
                    # Handle both file-based and generated graph results
                    if "graph_file" in result:
                        # File-based graph
                        print(f"  {result['graph_file']}{threads}: {result['mean_time']:.3f}s ± {result['std_time']:.3f}s")
                    elif "graph_name" in result:
                        # Generated graph
                        print(f"  {result['graph_name']}{threads}: {result['mean_time']:.3f}s ± {result['std_time']:.3f}s")
                else:
                    if "graph_file" in result:
                        print(f"  {result['graph_file']}: FAILED")
                    elif "graph_name" in result:
                        print(f"  {result['graph_name']}: FAILED")

def main():
    parser = argparse.ArgumentParser(description="Run EdgeLab algorithm benchmarks")
    parser.add_argument("--build-dir", help="Build directory (default: ../build)")
    parser.add_argument("--num-runs", type=int, default=3, help="Number of runs per benchmark")
    parser.add_argument("--output", default="algorithm_benchmark_results.json", help="Output file")
    parser.add_argument("--test-configs", nargs="+", help="Custom test configurations (overrides defaults)")
    parser.add_argument("--use-user-graphs", action="store_true", help="Include user-provided graph files")
    
    args = parser.parse_args()
    
    try:
        runner = BenchmarkRunner(args.build_dir)
    except FileNotFoundError as e:
        print(f"Error: {e}")
        print(f"Please ensure the build directory exists and contains the compiled executables.")
        return
    
    # Determine test configurations
    if args.test_configs:
        # Parse custom test configurations
        test_configs = []
        for config_str in args.test_configs:
            if config_str.startswith("(") and config_str.endswith(")"):
                # Parse tuple format
                try:
                    config = eval(config_str)
                    test_configs.append(config)
                except:
                    print(f"Warning: Could not parse configuration: {config_str}")
            else:
                # Treat as filepath
                test_configs.append(config_str)
    else:
        test_configs = DEFAULT_TEST_CONFIGS.copy()
    
    # Add user graph files if requested
    if args.use_user_graphs:
        test_configs.extend(USER_GRAPH_FILES)
    
    if not test_configs:
        print("No test configurations provided.")
        return
    
    # Get graph files from configurations
    graph_files = runner.get_graph_files(test_configs)
    
    # Get generated graph configurations
    generated_configs = runner.get_generated_graph_configs(test_configs)
    
    if not graph_files and not generated_configs:
        print("No valid graph files or generated graph configurations found.")
        return
    
    if graph_files:
        print(f"Testing with graph files: {graph_files}")
    
    if generated_configs:
        print(f"Testing with generated graphs: {len(generated_configs)} configurations")
        for config in generated_configs:
            scale, degree, gen_type, edge_type, vertex_type, graph_type = config
            num_vertices = 2 ** scale
            num_edges = num_vertices * degree
            print(f"  - scale={scale} (2^{scale}={num_vertices} vertices), degree={degree} ({num_edges} edges)")
            print(f"    Type: {gen_type}, Edge: {edge_type}, Vertex: {vertex_type}, Graph: {graph_type}")
    
    # Run benchmarks
    runner.run_bfs_benchmarks(graph_files, generated_configs, args.num_runs)
    runner.run_triangle_counting_benchmarks(graph_files, generated_configs, args.num_runs)
    
    # Save and print results
    runner.save_results(args.output)
    runner.print_summary()

if __name__ == "__main__":
    main() 