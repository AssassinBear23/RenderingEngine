#pragma once
#include <memory>
#include <string>

namespace core {
/// <summary>
/// Base class for engine objects (GameObject, Component, Scene, ...).
/// Provides name, enable/disable, and destroy flags.
/// </summary>
/// <remarks>
/// Must keep:
/// - Only Object derives from std::enable_shared_from_this<Object>.
///   Do NOT re-inherit enable_shared_from_this in subclasses.
/// </remarks>
class Object : public std::enable_shared_from_this<Object> {
public:
    virtual ~Object() = default;

    /// <summary>
    /// Set the display name of this object.
    /// </summary>
    void SetName(std::string n);

    /// <summary>
    /// Get the display name of this object.
    /// </summary>
    const std::string& GetName() const;

    /// <summary>
    /// Enable this object (OnEnable hook is called).
    /// </summary>
    void Enable();

    /// <summary>
    /// Disable this object (OnDisable hook is called).
    /// </summary>
    void Disable();

    /// <summary>
    /// Returns true if this object is enabled.
    /// </summary>
    bool IsEnabled() const;

    /// <summary>
    /// Mark this object for destruction (OnDestroy hook is called).
    /// </summary>
    void Destroy();

    /// <summary>
    /// Returns true if this object was destroyed.
    /// </summary>
    bool IsDestroyed() const;

protected:
    /// <summary>
    /// Called when the object becomes enabled.
    /// </summary>
    virtual void OnEnable() {}

    /// <summary>
    /// Called when the object becomes disabled.
    /// </summary>
    virtual void OnDisable() {}

    /// <summary>
    /// Called when the object is destroyed.
    /// </summary>
    virtual void OnDestroy() {}

private:
    std::string m_name{};
    bool m_enabled{true};
    bool m_destroyed{false};
};

} // namespace core
