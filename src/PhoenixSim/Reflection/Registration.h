#pragma once

#include "PhoenixSim/Reflection/EnumDescriptorBuilder.h"
#include "PhoenixSim/Reflection/TypeDescriptorBuilder.h"
#include "PhoenixSim/Reflection/TypeRegistry.h"

namespace Phoenix
{
    template <class T>
    struct TypeRegistrar
    {
        static void Register(const TypeDescriptorBuilder<T>&) { }
    };

    template <class T>
    struct EnumRegistrar
    {
        static void Register(const EnumDescriptorBuilder<T>&) { }
    };

    // Specialize this for external types when you can't modify the type declaration to include PHX_DECLARE_TYPE.
    template <class T>
    struct ExternalTypeRegistration {};

    template <class TRet, class T, class ...TArgs>
    auto MethodCast(auto fn)
    {
        return static_cast<TRet(T::*)(TArgs...)>(fn);
    }

    template <class TRet, class T, class ...TArgs>
    auto MethodCast(TRet(T::*fn)(TArgs...) const)
    {
        return static_cast<TRet(T::*)(TArgs...) const>(fn);
    }

    template <class TRet, class ...TArgs>
    auto StaticMethodCast(TRet(*fn)(TArgs...))
    {
        return static_cast<TRet(*)(TArgs...)>(fn);
    }

#define _PHX_TYPE_COMMON(type, ...) \
    private: \
        inline static const bool _s_phx_type_init_ = ( \
            [] { Phoenix::TypeDescriptorBuilder<type>{}.template Bases<__VA_ARGS__>(); }(), true); \
    public:

    // Use this in the declaration of a type with no virtual bases and no derivations.
    // Useful for final types or for interfaces that are not intended to be derived from.
#define PHX_DECLARE_TYPE(type, ...) \
    _PHX_TYPE_COMMON(type, __VA_ARGS__) \
    public: \
        const Phoenix::TypeDescriptor& GetTypeDescriptor() const { return Phoenix::TypeRegistry::Get<type>(); }

    // Use this in the declaration of a type with no bases that may have derivations.
    // Useful for non-final types that are intended to be derived from.
#define PHX_DECLARE_TYPE_INTERFACE(type) \
    _PHX_TYPE_COMMON(type) \
    public: \
        virtual const Phoenix::TypeDescriptor& GetTypeDescriptor() const { return Phoenix::TypeRegistry::Get<type>(); }

    // Use this in the declaration of a type with bases.
    // Useful for types that derive from other types, regardless of whether they are final or not.
#define PHX_DECLARE_TYPE_DERIVED(type, ...) \
    _PHX_TYPE_COMMON(type, __VA_ARGS__) \
    public: \
        const Phoenix::TypeDescriptor& GetTypeDescriptor() const override { return Phoenix::TypeRegistry::Get<type>(); }

#define PHX_DEFINE_TYPE(type) \
    namespace Phoenix { \
        template <> struct TypeRegistrar<type> { \
            static void Register(Phoenix::TypeDescriptorBuilder<type>& registration); \
        }; \
    } \
    inline void Phoenix::TypeRegistrar<type>::Register(Phoenix::TypeDescriptorBuilder<type>& registration)

#define PHX_DEFINE_ENUM(type) \
    namespace Phoenix { \
        template <> struct EnumRegistrar<type> { \
            static void Register(Phoenix::EnumDescriptorBuilder<type>& registration); \
        }; \
    } \
    inline void Phoenix::EnumRegistrar<type>::Register(Phoenix::EnumDescriptorBuilder<type>& registration)
} // namespace Phoenix
