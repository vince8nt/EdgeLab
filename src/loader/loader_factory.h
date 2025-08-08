#ifndef LOADER_FACTORY_H_
#define LOADER_FACTORY_H_

#include "loader_base.h"
#include <memory>
#include <functional>
#include <unordered_map>

// Forward declarations
class EdgeListLoader;
class MetisGraphLoader;
class CompactedGraphLoader;

// Simple factory with optional extensibility
class LoaderFactory {
private:
    using LoaderCreator = std::function<std::unique_ptr<LoaderBase>(FileType)>;
    static std::unordered_map<FileType, LoaderCreator>& get_registry() {
        static std::unordered_map<FileType, LoaderCreator> registry;
        return registry;
    }

public:
    // Register a custom loader for a file type
    static void register_loader(FileType file_type, LoaderCreator creator) {
        get_registry()[file_type] = creator;
    }

    // Create loader - uses registered loader if available, otherwise falls back to default
    static std::unique_ptr<LoaderBase> create_loader(const std::string& file_path);
    
    // Template helper for creating specific loader types
    template<typename LoaderType>
    static std::unique_ptr<LoaderBase> make_loader(FileType file_type) {
        return std::make_unique<LoaderType>(file_type);
    }
};

// Global function for backward compatibility and ease of use
inline std::unique_ptr<LoaderBase> create_loader(const std::string& file_path) {
    return LoaderFactory::create_loader(file_path);
}

#endif // LOADER_FACTORY_H_
