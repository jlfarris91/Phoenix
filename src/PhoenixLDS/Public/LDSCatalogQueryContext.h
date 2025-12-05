
#pragma once

#include "ObjectModel/LDSQueryContext.h"

namespace Phoenix::LDS
{
    template <class TCatalog>
    struct PHOENIX_LDS_API LDSCatalogQueryContext : ILDSQueryContext
    {
        LDSCatalogQueryContext(const TCatalog* catalog = nullptr)
            : Catalog(catalog)
        {
        }

        const LDSRecord* QueryObjectRecord(
            const LDSRecordPath& path,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const override
        {
            return nullptr;
            // return Catalog ? Catalog->FindObjectRecord(path.ObjectId, path.Path, flags) : nullptr;
        }

        const LDSRecord* QueryTypeRecord(
            const LDSRecordPath& path,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const override
        {
            return nullptr;
            // return Catalog ? Catalog->FindTypeRecord(path.ObjectId, path.Path, flags) : nullptr;
        }

    private:
        const TCatalog* Catalog;
    };
}
