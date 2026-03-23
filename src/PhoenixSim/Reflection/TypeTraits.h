#pragma once

#include "PhoenixSim/Name.h"

namespace Phoenix
{
    // ── TypeTraits<T> ─────────────────────────────────────────────────────────
    //
    // External type traits — specialize for types you don't control (typedefs,
    // template instantiations) to register them with the reflection system.
    //
    // Usage: PHX_REGISTER_EXTERNAL_TYPE(Vec2, "Vec2")

    template <class T>
    struct TypeTraits {};

    // ── PHX_REGISTER_EXTERNAL_TYPE ────────────────────────────────────────────
    //
    // Specialises TypeTraits<T> so that the reflection system (TypeRegistry,
    // GenericConverter, GenericValueTypeBuilder) can handle T without
    // modifying T's class body.

#define PHX_REGISTER_EXTERNAL_TYPE(CppType, TypeNameStr)                  \
    namespace Phoenix {                                                   \
    template <>                                                           \
    struct TypeTraits<CppType>                                            \
    {                                                                     \
        static constexpr Phoenix::FName StaticTypeName  = TypeNameStr##_n;\
        static constexpr const char*    StaticTypeCName = TypeNameStr;    \
    };                                                                    \
    } /* namespace Phoenix */

    // ── detail helpers ────────────────────────────────────────────────────────

    namespace detail
    {
        // Detects whether T has a StaticTypeName member (PHX_DECLARE_TYPE /
        // PHX_ENABLE_TYPE).
        template <class T, class = void>
        struct HasStaticTypeName : std::false_type {};

        template <class T>
        struct HasStaticTypeName<T, std::void_t<decltype(T::StaticTypeName)>>
            : std::true_type {};

        // Detects whether TypeTraits<T> has been specialized via PHX_REGISTER_EXTERNAL_TYPE.
        template <class T, class = void>
        struct HasExternalTypeTraits : std::false_type {};

        template <class T>
        struct HasExternalTypeTraits<T, std::void_t<decltype(TypeTraits<T>::StaticTypeName)>>
            : std::true_type {};

        // True if T is registered with the reflection system (either in-class
        // or externally).
        template <class T>
        constexpr bool IsRegisteredType_v =
            HasStaticTypeName<T>::value || HasExternalTypeTraits<T>::value;

        // ── Compile-time name accessors ───────────────────────────────────────
        //
        // Used by TypeRegistry::GetOrCreate<T>() to retrieve the type name
        // regardless of whether it was registered in-class or externally.

        template <class T>
        constexpr FName TypeFName()
        {
            if constexpr (HasStaticTypeName<T>::value)
                return T::StaticTypeName;
            else
                return TypeTraits<T>::StaticTypeName;
        }

        template <class T>
        constexpr const char* TypeCName()
        {
            if constexpr (HasStaticTypeName<T>::value)
                return T::StaticTypeCName;
            else
                return TypeTraits<T>::StaticTypeCName;
        }

    } // namespace detail
}