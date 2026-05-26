#pragma once

#include "Phoenix/Platform.h"
#include "Phoenix/Hashing.h"

#ifndef PHOENIX_SIM_NAME_ENTRIES
#define PHOENIX_SIM_NAME_ENTRIES 1
#endif

namespace Phoenix
{
    struct PHOENIX_SIM_API FName
    {
        static const FName None;
        static const FName Empty;

        constexpr FName() = default;
        constexpr FName(hash32_t hash) : Value(hash) {}
        constexpr FName(const std::string& string) : FName(string.data(), string.length()) {}

        constexpr FName(const char* chars, size_t len) : Value(Hashing::FNV1A32(chars, len))
        {
#if PHOENIX_SIM_NAME_ENTRIES
            if (!std::is_constant_evaluated())
            {
                RecordNameEntryAs(chars, len, Value);
            }
#endif
        }

        constexpr operator hash32_t() const
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
            return Append(&c, 1);
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
#if PHOENIX_SIM_NAME_ENTRIES
            if (!std::is_constant_evaluated())
            {
                AppendNameEntryAs(Value, str, len, result.Value);
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
#if PHOENIX_SIM_NAME_ENTRIES
            if (!std::is_constant_evaluated())
            {
                AppendNameEntryAs(Value, chars, N, result.Value);
            }
#endif
            return result;
        }

        // Combine is NOT SAFE for concatenation of names because of FNV1A.
        // Use Append instead.
        constexpr FName Combine(const FName& other) const
        {
            FName result;
            if (Value == 0)
            {
                result.Value = other.Value;
            }
            else
            {
                result.Value = Hashing::FNV1A32Combine(Value, other.Value);
#if PHOENIX_SIM_NAME_ENTRIES
                if (!std::is_constant_evaluated())
                {
                    CombineNameEntryAs(Value, other.Value, result.Value);
                }
#endif
            }
            return result;
        }

        constexpr static bool IsNoneOrEmpty(const FName& name)
        {
            return name == None || name == Empty;
        }

        // This is not safe for stateful use. Only use for visualizations!
        // Requires that PHOENIX_SIM_NAME_ENTRIES be defined.
        static const char* GetNameEntry(hash32_t hash);

        // This is not safe for stateful use. Only use for visualizations!
        // Requires that PHOENIX_SIM_NAME_ENTRIES be defined.
        const char* ToString() const;

    private:

#if PHOENIX_SIM_NAME_ENTRIES
        // These are unsafe and should only be used for natvis!
        static const char* RecordNameEntryAs(const char* chars, size_t len, hash32_t asHash);
        static const char* AppendNameEntryAs(hash32_t base, const char* chars, size_t len, hash32_t asHash);
        static const char* CombineNameEntryAs(hash32_t base, hash32_t other, hash32_t asHash);
#endif

        hash32_t Value = 0;
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
        return std::hash<Phoenix::hash32_t>{}((Phoenix::hash32_t)value);
    }
};