#pragma once

// ── GenericValue — reflection-aware runtime value ─────────────────────────────
//
// This header defines the value and type-descriptor types used by the generic
// invocation system (GenericFunction).  It is included by
// GenericFunction.h and by Reflection.h, and must NOT include
// Reflection.h itself (TypeDescriptor is forward-declared only).
//
// Also defines EGenericValueType and GenericValueTypeBuilder<T>, which were
// formerly in Reflection.h.  All files that include Reflection.h still receive
// these types transitively.

#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>

#include "PhoenixSim/ECS/EntityId.h"
#include "PhoenixSim/FixedPoint/FixedTypes.h"
#include "PhoenixSim/Name.h"
#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Reflection/TypeRegistry.h"
#include "PhoenixSim/Reflection/TypeTraits.h"

namespace Phoenix
{
    struct TypeDescriptor;  // complete definition is in Reflection.h

    // ── EGenericValueType ────────────────────────────────────────────────────
    //
    // Identifies the built-in scalar/string primitive types for the reflection
    // and generic invocation systems.
    //
    //  • Struct — any Phoenix-registered struct type (has StaticTypeName via
    //             PHX_DECLARE_TYPE / PHX_ENABLE_TYPE, or via
    //             PHX_REGISTER_EXTERNAL_TYPE).  PropertyDescriptor::StructDescriptor
    //             points at the nested TypeDescriptor.
    //
    //  • FixedPoint — Phoenix::TFixed<N, T> fixed-point scalar.  Metadata
    //                 carries "FractionalBits" so consumers can reconstruct the
    //                 real value.

