# EdgeLab

A high-performance C++20 graph library for efficient graph generation, manipulation, and analysis, supporting both directed and undirected graphs, weighted/unweighted vertices and edges, and custom data storage.

---

## 🚀 Features

- **Flexible Graph Types:** Directed and undirected graphs
- **Weighted & Unweighted Components:** Vertices and edges can be weighted or unweighted
- **Custom Data Storage:** Extensible data storage for vertices and edges
- **Multiple Graph Generators:** Built-in generators for common graph types
- **Efficient CSR Format:** Compressed Sparse Row (CSR) representation for memory efficiency
- **Modern C++20:** Concepts, templates, and other modern C++ features
- **Type Safety:** Strong type system with compile-time guarantees
- **CLI & Automated Testing:** Command-line interface for graph configuration and GitHub Actions for CI

---

## 📁 Project Structure

```
EdgeLab/
├── src/
│   ├── util.h
│   ├── graph_comp.h
│   ├── generator.h
│   ├── builder.h
│   ├── graph.cpp
│   ├── cli.h
│   └── cli_dispatch.h
├── test/
│   ├── iteration.cpp
│   └── verify_undirected.cpp
├── examples/
│   ├── triangle_counting.cpp
│   └── breadth_first_search.cpp
├── .github/
│   └── workflows/
│       └── cmake.yml
├── CMakeLists.txt
└── README.MD
```

---

## 🏗️ Architecture & API

### Core Graph API

- **Graph**: Now exposes a `VertexRef` type for user access, which is directly iterable for edge traversal.
- **VertexRef**: Used for all vertex access and iteration; edges are accessed directly via range-based for loops.
- **Graph Printing**: The graph can be printed directly to a stream using the `<<` operator.
- **No `.edges()` method**: Edges are accessed directly from `VertexRef` (e.g., `for (auto& edge : graph[u_id])`).

### CLI & Dispatch

- **CLI (src/cli.h):** Command-line interface for specifying graph type, vertex/edge type, scale, degree, and generator type.
- **Dispatching (src/cli_dispatch.h):** Generic dispatch system for instantiating template classes (e.g., `Tester`, BFS, triangle counting) based on CLI options.

### Algorithms

- **Triangle Counting:** Efficient triangle counting using neighbor intersection, with correct handling for undirected graphs.
- **Breadth-First Search:** BFS implemented using the new `VertexRef` API, returning the shortest path distance or throwing if unreachable.

### Automated Testing

- **GitHub Actions:** Workflow in `.github/workflows/cmake.yml` builds the project with CMake, fails on warnings, and runs all test binaries.
- **CMake:** Builds all test and example executables.

---

## 🛠️ Building and Usage

### Prerequisites

- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- CMake 3.16+ (for build system)

### Build & Run

```bash
mkdir build
cd build
cmake ..
cmake --build .
# Run any of the compiled binaries, e.g.:
./test_program
./verify_undirected
```

### Command-Line Interface

You can configure and run graph generation and algorithms via CLI:

```bash
./verify_undirected --graph-type undirected --vertex-type unweighted --edge-type unweighted --scale 10 --degree 3 --gen-type erdos_renyi
```

---

## 🔧 Advanced Usage

### Custom Data Types

```cpp
struct VertexData { std::string name; int priority; };
struct EdgeData { std::string label; double reliability; };
using CustomGraph = Graph<VertexWD<VertexData>, EdgeWD<EdgeData>, GraphType::DIRECTED>;
```

### Graph Traversal

```cpp
for (vertex_ID_t u = 0; u < graph.num_vertices(); ++u) {
    for (const auto& edge : graph[u]) {
        // edge.dest(), edge.weight(), etc.
    }
}
```

---

## 🧪 Testing & CI

- **Automated CI:** GitHub Actions workflow builds and tests all binaries, fails on warnings, and checks exit codes.
- **Test Programs:** `iteration.cpp`, `verify_undirected.cpp`, and algorithm examples.

---

## 🚧 Current Limitations

- Move assignment operators are not supported for edge/vertex classes with `const` members.
- Some graph generation algorithms (e.g., Watts-Strogatz) are planned but not yet implemented.
- Only unweighted/weighted (not data-carrying) types are fully supported in the CLI dispatch system.

---

## 🔮 Future Enhancements

- [ ] Additional graph generation algorithms
- [ ] Graph analysis algorithms (shortest paths, connectivity, etc.)
- [ ] Graph I/O formats (GraphML, GEXF, etc.)
- [ ] Parallel graph processing
- [ ] Memory pool optimization for large graphs

---

## 🤝 Contributing

1. Fork the repository.
2. Create a feature branch.
3. Make your changes.
4. Add tests for new functionality.
5. Submit a pull request.

---

## 📄 License

[Add your license information here]

---

## 📞 Support

For questions, issues, or contributions, please open an issue on the project repository.

---

**EdgeLab** - Efficient graph generation and manipulation in modern C++

