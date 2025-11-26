#pragma once

#include "../Component.h"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <memory>
#include <string>

namespace core
{   
    struct LightData
    {
        glm::vec4 positions[4];     // xyz = position
        glm::vec4 colors[4];        // rgba
        int numLights;              // Number of active lights
        float padding[3];           // Align to total of 128 bytes
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
        Property<int> intensity{ 1 };
        
        /// <summary>
        /// The type of light: 0 = point, 1 = directional, 2 = spot.
        /// </summary>
        Property<int> lightType{ 0 };

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