    enum class PHOENIX_SIM_API EGenericValueType
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
        Struct,
        COUNT
    };

    // ── GenericValueTypeBuilder<T> ──────────────────────────────────────────
    //
    // Maps a C++ type to its EGenericValueType and any extra metadata.
    // Used by TypeDescriptorBuilder field()/property() overloads.
    
    template <class T>
    struct GenericValueTypeBuilder
    {
        static EGenericValueType GetPropertyValueType()
        {
            if constexpr (detail::IsRegisteredType_v<T>)
            {
                return EGenericValueType::Struct;
            }

#define PHX_TYPE_TO_PROP_ENUM(type, enum_value) \
    if constexpr (std::is_same_v<T, type>) return EGenericValueType::enum_value;

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

#undef PHX_TYPE_TO_PROP_ENUM

            return EGenericValueType::Unknown;
        }
    };
    
    template <uint8 Tb, class T>
    struct GenericValueTypeBuilder<TFixed<Tb, T>>
    {
        static EGenericValueType GetPropertyValueType()
        {
            return EGenericValueType::FixedPoint;
        }
    };
    
    // ── TypeRef ──────────────────────────────────────────────────────────
    //
    // Describes the type of a single function parameter or return value.
    //
    //  • IsStruct()    — Descriptor is non-null: a Phoenix-registered struct type.
    //                    Inspect Descriptor->GetProperties() for field metadata.
    //  • IsPrimitive() — Descriptor is null, Primitive != Unknown.
    //  • IsVoid()      — Descriptor is null, Primitive == Unknown (void / nil).

    struct PHOENIX_SIM_API GenericValueTypeRef
    {
        EGenericValueType     Primitive   = EGenericValueType::Unknown;
        const TypeDescriptor* Descriptor  = nullptr;

        bool IsVoid()      const { return Primitive == EGenericValueType::Unknown && !Descriptor; }
        bool IsStruct()    const { return Descriptor != nullptr && Primitive == EGenericValueType::Unknown; }
        bool IsPrimitive() const { return !IsStruct() && !IsVoid(); }

        bool operator==(const GenericValueTypeRef&) const = default;
    };

    // ── MakeGenericValueTypeRef<T> ───────────────────────────────────────────────────
    //
    // Compile-time helper: maps a C++ parameter type to its TypeRef.
    //
    //   void                     → IsVoid()
    //   T with reflection traits → IsStruct() with Descriptor pointing to T's descriptor
    //   primitives               → IsPrimitive() with matching EGenericValueType

    template <class T>
    GenericValueTypeRef MakeGenericValueTypeRef()
    {
        using D = std::decay_t<T>;

        if constexpr (std::is_void_v<D>)
        {
            return {};
        }
        else if constexpr (detail::IsRegisteredType_v<D>)
        {
            // TypeDescriptor must be complete at instantiation sites that call this.
            if constexpr (detail::HasStaticTypeName<D>::value)
                return { EGenericValueType::Unknown, &D::GetStaticTypeDescriptor() };
            else
                return { EGenericValueType::Unknown, &TypeRegistry::GetOrCreate<D>() };
        }
        else
        {
            return { GenericValueTypeBuilder<D>::GetPropertyValueType(), nullptr };
        }
    }

    // ── GenericValue ──────────────────────────────────────────────────────────
    //
    // A tagged runtime value whose type is described by TypeRef.
    //
    // All values — both primitives and registered struct types — are stored in
    // a fixed-size 32-byte inline buffer via memcpy.  This avoids heap
    // allocation and the owning/non-owning distinction of the previous design.
    //
    //  • Primitives (int, float, bool, FName, …): raw bytes stored in Buffer.
    //  • Registered structs (Vec2, Transform2D, …): raw bytes stored in Buffer.
    //    Only types where sizeof(T) <= 32 are supported.
    //  • String (EGenericValueType::String): stored in the Str member.

    // ── GenericValue ──────────────────────────────────────────────────────────
    //
    // A tagged runtime value whose type is described by GenericValueTypeRef.
    //
    // All non-string values (primitives and registered structs) are stored in a
    // fixed-size inline buffer via memcpy.  String values use the std::string
    // member of the same union, sharing the same storage.
    //
    //  • Buffer  — active when Primitive != String (raw bytes).
    //  • String  — active when Primitive == String.
    //
    // The union requires explicit constructor/destructor/copy/move to handle
    // the non-trivial std::string member correctly.

    struct PHOENIX_SIM_API GenericValue
    {
        GenericValueTypeRef Type;

        union
        {
            alignas(std::max_align_t) std::byte Buffer[32];
            std::string String;
        };

        GenericValue()  noexcept { new(&Buffer) std::byte[32]{}; }
        ~GenericValue() noexcept { DestroyActive(); }

        GenericValue(const GenericValue& other) : Type(other.Type)
        {
            if (other.Type.Primitive == EGenericValueType::String)
                new(&String) std::string(other.String);
            else
                std::memcpy(Buffer, other.Buffer, sizeof(Buffer));
        }

        GenericValue(GenericValue&& other) noexcept : Type(other.Type)
        {
            if (other.Type.Primitive == EGenericValueType::String)
                new(&String) std::string(std::move(other.String));
            else
                std::memcpy(Buffer, other.Buffer, sizeof(Buffer));
        }

        GenericValue& operator=(const GenericValue& other)
        {
            if (this != &other)
            {
                DestroyActive();
                Type = other.Type;
                if (other.Type.Primitive == EGenericValueType::String)
                    new(&String) std::string(other.String);
                else
                    std::memcpy(Buffer, other.Buffer, sizeof(Buffer));
            }
            return *this;
        }

        GenericValue& operator=(GenericValue&& other) noexcept
        {
            if (this != &other)
            {
                DestroyActive();
                Type = other.Type;
                if (other.Type.Primitive == EGenericValueType::String)
                    new(&String) std::string(std::move(other.String));
                else
                    std::memcpy(Buffer, other.Buffer, sizeof(Buffer));
            }
            return *this;
        }

        static GenericValue Void() { return {}; }

        // Factory for any registered or primitive type.
        template <class T>
        static GenericValue Borrow(const T& v);

        // Extract the stored value as T.
        template <class T>
        T As() const;

    private:
        void DestroyActive() noexcept
        {
            if (Type.Primitive == EGenericValueType::String)
                String.~basic_string();
        }
    };

    // ── GenericConverter<T> ───────────────────────────────────────────────────
    //
    // Bidirectional mapping between a C++ type T and GenericValue.
    //
    // Primary template: registered struct types (PHX_DECLARE_TYPE / PHX_ENABLE_TYPE
    // or PHX_REGISTER_EXTERNAL_TYPE).  Raw bytes are stored in Buffer via memcpy.
    // sizeof(T) must not exceed 32 bytes.
    //
    // Primitive specializations below use the same inline Buffer storage.

    template <class T, class = void>
    struct GenericConverter
    {
        static_assert(detail::IsRegisteredType_v<T>,
            "GenericConverter requires T to have StaticTypeName (use PHX_DECLARE_TYPE / "
            "PHX_ENABLE_TYPE) or PHX_REGISTER_EXTERNAL_TYPE, or provide a "
            "GenericConverter<T> specialisation.");

        static_assert(sizeof(T) <= sizeof(GenericValue::Buffer),
            "Type is too large for GenericValue inline buffer (max 32 bytes). "
            "Provide a GenericConverter<T> specialisation for larger types.");

        static GenericValue Borrow(const T& v)
        {
            GenericValue gv;
            if constexpr (detail::HasStaticTypeName<T>::value)
                gv.Type.Descriptor = &T::GetStaticTypeDescriptor();
            else
                gv.Type.Descriptor = &TypeRegistry::GetOrCreate<T>();
            std::memcpy(gv.Buffer, &v, sizeof(T));
            return gv;
        }

        static T From(const GenericValue& gv)
        {
            T result;
            std::memcpy(&result, gv.Buffer, sizeof(T));
            return result;
        }
    };

    // ── Primitive specialisations ─────────────────────────────────────────────

