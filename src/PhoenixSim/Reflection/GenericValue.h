#pragma once

// ── GenericValue — reflection-aware runtime value ─────────────────────────────
//
// This header defines the value and type-descriptor types used by the generic
// invocation system (GenericFunction).  It is included by
// GenericFunction.h and by Reflection.h, and must NOT include
// Reflection.h itself (TypeDescriptor is forward-declared only).
//
// Also defines EPropertyValueType and PropertyDescriptorBuilder<T>, which were
// formerly in Reflection.h.  All files that include Reflection.h still receive
// these types transitively.

#include <any>
#include <cstdint>
#include <string>
#include <type_traits>
#include <unordered_map>

#include "PhoenixSim/ECS/EntityId.h"
#include "PhoenixSim/FixedPoint/FixedTypes.h"
#include "PhoenixSim/FixedPoint/FixedVector.h"
#include "PhoenixSim/Name.h"
#include "PhoenixSim/Platform.h"

namespace Phoenix
{
    struct TypeDescriptor;  // complete definition is in Reflection.h

    // ── EPropertyValueType ────────────────────────────────────────────────────
    //
    // Identifies the built-in scalar/string primitive types for the reflection
    // and generic invocation systems.  Struct types registered with
    // PHX_DECLARE_TYPE / PHX_ENABLE_TYPE use ParamTypeRef::Descriptor instead.

    enum class PHOENIX_SIM_API EPropertyValueType
    {
        Unknown,
        Int8,   UInt8,
        Int16,  UInt16,
        Int32,  UInt32,
        Int64,  UInt64,
        Float,  Double,
        Bool,
        String,
        Name,
        FixedPoint,
        Vec2,
        Transform2D,
        COUNT
    };

    // ── PropertyDescriptorBuilder<T> ──────────────────────────────────────────
    //
    // Maps a C++ type to its EPropertyValueType and any extra metadata.
    // Used by TypeRegistrationBuilder field()/property() overloads.

    template <class T>
    struct PropertyDescriptorBuilder
    {
        static EPropertyValueType GetPropertyValueType()
        {
#define PHX_TYPE_TO_PROP_ENUM(type, enum_value) \
            if constexpr (std::is_same_v<T, type>) \
                return EPropertyValueType::enum_value;

            PHX_TYPE_TO_PROP_ENUM(int8,        Int8)
            PHX_TYPE_TO_PROP_ENUM(uint8,       UInt8)
            PHX_TYPE_TO_PROP_ENUM(int16,       Int16)
            PHX_TYPE_TO_PROP_ENUM(uint16,      UInt16)
            PHX_TYPE_TO_PROP_ENUM(int32,       Int32)
            PHX_TYPE_TO_PROP_ENUM(uint32,      UInt32)
            PHX_TYPE_TO_PROP_ENUM(int64,       Int64)
            PHX_TYPE_TO_PROP_ENUM(uint64,      UInt64)
            PHX_TYPE_TO_PROP_ENUM(float,       Float)
            PHX_TYPE_TO_PROP_ENUM(double,      Double)
            PHX_TYPE_TO_PROP_ENUM(bool,        Bool)
            PHX_TYPE_TO_PROP_ENUM(std::string, String)
            PHX_TYPE_TO_PROP_ENUM(FName,       Name)
            PHX_TYPE_TO_PROP_ENUM(Phoenix::Vec2, Vec2)

#undef PHX_TYPE_TO_PROP_ENUM

            return EPropertyValueType::Unknown;
        }

        static std::unordered_map<std::string, std::string> GetMetadata()
        {
            return {};
        }
    };

    template <uint8 Tb, class T>
    struct PropertyDescriptorBuilder<TFixed<Tb, T>>
    {
        static EPropertyValueType GetPropertyValueType()
        {
            return EPropertyValueType::FixedPoint;
        }

        static std::unordered_map<std::string, std::string> GetMetadata()
        {
            return { { "FractionalBits", std::format("{}", Tb) } };
        }
    };

    // ── ParamTypeRef ──────────────────────────────────────────────────────────
    //
    // Describes the type of a single function parameter or return value.
    //
    //  • IsStruct()    — Descriptor is non-null: a Phoenix-registered struct type.
    //                    Inspect Descriptor->GetProperties() for field metadata.
    //  • IsPrimitive() — Descriptor is null, Primitive != Unknown.
    //  • IsVoid()      — Descriptor is null, Primitive == Unknown (void / nil).

