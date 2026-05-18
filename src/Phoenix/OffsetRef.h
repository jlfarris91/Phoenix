#pragma once

#include "Platform.h"

namespace Phoenix
{
    PHX_FORCEINLINE int32 CalculatePtrOffset(const void* a, const void* b)
    {
        const uint8* a8 = static_cast<const uint8*>(a);
        const uint8* b8 = static_cast<const uint8*>(b);
        int64 offset = b8 - a8;
        PHX_ASSERT(std::abs(offset) < INT32_MAX);
        return static_cast<int32>(offset);
    }

    struct PHOENIX_SIM_API OffsetRefBase
    {
        PHX_FORCEINLINE constexpr bool IsValid() const
        {
            return Offset != INT32_MAX;
        }

        PHX_FORCEINLINE constexpr operator bool() const
        {
            return IsValid();
        }

        PHX_FORCEINLINE void Reset()
        {
            Offset = INT32_MAX;
        }

        PHX_FORCEINLINE constexpr bool operator==(const OffsetRefBase& other) const = default;
        PHX_FORCEINLINE constexpr bool operator!=(const OffsetRefBase& other) const = default;

    protected:

        PHX_FORCEINLINE constexpr OffsetRefBase() = default;

        PHX_FORCEINLINE constexpr explicit OffsetRefBase(int32 offset)
            : Offset(offset)
        {
        }

        PHX_FORCEINLINE explicit OffsetRefBase(const void* base, const void* ptr)
            : Offset(CalculatePtrOffset(base, ptr))
        {
        }

        PHX_FORCEINLINE constexpr OffsetRefBase(const OffsetRefBase& other)
            : Offset(other.Offset)
        {
        }

        PHX_FORCEINLINE constexpr OffsetRefBase(OffsetRefBase&& other) noexcept
            : Offset(other.Offset)
        {
            other.Offset = INT32_MAX;
        }

        PHX_FORCEINLINE constexpr ~OffsetRefBase() = default;

        PHX_FORCEINLINE constexpr OffsetRefBase& operator=(const OffsetRefBase& other) = default;

        PHX_FORCEINLINE constexpr OffsetRefBase& operator=(OffsetRefBase&& other) noexcept
        {
            Offset = other.Offset;
            other.Offset = INT32_MAX;
            return *this;
        }

        void Reset(const void* base, const void* ptr)
        {
            Offset = CalculatePtrOffset(base, ptr);
        }

        int32 Offset = INT32_MAX;
    };

    struct PHOENIX_SIM_API OffsetRef : OffsetRefBase
    {
        PHX_FORCEINLINE constexpr OffsetRef() = default;
        PHX_FORCEINLINE constexpr OffsetRef(int32 offset) : OffsetRefBase(offset) {}
        PHX_FORCEINLINE OffsetRef(const void* base, const void* ptr) : OffsetRefBase(base, ptr) {}
        PHX_FORCEINLINE constexpr OffsetRef(const OffsetRef& other) = default;
        PHX_FORCEINLINE constexpr OffsetRef(OffsetRef&& other) noexcept = default;

        PHX_FORCEINLINE void* Get(void* base) const
        {
            PHX_ASSERT(IsValid());
            uint8* base8 = static_cast<uint8*>(base);
            return base8 + Offset;
        }

        PHX_FORCEINLINE const void* Get(const void* base) const
        {
            PHX_ASSERT(IsValid());
            const uint8* base8 = static_cast<const uint8*>(base);
            return base8 + Offset;
        }

        PHX_FORCEINLINE void Reset(const void* base, const void* ptr)
        {
            OffsetRefBase::Reset(base, ptr);
        }

        PHX_FORCEINLINE constexpr OffsetRef& operator=(const OffsetRef& other) = default;
        PHX_FORCEINLINE constexpr OffsetRef& operator=(OffsetRef&& other) noexcept = default;
    };

    template <class T>
    struct TOffsetRef : OffsetRefBase
    {
        PHX_FORCEINLINE constexpr TOffsetRef() = default;
        PHX_FORCEINLINE constexpr TOffsetRef(int32 offset) : OffsetRefBase(offset) {}
        PHX_FORCEINLINE TOffsetRef(const void* base, const void* ptr) : OffsetRefBase(base, ptr) {}
        PHX_FORCEINLINE constexpr TOffsetRef(const TOffsetRef& other) = default;
        PHX_FORCEINLINE constexpr TOffsetRef(TOffsetRef&& other) noexcept = default;

