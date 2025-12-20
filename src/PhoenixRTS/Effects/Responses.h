#pragma once

#include "PhoenixSim/Name.h"
#include "PhoenixSim/Session.h"
#include "PhoenixSim/Worlds.h"

#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Effects/EffectId.h"
#include "PhoenixRTS/Data/DataResponse.h"

namespace Phoenix::LDS
{
    class ILDSQueryContext;
}

namespace Phoenix::RTS
{
    class FeatureEffects;
    struct EffectComponent;

    struct PHOENIX_RTS_API ResponseContext
    {
        EffectNodeId EffectNodeId;
        EffectComponent* EffectComponent = nullptr;
        FName ResponseId;
        Data::EResponseTarget Target = Data::EResponseTarget::None;
        TSharedPtr<const LDS::ILDSQueryContext> LdsQueryContext;
    };

    class PHOENIX_RTS_API IResponseHandler : TSharedAsThis<IResponseHandler>
    {
    public:

        virtual ~IResponseHandler() = default;

        virtual void Initialize(SessionRef session);
        virtual void Shutdown(SessionRef session);

        virtual void OnWorldInitialize(WorldRef world);
        virtual void OnWorldShutdown(WorldRef world);

        virtual FName GetResponseId() const;

        virtual int32 GetPriority(WorldConstRef world, const ResponseContext& context) const;

        virtual bool Execute(WorldRef world, const ResponseContext& context) const;
        virtual bool CanExecute(WorldConstRef world, const ResponseContext& context) const;
    };

    class PHOENIX_RTS_API ResponseHandlerBase : public IResponseHandler
    {
    public:
        ResponseHandlerBase();
        ResponseHandlerBase(const FName& responseId);

        void Initialize(SessionRef session) override;
        void Shutdown(SessionRef session) override;

        FName GetResponseId() const override;

        int32 GetPriority(WorldConstRef world, const ResponseContext& context) const override;

        bool Execute(WorldRef world, const ResponseContext& context) const override;
        bool CanExecute(WorldConstRef world, const ResponseContext& context) const override;

        FName ResponseId;

        TSharedPtr<FeatureEffects> EffectsFeature;
    };
}
