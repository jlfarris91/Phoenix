
#pragma once

#include "LDSQueryContext.h"

#include "PhoenixSim/LDS/ObjectModel/LDSForwardDecls.h"

#include "PhoenixSim/LDS/ObjectModel/LDSArrayUtil.h"

#include "PhoenixSim/LDS/ObjectModel/LDSArrayPtr.h"
#include "PhoenixSim/LDS/ObjectModel/LDSEnumFlagsPtr.h"
#include "PhoenixSim/LDS/ObjectModel/LDSObjectArrayPtr.h"
#include "PhoenixSim/LDS/ObjectModel/LDSObjectPtr.h"
#include "PhoenixSim/LDS/ObjectModel/LDSObjectRefArrayPtr.h"
#include "PhoenixSim/LDS/ObjectModel/LDSObjectRefPtr.h"
#include "PhoenixSim/LDS/ObjectModel/LDSRecordPtr.h"
#include "PhoenixSim/LDS/ObjectModel/LDSValueArrayPtr.h"
#include "PhoenixSim/LDS/ObjectModel/LDSValuePtr.h"

#include "PhoenixSim/LDS/ObjectModel/Type/LDSArrayTypePtr.h"
#include "PhoenixSim/LDS/ObjectModel/Type/LDSEnumTypePtr.h"
#include "PhoenixSim/LDS/ObjectModel/Type/LDSNumericTypePtr.h"
#include "PhoenixSim/LDS/ObjectModel/Type/LDSObjectTypePtr.h"
#include "PhoenixSim/LDS/ObjectModel/Type/LDSObjectRefTypePtr.h"

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