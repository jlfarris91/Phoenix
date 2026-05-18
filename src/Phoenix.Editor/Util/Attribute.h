#pragma once

#include <functional>
#include <optional>

namespace Phoenix
{
    template <class T>
    struct Attribute
    {
        using AttributeGetFunc = std::function<T()>;

        Attribute() = default;
        Attribute(const T& value) : Value(value) {}
        Attribute(const AttributeGetFunc& func) : Getter(func) {}
        
        template <class ...TArgs>
        Attribute(TArgs&&... args) : Value(std::forward<TArgs>(args)...) {}

        T GetValue(const T& defaultValue = {}) const
        {
            return IsBound() ? Getter() : Value.value_or(defaultValue);
        }

        bool HasValue() const
        {
            return Value.has_value() || IsBound();
        }

        bool IsBound() const
        {
            return Getter != nullptr;
        }

        bool operator==(const Attribute& other) const
        {
            return Value == other.Value || Getter == other.Getter;
        }

        bool operator!=(const Attribute& other) const
        {
            return !(*this == other);
        }

        std::optional<T> Value;
        AttributeGetFunc Getter;
    };
}
