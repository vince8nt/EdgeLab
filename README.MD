# EdgeLab

A high-performance C++20 graph library for efficient graph generation, manipulation, and analysis with support for both directed and undirected graphs, weighted/unweighted vertices and edges, and custom data storage.

## 🚀 Features

- **Flexible Graph Types**: Support for both directed and undirected graphs
- **Weighted & Unweighted Components**: Vertices and edges can be weighted or unweighted
- **Custom Data Storage**: Extensible data storage for vertices and edges
- **Multiple Graph Generators**: Built-in generators for common graph types
- **Efficient CSR Format**: Compressed Sparse Row (CSR) representation for memory efficiency
- **Modern C++20**: Uses concepts, templates, and other modern C++ features
- **Type Safety**: Strong type system with compile-time guarantees

## 📁 Project Structure

```
EdgeLab/
├── src/
│   ├── util.h          # Core type definitions and utilities
│   ├── graph_comp.h    # Graph components (vertices, edges, concepts)
│   ├── generator.h     # Graph generation algorithms
│   ├── builder.h       # Graph construction and CSR conversion
│   ├── graph.cpp       # Graph class implementation
│   └── debug.h         # Debugging and visualization utilities
├── test/
│   └── test.cpp        # Example usage and testing
└── README.MD           # This file
```

## 🏗️ Architecture

### Core Components

#### Vertex Types
- `VertexUW` - Unweighted vertex with no data
- `VertexW` - Weighted vertex
- `VertexUWD<Data_t>` - Unweighted vertex with custom data
- `VertexWD<Data_t>` - Weighted vertex with custom data

#### Edge Types
- `EdgeUW` - Unweighted edge
- `EdgeW` - Weighted edge
- `EdgeUWD<Data_t>` - Unweighted edge with custom data
- `EdgeWD<Data_t>` - Weighted edge with custom data

#### Graph Types
- `GraphType::DIRECTED` - Directed graph
- `GraphType::UNDIRECTED` - Undirected graph

### Graph Generation

Currently supports:
- **Erdős-Rényi** - Random graph generation
- **Watts-Strogatz** - Small-world network generation (planned)
- **Barabási-Albert** - Scale-free network generation (planned)

## 🛠️ Building and Usage

### Prerequisites
- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- CMake 3.16+ (optional, for build system)

### Quick Start

```bash
# Compile the test program
g++ -std=c++20 -I. test/test.cpp -o test_program

# Run the example
./test_program
```

### Basic Usage

```cpp
#include "src/generator.h"
#include "src/builder.h"
#include "src/debug.h"

// Create a weighted undirected graph
using MyGraph = Graph<VertexW, EdgeW, GraphType::UNDIRECTED>;

// Generate a random graph
Generator<VertexW, EdgeW, GraphType::UNDIRECTED> generator(
    GenType::ERDOS_RENYI,  // Graph type
    4,                     // Scale (2^4 = 16 vertices)
    2                      // Average degree
);

VectorGraph<VertexW, EdgeW> vg = generator.Generate();

// Build the final graph in CSR format
Builder<VertexW, EdgeW, GraphType::UNDIRECTED> builder;
MyGraph graph = builder.BuildGraph(vg);
```

## 📊 Graph Representations

### Vector Graph (Input Format)
- Adjacency matrix representation
- Suitable for graph generation and loading
- Supports dynamic edge addition

### CSR Graph (Output Format)
- Compressed Sparse Row format
- Memory efficient for large sparse graphs
- Fast neighbor iteration
- Immutable after construction

## 🔧 Advanced Usage

### Custom Data Types

```cpp
// Define custom vertex data
struct VertexData {
    std::string name;
    int priority;
};

// Define custom edge data
struct EdgeData {
    std::string label;
    double reliability;
};

// Use with custom data
using CustomGraph = Graph<VertexWD<VertexData>, EdgeWD<EdgeData>, GraphType::DIRECTED>;
```

### Graph Traversal

```cpp
// Iterate over vertices
for (const auto& vertex : graph) {
    std::cout << "Vertex degree: " << vertex.degree() << std::endl;
    
    // Iterate over edges
    for (const auto& edge : vertex.edges()) {
        std::cout << "Edge to: " << edge.dest() 
                  << " Weight: " << edge.weight() << std::endl;
    }
}
```

### Undirected Graph Handling

For undirected graphs, the library enforces a specific convention:
- Only add edges where `src <= dest`
- The builder automatically adds inverse edges
- This ensures consistent weight handling for weighted graphs

## 🧪 Testing

The test suite demonstrates various graph configurations:

```cpp
// Test different vertex/edge combinations
Tester<VertexUW, EdgeW, GraphType::UNDIRECTED> tester1;  // Unweighted vertices, weighted edges
Tester<VertexW, EdgeUW, GraphType::DIRECTED> tester2;    // Weighted vertices, unweighted edges
Tester<VertexW, EdgeW, GraphType::UNDIRECTED> tester3;   // Both weighted
```

## 🔍 Debugging and Visualization

The `Debug` class provides utilities for graph inspection:

```cpp
Debug<VertexW, EdgeW> debug;

// Print adjacency matrix representation
debug.print(vg);

// Print CSR graph representation
debug.print_graph(graph);

// Print with iterators
debug.print_graph_it(graph);
```

## 🚧 Current Limitations

- Move assignment operators are not supported for edge/vertex classes with const members
- Some graph generation algorithms are planned but not yet implemented
- Limited graph analysis algorithms (focus is on generation and representation)

## 🔮 Future Enhancements

- [ ] Additional graph generation algorithms
- [ ] Graph analysis algorithms (shortest paths, connectivity, etc.)
- [ ] Graph I/O formats (GraphML, GEXF, etc.)
- [ ] Parallel graph processing
- [ ] Memory pool optimization for large graphs

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Submit a pull request

## 📄 License

[Add your license information here]

## 📞 Support

For questions, issues, or contributions, please open an issue on the project repository.

---

**EdgeLab** - Efficient graph generation and manipulation in modern C++