        PHX_FORCEINLINE constexpr T* Get(void* base) const
        {
            PHX_ASSERT(IsValid());
            uint8* base8 = static_cast<uint8*>(base);
            return reinterpret_cast<T*>(base8 + Offset);
        }

        PHX_FORCEINLINE constexpr const T* Get(const void* base) const
        {
            PHX_ASSERT(IsValid());
            const uint8* base8 = static_cast<const uint8*>(base);
            return reinterpret_cast<const T*>(base8 + Offset);
        }

        PHX_FORCEINLINE void Reset(const void* base, const T* ptr)
        {
            OffsetRefBase::Reset(base, ptr);
        }

        PHX_FORCEINLINE constexpr TOffsetRef& operator=(const TOffsetRef& other) = default;
        PHX_FORCEINLINE constexpr TOffsetRef& operator=(TOffsetRef&& other) noexcept = default;

        T* operator->() = delete;
        T& operator*() = delete;
    };

    struct PHOENIX_SIM_API SelfOffsetRef : OffsetRefBase
    {
        PHX_FORCEINLINE constexpr SelfOffsetRef() = default;
        PHX_FORCEINLINE constexpr SelfOffsetRef(int32 offset) : OffsetRefBase(offset) {}
        PHX_FORCEINLINE SelfOffsetRef(const void* ptr) : OffsetRefBase(this, ptr) {}
        PHX_FORCEINLINE constexpr SelfOffsetRef(const SelfOffsetRef& other) = default;
        PHX_FORCEINLINE constexpr SelfOffsetRef(SelfOffsetRef&& other) noexcept = default;

        PHX_FORCEINLINE void* Get()
        {
            PHX_ASSERT(IsValid());
            uint8* base8 = reinterpret_cast<uint8*>(this);
            return base8 + Offset;
        }

        PHX_FORCEINLINE const void* Get() const
        {
            PHX_ASSERT(SelfOffsetRef::IsValid());
            const uint8* base8 = reinterpret_cast<const uint8*>(this);
            return base8 + Offset;
        }

        PHX_FORCEINLINE void Reset(const void* ptr)
        {
            OffsetRefBase::Reset(this, ptr);
        }

        PHX_FORCEINLINE constexpr SelfOffsetRef& operator=(const SelfOffsetRef& other) = default;
        PHX_FORCEINLINE constexpr SelfOffsetRef& operator=(SelfOffsetRef&& other) noexcept = default;
    };

    template <class T>
    struct TSelfOffsetRef : OffsetRefBase
    {
        PHX_FORCEINLINE constexpr TSelfOffsetRef() = default;
        PHX_FORCEINLINE constexpr TSelfOffsetRef(int32 offset) : OffsetRefBase(offset) {}
        PHX_FORCEINLINE constexpr TSelfOffsetRef(const void* ptr) : OffsetRefBase(this, ptr) {}
        PHX_FORCEINLINE constexpr TSelfOffsetRef(const TSelfOffsetRef& other) = default;
        PHX_FORCEINLINE constexpr TSelfOffsetRef(TSelfOffsetRef&& other) noexcept = default;

        PHX_FORCEINLINE T* Get()
        {
            PHX_ASSERT(IsValid());
            uint8* base8 = reinterpret_cast<uint8*>(this);
            return reinterpret_cast<T*>(base8 + Offset);
        }

        PHX_FORCEINLINE const T* Get() const
        {
            PHX_ASSERT(IsValid());
            const uint8* base8 = reinterpret_cast<const uint8*>(this);
            return reinterpret_cast<const T*>(base8 + Offset);
        }

        PHX_FORCEINLINE constexpr TSelfOffsetRef& operator=(const TSelfOffsetRef& other) = default;
        PHX_FORCEINLINE constexpr TSelfOffsetRef& operator=(TSelfOffsetRef&& other) noexcept = default;

        PHX_FORCEINLINE void Reset(const T* ptr)
        {
            OffsetRefBase::Reset(this, ptr);
        }

        PHX_FORCEINLINE T* operator->()
        {
            return Get();
        }

        PHX_FORCEINLINE T& operator*()
        {
            return *Get();
        }
    };
}