
#pragma once

#include "LDSQueryContext.h"

#include "Phoenix.Sim/LDS/ObjectModel/LDSForwardDecls.h"

#include "Phoenix.Sim/LDS/ObjectModel/LDSArrayUtil.h"

#include "Phoenix.Sim/LDS/ObjectModel/LDSArrayPtr.h"
#include "Phoenix.Sim/LDS/ObjectModel/LDSEnumFlagsPtr.h"
#include "Phoenix.Sim/LDS/ObjectModel/LDSObjectArrayPtr.h"
#include "Phoenix.Sim/LDS/ObjectModel/LDSObjectPtr.h"
#include "Phoenix.Sim/LDS/ObjectModel/LDSObjectRefArrayPtr.h"
#include "Phoenix.Sim/LDS/ObjectModel/LDSObjectRefPtr.h"
#include "Phoenix.Sim/LDS/ObjectModel/LDSRecordPtr.h"
#include "Phoenix.Sim/LDS/ObjectModel/LDSValueArrayPtr.h"
#include "Phoenix.Sim/LDS/ObjectModel/LDSValuePtr.h"

#include "Phoenix.Sim/LDS/ObjectModel/Type/LDSArrayTypePtr.h"
#include "Phoenix.Sim/LDS/ObjectModel/Type/LDSEnumTypePtr.h"
#include "Phoenix.Sim/LDS/ObjectModel/Type/LDSNumericTypePtr.h"
#include "Phoenix.Sim/LDS/ObjectModel/Type/LDSObjectTypePtr.h"
#include "Phoenix.Sim/LDS/ObjectModel/Type/LDSObjectRefTypePtr.h"

#define PHX_LDS_DECLARE_OBJECT_PTR(ptr, type) \
    ptr() = default; \
    ptr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags = LDS::ELDSRecordQueryFlags::None); \
    ptr(const LDS::LDSRecordPtr& other) : ptr(other.GetPath(), other.GetFlags()) {}

#define PHX_LDS_DECLARE_OBJECT_PTR_FOR(type) PHX_LDS_DECLARE_OBJECT_PTR(type##Ptr, type)

#define PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(type) \
    using type##RefPtr = LDS::TLDSObjectRefPtr<type##Ptr>; \
    using type##RefArrayPtr = LDS::TLDSObjectRefArrayPtr<type##RefPtr>;

#define PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(type) \
    using type##ArrayPtr = LDS::TLDSObjectArrayPtr<type##Ptr>;