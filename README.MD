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

## Graph Types

EdgeLab supports a comprehensive set of graph types through flexible templating. The library provides different vertex and edge types that can be combined to create various graph configurations.

### Vertex Types

- **VertexUW (Unweighted):** Basic vertex with no additional data
    - Default weight: `1`
    - Memory efficient for simple graphs

- **VertexW (Weighted):** Vertex with floating-point weight
    - Custom weight value
    - Useful for algorithms that consider vertex importance

### Edge Types

- **EdgeUW (Unweighted):** Basic edge with destination vertex only
    - Default weight: `1`
    - Memory efficient for simple graphs

- **EdgeW (Weighted):** Edge with floating-point weight
    - Custom weight value
    - Essential for weighted graph algorithms

### Graph Direction Types

- **GraphType::DIRECTED:** Directed graph where edges have direction
    - Edge (u,v) exists independently of edge (v,u)
    - Useful for modeling asymmetric relationships

- **GraphType::UNDIRECTED:** Undirected graph where edges are bidirectional
    - Edge (u,v) implies edge (v,u) exists
    - Automatically symmetrized during graph construction
    - Useful for modeling symmetric relationships

### Graph File Formats

EdgeLab supports multiple file formats for loading and saving graphs.

- **Example Graph:** To aid in explaining file formats, the "mini" graph is provided as an example
    - Each example file encoding represents a variation of this graph
        - (potentially dropping vertex weights, edge weights, and/or directedness)
    - A complete list of "mini" encoding variations can be found in the graphs/ directory
```
[ ID: 0 ]        wt: 1      [ ID: 1 ]
[ wt: 1 ]  -------------->  [ wt: 6 ]
                          
   ^      \               /    |
   |       \wt: 2   wt: 1/     |
   |        \           /      |
   |         \         /       |
   |          \       /        |
   |           \     /         |
   |            \   /          |
   |             \ /           |
   |wt: 3         X            |wt: 5
   |             / \           |
   |            /   \          |
   |           /     \         |
   |          /       \        |
   |         /         \       |
   |        /           \      |
   |       /             \     |
   |      V               V    V
                          
[ ID: 3 ]        wt: 2      [ ID: 2 ]
[ wt: 4 ]  <--------------  [ wt: 3 ]
```

- **EL (Edge List):** Plain text edge list format
    - Unweighted edges and vertices
    - Defaults to undirected, but can be set to directed via comment on the first line
    - Edges are formatted as: `<source> <destination>`
    - Vertices must be unsigned 32-bit integers.
    - Vertices must be (mostly) densely packed (gaps are included as vertices with no edges)
    - Edges can come in any order and duplicate edges are ok
```markdown
# directed
0 1
0 2
1 2
1 3
2 3
3 0
```

- **WEL (Weighted Edge List):** Plain text weighted edge list format
    - Weighted edges, unweighted vertices
    - Edges are formatted as: `<source> <destination> <weight>`
    - Weights must be unsigned 32-bit integers.
    - If duplicate edges with differing weights are listed, the smallest weight will be used
```markdown
# directed
0 1 1
0 2 2
1 2 5
1 3 1
2 3 2
3 0 3
```

- **VEL (Edge List with Vertex Weights):** Plain text format with vertex weights
    - Unweighted edges, weighted vertices
    - Vertex weights are added as lines formatted as: `v <vertex> <weight>`
    - Vertex weights can be interleaved with edges
    - Vertices without a defined weight will default to `1`
```markdown
# directed
v 0 1
0 1
0 2
v 1 6
1 2
1 3
2 3
3 0
v 2 3

```

- **VWEL (Weighted Edge List with Vertex Weights):** Plain text format with both weights
    - Weighted edges and vertices
```markdown
# directed
v 0 1
v 1 6
v 2 3
v 3 1
0 1 1
0 2 2
1 2 5
1 3 1
2 3 2
3 0 3
```

- **GRAPH (METIS Graph):** METIS graph format
    - Supports weighted/unweighted edges and vertices
    - All graphs are assumed directed unless fully symmetrized (with symmetric edge weights)
        - Directedness is checked and set as final step in loading
    - Standard format for graph partitioning
    - Vertex IDs must be positive integers (excluding 0)
        - When loaded, vertex IDs will be shifted down by 1
    - Header line formatted as: `<num vertices> <num edges> <flag>?`
    - Flag is an optional parameter that defines if vertices/edges are weighted
    - Flag consists of 2 binary digits `<v_weighted><e_weighted>`
    - The body is written in sparse row format with each line representing a vertex (1-based)
    - Each line consists of space seperated unsigned integers
    - For weighted vertices, the first number of each line is the vertex weight
    - Otherwise, lines are a list of outgoing edges (any order is allowed)
    - For weighted edges, each edge is of the form `<weight> <destination>`
    - Comments can be added via `%` (must be the first character on the line)
    - EdgeLab is lenient when parsing GRAPH files to allow for easier readability/editing:
        - It recognizes header lines formatted as `<flag>?` (without specifying vertex/edge counts)
            - To easily add/remove components by hand (at the cost of additional loading time)
        - It recognizes flags consisting of 3 binary digits: `<v_weighted><e_weighted><undirected>`
            - Avoids the need to manually symmetrize every edge for undirected graphs
        - It allows duplicate edges
            - If duplicate edges with differing weights are listed, the smallest weight will be used
        - Caution: these features will likely break cross platform compatability when used
