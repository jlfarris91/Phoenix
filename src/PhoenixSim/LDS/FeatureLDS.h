
#pragma once

#include "PhoenixSim/Features.h"
#include "PhoenixSim/SessionFwd.h"
#include "PhoenixSim/LDS/LDSCatalog.h"
#include "PhoenixSim/LDS/LDSRecordQueryFlags.h"
#include "PhoenixSim/LDS/LDSRecordStore.h"
#include "PhoenixSim/LDS/LDSObjectModel.h"

namespace Phoenix::LDS
{
    struct LDSRecordPath;
}

namespace Phoenix::LDS
{
    struct PHOENIX_SIM_API FeatureLDSDynamicBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_WITH_ALLOC(FeatureLDSDynamicBlock)

        struct Config
        {
            uint32 MaxObjectRecords = 0;
            uint32 MaxTypeRecords = 0;
        };

        FixedLDSCatalog Catalog;
    };

    enum class PHOENIX_SIM_API ELDSFeatureRecordQueryFlags : uint8
    {
        None = 0,

        // Don't search base objects or types.
        Exact = 1,

        // Don't search static catalogs.
        DynamicOnly = 2,

        // Don't search dynamic catalogs.
        StaticOnly = 4,

        // Don't search session catalogs.
        WorldOnly = 8,

        // Don't search world catalogs.
        SessionOnly = 16,
    };

    class PHOENIX_SIM_API FeatureLDS : public IFeature
    {
        PHX_DECLARE_FEATURE_TYPE(FeatureLDS)

    public:

        FeatureLDS();

        // Gets the static session-level catalog.
        TSharedPtr<HeapLDSCatalog> GetStaticSessionCatalog();
        TSharedPtr<const HeapLDSCatalog> GetStaticSessionCatalog() const;

        // Gets the static catalog for a given world.
        TSharedPtr<const HeapLDSCatalog> GetStaticWorldCatalog(WorldConstRef world) const;

        TSharedPtr<const ILDSQueryContext> GetSessionQueryContext() const;
        TSharedPtr<const ILDSQueryContext> GetWorldQueryContext(WorldConstRef world) const;

        static TSharedPtr<const ILDSQueryContext> StaticGetSessionQueryContext(SessionConstRef session);
        static TSharedPtr<const ILDSQueryContext> StaticGetWorldQueryContext(WorldConstRef world);

        static const LDSRecord* QueryObjectRecord(
            WorldConstRef world,
            const LDSRecordPath& path,
            ELDSFeatureRecordQueryFlags flags = ELDSFeatureRecordQueryFlags::None,
            ELDSRecordQueryFlags flags2 = ELDSRecordQueryFlags::None);

        static const LDSRecord* QueryTypeRecord(
            WorldConstRef world,
            const LDSRecordPath& path,
            ELDSFeatureRecordQueryFlags flags = ELDSFeatureRecordQueryFlags::None,
            ELDSRecordQueryFlags flags2 = ELDSRecordQueryFlags::None);

        //
        // Pointer Accessors
        //

        template <IsNotRecordPtr TValue>
        static TValue GetValue(WorldConstRef world, const LDSValuePtr& ptr, const TValue& defaultValue = {});

        template <IsNotRecordPtr TValue>
        static bool TryGetValue(WorldConstRef world, const LDSValuePtr& ptr, TValue& outValue);

        template <IsNotRecordPtr TValue>
        static TValue GetValue(WorldConstRef world, const TLDSValuePtr<TValue>& ptr, const TValue& defaultValue = {});

        template <IsNotRecordPtr TValue>
        static bool TryGetValue(WorldConstRef world, const TLDSValuePtr<TValue>& ptr, TValue& outValue);

    protected:

        void Initialize(const TSharedPtr<Phoenix::Session>& session) override;
        void Shutdown() override;

        void OnWorldLayout(const WorldLayoutContext& context, BlockBufferLayoutBuilder& builder) override;
        void OnWorldInitialize(WorldRef world) override;
        void OnWorldShutdown(WorldRef world) override;

        static bool LoadCatalog(const PHXString& catalogAbsolutePath, HeapLDSCatalog& catalog);

        TSharedPtr<HeapLDSCatalog> StaticSessionCatalog;
        std::unordered_map<FName, TSharedPtr<HeapLDSCatalog>> StaticWorldCatalogs;

        TSharedPtr<ILDSQueryContext> SessionQueryContext;
        std::unordered_map<FName, TSharedPtr<ILDSQueryContext>> WorldQueryContexts;
    };
}

#include "PhoenixSim/LDS/FeatureLDS.inl"