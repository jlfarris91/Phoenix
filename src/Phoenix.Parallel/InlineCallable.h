#pragma once

#include <cstddef>
#include <memory>      // std::destroy_at — works around MSVC's pseudo-dtor quirk
#include <new>
#include <type_traits>
#include <utility>

#include "Phoenix/Platform.h"

namespace Phoenix
{
    // TInlineCallable<Sig, Capacity>
    //
    // A type-erased callable that stores its target *inline* in a fixed-size,
    // suitably-aligned byte buffer — no heap allocation, ever. Compared to
    // std::function:
    //
    //   • std::function will heap-allocate if the target exceeds its
    //     implementation-defined SBO (16 B on libstdc++, ~32 B on libc++,
    //     ~40 B on MSVC). That allocation costs ~100 ns and pulls in a cache
    //     line on every Submit.
    //   • TInlineCallable refuses oversized targets at compile time. The
    //     static_assert is the contract: if it trips, either reduce the
    //     capture or raise the buffer size at the typedef site.
    //
    // Layout: small invoker function pointer + small vtable pointer + the
    // inline storage. Move semantics are noexcept; copy is implemented
    // because Task is copied at one API point: ThreadPool::Submit
    // copy-assigns the incoming Task into its slab slot. The TInlineCallable
    // copy invokes the captured callable's copy constructor — which for the
    // lambdas used in the engine is itself trivial.
    //
    // The vtable is a static constexpr per target type, so the cost beyond
    // the buffer is one pointer per instance (Invoker).
    template <class TSig, std::size_t Capacity = 128>
    class TInlineCallable;

    template <class TRet, class... TArgs, std::size_t Capacity>
    class TInlineCallable<TRet(TArgs...), Capacity>
    {
        struct Ops
        {
            void (*CopyConstruct)(const void* src, void* dst);
            void (*MoveConstruct)(void* src, void* dst);
            void (*Destroy)(void* p);
        };

        template <class TDecayed>
        static constexpr Ops kOps = {
            [](const void* src, void* dst)
            {
                ::new (dst) TDecayed(*static_cast<const TDecayed*>(src));
            },
            [](void* src, void* dst)
            {
                ::new (dst) TDecayed(std::move(*static_cast<TDecayed*>(src)));
            },
            [](void* p)
            {
                // std::destroy_at rather than the explicit pseudo-destructor
                // call (->~TDecayed()). MSVC rejects the latter when
                // TDecayed is a lambda type with a non-trivial destructor —
                // the destructor name does not lexically match the lambda's
                // compiler-generated name, and MSVC's pseudo-destructor
                // lookup fails. std::destroy_at sidesteps the lookup.
                std::destroy_at(static_cast<TDecayed*>(p));
            },
        };

    public:
        static constexpr std::size_t kCapacity = Capacity;

        TInlineCallable() noexcept = default;

        // SFINAE on F to keep copy/move overloads preferred over this constructor
        // when an existing TInlineCallable is passed in.
        template <
            class F,
            class = std::enable_if_t<!std::is_same_v<std::decay_t<F>, TInlineCallable>>>
        TInlineCallable(F&& f)
        {
            using TDecayed = std::decay_t<F>;
            static_assert(
                sizeof(TDecayed) <= Capacity,
                "TInlineCallable: callable is too large for the inline buffer. "
                "Reduce capture size, or instantiate the holder with a larger Capacity.");
            static_assert(
                alignof(TDecayed) <= alignof(std::max_align_t),
                "TInlineCallable: callable alignment exceeds std::max_align_t.");

            ::new (&Storage) TDecayed(std::forward<F>(f));
            Invoke = [](void* storage, TArgs... args) -> TRet
            {
                return (*reinterpret_cast<TDecayed*>(storage))(std::forward<TArgs>(args)...);
            };
            OpsPtr = &kOps<TDecayed>;
        }

        TInlineCallable(const TInlineCallable& other)
        {
            if (other.OpsPtr)
            {
                other.OpsPtr->CopyConstruct(&other.Storage, &Storage);
                Invoke = other.Invoke;
                OpsPtr = other.OpsPtr;
            }
        }

        TInlineCallable(TInlineCallable&& other) noexcept
        {
            if (other.OpsPtr)
            {
                other.OpsPtr->MoveConstruct(&other.Storage, &Storage);
                Invoke = other.Invoke;
                OpsPtr = other.OpsPtr;
                // Leave 'other' in a destructible state by destroying its body.
                other.OpsPtr->Destroy(&other.Storage);
                other.Invoke = nullptr;
                other.OpsPtr = nullptr;
            }
        }

        TInlineCallable& operator=(const TInlineCallable& other)
        {
            if (this == &other) return *this;
            Reset();
            if (other.OpsPtr)
            {
                other.OpsPtr->CopyConstruct(&other.Storage, &Storage);
                Invoke = other.Invoke;
                OpsPtr = other.OpsPtr;
            }
            return *this;
        }

        TInlineCallable& operator=(TInlineCallable&& other) noexcept
        {
            if (this == &other) return *this;
            Reset();
            if (other.OpsPtr)
            {
                other.OpsPtr->MoveConstruct(&other.Storage, &Storage);
                Invoke = other.Invoke;
                OpsPtr = other.OpsPtr;
                other.OpsPtr->Destroy(&other.Storage);
                other.Invoke = nullptr;
                other.OpsPtr = nullptr;
            }
            return *this;
        }

        ~TInlineCallable()
        {
            Reset();
        }

        explicit operator bool() const noexcept { return Invoke != nullptr; }

        TRet operator()(TArgs... args) const
        {
            PHX_ASSERT(Invoke != nullptr);
            // The body is logically mutable: the callable may capture by value
            // and mutate its own state across invocations. Strip const off the
            // storage pointer here; the public API stays const-correct.
            void* mutableStorage = const_cast<void*>(static_cast<const void*>(&Storage));
            return Invoke(mutableStorage, std::forward<TArgs>(args)...);
        }

    private:
        void Reset() noexcept
        {
            if (OpsPtr)
            {
                OpsPtr->Destroy(&Storage);
                Invoke = nullptr;
                OpsPtr = nullptr;
            }
        }

        // Storage first so its alignment dictates the class alignment.
        alignas(std::max_align_t) std::byte Storage[Capacity]{};
        TRet (*Invoke)(void*, TArgs...) = nullptr;
        const Ops* OpsPtr = nullptr;
    };
}
