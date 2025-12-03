
#pragma once

#include "Platform.h"

namespace Phoenix
{
    typedef uint32 hash32_t;
    typedef uint64 hash64_t;

    struct PHOENIXCORE_API Hashing
    {
        //
        // 32-bit
        //

        static constexpr hash32_t prime32 = 0x1000193;
        static constexpr hash32_t basis32 = 0x811c9dc5;

        static constexpr hash32_t FNV1A32(const char* data, size_t length, hash32_t basis = basis32)
        {
            hash32_t result = basis;

            for (size_t i = 0; i < length; ++i)
            {
                result ^= static_cast<uint8>(data[i]);
                result *= prime32;
            }

            return result;
        }

        template <size_t N>
        static constexpr hash32_t FNV1A32(const char (&data)[N], hash32_t basis = basis32)
        {
            return FNV1A32(data, N - 1, basis);
        }

        template <class T>
        static hash32_t FNV1A32(const T& obj, hash32_t basis = basis32)
        {
            return FNV1A32(reinterpret_cast<const char*>(&obj), sizeof(T), basis);
        }

        static constexpr hash32_t FNV1A32Append(hash32_t basis, const char* data, size_t length)
        {
            return FNV1A32(data, length, basis);
        }

        template <size_t N>
        static constexpr hash32_t FNV1A32Append(hash32_t basis, const char (&data)[N])
        {
            return FNV1A32(data, N - 1, basis);
        }

        static constexpr hash32_t FNV1A32Combine(hash32_t basis)
        {
            return basis;
        }

        template <class TArg0, class ...TArgs>
        static constexpr hash32_t FNV1A32Combine(hash32_t basis, TArg0&& arg0, TArgs&&... args)
        {
            auto h = FNV1A32(arg0, basis);
            h = FNV1A32Combine(h, args...);
            return h;
        }

        //
        // 64-bit
        //

        static constexpr hash64_t prime64 = 0x100000001b3;
        static constexpr hash64_t basis64 = 0xcbf29ce484222325;
        
        static constexpr hash64_t FNV1A64(const char* data, size_t length, hash64_t basis = basis64)
        {
            hash64_t hash = basis;

            for (size_t i = 0; i < length; ++i)
            {
                hash = hash ^ data[i];
                hash *= prime64;
            }

            return hash;
        }

        template <class T>
        static constexpr hash64_t FNV1A64(const T& obj, hash64_t basis = basis64)
        {
            return FNV1A64(reinterpret_cast<const char*>(&obj), sizeof(T), basis);
        }

        static constexpr hash64_t FN1VA64Combine(hash64_t basis)
        {
            return basis;
        }

        template <class TArg0, class ...TArgs>
        static constexpr hash64_t FN1VA64Combine(hash64_t basis, TArg0&& arg0, TArgs&&... args)
        {
            auto h = FNV1A64(arg0, basis);
            h = FN1VA64Combine(h, args...);
            return h;
        }
    };
}
