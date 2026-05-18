#pragma once

#include "Phoenix.Sim.LDS/ObjectModel/Type/LDSArrayTypePtr.h"

namespace Phoenix::LDS
{
    template <IsRecordPtr TItemPtr, IsArrayPtr TArrayPtr>
    TLDSArrayTypePtrBase<TItemPtr, TArrayPtr>::TLDSArrayTypePtrBase(
        const LDSRecordPath& path,
        ELDSRecordQueryFlags flags)
        : LDSObjectPtr(path, flags)
    {
        InitCommon();
    }

    template <IsRecordPtr TItemPtr, IsArrayPtr TArrayPtr>
    TLDSArrayTypePtrBase<TItemPtr, TArrayPtr>::TLDSArrayTypePtrBase(const LDSRecordPtr& other)
        : LDSObjectPtr(other)
    {
        InitCommon();
    }

    template <IsRecordPtr TItemPtr, IsArrayPtr TArrayPtr>
    void TLDSArrayTypePtrBase<TItemPtr, TArrayPtr>::InitCommon()
    {
        Items = Object("items");
        DefaultValue = Array("default");
        MinItems = Value<uint32>("min_items");
        MaxItems = Value<uint32>("max_items");
    }
}