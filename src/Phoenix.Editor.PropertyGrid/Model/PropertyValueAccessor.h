#pragma once

#include <optional>

namespace Phoenix::PropertyGrid
{
    template <class T>
    class IPropertyValueAccessor
    {
    public:
        virtual ~IPropertyValueAccessor() = default;

        virtual std::optional<T> GetValue() const = 0;
        virtual void SetValue(const T& value) = 0;
    };
}
