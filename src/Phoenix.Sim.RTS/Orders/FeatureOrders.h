#pragma once

#include "PhoenixSim/Features.h"
#include "PhoenixSim/Containers/Optional.h"
#include "PhoenixSim/Worlds.h"

#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Orders/FixedOrderQueue.h"

#ifndef PHX_RTS_ORDER_QUEUE_MAX_ORDERS
#define PHX_RTS_ORDER_QUEUE_MAX_ORDERS 4096
#endif

namespace Phoenix::ECS
{
    struct EntityId;
}

namespace Phoenix::RTS
{
    struct AcquireRequest;
    struct UnitId;
    struct Command;
    struct Order;
    struct CommandContext;
    class ICommandHandler;

    struct PHOENIX_RTS_API FeatureOrdersDynamicBlock : BlockBufferBlock
    {
        PHX_DECLARE_BLOCK_WITH_ALLOC(FeatureOrdersDynamicBlock)
        {
            uint32 MaxOrders = PHX_RTS_ORDER_QUEUE_MAX_ORDERS;
        };

        FixedOrderQueue OrderQueue;
    };

    using PrioritizedCommandHandler = std::tuple<UnitId, uint32, std::shared_ptr<ICommandHandler>>;

    // Manages the order queues of all units in the game.
    class PHOENIX_RTS_API FeatureOrders : public IFeature
    {
        PHX_DECLARE_FEATURE_TYPE(FeatureOrders)
        {
            FEATURE_CHANNEL(FeatureChannels::HandleWorldAction)
            FEATURE_CHANNEL(FeatureChannels::PostWorldUpdate)
        }

    public:

        //
        // Command Handler Management
        //

        void RegisterCommandHandler(const std::shared_ptr<ICommandHandler>& handler);

        template <class T, class ...TArgs>
        std::shared_ptr<T> RegisterCommandHandler(TArgs&&... args)
        {
            auto handler = std::make_shared<T>(std::forward<TArgs>(args)...);
            RegisterCommandHandler(handler);
            return handler;
        }

        bool UnregisterCommandHandler(const FName& commandId);

        std::shared_ptr<ICommandHandler> FindCommandHandlerCached(WorldConstRef world, const FName& commandId);

        std::shared_ptr<ICommandHandler> FindCommandHandler(WorldConstRef world, const FName& commandId) const;

        static std::shared_ptr<ICommandHandler> StaticFindCommandHandler(WorldConstRef world, const FName& abilityId);

        //
        // Order Queue
        //

        // Attempts to enqueue a new order to a unit's order queue.
        static bool EnqueueOrder(WorldRef world, const UnitId& unit, const Order& order);

        // Attempts to insert an order at a given order index to a unit's order queue.
        // TODO (jfarris): do we want callers to be able to do this?
        static bool InsertOrder(WorldRef world, const UnitId& unit, const Order& order, uint32 orderIndex);

        // Gets the first order of a unit's order queue.
        static const Order* GetHeadOrder(WorldConstRef world, const UnitId& unit);

        // Gets the first order of a unit's order queue.
        // Use outIndex to find the next order. Note that it is an absolute index, not the unit's order index.
        static const Order* GetHeadOrder(WorldConstRef world, const UnitId& unit, uint32& outIndex);

        // Gets the next order in a unit's order queue.
        // Use outIndex to find the next order. Note that it is an absolute index, not the unit's order index.
        static const Order* GetNextOrder(WorldConstRef world, const UnitId& unit, uint32 currIndex, uint32& outIndex);

        // Gets the order at a specific index in a unit's order queue.
        // The index should be a value between 0 (the head order) and the value returned by GetNumOrders. 
        static const Order* GetOrder(WorldConstRef world, const UnitId& unit, uint32 orderIndex);

        // Gets the number of orders in a unit's order queue.
        static uint32 GetNumOrders(WorldConstRef world, const UnitId& unit);

        // Gets whether there is at least one order in a units order queue.
        static bool HasOrders(WorldConstRef world, const UnitId& unit);

        // Clears the order queue for a unit.
        static uint32 ClearOrderQueue(WorldRef world, const UnitId& unit);

