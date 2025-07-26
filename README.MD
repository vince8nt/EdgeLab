# EdgeLab

A high-performance C++20 graph library for efficient graph loading, generation, and analysis. Supports flexible templated graph types, multithreading, and OpenCL for parallelism.

---

## Features

- **Flexible Graph Types:** Directed/undirected graphs with weighted/unweighted vertices/edges
- **Custom Data Storage:** Extensible data storage for vertices and edges
- **Multiple Graph Generators:** Built-in generators for common graph types
- **Graph File Loading and Saving:** Support for multiple graph file formats
- **Efficient CSR Format:** Compressed Sparse Row (CSR) representation for memory efficiency
- **Multithreading:** Graph-specific thread-safe data structures with multithreaded example programs
- **OpenCL GPU Acceleration:** CSR interface for OpenCL with high-performance OpenCL example programs
- **Modern C++20:** Concepts, templates, and other modern C++ features
- **Type Safety:** Strong type system with compile-time guarantees
- **Command Line Interface (CLI):** CLI for graph configuration and dispatching of templated algorithms
- **Automated Testing (CI):** GitHub Actions for CI to ensure all EdgeLab features are working
- **Cross-Platform Build System:** Optional OpenCL support with graceful fallback

---

## Core Graph API

- **Graph:** Efficient CSR format graph.
    - **num_vertices():** Returns the number of vertices (densely packed) in the graph.
    - **num_edges():** Returns the number of edges in the graph (accounts for directed/undirected).
    - **Iteration/Indexing:** Exposes a `VertexRef` type for access via iteration and/or indexing.
    - **Printing:** The graph can be printed to a stream for debugging using the `<<` operator.
- **VertexRef:** An 8-byte struct for efficient passing by value.
    - **num_edges():** The number of outgoing edges from this vertex.
    - **Iteration/Indexing:** Exposes an `EdgeType` for access via iteration and/or indexing.
    - **has_edge_to(vertex):** Determines if the directed edge exists in O(log num_edges).
    - **edge_to(vertex):** Returns an `EdgeType` iterator or `end()` in O(log num_edges).
    - **weight():** Returns the weight or `1.0` for graphs with unweighted vertices.
    - **data():** Returns the optional custom data associated with the given vertex.
- **EdgeType:**
    - **weight():** Returns the weight or `1.0` for graphs with unweighted edges.
    - **data():** Returns the optional custom data associated with the given edge.

---

### CLI & Dispatch

- **CLI (src/cli.h):** Command-line interface for specifying generator options (graph type, vertex/edge type, scale, degree, generator type) or file loading options.
- **Dispatching (src/cli_dispatch.h):** Generic dispatch system for instantiating templated functions via CLI options. This allows users to write graph algorithms generalized for any combination of Vertex/Edge/Graph type, without any runtime overhead.

---

## Building and Usage

### Prerequisites

- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- CMake 3.16+ (for build system)
- OpenCL SDK (optional, for GPU acceleration)

### Build & Run

```bash
mkdir build
cd build
cmake ..
cmake --build .
# generate graph and run algorithm
./verify_undirected --graph-type undirected --vertex-type unweighted --edge-type unweighted --scale 20 --degree 5 --gen-type erdos_renyi

# load graph from file and run algorithm
./bfs_opencl --load-graph ../graphs/mini.el
```

**Note:** OpenCL support is optional. If OpenCL is not found, the build will succeed but the algorithms in `examples_opencl` will not be compiled. The system provides graceful fallback for environments without OpenCL support.

---

### Custom Data Types

Template support for storing Vertex/Edge data within the CLI structure for improved cache locality:

```cpp
struct VertexData { short count; int priority; };
struct EdgeData { int traversals; double reliability; };
using CustomGraph = Graph<VertexWD<VertexData>, EdgeWD<EdgeData>, GraphType::DIRECTED>;
```

---

### Graph Traversal

```cpp
// Traverse via indexing
for (vertex_ID_t u = 0; u < graph.num_vertices(); ++u) {
    for (vertex_ID_t i = 0; i < graph[v].size(); i++) {
        // graph[v][i].dest(), graph[v][i].weight(), etc.
    }
}
```

```cpp
// Traverse via iteration
for (auto vertexRef : graph) {
    for (const auto& edge : vertexRef) {
        // edge.dest(), edge.weight(), etc.
    }
}
```

---

### OpenCL GPU Acceleration

The library includes GPU-accelerated algorithms using OpenCL:

```cpp
// OpenCL BFS implementation
template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
long long breadth_first_search_opencl(Graph<Vertex_t, Edge_t, Graph_t>& graph, 
                                     vertex_ID_t src, vertex_ID_t dest);
```

**Features:**
- Automatic CSR format conversion for GPU memory
- Level-synchronous BFS with parallel edge exploration
- Device information and work group size optimization
- Graceful error handling and resource cleanup

---

## Testing & CI

- **Automated CI:** GitHub Actions workflow builds and tests all binaries, fails on warnings, and checks exit codes.
- **Cross-Platform Support:** Build system gracefully handles systems with and without OpenCL support.
- **OpenCL Testing:** GPU-accelerated algorithms are tested when OpenCL is available.

---

## Current Limitations

- Some graph generation algorithms (e.g., Watts-Strogatz) are planned but not yet implemented.
- Only unweighted/weighted (not data-carrying) types are fully supported in the CLI dispatch system.
- OpenCL BFS currently supports unweighted graphs only (weighted edge support planned).
- GPU memory management is currently not flexible for all combinations of Vertex/Edge/Graph type.

---

## Future Enhancements

- [ ] Additional graph generation algorithms
- [ ] More graph analysis algorithms (shortest paths, connectivity, etc.)
- [ ] More graph I/O formats (GraphML, GEXF, etc.)
- [ ] Memory pool optimization for large graphs
- [ ] Additional OpenCL algorithms (SSSP, PageRank, etc.)
- [ ] Weighted edge support for OpenCL BFS
- [ ] Multi-GPU support for distributed graph processing
- [ ] CUDA backend as alternative to OpenCL

---

## Contributing

1. Fork the repository.
2. Create a feature branch.
3. Make your changes.
4. Add tests for new functionality.
5. Submit a pull request.

---

## License

MIT License

Copyright (c) 2025 Vincent Titterton

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---

## Support

For questions, issues, or contributions, please open an issue on the project repository.

---

**EdgeLab** – Efficient graph generation and manipulation in modern C++

