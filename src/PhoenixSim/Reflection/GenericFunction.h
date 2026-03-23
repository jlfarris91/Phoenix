#pragma once

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

#include "PhoenixSim/Reflection/GenericValue.h"

namespace Phoenix
{
    struct TypeDescriptor;  // complete definition in Reflection.h

    struct PHOENIX_SIM_API GenericFunction
    {
        bool HasSelfParam = false;

        using TInvoker = std::function<GenericValue(void* self, std::span<const GenericValue>)>;
        TInvoker Invoke;
    };

    // ── Factory implementation helpers ────────────────────────────────────────

    namespace detail
    {
        // Builds a GenericValue from a function return value.
        // All types use Borrow — the buffer-based GenericValue copies bytes inline.
        template <class TRet>
        GenericValue MakeReturnValue(TRet&& v)
        {
            return GenericConverter<std::decay_t<TRet>>::Borrow(v);
        }

        // ── Free static function ──────────────────────────────────────────────

        template <class TRet, class... TArgs, size_t... I>
        GenericFunction MakeGenericFunctionImpl(TRet(*fn)(TArgs...), std::index_sequence<I...>)
        {
            GenericFunction d;
            d.HasSelfParam = false;
            d.Invoke = [fn](void*, std::span<const GenericValue> args) -> GenericValue
            {
                if constexpr (std::is_void_v<TRet>)
                {
                    fn(GenericConverter<std::decay_t<TArgs>>::From(args[I])...);
                    return GenericValue::Void();
                }
                else
                {
                    return MakeReturnValue(fn(GenericConverter<std::decay_t<TArgs>>::From(args[I])...));
                }
            };
            return d;
        }

        // ── Instance method ───────────────────────────────────────────────────

        template <class TClass, class TRet, class... TArgs, size_t... I>
        GenericFunction MakeGenericMethodImpl(TRet(TClass::*fn)(TArgs...), std::index_sequence<I...>)
        {
            GenericFunction d;
            d.HasSelfParam = true;
            d.Invoke = [fn](void* self, std::span<const GenericValue> args) -> GenericValue
            {
                auto* obj = static_cast<TClass*>(self);
                if constexpr (std::is_void_v<TRet>)
                {
                    (obj->*fn)(GenericConverter<std::decay_t<TArgs>>::From(args[I])...);
                    return GenericValue::Void();
                }
                else
                {
                    return MakeReturnValue((obj->*fn)(GenericConverter<std::decay_t<TArgs>>::From(args[I])...));
                }
            };
            return d;
        }

        // ── Const instance method ─────────────────────────────────────────────

        template <class TClass, class TRet, class... TArgs, size_t... I>
        GenericFunction MakeGenericConstMethodImpl(TRet(TClass::*fn)(TArgs...) const, std::index_sequence<I...>)
        {
            GenericFunction d;
            d.HasSelfParam = true;
            d.Invoke = [fn](void* self, std::span<const GenericValue> args) -> GenericValue
            {
                const auto* obj = static_cast<const TClass*>(self);
                if constexpr (std::is_void_v<TRet>)
                {
                    (obj->*fn)(GenericConverter<std::decay_t<TArgs>>::From(args[I])...);
                    return GenericValue::Void();
                }
                else
                {
                    return MakeReturnValue((obj->*fn)(GenericConverter<std::decay_t<TArgs>>::From(args[I])...));
                }
            };
            return d;
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

} // namespace Phoenix
