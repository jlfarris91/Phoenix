#pragma once

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Session.h"

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
        TSharedPtr<const LDS::ILDSQueryContext> LdsQueryContext;
    };

    struct PHOENIX_RTS_API AcquireContext
    {
        UnitId Unit;
        FName AbilityId;
        TSharedPtr<const LDS::ILDSQueryContext> LdsQueryContext;
    };

    class PHOENIX_RTS_API ICommandHandler : public IService
    {
        PHX_DECLARE_INTERFACE_WITH_BASE(ICommandHandler, IService)

    public:

        virtual FName GetCommandId() const;

        void Initialize(const TSharedPtr<Phoenix::Session>& session) override;
        void Shutdown() override;

        virtual bool IgnoreCommand(WorldConstRef world, const CommandContext& context, const Command& command) const;

        virtual uint32 GetCommandPriority(WorldConstRef world, const CommandContext& context, const Command& command) const;

        virtual AcquireResult AcquireOrder(WorldConstRef world, const AcquireContext& context, const AcquireRequest& request) const;

        virtual bool IsTransient(WorldConstRef world, const Order& order) const;

        virtual bool ExecuteOrder(WorldRef world, const UnitId& unit, const Order& order) const;

        virtual bool InterruptOrder(WorldRef world, const UnitId& unit, const Order& order) const;

        virtual bool SupportsMagicBox(const Order& order) const;

    protected:

        TSharedPtr<FeatureOrders> OrdersFeature;
    };
}
