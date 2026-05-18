#include "FeatureLDS.h"

#include "LDSFeatureQueryContext.h"
#include "LDSObjectModel.h"
#include "Json/JsonCatalogObjectBuilder.h"
#include "Json/JsonCatalogTypeBuilder.h"
#include "Json/JsonDataSource.h"
#include "PhoenixSim/Session.h"
#include "PhoenixSim/Worlds.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::LDS::Json;
namespace fs = std::filesystem;

void FeatureLDSDynamicBlock::Construct(BlockBufferAllocator& allocator, const Config& config)
{
    Catalog.Construct(allocator, config.MaxObjectRecords, config.MaxTypeRecords);
}

BlockBufferLayout FeatureLDSDynamicBlock::StaticLayout(const Config& config)
{
    return BlockBufferLayout::For<FeatureLDSDynamicBlock>()
        .Container<FixedLDSCatalog>("Catalog", config.MaxObjectRecords, config.MaxTypeRecords);
}

std::shared_ptr<HeapLDSCatalog> FeatureLDS::GetStaticSessionCatalog()
{
    return StaticSessionCatalog;
}

std::shared_ptr<const HeapLDSCatalog> FeatureLDS::GetStaticSessionCatalog() const
{
    return StaticSessionCatalog;
}

std::shared_ptr<const HeapLDSCatalog> FeatureLDS::GetStaticWorldCatalog(WorldConstRef world) const
{
    auto worldCatalog = StaticWorldCatalogs.find(world.GetId());
    if (worldCatalog != StaticWorldCatalogs.end())
    {
        return worldCatalog->second;
    }
    return nullptr;
}

std::shared_ptr<const ILDSQueryContext> FeatureLDS::GetSessionQueryContext() const
{
    return SessionQueryContext;
}

std::shared_ptr<const ILDSQueryContext> FeatureLDS::GetWorldQueryContext(WorldConstRef world) const
{
    auto iter = WorldQueryContexts.find(world.GetId());
    return iter != WorldQueryContexts.end() ? iter->second : std::shared_ptr<const ILDSQueryContext>();
}

std::shared_ptr<const ILDSQueryContext> FeatureLDS::StaticGetSessionQueryContext(SessionConstRef session)
{
    std::shared_ptr<FeatureLDS> featureLDS = session.GetFeature<FeatureLDS>();
    return featureLDS ? featureLDS->GetSessionQueryContext() : std::shared_ptr<const ILDSQueryContext>();
}

std::shared_ptr<const ILDSQueryContext> FeatureLDS::StaticGetWorldQueryContext(WorldConstRef world)
{
    std::shared_ptr<FeatureLDS> featureLDS = GetFeature<FeatureLDS>(world);
    return featureLDS ? featureLDS->GetWorldQueryContext(world) : std::shared_ptr<const ILDSQueryContext>();
}

const LDSRecord* FeatureLDS::QueryObjectRecord(
    WorldConstRef world,
    const LDSRecordPath& path,
    ELDSFeatureRecordQueryFlags flags,
    ELDSRecordQueryFlags flags2)
{
    std::shared_ptr<const ILDSQueryContext> queryContext = StaticGetWorldQueryContext(world);
    if (!queryContext)
    {
        return nullptr;
    }

    const LDSFeatureQueryContext* asdf = reinterpret_cast<const LDSFeatureQueryContext*>(queryContext.get());
    LDSFeatureQueryContext context = asdf->WithFlags(flags).WithMode(ELDSCatalogRecordStore::Object);

    return queryContext->QueryRecord(path, flags2);
}

const LDSRecord* FeatureLDS::QueryTypeRecord(
    WorldConstRef world,
    const LDSRecordPath& path,
    ELDSFeatureRecordQueryFlags flags,
    ELDSRecordQueryFlags flags2)
{
    std::shared_ptr<const ILDSQueryContext> queryContext = StaticGetWorldQueryContext(world);
    if (!queryContext)
    {
        return nullptr;
    }

    const LDSFeatureQueryContext* asdf = reinterpret_cast<const LDSFeatureQueryContext*>(queryContext.get());
    LDSFeatureQueryContext context = asdf->WithFlags(flags).WithMode(ELDSCatalogRecordStore::Type);

    return queryContext->QueryRecord(path, flags2);
}

