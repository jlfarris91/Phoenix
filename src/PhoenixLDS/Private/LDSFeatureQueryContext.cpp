
#include "LDSFeatureQueryContext.h"

using namespace Phoenix::LDS;

LDSFeatureQueryContext LDSFeatureQueryContext::Create(SessionConstRef session, WorldConstPtr world)
{
    LDSFeatureQueryContext context;

    if (TSharedPtr<FeatureLDS> featureLDS = session.GetFeature<FeatureLDS>())
    {
        context.SessionStaticCatalog = featureLDS->GetStaticSessionCatalog();

        if (world)
        {
            context.WorldStaticCatalog = featureLDS->GetStaticWorldCatalog(*world);
        }
    }

    if (const FeatureLDSSessionDynamicBlock* block = session.GetBlock<FeatureLDSSessionDynamicBlock>())
    {
        context.SessionDynamicCatalog = &block->Catalog;
    }

    if (world)
    {
        if (const FeatureLDSWorldDynamicBlock* block = world->GetBlock<FeatureLDSWorldDynamicBlock>())
        {
            context.WorldDynamicCatalog = &block->Catalog;
        }
    }

    return context;
}

LDSFeatureQueryContext LDSFeatureQueryContext::Create(WorldConstRef world)
{
    if (auto session = world.GetSession().lock())
    {
        return Create(*session, &world);
    }
    return {}; 
}

const LDSRecord* LDSFeatureQueryContext::QueryObjectRecord(const LDSRecordPath& path, ELDSRecordQueryFlags flags) const
{
    // 1. Check the dynamic world catalog
    if (WorldDynamicCatalog && HasNoneFlags(flags, ELDSRecordQueryFlags::SessionOnly, ELDSRecordQueryFlags::StaticOnly))
    {
        if (auto record = WorldDynamicCatalog->FindObjectRecord(path.ObjectId, path.Path))
        {
            return record;
        }
    }

    // 2. Check the static world catalog
    if (WorldStaticCatalog && HasNoneFlags(flags, ELDSRecordQueryFlags::SessionOnly))
    {
        if (auto record = WorldStaticCatalog->FindObjectRecord(path.ObjectId, path.Path))
        {
            return record;
        }
    }

    // 3. Check the dynamic session catalog
    if (SessionDynamicCatalog && HasNoneFlags(flags, ELDSRecordQueryFlags::StaticOnly))
    {
        if (auto record = SessionDynamicCatalog->FindObjectRecord(path.ObjectId, path.Path))
        {
            return record;
        }
    }

    // 4. Check the static session catalog
    if (SessionStaticCatalog)
    {
        if (auto record = SessionStaticCatalog->FindObjectRecord(path.ObjectId, path.Path))
        {
            return record;
        }
    }

    // Search for the same property in the base object
    if (HasNoneFlags(flags, ELDSRecordQueryFlags::Exact))
    {
        // Find the base object id
        if (const LDSRecord* baseRecord = QueryObjectRecord({ path.ObjectId, "/base"_n }, ELDSRecordQueryFlags::Exact))
        {
            FName baseId = baseRecord->GetValueAs<FName>();
            return QueryObjectRecord({ baseId, path.Path }, flags);
        }
    }

    return nullptr;
}

const LDSRecord* LDSFeatureQueryContext::QueryTypeRecord(const LDSRecordPath& path, ELDSRecordQueryFlags flags) const
{
    // 1. Check the dynamic world catalog
    if (WorldDynamicCatalog && HasNoneFlags(flags, ELDSRecordQueryFlags::SessionOnly, ELDSRecordQueryFlags::StaticOnly))
    {
        if (auto record = WorldDynamicCatalog->FindTypeRecord(path.ObjectId, path.Path))
        {
            return record;
        }
    }

    // 2. Check the static world catalog
    if (WorldStaticCatalog && HasNoneFlags(flags, ELDSRecordQueryFlags::SessionOnly))
    {
        if (auto record = WorldStaticCatalog->FindTypeRecord(path.ObjectId, path.Path))
        {
            return record;
        }
    }

    // 3. Check the dynamic session catalog
    if (SessionDynamicCatalog && HasNoneFlags(flags, ELDSRecordQueryFlags::StaticOnly))
    {
        if (auto record = SessionDynamicCatalog->FindTypeRecord(path.ObjectId, path.Path))
        {
            return record;
        }
    }

    // 4. Check the static session catalog
    if (SessionStaticCatalog)
    {
        if (auto record = SessionStaticCatalog->FindTypeRecord(path.ObjectId, path.Path))
        {
            return record;
        }
    }

    // Search for the same property in the base type
    if (HasNoneFlags(flags, ELDSRecordQueryFlags::Exact))
    {
        // Find the base type id
        if (const LDSRecord* baseRecord = QueryTypeRecord({ path.ObjectId, "/base"_n }, ELDSRecordQueryFlags::Exact))
        {
            FName baseId = baseRecord->GetValueAs<FName>();
            return QueryTypeRecord({ baseId, path.Path }, flags);
        }
    }

    return nullptr;
}

