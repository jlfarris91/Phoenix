
#include "FeatureLDS.h"

#include "Flags.h"

using namespace Phoenix;
using namespace Phoenix::LDS;

void FeatureLDS::Initialize()
{
    IFeature::Initialize();

    // TODO (jfarris): load session-level catalogs from disk
}

void FeatureLDS::Shutdown()
{
    IFeature::Shutdown();

    // All world catalogs should have been unloaded at this point.
    PHX_ASSERT(StaticWorldCatalogs.empty());

    // Unload the session catalog
    StaticSessionCatalog.reset();
}

void FeatureLDS::OnWorldInitialize(WorldRef world)
{
    IFeature::OnWorldInitialize(world);

    auto worldCatalog = MakeShared<Catalog>();
    StaticWorldCatalogs.emplace(world.GetName(), std::move(worldCatalog));

    // TODO (jfarris): load world-level catalogs from disk
}

void FeatureLDS::OnWorldShutdown(WorldRef world)
{
    IFeature::OnWorldShutdown(world);

    auto worldCatalog = StaticWorldCatalogs.find(world.GetName());
    if (worldCatalog != StaticWorldCatalogs.end())
    {
        StaticWorldCatalogs.erase(worldCatalog);
        worldCatalog->second.reset();
    }
}

TSharedPtr<const Catalog> FeatureLDS::GetStaticSessionCatalog() const
{
    return StaticSessionCatalog;
}

TSharedPtr<const Catalog> FeatureLDS::GetStaticWorldCatalog(WorldConstRef world) const
{
    auto worldCatalog = StaticWorldCatalogs.find(world.GetName());
    if (worldCatalog != StaticWorldCatalogs.end())
    {
        return worldCatalog->second;
    }
    return nullptr;
}

const LDSRecord* FeatureLDS::QueryObjectRecord(
    SessionConstPtr session,
    WorldConstPtr world,
    const FName& objectId,
    const FName& propertyId,
    ELDSRecordFlags flags)
{
    // 1. Check the dynamic world catalog
    if (world && HasNoneFlags(flags, ELDSRecordFlags::SessionOnly, ELDSRecordFlags::StaticOnly))
    {
        if (const FeatureLDSDynamicBlock* block = world->GetBlock<FeatureLDSDynamicBlock>())
        {
            if (auto record = block->Catalog.FindObjectRecord(objectId, propertyId))
            {
                return record;
            }
        }
    }

    TSharedPtr<FeatureLDS> featureLDS;
    if (session)
    {
        featureLDS = session->GetFeature<FeatureLDS>();
    }

    // 2. Check the static world catalog
    if (featureLDS && world && HasNoneFlags(flags, ELDSRecordFlags::SessionOnly))
    {
        if (TSharedPtr<const Catalog> worldStaticCatalog = featureLDS->GetStaticWorldCatalog(*world))
        {
            if (auto record = worldStaticCatalog->FindObjectRecord(objectId, propertyId))
            {
                return record;
            }
        }
    }

    // 3. Check the dynamic session catalog
    if (session && HasNoneFlags(flags, ELDSRecordFlags::StaticOnly))
    {
        if (const FeatureLDSStaticBlock* block = session->GetBlock<FeatureLDSStaticBlock>())
        {
            if (auto record = block->Catalog.FindObjectRecord(objectId, propertyId))
            {
                return record;
            }
        }
    }

    // 4. Check the static session catalog
    if (featureLDS)
    {
        if (TSharedPtr<const Catalog> sessionStaticCatalog = featureLDS->GetStaticSessionCatalog())
        {
            if (auto record = sessionStaticCatalog->FindObjectRecord(objectId, propertyId))
            {
                return record;
            }
        }
    }

    // Search for the same property in the base object
    if (HasNoneFlags(flags, ELDSRecordFlags::Exact))
    {
        // Find the base object id
        if (const LDSRecord* baseRecord = QueryObjectRecord(session, world, objectId, "/base"_n))
        {
            FName baseId = baseRecord->GetValueAs<FName>();
            return QueryObjectRecord(session, world, baseId, propertyId, flags);
        }
    }

    return nullptr;
}

const LDSRecord* FeatureLDS::QueryTypeRecord(
    SessionConstPtr session,
    WorldConstPtr world,
    const FName& typeId,
    const FName& propertyId,
    ELDSRecordFlags flags)
{
    // 1. Check the dynamic world catalog
    if (world && HasNoneFlags(flags, ELDSRecordFlags::SessionOnly, ELDSRecordFlags::StaticOnly))
    {
        if (const FeatureLDSDynamicBlock* block = world->GetBlock<FeatureLDSDynamicBlock>())
        {
            if (auto record = block->Catalog.FindTypeRecord(typeId, propertyId))
            {
                return record;
            }
        }
    }

    TSharedPtr<FeatureLDS> featureLDS;
    if (session)
    {
        featureLDS = session->GetFeature<FeatureLDS>();
    }

    // 2. Check the static world catalog
    if (featureLDS && world && HasNoneFlags(flags, ELDSRecordFlags::SessionOnly))
    {
        if (TSharedPtr<const Catalog> worldStaticCatalog = featureLDS->GetStaticWorldCatalog(*world))
        {
            if (auto record = worldStaticCatalog->FindTypeRecord(typeId, propertyId))
            {
                return record;
            }
        }
    }

    // 3. Check the dynamic session catalog
    if (session && HasNoneFlags(flags, ELDSRecordFlags::StaticOnly))
    {
        if (const FeatureLDSStaticBlock* block = session->GetBlock<FeatureLDSStaticBlock>())
        {
            if (auto record = block->Catalog.FindTypeRecord(typeId, propertyId))
            {
                return record;
            }
        }
    }

    // 4. Check the static session catalog
    if (featureLDS)
    {
        if (TSharedPtr<const Catalog> sessionStaticCatalog = featureLDS->GetStaticSessionCatalog())
        {
            if (auto record = sessionStaticCatalog->FindTypeRecord(typeId, propertyId))
            {
                return record;
            }
        }
    }

    // Search for the same property in the base type
    if (HasNoneFlags(flags, ELDSRecordFlags::Exact))
    {
        // Find the base type id
        if (const LDSRecord* baseRecord = QueryTypeRecord(session, world, typeId, "/base"_n))
        {
            FName baseId = baseRecord->GetValueAs<FName>();
            return QueryTypeRecord(session, world, baseId, propertyId, flags);
        }
    }

    return nullptr;
}
