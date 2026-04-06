#pragma once

#include "PhoenixSim/ECS/System.h"

#include "PhoenixRTS/Abilities/AbilityComponent.h"
#include "PhoenixRTS/Abilities/AbilityHandler.h"
#include "PhoenixRTS/Abilities/States/CommonAbilityStates.h"

namespace Phoenix::RTS
{
    class PHOENIX_RTS_API MoveAbilitySystem : public ECS::ISystem
    {
        PHX_ECS_DECLARE_SYSTEM(MoveAbilitySystem)

        void OnWorldUpdate(WorldRef world, const ECS::SystemUpdateArgs& args) override;
    };

    enum class PHOENIX_RTS_API EMoveAbilityState
    {
        Idle,
        MoveToPosition,
        FollowEntity
    };

    struct PHOENIX_RTS_API MoveAbilityComponent : IAbilityComponent
    {
        PHX_ECS_DECLARE_COMPONENT(MoveAbilityComponent)

        MoveAbilityComponent();

        union
        {
            MoveToLocationState MoveToPosition;
            FollowEntityState FollowEntity;
        } States;

        EMoveAbilityState ActiveState = EMoveAbilityState::Idle;

        AbilityStateResult Update(WorldRef world, const UnitId& unit);
        void Interrupt(WorldRef world, const UnitId& unit);
        void Exit(WorldRef world, const UnitId& unit);
    };

    class PHOENIX_RTS_API MoveAbilityHandler : public IAbilityHandler
    {
        PHX_DECLARE_TYPE(MoveAbilityHandler, IAbilityHandler)

    public:

        struct Commands
        {
            static constexpr uint8 Move = 0;
            static constexpr uint8 Patrol = 1;
        };

        MoveAbilityHandler();

        FName GetCommandId() const override;

        void Initialize(const std::shared_ptr<Phoenix::Session>& session) override;
        void Shutdown() override;

        void OnWorldInitialize(WorldRef world) override;
        void OnWorldShutdown(WorldRef world) override;

        bool AddAbility(WorldRef world, const UnitId& unit) const override;
        bool RemoveAbility(WorldRef world, const UnitId& unit) const override;
        bool HasAbility(WorldConstRef world, const UnitId& unit) const override;

        bool IgnoreCommand(WorldConstRef world, const CommandContext& context, const Command& command) const override;

        uint32 GetCommandPriority(WorldConstRef world, const CommandContext& context, const Command& command) const override;

        bool ExecuteOrder(WorldRef world, const UnitId& unit, const Order& order) const override;

        bool InterruptOrder(WorldRef world, const UnitId& unit, const Order& order) const override;

    private:

        static uint32 GetSmartCommandPriority(
            WorldConstRef world,
            const CommandContext& context,
            const Command& command);

        std::shared_ptr<MoveAbilitySystem> System;
    };
}
