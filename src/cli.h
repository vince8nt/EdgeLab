#ifndef CLI_H_
#define CLI_H_

#include <string>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <utility> // For std::forward
#include "util.h"
#include "graph_comp.h"
#include "generator.h" // for GenType
#include "graph_maker.h"


inline std::string to_lower(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return std::tolower(c); });
    return out;
}

inline void print_usage(const char* prog_name) {
    std::cout << "Usage: " << prog_name << " [options]\n";
    std::cout << "\nOptions:\n";
    std::cout << "  --load-file <path>                      (mutually exclusive with all other options)\n";
    std::cout << "  --save-file <path>                      (optional, save graph to file)\n";
    std::cout << "  --graph-type <d|u>                      (default: d)\n";
    std::cout << "      d = directed, u = undirected\n";
    std::cout << "  --vertex-type <w|uw>                    (default: w)\n";
    std::cout << "  --edge-type <w|uw>                      (default: w)\n";
    std::cout << "      w = weighted, uw = unweighted\n";
    std::cout << "  --scale <int>                           (required for generation)\n";
    std::cout << "  --degree <int>                          (required for generation)\n";
    std::cout << "  --gen-type <er|ws|ba>                   (required for generation)\n";
    std::cout << "      er = erdos_renyi, ws = watts_strogatz, ba = barabasi_albert\n";
    std::cout << std::endl;
}

inline bool parse_enum(const std::string& value, GraphType& out) {
    std::string v = to_lower(value);
    if (v == "u") { out = GraphType::UNDIRECTED; return true; }
    if (v == "d")   { out = GraphType::DIRECTED;   return true; }
    return false;
}

inline bool parse_enum(const std::string& value, CLIVertexType& out) {
    std::string v = to_lower(value);
    if (v == "uw")        { out = CLIVertexType::UNWEIGHTED;        return true; }
    if (v == "w")          { out = CLIVertexType::WEIGHTED;          return true; }
    return false;
}

inline bool parse_enum(const std::string& value, CLIEdgeType& out) {
    std::string v = to_lower(value);
    if (v == "uw")        { out = CLIEdgeType::UNWEIGHTED;        return true; }
    if (v == "w")          { out = CLIEdgeType::WEIGHTED;          return true; }
    return false;
}

inline bool parse_enum(const std::string& value, GenType& out) {
    std::string v = to_lower(value);
    if (v == "er")       { out = GenType::ERDOS_RENYI;       return true; }
    if (v == "ws")    { out = GenType::WATTS_STROGATZ;    return true; }
    if (v == "ba")   { out = GenType::BARABASI_ALBERT;   return true; }
    return false;
}

// Parse CLI arguments and return CLIOptions. Prints usage and exits on error.
inline CLIOptions parse_cli(int argc, char** argv) {
    CLIOptions opts{};
    bool got_scale = false, got_degree = false, got_gen_type = false;
    bool got_load_file = false;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--load-file" && i+1 < argc) {
            if (got_load_file) {
                std::cerr << "Duplicate --load-file option." << std::endl;
                print_usage(argv[0]);
                exit(1);
            }
            opts.load_file_path = argv[++i];
            got_load_file = true;
        }
    }
    
    if (got_load_file) {
        // Only --load-file, --save-file, --graph-type, and program name are allowed
        if (argc > 6) {
            std::cerr << "--load-file is mutually exclusive with generation options." << std::endl;
            print_usage(argv[0]);
            exit(1);
        }
        // Check for --save-file and --graph-type options
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--save-file" && i+1 < argc) {
                opts.save_file_path = argv[++i];
            } else if (arg == "--graph-type" && i+1 < argc) {
                if (!parse_enum(argv[++i], opts.graph_type)) {
                    std::cerr << "Invalid graph type: " << argv[i] << std::endl;
                    print_usage(argv[0]);
                    exit(1);
                }
            }
        }
        return opts;
    }
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--graph-type" && i+1 < argc) {
            if (!parse_enum(argv[++i], opts.graph_type)) {
                std::cerr << "Invalid graph type: " << argv[i] << std::endl;
                print_usage(argv[0]);
                exit(1);
            }
        } else if (arg == "--vertex-type" && i+1 < argc) {
            if (!parse_enum(argv[++i], opts.vertex_type)) {
                std::cerr << "Invalid vertex type: " << argv[i] << std::endl;
                print_usage(argv[0]);
                exit(1);
            }
        } else if (arg == "--edge-type" && i+1 < argc) {
            if (!parse_enum(argv[++i], opts.edge_type)) {
                std::cerr << "Invalid edge type: " << argv[i] << std::endl;
                print_usage(argv[0]);
                exit(1);
            }
        } else if (arg == "--scale" && i+1 < argc) {
            opts.scale = std::stoi(argv[++i]);
            got_scale = true;
        } else if (arg == "--degree" && i+1 < argc) {
            opts.degree = std::stoi(argv[++i]);
            got_degree = true;
        } else if (arg == "--gen-type" && i+1 < argc) {
            if (!parse_enum(argv[++i], opts.gen_type)) {
                std::cerr << "Invalid gen type: " << argv[i] << std::endl;
                print_usage(argv[0]);
                exit(1);
            }
            got_gen_type = true;
        } else if (arg == "--save-file" && i+1 < argc) {
            opts.save_file_path = argv[++i];
        } else {
            std::cerr << "Unknown or incomplete option: " << arg << std::endl;
            print_usage(argv[0]);
            exit(1);
        }
    }
    
    if (!(got_scale && got_degree && got_gen_type)) {
        std::cerr << "Missing required options." << std::endl;
        print_usage(argv[0]);
        exit(1);
    }
    return opts;
}

#endif // CLI_H_


