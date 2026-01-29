
#pragma once

#include <algorithm>

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Name.h"
#include "PhoenixSim/Profiling.h"
#include "PhoenixSim/Containers/FixedSortedList.h"

namespace Phoenix::Blackboard
{
    using blackboard_key_t = uint64;
    using blackboard_value_t = int64;
    using blackboard_type_t = uint8;

    static constexpr uint32 IgnoreKey = -1;
    static constexpr blackboard_type_t IgnoreType = -1;
    static constexpr blackboard_type_t UnknownType = 0;

    namespace BlackboardKey
    {
        // [FFFF'FF][FF][FFFF'FFFF]
        // [hi     ][ty][lo       ]
        static constexpr uint8 KeyHiShift       = 40;
        static constexpr uint8 KeyTypeShift     = 32;
        static constexpr uint64 KeyLoMask       = 0xFFFF'FFFFULL;
        static constexpr uint64 KeyHiMask       = 0xFF'FFFFULL << KeyHiShift;
        static constexpr uint64 KeyNoTypeMask   = KeyLoMask | KeyHiMask;
        static constexpr uint64 KeyTypeMask     = 0xFFULL << KeyTypeShift;

        PHX_FORCEINLINE constexpr uint32 GetKeyLo(blackboard_key_t key)
        {
            return static_cast<uint32>(key & KeyLoMask);
        }

        PHX_FORCEINLINE constexpr uint32 GetKeyHi(blackboard_key_t key)
        {
            return static_cast<uint32>((key & KeyHiMask) >> KeyHiShift);
        }

        PHX_FORCEINLINE constexpr blackboard_key_t GetKeyNoType(blackboard_key_t key)
        {
            return key & KeyNoTypeMask;
        }

        PHX_FORCEINLINE constexpr blackboard_type_t GetKeyType(blackboard_key_t key)
        {
            return static_cast<blackboard_type_t>((key & KeyTypeMask) >> KeyTypeShift);
        }

        PHX_FORCEINLINE constexpr bool IsType(blackboard_key_t key, blackboard_type_t type)
        {
            return GetKeyType(key) == type;
        }

        PHX_FORCEINLINE constexpr blackboard_key_t Create(uint32 lo, uint32 hi, blackboard_type_t type)
        {
            blackboard_key_t key = lo & KeyLoMask;
            key |= (static_cast<uint64>(hi) << KeyHiShift) & KeyHiMask;
            key |= (static_cast<uint64>(type) << KeyTypeShift) & KeyTypeMask;
            return key;
        }

        PHX_FORCEINLINE constexpr blackboard_key_t Create(blackboard_key_t keyNoType, blackboard_type_t type)
        {
            blackboard_key_t keyWithType = keyNoType & KeyNoTypeMask;
            return keyWithType | ((static_cast<uint64>(type) << KeyTypeShift) & KeyTypeMask);
        }

        template <size_t N>
        PHX_FORCEINLINE constexpr blackboard_key_t AppendKeyLo(blackboard_key_t key, const char (&chars)[N])
        {
            hash32_t newLo = Hashing::FNV1A32Append(GetKeyLo(key), chars);
            return Create(newLo, GetKeyHi(key), GetKeyType(key));
        }

        PHX_FORCEINLINE constexpr blackboard_key_t ClearKeyLo(blackboard_key_t key)
        {
            return Create(0, GetKeyHi(key), GetKeyType(key));
        }
    };

    static_assert(BlackboardKey::GetKeyLo(BlackboardKey::Create(123, 456, 16)) == 123);
    static_assert(BlackboardKey::GetKeyHi(BlackboardKey::Create(123, 456, 16)) == 456);
    static_assert(BlackboardKey::GetKeyType(BlackboardKey::Create(123, 456, 16)) == 16);
    static_assert(BlackboardKey::Create(123, 456, 16) == 0x1C8100000007B);

    enum class PHOENIX_SIM_API EBlackboardValueTypes : blackboard_type_t
    {
        Unknown = UnknownType,
        Invalid,

        // Standard PHX types
        Bool,
        UInt32,
        Int32,
        Name,
        Color,

        // Fixed Point types
        FIXED_POINT = 20,
        Value,
        InvValue,
        Distance,
        Time,
        Angle,
        Speed,
        Vec2,

        USER_DEFINED = 50,
    };

    // Fallback for unknown types
    template <class>
    struct BlackboardValueType
    {
        static constexpr blackboard_type_t Type = static_cast<blackboard_type_t>(EBlackboardValueTypes::Unknown);
    };

#define PHX_DECLARE_BLACKBOARD_TYPE(T, ValueType) \
    template <> \
    struct BlackboardValueType<T> \
    { \
        static constexpr blackboard_type_t Type = static_cast<blackboard_type_t>(ValueType); \
    }

