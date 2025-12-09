
#pragma once

#include "ObjectModel/LDSForwardDecls.h"

#include "ObjectModel/LDSQueryContext.h"
#include "ObjectModel/LDSArrayUtil.h"

#include "ObjectModel/LDSArrayPtr.h"
#include "ObjectModel/LDSEnumFlagsPtr.h"
#include "ObjectModel/LDSObjectArrayPtr.h"
#include "ObjectModel/LDSObjectPtr.h"
#include "ObjectModel/LDSObjectRefArrayPtr.h"
#include "ObjectModel/LDSObjectRefPtr.h"
#include "ObjectModel/LDSRecordPtr.h"
#include "ObjectModel/LDSValueArrayPtr.h"
#include "ObjectModel/LDSValuePtr.h"

#include "ObjectModel/Type/LDSArrayTypePtr.h"
#include "ObjectModel/Type/LDSEnumTypePtr.h"
#include "ObjectModel/Type/LDSNumericTypePtr.h"
#include "ObjectModel/Type/LDSObjectTypePtr.h"
#include "ObjectModel/Type/LDSObjectRefTypePtr.h"

// #include "ObjectModel/LDSQueryContext.inl"
// #include "ObjectModel/LDSArrayUtil.inl"
//
// #include "ObjectModel/LDSArrayPtr.inl"
// #include "ObjectModel/LDSEnumFlagsPtr.inl"
// #include "ObjectModel/LDSObjectArrayPtr.inl"
// #include "ObjectModel/LDSObjectPtr.inl"
// #include "ObjectModel/LDSObjectRefArrayPtr.inl"
// #include "ObjectModel/LDSObjectRefPtr.inl"
// #include "ObjectModel/LDSRecordPtr.inl"
// #include "ObjectModel/LDSValueArrayPtr.inl"
// #include "ObjectModel/LDSValuePtr.inl"
//
// #include "ObjectModel/Type/LDSArrayTypePtr.inl"
// #include "ObjectModel/Type/LDSEnumTypePtr.inl"
// #include "ObjectModel/Type/LDSNumericTypePtr.inl"
// #include "ObjectModel/Type/LDSObjectTypePtr.inl"
// #include "ObjectModel/Type/LDSObjectRefTypePtr.inl"

#define PHX_LDS_DECLARE_OBJECT_PTR(ptr, type) \
    ptr() = default; \
    ptr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags = LDS::ELDSRecordQueryFlags::None); \
    ptr(const LDS::LDSRecordPtr& other) : ptr(other.GetPath(), other.GetFlags()) {}

#define PHX_LDS_DECLARE_OBJECT_PTR_FOR(type) PHX_LDS_DECLARE_OBJECT_PTR(type##Ptr, type)