void FeatureLDS::Initialize(const std::shared_ptr<Phoenix::Session>& session)
{
    IFeature::Initialize(session);

    // TODO (jfarris): load session-level catalogs from disk
    StaticSessionCatalog = std::make_shared<HeapLDSCatalog>();

    SessionQueryContext = std::make_shared<LDSFeatureQueryContext>(LDSFeatureQueryContext::Create(*Session, nullptr));

    auto catalogPathIter = Config.find("catalog");
    if (catalogPathIter != Config.end())
    {
        fs::path dataDir = Session->GetDataDirectory();
        fs::path catalogPath = absolute(dataDir / catalogPathIter->get<std::string>());
        LoadCatalog(catalogPath.generic_string(), *StaticSessionCatalog);
    }
}

void FeatureLDS::Shutdown()
{
    // All world query contexts should have been freed at this point.
    PHX_ASSERT(WorldQueryContexts.empty());

    SessionQueryContext.reset();

    // All world catalogs should have been freed at this point.
    PHX_ASSERT(StaticWorldCatalogs.empty());

    // Unload the session catalog
    StaticSessionCatalog.reset();

    IFeature::Shutdown();
}

void FeatureLDS::OnWorldLayout(const WorldLayoutContext& context, BlockBufferConfigBuilder& builder)
{
    IFeature::OnWorldLayout(context, builder);

    FeatureLDSDynamicBlock::Config dynamicBlockConfig;
    dynamicBlockConfig.MaxObjectRecords = 8192;
    dynamicBlockConfig.MaxTypeRecords = 1024;

    if (const FeatureJsonConfig* featureConfig = context.Config.GetFeatureConfig(GetFeatureId()))
    {
        const nlohmann::json& featureConfigData = featureConfig->GetData();
        dynamicBlockConfig.MaxObjectRecords = featureConfigData.value("max_dynamic_object_records", dynamicBlockConfig.MaxObjectRecords);
        dynamicBlockConfig.MaxTypeRecords = featureConfigData.value("max_dynamic_type_records", dynamicBlockConfig.MaxTypeRecords);
    }

    builder.RegisterBlockWithAlloc<FeatureLDSDynamicBlock>(EBufferBlockType::Dynamic, dynamicBlockConfig);
}

void FeatureLDS::OnWorldInitialize(WorldRef world)
{
    IFeature::OnWorldInitialize(world);

    std::shared_ptr<HeapLDSCatalog> staticWorldCatalog = StaticWorldCatalogs[world.GetId()];
    if (!staticWorldCatalog)
    {
        staticWorldCatalog = std::make_shared<HeapLDSCatalog>();
        StaticWorldCatalogs.emplace(world.GetId(), staticWorldCatalog);
    }

    if (const FeatureJsonConfig* config = world.GetFeatureConfig(GetFeatureId()))
    {
        const nlohmann::json& featureConfigData = config->GetData();
        auto catalogPathIter = featureConfigData.find("catalog");
        if (catalogPathIter != featureConfigData.end())
        {
            fs::path worldsDir = Session->GetWorldsDirectory();
            fs::path catalogPath = absolute(worldsDir / catalogPathIter->get<std::string>());
            LoadCatalog(catalogPathIter->get<std::string>(), *staticWorldCatalog);
        }
    }

    std::shared_ptr<LDSFeatureQueryContext> queryContext = std::make_shared<LDSFeatureQueryContext>(LDSFeatureQueryContext::Create(*Session, &world));
    WorldQueryContexts.emplace(world.GetId(), queryContext);
}

void FeatureLDS::OnWorldShutdown(WorldRef world)
{
    IFeature::OnWorldShutdown(world);

    auto staticCatalogIter = StaticWorldCatalogs.find(world.GetId());
    if (staticCatalogIter != StaticWorldCatalogs.end())
    {
        StaticWorldCatalogs.erase(staticCatalogIter);
    }

    auto queryContextIter = WorldQueryContexts.find(world.GetId());
    if (queryContextIter != WorldQueryContexts.end())
    {
        WorldQueryContexts.erase(queryContextIter);
    }
}

bool FeatureLDS::LoadCatalog(const std::string& catalogAbsolutePath, HeapLDSCatalog& catalog)
{
    std::shared_ptr<JsonDataSource> dataSource = JsonDataSource::LoadFromCatalog(catalogAbsolutePath);
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