#include "util.h"

// timer for benchmarking
std::chrono::steady_clock::time_point timer_start() { 
    return std::chrono::steady_clock::now(); 
}

double timer_stop(const std::chrono::steady_clock::time_point &start) {
    auto end = std::chrono::steady_clock::now(); // Capture end time
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
}

// specification of graph types
std::ostream& operator<<(std::ostream& os, GraphType Graph_t) {
    switch (Graph_t) {
        case GraphType::UNDIRECTED:
            os << "Undirected";
            break;
        case GraphType::DIRECTED:
            os << "Directed";
            break;
        default:
            os << "Unknown Graph Type";
            break;
    }
    return os;
}

// specification of generation types
std::ostream& operator<<(std::ostream& os, GenType Gen_t) {
    switch (Gen_t) {
        case GenType::ERDOS_RENYI:
            os << "Erdos-Renyi";
            break;
        case GenType::WATTS_STROGATZ:
            os << "Watts-Strogatz";
            break;
        case GenType::BARABASI_ALBERT:
            os << "Barabasi-Albert";
            break;
        default:
            os << "Unknown Generation Type";
            break;
    }
    return os;
} 