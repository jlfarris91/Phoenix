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
#include <cstring>

#include "PhoenixSim/Flags.h"
#include "PhoenixSim/Name.h"
#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Reflection/TypeName.h"

namespace Phoenix
{
    class TypeDescriptor;

    enum class EVariantFlags : uint8_t
    {
        None        = 0,
        OwnsData    = 1 << 0,
        Ref         = 1 << 1,
        Const       = 1 << 2,
        ConstRef    = Ref | Const
    };

    class PHOENIX_SIM_API Variant
    {
    public:
        static constexpr uint32 Capacity = 40;

        Variant() noexcept;
        ~Variant() noexcept;
    
        template <class T>
        Variant(const T& value) noexcept
        {
            TypeId = StaticTypeName<std::decay_t<T>>::TypeId;
            if constexpr (sizeof(T) < sizeof(Buffer))
            {
                Flags = EVariantFlags::OwnsData;
                std::memcpy(Buffer, &value, sizeof(T));
            }
            else
            {
                Flags = EVariantFlags::ConstRef;
                *reinterpret_cast<T* const*>(&Buffer[0]) = &value;
            }
        }

        template <class T>
        Variant(T& ref) noexcept
        {
            TypeId = StaticTypeName<std::decay_t<T>>::TypeId;
            Flags = EVariantFlags::Ref;
            *reinterpret_cast<T**>(&Buffer[0]) = &ref;
        }

        Variant(const TypeDescriptor& desc);

        Variant(const Variant& other);

        Variant(Variant&& other) noexcept;

        static Variant Void();

        Variant& operator=(const Variant& other);
        Variant& operator=(Variant&& other) noexcept;

        bool operator==(const Variant& other) const;
        bool operator!=(const Variant& other) const;

        bool OwnsData() const;
        bool IsRef() const;
        bool IsConstRef() const;

        FName GetTypeId() const;

        const TypeDescriptor* GetType() const;

        void* GetData();
        const void* GetData() const;

        uint32 GetSize() const;

        template <class T>
        T* GetData()
        {
            return static_cast<std::decay_t<T>*>(GetData());
        }

        template <class T>
        const T* GetData() const
        {
            return static_cast<const std::decay_t<T>*>(GetData());
        }

        template <class T>
        auto Deref() const
        {
            assert(IsRef());
            return *static_cast<std::decay_t<T>* const*>(GetData());
        }

        template <class T>
        auto DerefConst() const
        {
            assert(IsRef());
            return *static_cast<const std::decay_t<T>* const*>(GetData());
        }

        template <class T>
        T As()
        {
            if constexpr (std::is_reference_v<T>)
            {
                if constexpr (std::is_const_v<T>)
                {
                    if (IsRef())
                    {
                        return *DerefConst<T>();
                    }
                    return *GetData<T>();
                }
                else
                {
                    assert(!IsConstRef());
                    if (IsRef())
                    {
                        return *Deref<T>();
                    }
                    return *GetData<std::remove_reference_t<T>>();
                }
            }
            else if constexpr (std::is_pointer_v<T>)
            {
                if constexpr (std::is_const_v<T>)
                {
                    if (IsRef())
                    {
                        return DerefConst<T>();
                    }
                    return GetData<T>();
                }
                else
                {
                    if (IsRef())
                    {
                        return *Deref<T>();
                    }
                    assert(OwnsData());
                    return *GetData<T>();
                }
            }
            else
            {
                if (IsRef())
                {
                    return *DerefConst<T>();
                }
                assert(OwnsData());
                return *GetData<T>();
            }
        }

        template <class T>
        T As() const
        {
            if constexpr (std::is_reference_v<T>)
            {
                if constexpr (std::is_const_v<T>)
                {
                    if (IsRef())
                    {
                        return *DerefConst<T>();
                    }
                    return *GetData<T>();
                }
                else
                {
                    assert(!IsConstRef());
                    if (IsRef())
                    {
                        return *Deref<T>();
                    }
                    return *const_cast<std::remove_reference_t<T>*>(GetData<std::remove_reference_t<T>>());
                }
            }
            else if constexpr (std::is_pointer_v<T>)
            {
                if constexpr (std::is_const_v<T>)
                {
                    if (IsRef())
                    {
                        return DerefConst<T>();
                    }
                    return GetData<T>();
                }
                else
                {
                    if (IsRef())
                    {
                        return *Deref<T>();
                    }
                    assert(OwnsData());
                    return *GetData<T>();
                }
            }
            else
            {
                if (IsRef())
                {
                    return *DerefConst<T>();
                }
                assert(OwnsData());
                return *GetData<T>();
            }
        }

    private:

        void DestructActive();

        FName TypeId;
        alignas(std::max_align_t) uint8_t Buffer[Capacity];
        EVariantFlags Flags = EVariantFlags::None;
        uint8 Size = 0;
    };
} // namespace Phoenix
