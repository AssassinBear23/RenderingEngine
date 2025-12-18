#pragma once

#include "../Component.h"
#include "../../property.h"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <memory>
#include <string>

namespace core
{   
    /// <summary>
    /// Defines the type of light source.
    /// </summary>
    enum class LightType : int
    {
        Point = 0,       // Emits light in all directions from a point
        Directional = 1, // Parallel rays like sunlight
        Spot = 2,        // Cone-shaped light like a flashlight
        
        // Helpful for cycling
        Count = 3
    };

    // Enable increment/decrement for LightType enum
    inline LightType& operator++(LightType& type)
    {
        type = static_cast<LightType>((static_cast<int>(type) + 1) % static_cast<int>(LightType::Count));
        return type;
    }

    inline LightType operator++(LightType& type, int)
    {
        LightType temp = type;
        ++type;
        return temp;
    }

    inline LightType& operator--(LightType& type)
    {
        int value = static_cast<int>(type);
        type = static_cast<LightType>((value - 1 + static_cast<int>(LightType::Count)) % static_cast<int>(LightType::Count));
        return type;
    }

    inline LightType operator--(LightType& type, int)
    {
        LightType temp = type;
        --type;
        return temp;
    }

    // Helper function to convert enum to string
    inline const char* ToString(LightType type)
    {
        switch (type)
        {
            case LightType::Point:       return "Point";
            case LightType::Directional: return "Directional";
            case LightType::Spot:        return "Spot";
            default:                     return "Unknown";
        }
    }
    
    // Helper function to convert enum to int
    inline const int ToInt(LightType type)
    {
        return static_cast<int>(type);
    }

    struct LightData
    {
        glm::vec4 positions[4];     // 64 bytes (use vec4 for vec3 data, std140 pads anyway)
        glm::vec4 directions[4];    // 64 bytes
        glm::vec4 colors[4];        // 64 bytes
        glm::ivec4 lightTypes[4];   // 64 bytes (each int padded to 16 bytes in std140)
        alignas(16) int numLights;  // 16 bytes
        int _pad[3];                // padding
        // Total: 272 bytes
    };

    class Scene;
    class Renderer; // Forward declaration

    /// <summary>
    /// Light component that provides illumination in the scene.
    /// Supports point, directional, and spot light types with configurable color and intensity.
    /// </summary>
    /// <remarks>
    /// The Light component automatically registers itself with the scene's lighting system
    /// when attached to a GameObject and unregisters when detached.
    /// </remarks>
    class Light : public Component
    {
    public:
        /// <summary>
        /// Default constructor that creates a white light at the origin.
        /// </summary>
        Light() : Light(glm::vec3(0.0f), glm::vec4(1.0f)) {}
        
        /// <summary>
        /// Constructs a light with specified position and color.
        /// </summary>
        /// <param name="position">The initial global position of the light.</param>
        /// <param name="color">The RGBA color of the light (alpha typically unused).</param>
        Light(glm::vec3 position, glm::vec4 color)
            : m_globalPosition(position), color(color) {}
        
        /// <summary>
        /// Virtual destructor for proper cleanup of derived classes.
        /// </summary>
        ~Light() override = default;
        
        /// <summary>
        /// Copy assignment operator.
        /// </summary>
        Light& operator=(const Light&) = default;

        /// <summary>
        /// Returns the type name of this component.
        /// </summary>
        /// <returns>The string "Light".</returns>
        std::string GetTypeName() const override { return "Light"; }
        
        /// <summary>
        /// Renders the ImGui interface for editing light properties.
        /// </summary>
        void DrawGui() override;

        /// <summary>
        /// Called when this light is attached to a GameObject.
        /// Registers the light with the scene's lighting system.
        /// </summary>
        /// <param name="owner">Weak pointer to the owning GameObject.</param>
        void OnAttach(std::weak_ptr<GameObject> owner) override;
        
        /// <summary>
        /// Called when this light is detached from a GameObject.
        /// Unregisters the light from the scene's lighting system.
        /// </summary>
        void OnDetach() override;

        /// <summary>
        /// The RGBA color of the light.
        /// </summary>
        Property<glm::vec4> color;
        
        /// <summary>
        /// The intensity/brightness of the light.
        /// </summary>
        Property<float> intensity{ 1 };
        
        /// <summary>
        /// The type of light: 0 = point, 1 = directional, 2 = spot.
        /// </summary>
        Property<LightType> lightType{ LightType::Point };

        /// <summary>
        /// Gets the current color value of the light.
        /// </summary>
        /// <returns>The RGBA color vector.</returns>
        glm::vec4 GetColor() const { return color.Get(); }
        
    private:
        /// <summary>
        /// Updates the cached renderer's material color when the light color changes.
        /// Used to synchronize light visualization with light properties.
        /// </summary>
        /// <param name="newColor">The new color to apply to the renderer's material.</param>
        void UpdateRendererColor(glm::vec4 newColor);

        /// <summary>
        /// 
        /// </summary>
        /// <param name="newIntensity"></param>
        void UpdateRendererIntensity(float newIntensity);
        
        /// <summary>
        /// Cached global position of the light in world space.
        /// </summary>
        glm::vec3 m_globalPosition;
        
        /// <summary>
        /// Weak reference to the scene this light belongs to.
        /// </summary>
        std::weak_ptr<Scene> m_scene;
        
        /// <summary>
        /// Weak reference to the renderer component for light visualization.
        /// </summary>
        std::weak_ptr<Renderer> m_renderer; // Cache renderer reference
    };

} // namespace core