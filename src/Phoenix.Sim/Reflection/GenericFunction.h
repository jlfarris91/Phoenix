#pragma once

#include <type_traits>

// ── GenericFunction — type-erased callable ────────────────────────────────────
//
// A GenericFunction is a slim, type-erased callable.  It captures an Invoke
// function and a HasSelfParam flag — nothing more.
//
// Parameter/return type metadata and naming live in MethodDescriptor (defined
// in Reflection.h), which owns a GenericFunction as its invocation primitive.
//
// Calling convention:
//   result = fn.Invoke(self, { arg0, arg1, … })
//   'self' is the object pointer for instance methods; nullptr for static
//   functions.  For constructors, 'self' is the pre-allocated memory to
//   construct into.
//
// Factory helpers produce a GenericFunction from a raw pointer:
//   MakeGenericFunction(TRet(*)(TArgs...))               — free/static function
//   MakeGenericFunction(TRet(TClass::*)(TArgs...))       — instance method
//   MakeGenericFunction(TRet(TClass::*)(TArgs...) const) — const instance method

#include <functional>
#include <span>
#include <type_traits>

#include "Phoenix.Sim/Reflection/Variant.h"

namespace Phoenix
{
    class TypeDescriptor;

    class PHOENIX_SIM_API GenericFunction
    {
        using TInvoker = std::function<Variant(void* self, const std::span<const Variant>&)>;

    public:

        GenericFunction() = default;

        GenericFunction(TInvoker&& invoker, bool hasSelfParam)
            : Invoke(std::move(invoker))
            , bHasSelf(hasSelfParam)
        {
        }

        GenericFunction(const GenericFunction&) = default;
        GenericFunction(GenericFunction&&) = default;

        GenericFunction& operator=(const GenericFunction&) = default;
        GenericFunction& operator=(GenericFunction&&) = default;

        bool IsBound() const
        {
            return Invoke != nullptr;
        }

        bool HasSelf() const
        {
            return bHasSelf;
        }

        Variant operator()(void* self, const std::span<const Variant>& args) const
        {
            return Invoke(self, args);
        }

    private:
        TInvoker Invoke;
        bool bHasSelf = false;
    };

    // ── Factory implementation helpers ────────────────────────────────────────

    namespace detail
    {
        // Strips rvalue-reference only (T&& → T), leaving T& and T unchanged.
        // Used so Variant::As<> reads values for rvalue params without breaking
        // lvalue-reference params like WorldRef (= World&).
        template <class T> struct RemoveRvalueRef     { using type = T; };
        template <class T> struct RemoveRvalueRef<T&&>{ using type = T; };
        template <class T> using RemoveRvalueRef_t = typename RemoveRvalueRef<T>::type;
        // ── Free static function ──────────────────────────────────────────────

        template <class TRet, class... TArgs, size_t... I>
        GenericFunction MakeGenericFunctionImpl(TRet(*fn)(TArgs...), std::index_sequence<I...>)
        {
            auto wrapper = [fn](void*, const std::span<const Variant>& args) -> Variant
            {
                if constexpr (std::is_void_v<TRet>)
                {
                    fn(args[I].template As<RemoveRvalueRef_t<TArgs>>()...);
                    return Variant::Void();
                }
                else
                {
                    return fn(args[I].template As<RemoveRvalueRef_t<TArgs>>()...);
                }
            };
            return { std::move(wrapper), false };
        }

        // ── Instance method ───────────────────────────────────────────────────

        template <class TClass, class TRet, class... TArgs, size_t... I>
        GenericFunction MakeGenericMethodImpl(TRet(TClass::*fn)(TArgs...), std::index_sequence<I...>)
        {
            auto wrapper = [fn](void* self, const std::span<const Variant>& args) -> Variant
            {
                auto* obj = static_cast<TClass*>(self);
                if constexpr (std::is_void_v<TRet>)
                {
                    (obj->*fn)(args[I].template As<RemoveRvalueRef_t<TArgs>>()...);
                    return Variant::Void();
                }
                else
                {
                    return (obj->*fn)(args[I].template As<RemoveRvalueRef_t<TArgs>>()...);
                }
            };
            return { std::move(wrapper), true };
        }

