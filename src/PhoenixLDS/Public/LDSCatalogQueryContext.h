
#pragma once

#include "LDSCatalog.h"
#include "ObjectModel/LDSQueryContext.h"

namespace Phoenix::LDS
{
    template <class TCatalog = Catalog>
    struct PHOENIX_LDS_API LDSCatalogQueryContext : ILDSQueryContext
    {
        LDSCatalogQueryContext(
            const TCatalog* catalog = nullptr,
            ELDSCatalogRecordStore mode = ELDSCatalogRecordStore::Object)
            : Catalog(catalog)
            , Mode(mode)
        {
        }

        const LDSRecord* QueryRecord(
            const LDSRecordPath& path,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const override
        {
            switch (Mode)
            {
                case ELDSCatalogRecordStore::Object:    return QueryObjectRecord(path, flags);
                case ELDSCatalogRecordStore::Type:      return QueryTypeRecord(path, flags);
            }
            return nullptr;
        }

        const LDSRecord* QueryObjectRecord(
            const LDSRecordPath& path,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const
        {
            return Catalog ? Catalog->FindObjectRecord(path.ObjectId, path.Path, flags) : nullptr;
        }

        const LDSRecord* QueryTypeRecord(
            const LDSRecordPath& path,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const
        {
            return Catalog ? Catalog->FindTypeRecord(path.ObjectId, path.Path, flags) : nullptr;
        }

    private:
        const TCatalog* Catalog;
        ELDSCatalogRecordStore Mode = ELDSCatalogRecordStore::Object;
    };
}
