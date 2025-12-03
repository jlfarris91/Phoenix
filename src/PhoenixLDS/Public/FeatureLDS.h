
#pragma once

#include "DLLExport.h"
#include "Features.h"
#include "LDSRecordStore.h"
#include "LDSCatalog.h"
#include "Session.h"

namespace Phoenix::LDS
{
    struct FeatureLDSStaticBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_STATIC(FeatureLDSStaticBlock)

        TFixedCatalog<16384, 1024> Catalog;
    };

    struct FeatureLDSDynamicBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK_STATIC(FeatureLDSDynamicBlock)

        TFixedCatalog<8192, 1024> Catalog;
    };

    enum class ELDSFeatureQueryRecordFlags : uint8
    {
        None = 0,

        // Don't search base objects
        Exact = 1,

        // Don't search dynamic catalogs
        StaticOnly = 2,

        // Don't search world catalogs
        SessionOnly = 4,
    };

    class PHOENIX_LDS_API FeatureLDS : public IFeature
    {
        PHX_FEATURE_BEGIN(FeatureLDS)
            FEATURE_SESSION_BLOCK(FeatureLDSStaticBlock)
            FEATURE_SESSION_BLOCK(FeatureLDSDynamicBlock)
        PHX_FEATURE_END()

    public:

        // Gets the static session-level catalog.
        TSharedPtr<Catalog> GetStaticSessionCatalog();
        TSharedPtr<const Catalog> GetStaticSessionCatalog() const;

        // Gets the static catalog for a given world.
        TSharedPtr<const Catalog> GetStaticWorldCatalog(WorldConstRef world) const;

        static const LDSRecord* QueryObjectRecord(
            SessionConstPtr session,
            WorldConstPtr world,
            const FName& objectId,
            const FName& propertyId,
            ELDSFeatureQueryRecordFlags flags = ELDSFeatureQueryRecordFlags::None);

        static const LDSRecord* QueryTypeRecord(
            SessionConstPtr session,
            WorldConstPtr world,
            const FName& typeId,
            const FName& propertyId,
            ELDSFeatureQueryRecordFlags flags = ELDSFeatureQueryRecordFlags::None);

    protected:

        void Initialize() override;
        void Shutdown() override;

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
