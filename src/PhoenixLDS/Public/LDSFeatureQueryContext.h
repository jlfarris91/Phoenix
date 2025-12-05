
#pragma once

#include "FeatureLDS.h"
#include "LDSCatalog.h"
#include "ObjectModel/LDSQueryContext.h"

namespace Phoenix::LDS
{
    struct PHOENIX_LDS_API LDSFeatureQueryContext : ILDSQueryContext
    {
        TSharedPtr<const Catalog> SessionStaticCatalog = nullptr;
        TSharedPtr<const Catalog> WorldStaticCatalog = nullptr;
        const decltype(FeatureLDSSessionDynamicBlock::Catalog)* SessionDynamicCatalog = nullptr;
        const decltype(FeatureLDSWorldDynamicBlock::Catalog)* WorldDynamicCatalog = nullptr;

        static LDSFeatureQueryContext Create(SessionConstRef session, WorldConstPtr world);
        static LDSFeatureQueryContext Create(WorldConstRef world);

        const LDSRecord* QueryObjectRecord(
            const LDSRecordPath& path,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const override;

        const LDSRecord* QueryTypeRecord(
            const LDSRecordPath& path,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const override;
    };
}
