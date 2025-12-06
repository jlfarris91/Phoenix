
#pragma once

#include "DLLExport.h"
#include "LDSRecordQueryFlags.h"
#include "Features.h"
#include "LDSRecordStore.h"
#include "LDSCatalog.h"
#include "Session.h"
#include "ObjectModel/LDSQueryContext.h"

namespace Phoenix::LDS
{
    struct LDSRecordPath;
}

namespace Phoenix::LDS
{
    struct FeatureLDSSessionDynamicBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_STATIC(FeatureLDSSessionDynamicBlock)

        TFixedCatalog<8192, 1024> Catalog;
    };

    struct FeatureLDSWorldDynamicBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_STATIC(FeatureLDSWorldDynamicBlock)

        TFixedCatalog<1024, 1024> Catalog;
    };

    enum class ELDSFeatureRecordQueryFlags : uint8
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

    class PHOENIX_LDS_API FeatureLDS : public IFeature
    {
        PHX_FEATURE_BEGIN(FeatureLDS)
            FEATURE_SESSION_BLOCK(FeatureLDSSessionDynamicBlock)
            FEATURE_WORLD_BLOCK(FeatureLDSWorldDynamicBlock)
            FEATURE_CHANNEL(FeatureChannels::WorldInitialize)
            FEATURE_CHANNEL(FeatureChannels::WorldShutdown)
        PHX_FEATURE_END()

    public:

        // Gets the static session-level catalog.
        TSharedPtr<Catalog> GetStaticSessionCatalog();
        TSharedPtr<const Catalog> GetStaticSessionCatalog() const;

        // Gets the static catalog for a given world.
        TSharedPtr<const Catalog> GetStaticWorldCatalog(WorldConstRef world) const;

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

    protected:

        void Initialize() override;
        void Shutdown() override;

        static bool LoadCatalog(const PHXString& catalogAbsolutePath, Catalog& catalog);

        void OnWorldInitialize(WorldRef world) override;
        void OnWorldShutdown(WorldRef world) override;

        // A catalog that holds static LDS data for the session.
        // These catalogs are stored on the heap.
        TSharedPtr<Catalog> StaticSessionCatalog;

        // Catalogs holding static LDS data for each world.
        // These catalogs are stored on the heap.
        TMap<FName, TSharedPtr<Catalog>> StaticWorldCatalogs;
    };
}
