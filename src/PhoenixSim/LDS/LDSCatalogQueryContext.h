
#pragma once

#include "PhoenixSim/LDS/LDSCatalog.h"
#include "PhoenixSim/LDS/LDSQueryContext.h"

namespace Phoenix::LDS
{
    template <class TCatalog = Catalog>
    struct LDSCatalogQueryContext : ILDSQueryContext
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

        bool Exists(const FName& objectId) const override
        {
            if (!Catalog)
            {
                return false;
            }
            switch (Mode)
            {
                case ELDSCatalogRecordStore::Object:    return Catalog->HasObject(objectId);
                case ELDSCatalogRecordStore::Type:      return Catalog->HasType(objectId);
            }
            return false;
        }

    private:
        const TCatalog* Catalog;
        ELDSCatalogRecordStore Mode = ELDSCatalogRecordStore::Object;
    };
}