    struct PHOENIX_SIM_API ParamTypeRef
    {
        EPropertyValueType    Primitive   = EPropertyValueType::Unknown;
        const TypeDescriptor* Descriptor  = nullptr;

        bool IsVoid()      const { return Primitive == EPropertyValueType::Unknown && !Descriptor; }
        bool IsStruct()    const { return Descriptor != nullptr; }
        bool IsPrimitive() const { return !IsStruct() && !IsVoid(); }

        bool operator==(const ParamTypeRef&) const = default;
    };

    // ── GenericValue ──────────────────────────────────────────────────────────
    //
    // A tagged runtime value whose type is described by ParamTypeRef.
    //
    // Primitive types are stored inline (B / I / N union + Str for strings).
    //
    // Struct types are stored in one of two ways:
    //   • Non-owning: Ptr points to an object the CALLER owns.
    //     Use GenericValue::Borrow(const T&).  Caller must keep the object alive
    //     for the duration of the call.
    //   • Owning: OwnedData holds the value via std::any.
    //     Use GenericConverter<T>::Own(T&&) — produced by invoker factories for
    //     functions that return a struct by value.

    struct PHOENIX_SIM_API GenericValue
    {
        ParamTypeRef Type;

        union
        {
            bool    B   = false;
            int64_t I;
            double  N;
        };

        std::string  Str;       // populated when Primitive == String
        void*        Ptr = nullptr; // non-owning struct pointer (null when OwnedData is set)
        std::any     OwnedData; // owning struct storage (empty for primitives / non-owning)

        static GenericValue Void() { return {}; }

        // Convenience factory for primitives and non-owning struct borrows.
        template <class T>
        static GenericValue Borrow(const T& v);

        // Extract the stored value as T.  Works for both primitive and struct kinds.
        template <class T>
        T As() const;
    };

    // ── detail helpers ────────────────────────────────────────────────────────

    namespace detail
    {
        // Detects whether T has a StaticTypeName member (i.e. uses PHX_DECLARE_TYPE
        // or PHX_ENABLE_TYPE and is therefore registered in TypeRegistry).
        template <class T, class = void>
        struct HasStaticTypeName : std::false_type {};

        template <class T>
        struct HasStaticTypeName<T, std::void_t<decltype(T::StaticTypeName)>>
            : std::true_type {};
    }

    // ── GenericConverter<T> ───────────────────────────────────────────────────
    //
    // Bidirectional mapping between a C++ type T and GenericValue.
    //
    // Primary template: Phoenix-registered struct types (have StaticTypeName).
    //   • Borrow(const T&)  — non-owning; caller keeps object alive.
    //   • Own(T&&)          — moves T into std::any; safe for function return values.
    //   • From(GenericValue) — extracts T& from either Ptr or OwnedData.
    //
    // Primitive specialisations below use the inline union storage.

    template <class T, class = void>
    struct GenericConverter
    {
        static_assert(detail::HasStaticTypeName<T>::value,
            "GenericConverter requires T to have StaticTypeName (use PHX_DECLARE_TYPE / "
            "PHX_ENABLE_TYPE) or a GenericConverter<T> specialisation.");

        static GenericValue Borrow(const T& v)
        {
            GenericValue gv;
            gv.Type.Descriptor = &T::GetStaticTypeDescriptor();
            gv.Ptr = const_cast<void*>(static_cast<const void*>(&v));
            return gv;
        }

        static GenericValue Own(T&& v)
        {
            GenericValue gv;
            gv.Type.Descriptor = &T::GetStaticTypeDescriptor();
            gv.OwnedData = std::move(v);
            return gv;
        }

        static T& From(const GenericValue& gv)
        {
            if (gv.OwnedData.has_value())
                return std::any_cast<T&>(const_cast<std::any&>(gv.OwnedData));
            return *static_cast<T*>(gv.Ptr);
        }
    };