    template <class T>
    struct BlackboardValueConverter
    {
        static T ConvertFrom(blackboard_value_t value)
        {
            return static_cast<T>(value);
        }
        static blackboard_value_t ConvertTo(const T& value)
        {
            return static_cast<blackboard_value_t>(value);
        }
    };

    struct PHOENIX_SIM_API BlackboardItem
    {
        BlackboardItem() = default;
        BlackboardItem(blackboard_key_t key, blackboard_value_t value);

        bool operator==(const BlackboardItem& other) const;

        bool IsValid() const;

        void Invalidate();

        struct GetItemKey
        {
            blackboard_key_t operator()(const BlackboardItem& item) const;
        };

        blackboard_key_t Key;
        blackboard_value_t Value;
    };

    struct PHOENIX_SIM_API BlackboardQuery
    {
        BlackboardQuery() = default;
        BlackboardQuery(blackboard_key_t key);
        BlackboardQuery(blackboard_key_t key, blackboard_type_t type);
        BlackboardQuery(uint32 keyLo, uint32 keyHi, blackboard_type_t type);

        template <size_t N>
        PHX_FORCEINLINE BlackboardQuery AppendKeyLo(const char (&chars)[N]) const
        {
            return BlackboardKey::AppendKeyLo(Filter, chars);
        }

        BlackboardQuery WithType(blackboard_type_t type) const;

        bool operator()(const BlackboardItem& item) const noexcept;

        bool operator==(const BlackboardQuery& other) const = default;
        bool operator!=(const BlackboardQuery& other) const = default;

        blackboard_key_t Filter;
    };

    template <class, class>
    struct BlackboardComplexValueAccessor{};

    template <class TBlackboard, class TValue>
    concept BlackboardComplexValueAccessor_HasValue = requires(const TBlackboard& set, const BlackboardQuery& query)
    {
        { BlackboardComplexValueAccessor<TBlackboard, TValue>::HasValue(set, query) } -> std::same_as<bool>;
    };

    template <class TBlackboard, class TValue>
    concept BlackboardComplexValueAccessor_SetValue = requires(TBlackboard& set, blackboard_key_t key, const TValue& value)
    {
        { BlackboardComplexValueAccessor<TBlackboard, TValue>::SetValue(set, key, value) } -> std::same_as<bool>;
    };

    template <class TBlackboard, class TValue>
    concept BlackboardComplexValueAccessor_GetValue = requires(const TBlackboard& set, const BlackboardQuery& query, TValue& outValue)
    {
        { BlackboardComplexValueAccessor<TBlackboard, TValue>::GetValue(set, query, outValue) } -> std::same_as<bool>;
    };

    template <class TBlackboard, class TValue>
    concept BlackboardComplexValueAccessor_RemoveValue = requires(TBlackboard& set, const BlackboardQuery& query)
    {
        { BlackboardComplexValueAccessor<TBlackboard, TValue>::RemoveValue(set, query) } -> std::same_as<bool>;
    };

    class PHOENIX_SIM_API FixedBlackboard
    {
    public:

        friend struct BlackboardValues;

        using TItem = BlackboardItem;
        using TStorage = TFixedSortedList<BlackboardItem, BlackboardItem::GetItemKey>;

        FixedBlackboard() = default;

        template <class TAllocator>
        FixedBlackboard(TAllocator& allocator, uint32 capacity)
            : Storage(allocator, capacity)
        {
        }

        template <class TAllocator>
        FixedBlackboard(TAllocator& allocator, uint32 capacity, const FixedBlackboard& other)
            : Storage(allocator, capacity, other.Storage)
        {
        }

        uint32 GetCapacity() const;
        TItem* GetData();
        const TItem* GetData() const;
        static uint32 GetAllocSizeBytes(uint32 capacity);
        uint32 GetAllocSizeBytes() const;
        void Sort();

        uint32 GetNum() const;
        uint32 GetNumValidItems() const;

        // Returns true if the blackboard has a value for the given key query.
        bool HasValue(const BlackboardQuery& query) const;

        // Returns true if the blackboard has a value for the given key query.
        template <class T>
        bool HasValue(const BlackboardQuery& query) const requires(BlackboardComplexValueAccessor_HasValue<FixedBlackboard, T>)
        {
            return BlackboardComplexValueAccessor<FixedBlackboard, T>::HasValue(*this, query);
        }

        // Returns true if the blackboard has a value for the given key that is the expected type.
        template <class T>
        bool HasValue(blackboard_key_t key, blackboard_type_t expectedType = BlackboardValueType<T>::Type) const
        {
            return HasValue<T>(BlackboardQuery(key, expectedType));
        }

        bool SetValue(blackboard_key_t key, blackboard_value_t value);

