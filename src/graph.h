#ifndef GRAPH_H_
#define GRAPH_H_

#include <util.h>
#include <vector>
#include <optional>


template<typename Adjacency_t>
class Graph {
public:
    Graph(Adjacency_t* edges) : edges_(edges) {

    }
private:
    const Adjacency_t* edges_;
}

// first load graph with each node having a vector of neighbors
// then condense into CSR format

// offsetVector: vector of pairs {neightOffset, numNeighbors}
// neighborVector: vector of neighbors




