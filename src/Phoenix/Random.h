#pragma once

#include "Hashing.h"
#include "Utils.h"
#include "FixedPoint/FixedVector.h"
#include "Phoenix/Platform.h"

namespace Phoenix
{
    namespace RandomDetail
    {
        // https://prng.di.unimi.it/xoroshiro64starstar.c
        struct Xoroshiro64SS
        {
            using Type = uint32;

            static Type RotL(Type x, int32 k)
            {
                return (x << k) | (x >> (32 - k));
            }

            void Seed(const char* data, size_t length)
            {
                S[0] = Hashing::FNV1A32(data, length);
                S[1] = Hashing::FNV1A32Append(S[0], data, length);
            }

            template <size_t N>
            void Seed(const char (&data)[N])
            {
                S[0] = Hashing::FNV1A32(data);
                S[1] = Hashing::FNV1A32Append(S[0], data);
            }

            Type Next()
            {
                const Type s0 = S[0];
                Type s1 = S[1];
                const Type result = RotL(s0 * 0x9E3779BB, 5) * 5;
                s1 ^= s0;
                S[0] = RotL(s0, 26) ^ s1 ^ (s1 << 9);
                S[1] = RotL(s1, 13);
                return result;
            }

            uint32 Next32()
            {
                return Next();
            }

            uint64 Next64()
            {
                uint64 v = Next();
                return (v << 32) | Next();
            }

            Type S[2] = {};
        };

        // https://prng.di.unimi.it/xoroshiro128plusplus.c
        struct Xoroshiro128PP
        {
            using Type = uint64;

            static Type RotL(Type x, int32 k)
            {
                return (x << k) | (x >> (64 - k));
            }

            void Seed(const char* data, size_t length)
            {
                S[0] = Hashing::FNV1A64(data, length);
                S[1] = Hashing::FNV1A64Append(S[0], data, length);
            }

            template <size_t N>
            void Seed(const char (&data)[N])
            {
                S[0] = Hashing::FNV1A64(data);
                S[1] = Hashing::FNV1A64Append(S[0], data);
            }

            Type Next()
            {
                const Type s0 = S[0];
                Type s1 = S[1];
                const Type result = RotL(s0 + s1, 17) + s0;
                s1 ^= s0;
                S[0] = RotL(s0, 49) ^ s1 ^ (s1 << 21);
                S[1] = RotL(s1, 28);
                return result;
            }

            uint32 Next32()
            {
                return static_cast<uint32>(Next());
            }

            uint64 Next64()
            {
                return Next();
            }

            Type S[2] = {};
        };

        // https://github.com/imneme/pcg-cpp
        struct PCG
        {
            using Type = uint64;

            static Type RotR(Type x, int32 k)
            {
                return (x >> k) | (x << (32 - k));
            }

            static Type Step(Type g)
            {
                return 0x5851f42d4c957f2d * g + 0xda3e39cb94b95bdb;
            }

            void Seed(Type state)
            {
                G = Step(Step({}) + state);
            }

            void Seed(const char* data, size_t length)
            {
                Seed(Hashing::FNV1A64(data, length));
            }

            template <size_t N>
            void Seed(const char (&data)[N])
            {
                Seed(Hashing::FNV1A64(data));
            }

            Type Next()
            {
                Type v = Read();
                G = Step(G);
                return v;
            }

            uint32 Next32()
            {
                return static_cast<uint32>(Next());
            }

            uint64 Next64()
            {
                return Next();
            }

            Type Read() const
            {
                const uint64 g = G;
                const int32 k = int32(g >> 59);
                const uint64 x = g >> 27 ^ g >> 45;
                return RotR(x, k);
            }

            Type G = 0;
        };
    }

    template <class>
    struct RandomNextImpl;

    template <class>
    struct RandomRangeImpl;

    template <class TStrategy>
    struct TRandom : TStrategy
    {
        using Strategy = TStrategy;
        using Type = typename Strategy::Type;

        void Seed(uint32 seed)
        {
            Strategy::Seed(reinterpret_cast<const char*>(&seed), sizeof(seed));
        }

        void Seed(uint64 seed)
        {
            Strategy::Seed(reinterpret_cast<const char*>(&seed), sizeof(seed));
        }

        void Seed(const char* data, size_t length)
        {
            Strategy::Seed(data, length);
        }

        template <size_t N>
        void Seed(const char (&data)[N])
        {
            Strategy::Seed(data);
        }

        Type Next()
        {
            return Strategy::Next();
        }

        template <class T>
        T Next()
        {
            return RandomNextImpl<T>::Next(*this);
        }

        template <class T>
        T Range(const T& inclusiveMin, const T& exclusiveMax)
        {
            return RandomRangeImpl<T>::Range(*this, inclusiveMin, exclusiveMax);
        }

        template <class T>
        TVec2<T> PointOnCircle(T radius)
        {
            if (radius == 0)
            {
                return TVec2<T>::Zero;
            }

            return Cordic::Rotate<T>(radius, 0, Next<Angle>());
        }

        template <class T>
        TVec2<T> PointInCircle(T radius)
        {
            if (radius == 0)
            {
                return TVec2<T>::Zero;
            }

            return Cordic::Rotate<T>(Range<Distance>(0.0, radius), 0, Next<Angle>());
        }
    };

    template <class T>
    struct RandomNextImpl
    {
        template <class TStrategy>
        static T Next(TRandom<TStrategy>& random)
        {
            return random.Next();
        }
    };

    template <class T>
    struct RandomRangeImpl
    {
        template <class TRandom>
        static int32 Range(TRandom& random, int32 inclusiveMin, int32 exclusiveMax)
        {
            if (inclusiveMin == exclusiveMax)
            {
                return inclusiveMin;
            }
            if (inclusiveMin > exclusiveMax)
            {
                Swap(inclusiveMin, exclusiveMax); 
            }
            uint32 range = exclusiveMax - inclusiveMin;
            uint64 value64 = random.template Next<uint32>();
            value64 *= range;
            uint32 result = value64 >> 32;
            return inclusiveMin + result;
        }
    };

    template <uint8 Tb, class T>
    struct RandomRangeImpl<TFixed<Tb, T>>
    {
        template <class TRandom>
        static TFixed<Tb, T> Range(TRandom& random, TFixed<Tb, T> inclusiveMin, TFixed<Tb, T> exclusiveMax)
        {
            return TFixedQ_T<T>(random.template Range<T>(inclusiveMin.Value, exclusiveMax.Value));
        }
    };

    template <uint8 Tb, class T>
    struct RandomNextImpl<TFixed<Tb, T>>
    {
        template <class TRandom>
        static TFixed<Tb, T> Next(TRandom& random)
        {
            // QMIN/QMAX are raw fixed-point storage values (integer domain).
            // Range<T> operates on them directly, then TFixedQ_T wraps the result.
            auto constexpr min = static_cast<T>(TFixed<Tb, T>::QMIN);
            auto constexpr max = static_cast<T>(TFixed<Tb, T>::QMAX);
            return TFixedQ_T<T>(random.template Range<T>(min, max));
        }
    };

    using Random = TRandom<RandomDetail::Xoroshiro64SS>;
}