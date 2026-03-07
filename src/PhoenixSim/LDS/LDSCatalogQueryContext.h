
#pragma once

#include "PhoenixSim/LDS/LDSCatalog.h"
#include "PhoenixSim/LDS/LDSQueryContext.h"

namespace Phoenix::LDS
{
    struct PHOENIX_SIM_API LDSCatalogQueryContext : ILDSQueryContext
    {
        LDSCatalogQueryContext(
            const HeapLDSCatalog* catalog = nullptr,
            ELDSCatalogRecordStore mode = ELDSCatalogRecordStore::Object);

        const LDSRecord* QueryRecord(
            const LDSRecordPath& path,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const override;

        const LDSRecord* QueryObjectRecord(
            const LDSRecordPath& path,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const;

        const LDSRecord* QueryTypeRecord(
            const LDSRecordPath& path,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const;

        bool Exists(const FName& objectId) const override;

    private:
        const HeapLDSCatalog* Catalog;
        ELDSCatalogRecordStore Mode = ELDSCatalogRecordStore::Object;
    };
}
