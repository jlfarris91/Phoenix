
#pragma once

#include <stdlib.h>

#include "Platform.h"
#include "Hashing.h"

namespace Phoenix
{
    struct PHOENIXCORE_API FName
    {
        static const FName None;
        static const FName Empty;

        constexpr FName() = default;
        constexpr FName(hash32_t hash) : Value(hash) {}

        constexpr FName(const char* chars, size_t len)
            : Value(Hashing::FNV1A32(chars, len))
        {
#if DEBUG
            for (size_t i = 0; i < len && i < _countof(Debug); ++i)
                Debug[i] = *(chars + i);
#endif
        }

        constexpr FName(const PHXString& string)
            : FName(string.data(), string.length())
        {
        }

        constexpr explicit operator hash32_t() const
        {
            return Value;
        }

        constexpr bool operator==(const FName& other) const
        {
            return Value == other.Value;
        }

        constexpr bool operator!=(const FName& other) const
        {
            return Value != other.Value;
        }

        constexpr std::strong_ordering operator<=>(const FName& other) const
        {
            return Value <=> other.Value;
        }

        constexpr FName operator+(const FName& other) const
        {
            FName result = *this;
            result += other;
            return result;
        }

        constexpr FName& operator+=(const FName& other)
        {
            Value = Hashing::FNV1A32Combine(Value, other.Value);
#if DEBUG
            if (!std::is_constant_evaluated())
            {
                (void)snprintf(Debug, _countof(Debug), "%s+%s", Debug, other.Debug);
            }
#endif
            return *this;
        }

        template <size_t N>
        constexpr FName operator+(const char (&chars)[N]) const
        {
            FName result = *this;
            result += chars;
            return result;
        }

        template <size_t N>
        constexpr FName& operator+=(const char (&chars)[N])
        {
            Value = Hashing::FNV1A32Append(Value, chars);
#if DEBUG
            if (!std::is_constant_evaluated())
            {
                (void)snprintf(Debug, _countof(Debug), "%s%s", Debug, chars);
            }
#endif
            return *this;
        }

        constexpr static bool IsNoneOrEmpty(const FName& name)
        {
            return name == None || name == Empty;
        }

    private:
        hash32_t Value = 0;

#if DEBUG
    public:
        char Debug[64] = {};
#endif
    };

    PHOENIXCORE_API constexpr FName operator ""_n(const char* chars, size_t len)
    {
        return FName(chars, len);
    }
}

template <>
struct std::hash<Phoenix::FName>
{
    std::size_t operator()(const Phoenix::FName& value) const noexcept
    {
        return std::hash<Phoenix::hash32_t>::operator()((Phoenix::hash32_t)value);
    }
};