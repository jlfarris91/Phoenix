
#include "FeatureLDS.h"

#include "Flags.h"
#include "LDSFeatureQueryContext.h"
#include "LDSObjectModel.h"
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

    catalog.Sort();
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
    WorldConstRef world,
    const LDSRecordPath& path,
    ELDSRecordQueryFlags flags)
{
    LDSFeatureQueryContext queryContext = LDSFeatureQueryContext::Create(world);
    return queryContext.QueryObjectRecord(path, flags);
}

const LDSRecord* FeatureLDS::QueryTypeRecord(
    WorldConstRef world,
    const LDSRecordPath& path,
    ELDSRecordQueryFlags flags)
{
    LDSFeatureQueryContext queryContext = LDSFeatureQueryContext::Create(world);
    return queryContext.QueryTypeRecord(path, flags);
}