#define PHX_GC(CppType, EKind) \
    template <> struct GenericConverter<CppType> \
    { \
        static GenericValue Borrow(CppType v) \
        { \
            GenericValue gv; \
            gv.Type.Primitive = EGenericValueType::EKind; \
            std::memcpy(gv.Buffer, &v, sizeof(v)); \
            return gv; \
        } \
        static CppType From(const GenericValue& gv) \
        { \
            CppType v; \
            std::memcpy(&v, gv.Buffer, sizeof(v)); \
            return v; \
        } \
    };

    PHX_GC(int8_t,          Int8)
    PHX_GC(uint8_t,         UInt8)
    PHX_GC(int16_t,         Int16)
    PHX_GC(uint16_t,        UInt16)
    PHX_GC(int32_t,         Int32)
    PHX_GC(uint32_t,        UInt32)
    PHX_GC(int64_t,         Int64)
    PHX_GC(uint64_t,        UInt64)
    PHX_GC(bool,            Bool)
    PHX_GC(float,           Float)
    PHX_GC(double,          Double)
    PHX_GC(FName,           Name)
    PHX_GC(ECS::EntityId,   UInt32)

#undef PHX_GC

    template <> struct GenericConverter<std::string>
    {
        static GenericValue Borrow(std::string v)
        {
            GenericValue gv;
            gv.Type.Primitive = EGenericValueType::String;
            new(&gv.String) std::string(std::move(v));
            return gv;
        }
        static std::string From(const GenericValue& gv) { return gv.String; }
    };

    template <uint8 Tb, class T>
    struct GenericConverter<TFixed<Tb, T>>
    {
        static GenericValue Borrow(TFixed<Tb, T> v)
        {
            GenericValue gv;
            gv.Type.Primitive = EGenericValueType::FixedPoint;
            if constexpr (detail::HasExternalTypeTraits<TFixed<Tb, T>>::value)
                gv.Type.Descriptor = &TypeRegistry::GetOrCreate<TFixed<Tb, T>>();
            std::memcpy(gv.Buffer, &v, sizeof(v));
            return gv;
        }
        static TFixed<Tb, T> From(const GenericValue& gv)
        {
            TFixed<Tb, T> v;
            std::memcpy(&v, gv.Buffer, sizeof(v));
            return v;
        }
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

} // namespace Phoenix
