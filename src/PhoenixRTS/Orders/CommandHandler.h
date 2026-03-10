#pragma once

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Services/Service.h"

#include "PhoenixRTS/Orders/Commands.h"
#include "PhoenixRTS/Orders/Orders.h"
#include "PhoenixRTS/Units/UnitId.h"

namespace Phoenix::RTS
{
    class FeatureOrders;

    struct PHOENIX_RTS_API CommandContext
    {
        UnitId Unit;
        FName SelectionGroupId;
        std::shared_ptr<const LDS::ILDSQueryContext> LdsQueryContext;
    };

    struct PHOENIX_RTS_API AcquireContext
    {
        UnitId Unit;
        FName AbilityId;
        std::shared_ptr<const LDS::ILDSQueryContext> LdsQueryContext;
    };

    class PHOENIX_RTS_API ICommandHandler : public IService
    {
        PHX_DECLARE_INTERFACE_WITH_BASE(ICommandHandler, IService)

    public:

        virtual FName GetCommandId() const;

        void Initialize(const std::shared_ptr<Phoenix::Session>& session) override;
        void Shutdown() override;

        virtual bool IgnoreCommand(WorldConstRef world, const CommandContext& context, const Command& command) const;

        virtual uint32 GetCommandPriority(WorldConstRef world, const CommandContext& context, const Command& command) const;

        virtual AcquireResult AcquireOrder(WorldConstRef world, const AcquireContext& context, const AcquireRequest& request) const;

        virtual bool IsTransient(WorldConstRef world, const Order& order) const;

        virtual bool ExecuteOrder(WorldRef world, const UnitId& unit, const Order& order) const;

        virtual bool InterruptOrder(WorldRef world, const UnitId& unit, const Order& order) const;

        virtual bool SupportsMagicBox(const Order& order) const;

    protected:

        std::shared_ptr<FeatureOrders> OrdersFeature;
    };
}
