#ifndef LOADER_FACTORY_IMPL_H_
#define LOADER_FACTORY_IMPL_H_

#include "loader_factory.h"
#include "edge_list_loader.h"
#include "metis_graph_loader.h"
#include "compacted_graph_loader.h"

// Implementation of the factory function
inline std::unique_ptr<LoaderBase> LoaderFactory::create_loader(const std::string& file_path) {
    const FileType file_type = GetFileExtension(file_path);
    
    // Check if there's a custom loader registered
    auto& registry = get_registry();
    auto it = registry.find(file_type);
    if (it != registry.end()) {
        return it->second(file_type);
    }
    
    // Fall back to default loaders
    switch (file_type) {
        case FileType::EL:
        case FileType::WEL:
        case FileType::VEL:
        case FileType::VWEL:
            return make_loader<EdgeListLoader>(file_type);
        case FileType::GRAPH:
            return make_loader<MetisGraphLoader>(file_type);
        case FileType::CG:
            return make_loader<CompactedGraphLoader>(file_type);
        default:
            std::cerr << "Unsupported file type for loader factory: " << file_type << std::endl;
            exit(1);
    }
}

#endif // LOADER_FACTORY_IMPL_H_
