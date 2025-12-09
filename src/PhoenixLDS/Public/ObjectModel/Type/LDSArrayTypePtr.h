
#pragma once

#include "ObjectModel/LDSArrayPtr.h"
#include "ObjectModel/LDSObjectPtr.h"
#include "ObjectModel/LDSValuePtr.h"

namespace Phoenix::LDS
{
    template <IsRecordPtr TItemPtr, IsArrayPtr TArrayPtr>
    struct TLDSArrayTypePtrBase : LDSObjectPtr
    {
        TLDSArrayTypePtrBase() = default;
        TLDSArrayTypePtrBase(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        TLDSArrayTypePtrBase(const LDSRecordPtr& other);

        TItemPtr Items;
        TArrayPtr DefaultValue;
        TLDSValuePtr<uint32> MinItems;
        TLDSValuePtr<uint32> MaxItems;

    private:
        void InitCommon();
    };

    // An anonymous array ptr useful when you don't know the item type.
    using LDSArrayTypePtr = TLDSArrayTypePtrBase<LDSRecordPtr, LDSArrayPtr>;

    template <class TValueItemPtr = LDSValuePtr, class TValueArrayPtr = LDSValueArrayPtr>
    using TLDSValueArrayTypePtr = TLDSArrayTypePtrBase<TValueItemPtr, TValueArrayPtr>;

    template <class TObjectItemPtr = LDSObjectPtr, class TObjectArrayPtr = LDSObjectArrayPtr>
    using TLDSObjectArrayTypePtr = TLDSArrayTypePtrBase<TObjectItemPtr, TObjectArrayPtr>;

    template <class TObjectRefItemPtr = LDSObjectRefPtr, class TObjectRefArrayPtr = LDSObjectRefArrayPtr>
    using TLDSObjectRefArrayTypePtr = TLDSArrayTypePtrBase<TObjectRefItemPtr, TObjectRefArrayPtr>;
}

#include "LDSArrayTypePtr.inl"