        template <class TCallback>
        static void ForEachOrder(WorldConstRef world, const UnitId& unit, const TCallback& callback)
        {
            const FeatureOrdersDynamicBlock& block = world.GetBlockRef<FeatureOrdersDynamicBlock>();
            block.OrderQueue.ForEachOrder(unit, callback);
        }

        static TOptional<ECS::EntityId> GetHeadOrderTargetEntity(WorldConstRef world, const UnitId& unit);

        static TOptional<Vec2> GetHeadOrderTargetLocation(WorldConstRef world, const UnitId& unit);

        //
        // Command Handling
        //

        static bool StaticHandleCommand(WorldRef world, const Command& command);

        bool HandleCommand(WorldRef world, const Command& command);

        static bool StaticIssueCommand(WorldRef world, const UnitId& unit, const Command& command);

        bool IssueCommand(WorldRef world, const UnitId& unit, const Command& command);

        //
        // Order Handling
        //

        static bool StaticRequestAcquireOrder(WorldRef world, const UnitId& unit, const AcquireRequest& request);

        bool RequestAcquireOrder(WorldRef world, const UnitId& unit, const AcquireRequest& request);
        
        static void StaticOnActiveOrderCompleted(WorldRef world, const UnitId& unit, bool success);

        void OnActiveOrderCompleted(WorldRef world, const UnitId& unit, bool success);

    protected:

        void Initialize(const std::shared_ptr<Phoenix::Session>& session) override;
        void Shutdown() override;

        void OnWorldLayout(const WorldLayoutContext& context, BlockBufferConfigBuilder& builder) override;
        void OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;
        bool OnHandleWorldAction(WorldRef world, const FeatureActionArgs& args) override;

        static void OnEntityReleasing(WorldRef world, ECS::EntityId entity);

        static void SortOrderQueue(WorldRef world);

        static bool HandleCommandForUnitInternal(
            WorldRef world,
            const UnitId& unit,
            const Command& command,
            const std::shared_ptr<ICommandHandler>& handler);

        static bool StaticExecuteHeadOrder(WorldRef world, const UnitId& unit);
        bool ExecuteHeadOrder(WorldRef world, const UnitId& unit);

        static bool ExecuteTransientOrder(
            WorldRef world,
            const UnitId& unit,
            const Order& order,
            const std::shared_ptr<ICommandHandler>& handler);

        static bool StaticInterruptHeadOrder(WorldRef world, const UnitId& unit);
        bool InterruptHeadOrder(WorldRef world, const UnitId& unit);
        
        bool InterruptOrder(WorldRef world, const UnitId& unit, const Order& order);

        // Interrupts the current order and inserts a new order at the head of the order queue.
        bool AcquireOrder(WorldRef world, const UnitId& unit, const Order& order);

        // Attempts to interrupt and remove the head order from a unit's order queue.
        // Note that this DOES NOT automatically execute the new head order, if there is one.
        bool RemoveHeadOrder(WorldRef world, const UnitId& unit);

        uint32 GetPrioritizedHandlers(
            WorldConstRef world,
            const UnitId& unit,
            const CommandContext& context,
            const Command& command,
            std::vector<PrioritizedCommandHandler>& outHandlers);

        bool GetHighestPriorityHandler(
            WorldConstRef world,
            const UnitId& unit,
            const CommandContext& context,
            const Command& command,
            PrioritizedCommandHandler& outHandler);

        uint32 GetHighestPriorityHandlersForSelection(
            WorldConstRef world,
            const Command& command,
            std::vector<PrioritizedCommandHandler>& outHandlers);

        std::unordered_map<FName, std::shared_ptr<ICommandHandler>> CommandIdToHandlerMap;
    };
}

PHX_DEFINE_TYPE(Phoenix::RTS::FeatureOrders)
{
    registration
        .Namespace("Phoenix.Orders")
        .StaticMethod("IssueCommand(world, unit, command)", &RTS::FeatureOrders::StaticIssueCommand)
        .StaticMethod("HasOrders(world, unit)",    &RTS::FeatureOrders::HasOrders);
}