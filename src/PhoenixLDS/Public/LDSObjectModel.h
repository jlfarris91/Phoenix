
#pragma once

#include "ObjectModel/LDSQueryContext.h"

#include "ObjectModel/LDSArrayPtr.h"
#include "ObjectModel/LDSEnumFlagsPtr.h"
#include "ObjectModel/LDSEnumTypePtr.h"
#include "ObjectModel/LDSObjectPtr.h"
#include "ObjectModel/LDSObjectRefPtr.h"
#include "ObjectModel/LDSRecordPtr.h"
#include "ObjectModel/LDSValuePtr.h"

#include "ObjectModel/LDSQueryContext.inl"

#include "ObjectModel/LDSArrayPtr.inl"
#include "ObjectModel/LDSEnumFlagsPtr.inl"
#include "ObjectModel/LDSObjectPtr.inl"
#include "ObjectModel/LDSObjectRefPtr.inl"
#include "ObjectModel/LDSRecordPtr.inl"
#include "ObjectModel/LDSValuePtr.inl"

#define PHX_LDS_DECLARE_OBJECT_PTR(ptr, type) \
    ptr() = default; \
    ptr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags = LDS::ELDSRecordQueryFlags::None); \
    ptr(const LDS::LDSRecordPtr& other) : ptr(other.GetPath(), other.GetFlags()) {}

#define PHX_LDS_DECLARE_OBJECT_PTR_FOR(type) PHX_LDS_DECLARE_OBJECT_PTR(type##Ptr, type)