#pragma once

#include "PhoenixSim/Name.h"

#include <cstring>
#include <string>
#include <string_view>
#include <typeinfo>

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

#define PHX_REGISTER_EXTERNAL_TYPE_AS(CppType, TypeNameStr)               \
    namespace Phoenix {                                                   \
    template <>                                                           \
    struct TypeTraits<CppType>                                            \
    {                                                                     \
        static constexpr Phoenix::FName StaticTypeName  = TypeNameStr##_n;\
        static constexpr const char*    StaticTypeCName = TypeNameStr;    \
    };                                                                    \
    }

#define PHX_REGISTER_EXTERNAL_TYPE(CppType) PHX_REGISTER_EXTERNAL_TYPE_AS(CppType, #CppType)

    // ── detail helpers ────────────────────────────────────────────────────────

    namespace detail
    {
        // Detects whether T has a StaticTypeName member (PHX_DECLARE_TYPE /
        // PHX_DECLARE_TYPE).
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

        // Returns the fully qualified C++ name of T (e.g. "Phoenix::RTS::FeatureUnit").
        // On MSVC, typeid(T).name() produces a readable decorated name; we strip the
        // leading "class"/"struct"/"enum" keyword.
        // On GCC/Clang, __PRETTY_FUNCTION__ is parsed into a static std::string.
        // The returned pointer is valid for the lifetime of the program.
        template <class T>
        const char* QualifiedTypeCName()
        {
#if defined(_MSC_VER)
            const char* raw = typeid(T).name();
            if (strncmp(raw, "class ",  6) == 0) return raw + 6;
            if (strncmp(raw, "struct ", 7) == 0) return raw + 7;
            if (strncmp(raw, "enum ",   5) == 0) return raw + 5;
            return raw;
#else
            static const std::string name = []() -> std::string {
                std::string_view f = __PRETTY_FUNCTION__;
                const auto start = f.find("T = ");
                if (start == std::string_view::npos) return "";
                const auto end = f.find_first_of(";]", start + 4);
                return std::string(f.substr(start + 4, end - (start + 4)));
            }();
            return name.c_str();
#endif
        }

    } // namespace detail
}