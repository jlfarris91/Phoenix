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

    class PHOENIX_RTS_API ICommandHandler : public IService
    {
        PHX_DECLARE_TYPE_BEGIN(ICommandHandler)
            PHX_REGISTER_BASE(IService)
        PHX_DECLARE_TYPE_END()

    public:

        virtual FName GetCommandId() const;

        void Initialize(const TSharedPtr<Phoenix::Session>& session) override;
        void Shutdown() override;

        virtual void OnWorldInitialize(WorldRef world);
        virtual void OnWorldShutdown(WorldRef world);

        virtual bool IgnoreCommand(WorldConstRef world, const CommandContext& context, const Command& command) const;

        virtual uint32 GetCommandPriority(WorldConstRef world, const CommandContext& context, const Command& command) const;

        virtual bool IsTransient(WorldConstRef world, const Order& order) const;

        virtual bool ExecuteOrder(WorldRef world, const UnitId& unit, const Order& order) const;

        virtual bool InterruptOrder(WorldRef world, const UnitId& unit, const Order& order) const;

        virtual uint32 AcquireOrder(WorldRef world, const UnitId& unit, const Order& order) const;

        virtual bool SupportsMagicBox(const Order& order) const;

    protected:

        TSharedPtr<FeatureOrders> OrdersFeature;
    };
}
