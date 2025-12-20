
#pragma once

#include "PhoenixSim/LDS/FeatureLDS.h"
#include "PhoenixSim/LDS/LDSCatalog.h"
#include "PhoenixSim/LDS/LDSQueryContext.h"

namespace Phoenix::LDS
{
    struct PHOENIX_SIM_API LDSFeatureQueryContext : ILDSQueryContext
    {
        static LDSFeatureQueryContext Create(SessionConstRef session, WorldConstPtr world);
        static LDSFeatureQueryContext Create(WorldConstRef world);

        LDSFeatureQueryContext WithMode(ELDSCatalogRecordStore mode) const;
        LDSFeatureQueryContext WithFlags(ELDSFeatureRecordQueryFlags flags) const;

        const LDSRecord* QueryRecord(
            const LDSRecordPath& path,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const override;

        const LDSRecord* QueryObjectRecord(
            const LDSRecordPath& path,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const;

        const LDSRecord* QueryTypeRecord(
            const LDSRecordPath& path,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const;

    private:
        
        TSharedPtr<const Catalog> SessionStaticCatalog = nullptr;
        TSharedPtr<const Catalog> WorldStaticCatalog = nullptr;
        const decltype(FeatureLDSSessionDynamicBlock::Catalog)* SessionDynamicCatalog = nullptr;
        const decltype(FeatureLDSWorldDynamicBlock::Catalog)* WorldDynamicCatalog = nullptr;

        ELDSCatalogRecordStore Mode = ELDSCatalogRecordStore::Object;
        ELDSFeatureRecordQueryFlags FeatureQueryFlags = ELDSFeatureRecordQueryFlags::None;
    };
}
