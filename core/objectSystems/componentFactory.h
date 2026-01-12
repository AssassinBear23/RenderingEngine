#pragma once
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include "component.h"

namespace core
{
    /// <summary>
    /// Simple factory for creating components by type name.
    /// Each component type registers itself with a creation function.
    /// </summary>
    class ComponentFactory {
    public:
        /// <summary>
        /// Type alias for a function that creates a component.
        /// </summary>
        using CreateFunc = std::function<std::shared_ptr<Component>()>;

        /// <summary>
        /// Register a component type with its creation function.
        /// </summary>
        /// <param name="typeName">The type name string (e.g., "Transform")</param>
        /// <param name="func">A function that creates and returns a new component</param>
        static void Register(const std::string& typeName, CreateFunc func) {
            GetRegistry()[typeName] = func;
        }

        /// <summary>
        /// Create a component by its type name.
        /// </summary>
        /// <param name="typeName">The type name to create</param>
        /// <returns>A new component instance, or nullptr if type not found</returns>
        static std::shared_ptr<Component> Create(const std::string& typeName) {
            auto& registry = GetRegistry();
            auto it = registry.find(typeName);
            if (it != registry.end()) {
                return it->second(); // Call the creation function
            }
            return nullptr;
        }

        /// <summary>
        /// Get all registered component type names.
        /// </summary>
        /// <returns>Vector of all registered type name strings</returns>
        static std::vector<std::string> GetRegisteredTypes() {
            std::vector<std::string> types;
            auto& registry = GetRegistry();
            types.reserve(registry.size());
            for (const auto& pair : registry) {
                types.push_back(pair.first);
            }
            return types;
        }

    private:
        /// <summary>
        /// Get the registry map. Using a static function ensures proper initialization.
        /// </summary>
        static std::unordered_map<std::string, CreateFunc>& GetRegistry() {
            static std::unordered_map<std::string, CreateFunc> registry;
            return registry;
        }
    };

    /// <summary>
    /// Helper class to auto-register a component type.
    /// Create a static instance of this in each component's .cpp file.
    /// </summary>
    template<typename T>
    class ComponentRegistrar {
    public:
        explicit ComponentRegistrar(const std::string& typeName) {
            ComponentFactory::Register(typeName, []() {
                return std::make_shared<T>();
                });
        }
    };

    // Macro to make registration easy - use this in each component's .cpp file
#define REGISTER_COMPONENT(ComponentType) \
    static core::ComponentRegistrar<ComponentType> g_##ComponentType##Registrar(#ComponentType)
} // namespace core