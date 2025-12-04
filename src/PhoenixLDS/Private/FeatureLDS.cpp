
#include "FeatureLDS.h"

#include "Flags.h"
#include "Json/JsonCatalogObjectBuilder.h"
#include "Json/JsonCatalogTypeBuilder.h"
#include "Json/JsonDataSource.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::LDS::Json;
namespace fs = std::filesystem;

void FeatureLDS::Initialize()
{
    IFeature::Initialize();

    // TODO (jfarris): load session-level catalogs from disk
    StaticSessionCatalog = MakeShared<Catalog>();

    auto catalogPathIter = Config.find("catalog");
    if (catalogPathIter != Config.end())
    {
        fs::path dataDir = Session->GetDataDirectory();
        fs::path catalogPath = absolute(dataDir / catalogPathIter->get<PHXString>());
        LoadCatalog(catalogPath.generic_string(), *StaticSessionCatalog);
    }
}

void FeatureLDS::Shutdown()
{
    IFeature::Shutdown();

    // All world catalogs should have been unloaded at this point.
    PHX_ASSERT(StaticWorldCatalogs.empty());

    // Unload the session catalog
    StaticSessionCatalog.reset();
}

bool FeatureLDS::LoadCatalog(const PHXString& catalogAbsolutePath, Catalog& catalog)
{
    TSharedPtr<JsonDataSource> dataSource = JsonDataSource::LoadFromCatalog(catalogAbsolutePath);
    if (!dataSource)
    {
        // Error: could not load catalog
        return false;
    }

    JsonCatalogTypeBuilder typeBuilder(dataSource.get(), &catalog);
    if (!typeBuilder.RegisterAllTypes())
    {
        // Error: could not load types
        return false;
    }

    JsonCatalogObjectBuilder objectBuilder(dataSource.get(), &catalog);
    if (!objectBuilder.RegisterAllObjects())
    {
        // Error: could not load objects
        return false;
    }

    return true;
}

void FeatureLDS::OnWorldInitialize(WorldRef world)
{
    IFeature::OnWorldInitialize(world);

    auto worldCatalog = MakeShared<Catalog>();
    StaticWorldCatalogs.emplace(world.GetId(), worldCatalog);

    if (const nlohmann::json* config = world.GetFeatureConfig("FeatureLDS"))
    {
        auto catalogPathIter = config->find("catalog");
        if (catalogPathIter != config->end())
        {
            fs::path worldsDir = Session->GetWorldsDirectory();
            fs::path catalogPath = absolute(worldsDir / catalogPathIter->get<PHXString>());
            LoadCatalog(catalogPathIter->get<PHXString>(), *worldCatalog);
        }
    }
}

void FeatureLDS::OnWorldShutdown(WorldRef world)
{
    IFeature::OnWorldShutdown(world);

    auto worldCatalog = StaticWorldCatalogs.find(world.GetId());
    if (worldCatalog != StaticWorldCatalogs.end())
    {
        StaticWorldCatalogs.erase(worldCatalog);
        worldCatalog->second.reset();
    }
}

TSharedPtr<Catalog> FeatureLDS::GetStaticSessionCatalog()
{
    return StaticSessionCatalog;
}

TSharedPtr<const Catalog> FeatureLDS::GetStaticSessionCatalog() const
{
    return StaticSessionCatalog;
}

TSharedPtr<const Catalog> FeatureLDS::GetStaticWorldCatalog(WorldConstRef world) const
{
    auto worldCatalog = StaticWorldCatalogs.find(world.GetId());
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
    ELDSFeatureQueryRecordFlags flags)
{
    // 1. Check the dynamic world catalog
    if (world && HasNoneFlags(flags, ELDSFeatureQueryRecordFlags::SessionOnly, ELDSFeatureQueryRecordFlags::StaticOnly))
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
    if (featureLDS && world && HasNoneFlags(flags, ELDSFeatureQueryRecordFlags::SessionOnly))
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
    if (session && HasNoneFlags(flags, ELDSFeatureQueryRecordFlags::StaticOnly))
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
    if (HasNoneFlags(flags, ELDSFeatureQueryRecordFlags::Exact))
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
    ELDSFeatureQueryRecordFlags flags)
{
    // 1. Check the dynamic world catalog
    if (world && HasNoneFlags(flags, ELDSFeatureQueryRecordFlags::SessionOnly, ELDSFeatureQueryRecordFlags::StaticOnly))
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
    if (featureLDS && world && HasNoneFlags(flags, ELDSFeatureQueryRecordFlags::SessionOnly))
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
    if (session && HasNoneFlags(flags, ELDSFeatureQueryRecordFlags::StaticOnly))
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
    if (HasNoneFlags(flags, ELDSFeatureQueryRecordFlags::Exact))
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