        // ── Const instance method ─────────────────────────────────────────────

        template <class TClass, class TRet, class... TArgs, size_t... I>
        GenericFunction MakeGenericConstMethodImpl(TRet(TClass::*fn)(TArgs...) const, std::index_sequence<I...>)
        {
            auto wrapper = [fn](void* self, const std::span<const Variant>& args) -> Variant
            {
                const auto* obj = static_cast<const TClass*>(self);
                if constexpr (std::is_void_v<TRet>)
                {
                    (obj->*fn)(args[I].template As<RemoveRvalueRef_t<TArgs>>()...);
                    return Variant::Void();
                }
                else
                {
                    return (obj->*fn)(args[I].template As<RemoveRvalueRef_t<TArgs>>()...);
                }
            };
            return { std::move(wrapper), true };
        }

        template <class TRet, class... TArgs, size_t... I>
        GenericFunction MakeGenericFunctionImpl(
            std::function<TRet(TArgs...)>&& fn,
            std::index_sequence<I...>)
        {
            auto wrapper = [fn = std::move(fn)](void*, const std::span<const Variant>& args) -> Variant
            {
                if constexpr (std::is_void_v<TRet>)
                {
                    fn(args[I].template As<RemoveRvalueRef_t<TArgs>>()...);
                    return Variant::Void();
                }
                else
                {
                    return fn(args[I].template As<RemoveRvalueRef_t<TArgs>>()...);
                }
            };
            return { std::move(wrapper), false };
        }

        template <class TRet, class... TArgs, size_t... I>
        GenericFunction MakeGenericFunctionTakingSelfImpl(
            std::function<TRet(void*, TArgs...)>&& fn,
            std::index_sequence<I...>)
        {
            auto wrapper = [fn = std::move(fn)](void* self, const std::span<const Variant>& args) -> Variant
            {
                if constexpr (std::is_void_v<TRet>)
                {
                    fn(self, args[I].template As<RemoveRvalueRef_t<TArgs>>()...);
                    return Variant::Void();
                }
                else
                {
                    return fn(self, args[I].template As<RemoveRvalueRef_t<TArgs>>()...);
                }
            };
            return { std::move(wrapper), true };
        }
    } // namespace detail

    // ── Public factory overloads ──────────────────────────────────────────────

    template <class TRet, class... TArgs>
    GenericFunction MakeGenericFunction(TRet(*fn)(TArgs...))
    {
        return detail::MakeGenericFunctionImpl(fn, std::index_sequence_for<TArgs...>{});
    }

    template <class TClass, class TRet, class... TArgs>
    GenericFunction MakeGenericFunction(TRet(TClass::*fn)(TArgs...))
    {
        return detail::MakeGenericMethodImpl(fn, std::index_sequence_for<TArgs...>{});
    }

    template <class TClass, class TRet, class... TArgs>
    GenericFunction MakeGenericFunction(TRet(TClass::*fn)(TArgs...) const)
    {
        return detail::MakeGenericConstMethodImpl(fn, std::index_sequence_for<TArgs...>{});
    }

    template <class TRet, class... TArgs>
    GenericFunction MakeGenericFunction(std::function<TRet(TArgs...)>&& fn)
    {
        return detail::MakeGenericFunctionImpl(std::move(fn), std::index_sequence_for<TArgs...>{});
    }

    template <class TRet, class... TArgs>
    GenericFunction MakeGenericFunctionTakingSelf(std::function<TRet(void*, TArgs...)>&& fn)
    {
        return detail::MakeGenericFunctionTakingSelfImpl(std::move(fn), std::index_sequence_for<TArgs...>{});
    }
} // namespace Phoenix
