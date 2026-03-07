
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

FeatureLDSDynamicBlock::FeatureLDSDynamicBlock(BlockBufferAllocator& allocator, const Config& config)
    : Catalog(allocator, { config.MaxObjectRecords, config.MaxTypeRecords })
{
}

FeatureLDSDynamicBlock::FeatureLDSDynamicBlock(
    BlockBufferAllocator& allocator,
    const Config& config,
    const FeatureLDSDynamicBlock& other)
    : Catalog(allocator, { config.MaxObjectRecords, config.MaxTypeRecords }, other.Catalog)
{
}

BufferBlockLayout FeatureLDSDynamicBlock::Layout(Config config)
{
    BufferBlockLayout layout;
    layout.BlockSize = sizeof(FeatureLDSDynamicBlock);

    struct FixedLDSCatalog::Config catalogConfig;
    catalogConfig.MaxObjectRecords = config.MaxObjectRecords;
    catalogConfig.MaxTypeRecords = config.MaxTypeRecords;
    layout.AllocSize = FixedLDSCatalog::GetAllocSizeBytes(catalogConfig);

    return layout;
}

void FeatureLDSDynamicBlock::Construct(void* dest, BlockBufferAllocator& allocator, Config config)
{
    new (dest) FeatureLDSDynamicBlock(allocator, config);
}

FeatureLDS::FeatureLDS()
{
    FEATURE_CHANNEL(FeatureChannels::WorldInitialize)
    FEATURE_CHANNEL(FeatureChannels::WorldShutdown)
}

TSharedPtr<HeapLDSCatalog> FeatureLDS::GetStaticSessionCatalog()
{
    return StaticSessionCatalog;
}

TSharedPtr<const HeapLDSCatalog> FeatureLDS::GetStaticSessionCatalog() const
{
    return StaticSessionCatalog;
}

TSharedPtr<const HeapLDSCatalog> FeatureLDS::GetStaticWorldCatalog(WorldConstRef world) const
{
    auto worldCatalog = StaticWorldCatalogs.find(world.GetId());
    if (worldCatalog != StaticWorldCatalogs.end())
    {
        return worldCatalog->second;
    }
    return nullptr;
}

TSharedPtr<const ILDSQueryContext> FeatureLDS::GetSessionQueryContext() const
{
    return SessionQueryContext;
}

TSharedPtr<const ILDSQueryContext> FeatureLDS::GetWorldQueryContext(WorldConstRef world) const
{
    auto iter = WorldQueryContexts.find(world.GetId());
    return iter != WorldQueryContexts.end() ? iter->second : TSharedPtr<const ILDSQueryContext>();
}

TSharedPtr<const ILDSQueryContext> FeatureLDS::StaticGetSessionQueryContext(SessionConstRef session)
{
    TSharedPtr<FeatureLDS> featureLDS = session.GetFeature<FeatureLDS>();
    return featureLDS ? featureLDS->GetSessionQueryContext() : TSharedPtr<const ILDSQueryContext>();
}

TSharedPtr<const ILDSQueryContext> FeatureLDS::StaticGetWorldQueryContext(WorldConstRef world)
{
    TSharedPtr<FeatureLDS> featureLDS = GetFeature<FeatureLDS>(world);
    return featureLDS ? featureLDS->GetWorldQueryContext(world) : TSharedPtr<const ILDSQueryContext>();
}

const LDSRecord* FeatureLDS::QueryObjectRecord(
    WorldConstRef world,
    const LDSRecordPath& path,
    ELDSFeatureRecordQueryFlags flags,
    ELDSRecordQueryFlags flags2)
{
    TSharedPtr<const ILDSQueryContext> queryContext = StaticGetWorldQueryContext(world);
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
    TSharedPtr<const ILDSQueryContext> queryContext = StaticGetWorldQueryContext(world);
    if (!queryContext)
    {
        return nullptr;
    }

    const LDSFeatureQueryContext* asdf = reinterpret_cast<const LDSFeatureQueryContext*>(queryContext.get());
    LDSFeatureQueryContext context = asdf->WithFlags(flags).WithMode(ELDSCatalogRecordStore::Type);

    return queryContext->QueryRecord(path, flags2);
}

void FeatureLDS::Initialize(const TSharedPtr<Phoenix::Session>& session)
{
    IFeature::Initialize(session);

    // TODO (jfarris): load session-level catalogs from disk
    StaticSessionCatalog = MakeShared<HeapLDSCatalog>();

    SessionQueryContext = MakeShared<LDSFeatureQueryContext>(LDSFeatureQueryContext::Create(*Session, nullptr));

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

    // All world query contexts should have been freed at this point.
    PHX_ASSERT(WorldQueryContexts.empty());

    SessionQueryContext.reset();

    // All world catalogs should have been freed at this point.
    PHX_ASSERT(StaticWorldCatalogs.empty());

    // Unload the session catalog
    StaticSessionCatalog.reset();
}

void FeatureLDS::OnWorldLayout(const WorldLayoutContext& context, BlockBufferLayoutBuilder& builder)
{
    IFeature::OnWorldLayout(context, builder);

    FeatureLDSDynamicBlock::Config dynamicBlockConfig;
    dynamicBlockConfig.MaxObjectRecords = 8192;
    dynamicBlockConfig.MaxTypeRecords = 1024;

    if (const FeatureJsonConfig* featureConfig = context.Config.GetFeatureConfig(StaticTypeName))
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

    TSharedPtr<HeapLDSCatalog> staticWorldCatalog = StaticWorldCatalogs[world.GetId()];
    if (!staticWorldCatalog)
    {
        staticWorldCatalog = MakeShared<HeapLDSCatalog>();
        StaticWorldCatalogs.emplace(world.GetId(), staticWorldCatalog);
    }

    if (const FeatureJsonConfig* config = world.GetFeatureConfig(StaticTypeName))
    {
        const nlohmann::json& featureConfigData = config->GetData();
        auto catalogPathIter = featureConfigData.find("catalog");
        if (catalogPathIter != featureConfigData.end())
        {
            fs::path worldsDir = Session->GetWorldsDirectory();
            fs::path catalogPath = absolute(worldsDir / catalogPathIter->get<PHXString>());
            LoadCatalog(catalogPathIter->get<PHXString>(), *staticWorldCatalog);
        }
    }

    TSharedPtr<LDSFeatureQueryContext> queryContext = MakeShared<LDSFeatureQueryContext>(LDSFeatureQueryContext::Create(*Session, &world));
    WorldQueryContexts.emplace(world.GetId(), queryContext);
}

void FeatureLDS::OnWorldShutdown(WorldRef world)
{
    IFeature::OnWorldShutdown(world);

    auto staticCatalogIter = StaticWorldCatalogs.find(world.GetId());
    if (staticCatalogIter != StaticWorldCatalogs.end())
    {
        StaticWorldCatalogs.erase(staticCatalogIter);
        staticCatalogIter->second.reset();
    }

    auto queryContextIter = WorldQueryContexts.find(world.GetId());
    if (queryContextIter != WorldQueryContexts.end())
    {
        WorldQueryContexts.erase(queryContextIter);
        queryContextIter->second.reset();
    }
}

bool FeatureLDS::LoadCatalog(const PHXString& catalogAbsolutePath, HeapLDSCatalog& catalog)
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