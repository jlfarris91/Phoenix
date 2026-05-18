#include "Phoenix.Sim.LDS/LDSCatalogQueryContext.h"

Phoenix::LDS::LDSCatalogQueryContext::LDSCatalogQueryContext(
    const HeapLDSCatalog* catalog,
    ELDSCatalogRecordStore mode)
    : Catalog(catalog)
    , Mode(mode)
{
}

const Phoenix::LDS::LDSRecord* Phoenix::LDS::LDSCatalogQueryContext::QueryRecord(
    const LDSRecordPath& path,
    ELDSRecordQueryFlags flags) const
{
    switch (Mode)
    {
        case ELDSCatalogRecordStore::Object:    return QueryObjectRecord(path, flags);
        case ELDSCatalogRecordStore::Type:      return QueryTypeRecord(path, flags);
    }
    return nullptr;
}

const Phoenix::LDS::LDSRecord* Phoenix::LDS::LDSCatalogQueryContext::QueryObjectRecord(
    const LDSRecordPath& path,
    ELDSRecordQueryFlags flags) const
{
    return Catalog ? Catalog->FindObjectRecord(path.ObjectId, path.Path, flags) : nullptr;
}

const Phoenix::LDS::LDSRecord* Phoenix::LDS::LDSCatalogQueryContext::QueryTypeRecord(
    const LDSRecordPath& path,
    ELDSRecordQueryFlags flags) const
{
    return Catalog ? Catalog->FindTypeRecord(path.ObjectId, path.Path, flags) : nullptr;
}

bool Phoenix::LDS::LDSCatalogQueryContext::Exists(const FName& objectId) const
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