```tex
% header line for: 4 vertices, 6 edges, weighted vertices and edges
4 6 11
% vertexID: 1
1 1 2 2 3
% vertexID: 2, weight: 6, edges: [[weight: 5, dest: 3], [weight: 1, dest: 4]]
6 5 3 1 4
% vertexID: 3
3 2 4
% vertexID: 3
4 3 1
```

- **ELAB (EdgeLab Binary):** Custom binary format
    - Supports all graph types (directed/undirected, weighted/unweighted)
    - Fast loading and saving with small file sizes (especially for large sparse graphs)
    - optimized to use 50% less space for undirected graphs vs symmetrized directed graphs
    - Platform-independent binary format
    - all numerical values are unsigned integer types (for well defined bit placement)
    - 16 byte Header formatted as: `<flags>(4 bytes) <num vertices>(4 bytes) <num edges>(8 bytes)`
        - Flag bytes specify the booleans: `<directed> <v weighted> <e weighted> <unused>`
        - The unused flag alligns the file's integers on a 4-byte boundary (for maximum performance)
    - Vertex info (repeated `<num vertices>` times):
        - Format: `<weight>(4 bytes)? <degree (num outgoing edges)>(4 bytes)`
    - Edges info (repeated `<num edges>` times):
        - Format: `<weight>(4 bytes)? <destination>(4 bytes)`
        - Edges are sorted in order of source ID (implied via vertex info) 
        - For each vertex, edges are sorted by strictly increasing destination ID
        - For undirected graphs, only edges where `src < dest` are stored in ELAB files
            - These edges will be symmetrized in place during graph construction
            - ELAB `<num edges>` and `<degree>` values represent directed edge counts **after symmetrization**
            - Self loops are not possible for directed graphs
```c
// ELAB header data (16 bytes)
0x00 0x01 0x01 0x00   // 4 x 8-bit flags for undirected graph with vertex and edge weights
0x00000004            // 32-bit vertex count for 4 vertices
0x0000000000000006    // 64-bit edge count for 6 edges
// ELAB vertex data
0x00000001 0x00000003 // Vertex ID=0, weight=1, degree=3
0x00000006 0x00000003
0x00000003 0x00000003
0x00000004 0x00000003
// ELAB edge data
0x00000001 0x00000001   0x00000002 0x00000002   0x00000003 0x00000003
0x00000005 0x00000002   0x00000001 0x00000003
0x00000002 0x00000003 // Vertex 2 has 1 undirected edge (weight=2) to vertex 3
                      // Its edges to vertex 0 and 1 are not included since !(src < dest)
// File size for this graph is 96 bytes
```

### Example Graph Type Combinations

```cpp
// Simple unweighted undirected graph
using SimpleGraph = Graph<VertexUW, EdgeUW, GraphType::UNDIRECTED>;

// Weighted directed graph
using WeightedGraph = Graph<VertexW, EdgeW, GraphType::DIRECTED>;

// Graph with custom vertex and edge data
struct VertexData { std::string label; int category; };
struct EdgeData { double reliability; int timestamp; };
using CustomGraph = Graph<VertexWD<VertexData>, EdgeWD<EdgeData>, GraphType::DIRECTED>;

// Memory-efficient unweighted graph
using EfficientGraph = Graph<VertexUW, EdgeUW, GraphType::DIRECTED>;
```

### Type Concepts and Constraints

EdgeLab uses C++20 concepts to ensure type safety at compile time:

- **VertexType:** Ensures vertex types derive from `VertexUW`
- **EdgeType:** Ensures edge types derive from `EdgeUW`
- **WeightedVertexType:** Identifies vertex types with weights
- **WeightedEdgeType:** Identifies edge types with weights
- **DataVertexType:** Identifies vertex types with custom data
- **DataEdgeType:** Identifies edge types with custom data

These concepts enable compile-time validation and optimization based on the specific graph type being used.

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

### Running Tests

```bash
# Run all tests (from build directory)
./triangle_counting_test
./breadth_first_search_test
./verify_undirected --edge-type unweighted --scale 8 --degree 8 --gen-type erdos_renyi

# Run load/save tests (Windows PowerShell)
powershell -ExecutionPolicy Bypass -File ../test/test_load_save.ps1

# Run load/save tests (Linux/macOS)
./test/test_load_save.sh
```

The load/save tests verify that graphs can be saved and loaded correctly across all combinations of:
- Graph types: directed/undirected
- Vertex types: weighted/unweighted
- Edge types: weighted/unweighted  
- File formats: EL/WEL/VEL/VWEL (text) and ELAB (binary)

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
- **Load/Save Testing:** Comprehensive testing of graph I/O functionality across all combinations of graph types (directed/undirected), vertex types (weighted/unweighted), edge types (weighted/unweighted), and file formats (EL text format and ELAB binary format).

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