    // ── Primitive specialisations ─────────────────────────────────────────────

#define PHX_GC_INT(CppType, EKind) \
    template <> struct GenericConverter<CppType> \
    { \
        static GenericValue Borrow(CppType v) \
        { \
            GenericValue gv; \
            gv.Type.Primitive = EPropertyValueType::EKind; \
            gv.I = static_cast<int64_t>(v); \
            return gv; \
        } \
        static CppType From(const GenericValue& gv) { return static_cast<CppType>(gv.I); } \
    };

#define PHX_GC_FP(CppType, EKind) \
    template <> struct GenericConverter<CppType> \
    { \
        static GenericValue Borrow(CppType v) \
        { \
            GenericValue gv; \
            gv.Type.Primitive = EPropertyValueType::EKind; \
            gv.N = static_cast<double>(v); \
            return gv; \
        } \
        static CppType From(const GenericValue& gv) { return static_cast<CppType>(gv.N); } \
    };

    template <> struct GenericConverter<bool>
    {
        static GenericValue Borrow(bool v)
        {
            GenericValue gv;
            gv.Type.Primitive = EPropertyValueType::Bool;
            gv.B = v;
            return gv;
        }
        static bool From(const GenericValue& gv) { return gv.B; }
    };

    PHX_GC_INT(int8_t,   Int8)
    PHX_GC_INT(uint8_t,  UInt8)
    PHX_GC_INT(int16_t,  Int16)
    PHX_GC_INT(uint16_t, UInt16)
    PHX_GC_INT(int32_t,  Int32)
    PHX_GC_INT(uint32_t, UInt32)
    PHX_GC_INT(int64_t,  Int64)
    PHX_GC_INT(uint64_t, UInt64)
    PHX_GC_FP(float,  Float)
    PHX_GC_FP(double, Double)

#undef PHX_GC_INT
#undef PHX_GC_FP

    template <> struct GenericConverter<std::string>
    {
        static GenericValue Borrow(std::string v)
        {
            GenericValue gv;
            gv.Type.Primitive = EPropertyValueType::String;
            gv.Str = std::move(v);
            return gv;
        }
        static std::string From(const GenericValue& gv) { return gv.Str; }
    };

    template <> struct GenericConverter<FName>
    {
        static GenericValue Borrow(FName v)
        {
            GenericValue gv;
            gv.Type.Primitive = EPropertyValueType::Name;
            gv.I = static_cast<int64_t>(static_cast<hash32_t>(v));
            return gv;
        }
        static FName From(const GenericValue& gv) { return FName(static_cast<hash32_t>(gv.I)); }
    };

    template <> struct GenericConverter<ECS::EntityId>
    {
        static GenericValue Borrow(ECS::EntityId v)
        {
            GenericValue gv;
            gv.Type.Primitive = EPropertyValueType::UInt32;
            gv.I = static_cast<int64_t>(static_cast<uint32_t>(v));
            return gv;
        }
        static ECS::EntityId From(const GenericValue& gv)
        {
            return ECS::EntityId(static_cast<uint32_t>(gv.I));
        }
    };

    template <uint8 Tb, class T>
    struct GenericConverter<TFixed<Tb, T>>
    {
        static GenericValue Borrow(TFixed<Tb, T> v)
        {
            GenericValue gv;
            gv.Type.Primitive = EPropertyValueType::FixedPoint;
            gv.N = static_cast<double>(v);
            return gv;
        }
        static TFixed<Tb, T> From(const GenericValue& gv) { return TFixed<Tb, T>(gv.N); }
    };

    // ── GenericValue member template definitions ──────────────────────────────

    template <class T>
    GenericValue GenericValue::Borrow(const T& v)
    {
        return GenericConverter<std::decay_t<T>>::Borrow(v);
    }

    template <class T>
    T GenericValue::As() const
    {
        return GenericConverter<std::decay_t<T>>::From(*this);
    }

    // ── MakeParamTypeRef<T> ───────────────────────────────────────────────────
    //
    // Compile-time helper: maps a C++ parameter type to its ParamTypeRef.
    //
    //   void              → IsVoid()
    //   T with StaticTypeName → IsStruct() with Descriptor pointing to T's descriptor
    //   primitives        → IsPrimitive() with matching EPropertyValueType

    template <class T>
    ParamTypeRef MakeParamTypeRef()
    {
        using D = std::decay_t<T>;

        if constexpr (std::is_void_v<D>)
        {
            return {};
        }
        else if constexpr (detail::HasStaticTypeName<D>::value)
        {
            // TypeDescriptor must be complete at instantiation sites that call this.
            return { EPropertyValueType::Unknown, &D::GetStaticTypeDescriptor() };
        }
        else
        {
            return { PropertyDescriptorBuilder<D>::GetPropertyValueType(), nullptr };
        }
    }

} // namespace Phoenix
