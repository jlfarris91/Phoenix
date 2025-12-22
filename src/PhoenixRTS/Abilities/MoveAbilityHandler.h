#pragma once

#include "PhoenixSim/ECS/System.h"

#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Abilities/Ability.h"
#include "PhoenixRTS/Abilities/AbilityStates.h"

namespace Phoenix::RTS
{
    class PHOENIX_RTS_API MoveAbilitySystem : public ECS::ISystem
    {
        PHX_ECS_DECLARE_SYSTEM_BEGIN(MoveAbilitySystem)
        PHX_ECS_DECLARE_SYSTEM_END()

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
        PHX_ECS_DECLARE_COMPONENT_BEGIN(MoveAbilityComponent)
        PHX_ECS_DECLARE_COMPONENT_END()

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

    class PHOENIX_RTS_API MoveAbilityHandler : public AbilityHandlerBase
    {
        PHX_DECLARE_DERIVED_TYPE_BEGIN(MoveAbilityHandler, AbilityHandlerBase)
        PHX_DECLARE_DERIVED_TYPE_END()

    public:

        struct Commands
        {
            static constexpr uint8 Move = 0;
            static constexpr uint8 Patrol = 1;
        };

        MoveAbilityHandler();

        void Initialize(SessionRef session) override;
        void Shutdown(SessionRef session) override;

        void OnWorldInitialize(WorldRef world) override;
        void OnWorldShutdown(WorldRef world) override;

        bool AddAbility(WorldRef world, const UnitId& unit) const override;
        bool RemoveAbility(WorldRef world, const UnitId& unit) const override;
        bool HasAbility(WorldConstRef world, const UnitId& unit) const override;

        bool IgnoreCommand(WorldConstRef world, const AbilityCommandContext& context, const Command& command) const override;

        uint32 GetCommandPriority(WorldConstRef world, const AbilityCommandContext& context, const Command& command) const override;
        uint32 GetSmartCommandPriority(WorldConstRef world, const AbilityCommandContext& context, const Command& command) const override;

        bool ExecuteOrder(WorldRef world, const UnitId& unit, const Order& order) const override;

        bool InterruptOrder(WorldRef world, const UnitId& unit, const Order& order) const override;

        uint32 Acquire(const Order& order) const override;

        bool SupportsMagicBox(const Order& order) const override;

    private:

        TSharedPtr<MoveAbilitySystem> System;
    };
}
