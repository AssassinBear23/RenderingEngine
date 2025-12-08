#pragma once
#include <glm/gtc/type_ptr.hpp>
#include <concepts>
#include <functional>
#include <utility>

namespace core
{
    // Concepts to check if a type supports increment/decrement
    template<typename T>
    concept Incrementable = requires(T a)
    {
        { ++a } -> std::convertible_to<T&>;
        { a++ } -> std::convertible_to<T>;
    };

    template<typename T>
    concept Decrementable = requires(T a)
    {
        { --a } -> std::convertible_to<T&>;
        { a-- } -> std::convertible_to<T>;
    };

    template<typename T>
    concept AdditionAssignable = requires(T a, T b)
    {
        { a += b } -> std::convertible_to<T&>;
    };

    /// <summary>
    /// Property wrapper that triggers callbacks when value changes.
    /// </summary>
    template<typename T>
    class Property
    {
    private:
        // Proxy class that detects when the value is modified via pointer
        class Proxy
        {
        public:
            Proxy(Property* prop) : m_property(prop), m_initialValue(prop->m_value) {}

            ~Proxy()
            {
                // Check if value changed when proxy is destroyed
                if (m_property->m_value != m_initialValue && m_property->m_onChange)
                {
                    m_property->m_onChange(m_property->m_value);
                }
            }

            // Implicit conversion to pointer
            operator T* () { return &m_property->m_value; }

            // For glm types, provide access to underlying value_ptr
            // This allows ImGui functions to work directly
            template<typename U = T>
            auto ValuePtr() -> decltype(glm::value_ptr(std::declval<U&>()))
            {
                return glm::value_ptr(m_property->m_value);
            }

        private:
            Property* m_property;
            T m_initialValue;
        };

    public:
        Property(T initialValue = T{})
            : m_value(initialValue)
        {}

        // Implicit conversion to T (acts like reading the value)
        operator T() const { return m_value; }

        // Arithmetric operators
        // Assignment operator (acts like writing the value)
        Property& operator=(const T& newValue)
        {
            if (m_value != newValue)
            {
                m_value = newValue;
                if (m_onChange)
                {
                    m_onChange(m_value);
                }
            }
            return *this;
        }

        // Increment and decrement operators
        Property& operator++() requires Incrementable<T>
        {
            ++m_value;
            if (m_onChange)
            {
                m_onChange(m_value);
            }
            return *this;
        }

        Property operator++(int) requires Incrementable<T>
        {
            Property temp = *this;
            ++m_value;
            if (m_onChange)
            {
                m_onChange(m_value);
            }
            return temp;
        }

        Property& operator--() requires Decrementable<T>
        {
            --m_value;
            if (m_onChange)
            {
                m_onChange(m_value);
            }
            return *this;
        }

        Property operator--(int) requires Decrementable<T>
        {
            Property temp = *this;
            --m_value;
            if (m_onChange)
            {
                m_onChange(m_value);
            }
            return temp;
        }

        // Address-of operator returns a proxy
        Proxy operator&() { return Proxy(this); }
        const T* operator&() const { return &m_value; }

        // Set the callback function
        void SetOnChange(std::function<void(T)> callback)
        {
            m_onChange = callback;
        }

        // Get the raw value
        T Get() const { return m_value; }

        // Direct access to value_ptr for glm types
        template<typename U = T>
        auto ValuePtr() -> decltype(glm::value_ptr(std::declval<U&>()))
        {
            return glm::value_ptr(m_value);
        }

    private:
        friend class Proxy;
        T m_value;
        std::function<void(T)> m_onChange;
    };

} // namespace core