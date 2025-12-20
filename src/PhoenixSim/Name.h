
#pragma once

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Hashing.h"

namespace Phoenix
{
    struct PHOENIX_SIM_API FName
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
            if (Value == 0)
            {
                Value = other.Value;
            }
            else
            {
                Value = Hashing::FNV1A32Combine(Value, other.Value);
            }
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
            return Append(chars);
        }

        template <size_t N>
        constexpr FName& operator+=(const char (&chars)[N])
        {
            *this = Append(chars);
            return *this;
        }

        // Append a single character to the hash.
        constexpr FName Append(char c) const
        {
            FName result = *this;
            if (result.Value == 0)
            {
                result.Value = Hashing::FNV1A32(c);
            }
            else
            {
                result.Value = Hashing::FNV1A32Append(result.Value, c);
            }
#if DEBUG
            if (!std::is_constant_evaluated())
            {
                (void)snprintf(result.Debug, _countof(result.Debug), "%s%c", result.Debug, c);
            }
#endif
            return result;
        }

        // Append a string to the hash.
        constexpr FName Append(const char* str, size_t len) const
        {
            FName result = *this;
            if (result.Value == 0)
            {
                result.Value = Hashing::FNV1A32(str, len);
            }
            else
            {
                result.Value = Hashing::FNV1A32Append(result.Value, str, len);
            }
#if DEBUG
            if (!std::is_constant_evaluated())
            {
                (void)snprintf(result.Debug, _countof(result.Debug), "%s%s", result.Debug, str);
            }
#endif
            return result;
        }

        // Append a fixed-length string to the hash.
        template <size_t N>
        constexpr FName Append(const char (&chars)[N]) const
        {
            FName result = *this;
            if (result.Value == 0)
            {
                result.Value = Hashing::FNV1A32(chars);
            }
            else
            {
                result.Value = Hashing::FNV1A32Append(result.Value, chars);
            }
#if DEBUG
            if (!std::is_constant_evaluated())
            {
                (void)snprintf(result.Debug, _countof(result.Debug), "%s%s", result.Debug, chars);
            }
#endif
            return result;
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
}

PHOENIX_SIM_API constexpr Phoenix::FName operator ""_n(const char* chars, size_t len)
{
    return Phoenix::FName(chars, len);
}

template <>
struct std::hash<Phoenix::FName>
{
    std::size_t operator()(const Phoenix::FName& value) const noexcept
    {
        return std::hash<Phoenix::hash32_t>::operator()((Phoenix::hash32_t)value);
    }
};