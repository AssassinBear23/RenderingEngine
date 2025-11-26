#pragma once
#include <functional>
#include <type_traits>
#include <glm/gtc/type_ptr.hpp>

namespace core {

/// <summary>
/// Property wrapper that triggers callbacks when value changes.
/// </summary>
template<typename T>
class Property {
private:
    // Proxy class that detects when the value is modified via pointer
    class Proxy {
    public:
        Proxy(Property* prop) : m_property(prop), m_initialValue(prop->m_value) {}
        
        ~Proxy() {
            // Check if value changed when proxy is destroyed
            if (m_property->m_value != m_initialValue && m_property->m_onChange) {
                m_property->m_onChange(m_property->m_value);
            }
        }
        
        // Implicit conversion to pointer
        operator T*() { return &m_property->m_value; }
        
        // For glm types, provide access to underlying value_ptr
        // This allows ImGui functions to work directly
        template<typename U = T>
        auto ValuePtr() -> decltype(glm::value_ptr(std::declval<U&>())) {
            return glm::value_ptr(m_property->m_value);
        }
        
    private:
        Property* m_property;
        T m_initialValue;
    };

public:
    Property(T initialValue = T{})
        : m_value(initialValue) {}

    // Implicit conversion to T (acts like reading the value)
    operator T() const { return m_value; }

    // Assignment operator (acts like writing the value)
    Property& operator=(const T& newValue) {
        if (m_value != newValue) {
            m_value = newValue;
            if (m_onChange) {
                m_onChange(m_value);
            }
        }
        return *this;
    }

    // Address-of operator returns a proxy
    Proxy operator&() { return Proxy(this); }
    const T* operator&() const { return &m_value; }

    // Set the callback function
    void SetOnChange(std::function<void(T)> callback) {
        m_onChange = callback;
    }

    // Get the raw value
    T Get() const { return m_value; }
    
    // Direct access to value_ptr for glm types
    template<typename U = T>
    auto ValuePtr() -> decltype(glm::value_ptr(std::declval<U&>())) {
        return glm::value_ptr(m_value);
    }

private:
    friend class Proxy;
    T m_value;
    std::function<void(T)> m_onChange;
};

} // namespace core