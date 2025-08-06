#ifndef LOADER_FACTORY_H_
#define LOADER_FACTORY_H_

#include "loader_base.h"
#include "edge_list_loader.h"
#include "metis_graph_loader.h"
#include "compacted_graph_loader.h"

// Simple factory pattern to create loader based on file type
std::unique_ptr<LoaderBase> create_loader(const std::string& file_path) {
    const FileType file_type = GetFileExtension(file_path);
    switch (file_type) {
        case FileType::EL:
            return std::make_unique<EdgeListLoader>(file_type);
        case FileType::WEL:
            return std::make_unique<EdgeListLoader>(file_type);
        case FileType::VEL:
            return std::make_unique<EdgeListLoader>(file_type);
        case FileType::VWEL:
            return std::make_unique<EdgeListLoader>(file_type);
        case FileType::GRAPH:
            return std::make_unique<MetisGraphLoader>(file_type);
        case FileType::CG:
            return std::make_unique<CompactedGraphLoader>(file_type);
        default:
            std::cerr << "Unsupported file type for loader factory: " << file_type << std::endl;
            exit(1);
    }
}

#endif // LOADER_FACTORY_H_
