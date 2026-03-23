#pragma once

#include "PhoenixSim/Name.h"
#include "PhoenixSim/Services/Service.h"
#include "PhoenixSim/WorldsFwd.h"

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
        std::shared_ptr<const LDS::ILDSQueryContext> LdsQueryContext;
    };

    class PHOENIX_RTS_API IResponseHandler : public IService
    {
        PHX_ENABLE_TYPE(IResponseHandler, IService)
        
    public:

        void Initialize(const std::shared_ptr<Phoenix::Session>& session) override;
        void Shutdown() override;

        virtual FName GetResponseId() const;

        virtual int32 GetPriority(WorldConstRef world, const ResponseContext& context) const;

        virtual bool Execute(WorldRef world, const ResponseContext& context) const;
        virtual bool CanExecute(WorldConstRef world, const ResponseContext& context) const;

    protected:

        std::shared_ptr<FeatureEffects> EffectsFeature;
    };

    class PHOENIX_RTS_API ResponseHandlerBase : public IResponseHandler
    {
        PHX_ENABLE_TYPE(ResponseHandlerBase, IResponseHandler)

    public:

        ResponseHandlerBase();
        ResponseHandlerBase(const FName& responseId);

        FName GetResponseId() const override;

        int32 GetPriority(WorldConstRef world, const ResponseContext& context) const override;

        bool Execute(WorldRef world, const ResponseContext& context) const override;
        bool CanExecute(WorldConstRef world, const ResponseContext& context) const override;

        FName ResponseId;
    };
}
