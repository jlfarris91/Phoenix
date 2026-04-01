#pragma once

#include "Hashing.h"
#include "Utils.h"
#include "FixedPoint/FixedVector.h"
#include "PhoenixSim/Platform.h"

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

        int32 Next32()
        {
            return Strategy::Next32();
        }

        uint32 NextU32()
        {
            return Strategy::Next32();
        }

        int64 Next64()
        {
            return Strategy::Next64();
        }

        uint64 NextU64()
        {
            return Strategy::Next64();
        }

        template <class T>
        T NextT()
        {
            if constexpr (std::is_same_v<T, int32> || std::is_same_v<T, uint32>)
            {
                return NextU32();
            }
            else if constexpr (std::is_same_v<T, int64> || std::is_same_v<T, uint64>)
            {
                return NextU64();
            }
            else
            {
                static_assert(0);
            }
        }

        int32 RandomRange32(int32 inclusiveMin, int32 exclusiveMax)
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
            uint64 value64 = NextU32();
            value64 *= range;
            uint32 result = value64 >> 32;
            return inclusiveMin + result;
        }

        uint32 RandomRangeU32(uint32 inclusiveMin, uint32 exclusiveMax)
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
            uint64 value64 = NextU32();
            value64 *= range;
            uint32 result = value64 >> 32;
            return inclusiveMin + result;
        }

        template <uint8 Tb, class T>
        TFixed<Tb, T> Next()
        {
            return TFixedQ_T<T>(NextT<T>());
        }

        template <class T>
        struct RandomRangeImpl
        {
            static T RandomRange(TRandom& random, T inclusiveMin, T exclusiveMax)
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
                uint64 value64 = random.NextU32();
                value64 *= range;
                uint32 result = value64 >> 32;
                return inclusiveMin + result;
            }
        };

        template <uint8 Tb, class T>
        struct RandomRangeImpl<TFixed<Tb, T>>
        {
            static TFixed<Tb, T> RandomRange(TRandom& random, TFixed<Tb, T> inclusiveMin, TFixed<Tb, T> exclusiveMax)
            {
                if (inclusiveMin == exclusiveMax)
                {
                    return inclusiveMin;
                }
                if (inclusiveMin > exclusiveMax)
                {
                    Swap(inclusiveMin, exclusiveMax); 
                }
                uint64 range = exclusiveMax.Value - inclusiveMin.Value;
                uint64 value64 = random.NextU32();
                value64 *= range;
                uint32 result = value64 >> 32;
                return TFixedQ_T<T>(T(inclusiveMin.Value + result));
            }
        };

        template <class T>
        T RandomRange(const T& inclusiveMin, const T& exclusiveMax)
        {
            return RandomRangeImpl<T>::RandomRange(*this, inclusiveMin, exclusiveMax);
        }

        template <class T>
        TVec2<T> RandomPointOnCircle(T radius)
        {
            if (radius == 0)
            {
                return TVec2<T>::Zero;
            }

            Angle theta = Angle::QT(NextT<Angle::ValueT>());
            return Cordic::Rotate<T>(radius, 0, theta);
        }

        template <class T>
        TVec2<T> RandomPointInCircle(T radius)
        {
            if (radius == 0)
            {
                return TVec2<T>::Zero;
            }

            auto rr = SqrxQ(radius);
            uint64 random64 = NextU64();
            T randomRadius = Cordic::Sqrt(T(T::QT(random64 % rr)));
            Angle theta = Angle::QT(NextT<Angle::ValueT>());
            return Cordic::Rotate<T>(randomRadius, 0, theta);
        }
    };

    using Random = TRandom<RandomDetail::Xoroshiro64SS>;
}

PHX_DEFINE_TYPE(Phoenix::Random)
{
    registration
        .Alias("Random")
        .Method("Next32", &Random::Next32)
        .Method("NextU32", &Random::NextU32)
        .Method("Next64", &Random::Next64)
        .Method("NextU64", &Random::NextU64)
        .Method("RandomRange32", &Random::RandomRange32)
        .Method("RandomRangeU32", &Random::RandomRangeU32)
        .Method("RandomPointOnCircle", &Random::RandomPointOnCircle<Distance>)
        .Method("RandomPointInCircle", &Random::RandomPointInCircle<Distance>);
}