        template <class T>
        bool SetValue(blackboard_key_t key, const T& value, blackboard_type_t type)
        {
            PHX_PROFILE_ZONE_SCOPED;

            blackboard_key_t keyWithType = BlackboardKey::Create(key, type);
            if constexpr (BlackboardComplexValueAccessor_SetValue<FixedBlackboard, T>)
            {
                return BlackboardComplexValueAccessor<FixedBlackboard, T>::SetValue(*this, keyWithType, value);
            }
            else
            {
                return SetValue(keyWithType, BlackboardValueConverter<T>::ConvertTo(value));
            }
        }

        template <class T>
        bool SetValue(blackboard_key_t key, const T& value)
        {
            PHX_PROFILE_ZONE_SCOPED;

            blackboard_key_t keyWithType = BlackboardKey::Create(key, BlackboardValueType<T>::Type);
            if constexpr (BlackboardComplexValueAccessor_SetValue<FixedBlackboard, T>)
            {
                return BlackboardComplexValueAccessor<FixedBlackboard, T>::SetValue(*this, keyWithType, value);
            }
            else
            {
                return SetValue(keyWithType, BlackboardValueConverter<T>::ConvertTo(value));
            }
        }

        bool GetValue(const BlackboardQuery& query, blackboard_value_t& outValue) const;

        bool GetValue(blackboard_key_t key, blackboard_value_t& outValue) const;

        template <class T>
        bool GetValue(const BlackboardQuery& query, T& outValue) const
        {
            PHX_PROFILE_ZONE_SCOPED;

            if constexpr (BlackboardComplexValueAccessor_GetValue<FixedBlackboard, T>)
            {
                return BlackboardComplexValueAccessor<FixedBlackboard, T>::GetValue(*this, query, outValue);
            }
            else
            {
                blackboard_value_t value;
                if (!GetValue(query, value))
                {
                    return false;
                }
                outValue = BlackboardValueConverter<T>::ConvertFrom(value);
                return true;
            }
        }

        template <class T>
        bool GetValue(blackboard_key_t key, T& outValue) const
        {
            BlackboardQuery query(key, BlackboardValueType<T>::Type);
            return GetValue<T>(query, outValue);
        }

        template <class T = blackboard_value_t>
        bool RemoveValue(const BlackboardQuery& query)
        {
            PHX_PROFILE_ZONE_SCOPED;

            if constexpr (BlackboardComplexValueAccessor_RemoveValue<FixedBlackboard, T>)
            {
                return BlackboardComplexValueAccessor<FixedBlackboard, T>::RemoveValue(*this, query);
            }
            else
            {
                return RemoveValuesMatchingQueryInternal(query);
            }
        }

        template <class T = blackboard_value_t>
        bool RemoveValue(blackboard_key_t key, blackboard_type_t expectedType = BlackboardValueType<T>::Type)
        {
            return RemoveValue<T>(BlackboardQuery(key, expectedType));
        }

        uint32 RemoveAll(const BlackboardQuery& query);

        BlackboardValues Enumerate(uint32 keyHi) const;

    private:

        BlackboardItem* FindItemWithQuery(const BlackboardQuery& query);
        const BlackboardItem* FindItemWithQuery(const BlackboardQuery& query) const;

        BlackboardItem* FindFirstItemWithQuery(const BlackboardQuery& query, uint32& outIndex);
        const BlackboardItem* FindFirstItemWithQuery(const BlackboardQuery& query, uint32& outIndex) const;

        BlackboardItem* FindNextItemWithQuery(const BlackboardQuery& query, uint32 currIndex, uint32& outIndex);
        const BlackboardItem* FindNextItemWithQuery(const BlackboardQuery& query, uint32 currIndex, uint32& outIndex) const;

        uint32 RemoveValuesMatchingQueryInternal(const BlackboardQuery& query);

        bool InsertKVP(blackboard_key_t key, blackboard_value_t value);

        TStorage Storage;
    };

    struct BlackboardValues
    {
        BlackboardValues() = default;
        BlackboardValues(const FixedBlackboard* set, const BlackboardQuery& query);

        struct KeyHiIter
        {
            KeyHiIter(const FixedBlackboard* owner, const BlackboardQuery& query);
            KeyHiIter(const FixedBlackboard* owner, const BlackboardQuery& query, const BlackboardItem* item, uint32 index);

            const BlackboardItem& operator*() const;

            KeyHiIter& operator++();

            bool operator==(const KeyHiIter& other) const = default;

            const FixedBlackboard* Owner;
            BlackboardQuery Query;
            const BlackboardItem* Item = nullptr;
            uint32 Index = Phoenix::Index<uint32>::None;
        };

        KeyHiIter begin() const;
        KeyHiIter end() const;

    private:

        const FixedBlackboard* Owner;
        BlackboardQuery Query;
    };
}

#include "FixedBlackboard.inl"