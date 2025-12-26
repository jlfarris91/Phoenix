#pragma once

#include "PhoenixSim/ECS/System.h"

#include "PhoenixRTS/Abilities/AbilityComponent.h"
#include "PhoenixRTS/Abilities/AbilityHandler.h"
#include "PhoenixRTS/Abilities/Attack/AttackAbilityStates.h"

namespace Phoenix::RTS
{
    namespace Data
    {
        struct AttackAbilityPtr;
    }

    class PHOENIX_RTS_API AttackAbilitySystem : public ECS::ISystem
    {
        PHX_ECS_DECLARE_SYSTEM_BEGIN(AttackAbilitySystem)
        PHX_ECS_DECLARE_SYSTEM_END()

        void OnWorldUpdate(WorldRef world, const ECS::SystemUpdateArgs& args) override;
    };

    struct PHOENIX_RTS_API AttackAbilityComponent : IAbilityComponent
    {
        PHX_ECS_DECLARE_COMPONENT_BEGIN(AttackAbilityComponent)
        PHX_ECS_DECLARE_COMPONENT_END()

        AttackAbilityComponent();

        union
        {
            AttackTargetState AttackEntity;
            AttackLocationState AttackLocation;
            AttackMoveState AttackMove;
            FollowEntityState FollowEntity;
        } States;

        EAttackAbilityState ActiveState = EAttackAbilityState::None;

        AbilityStateResult Update(WorldRef world, const UnitId& unit);
        void Interrupt(WorldRef world, const UnitId& unit);
        void Exit(WorldRef world, const UnitId& unit);
    };

    class PHOENIX_RTS_API AttackAbilityHandler : public IAbilityHandler
    {
        PHX_DECLARE_TYPE_BEGIN(AttackAbilityHandler)
            PHX_REGISTER_BASE(IAbilityHandler)
        PHX_DECLARE_TYPE_END()

    public:

        struct Commands
        {
            static constexpr uint8 Attack = 0;
            static constexpr uint8 AttackGround = 1;
            static constexpr uint8 AttackMove = 2;
            static constexpr uint8 AttackHold = 3;
        };

        AttackAbilityHandler();

        FName GetCommandId() const override;

        void Initialize(const TSharedPtr<Phoenix::Session>& session) override;
        void Shutdown() override;

        void OnWorldInitialize(WorldRef world) override;
        void OnWorldShutdown(WorldRef world) override;

        bool AddAbility(WorldRef world, const UnitId& unit) const override;
        bool RemoveAbility(WorldRef world, const UnitId& unit) const override;
        bool HasAbility(WorldConstRef world, const UnitId& unit) const override;

        uint32 GetCommandPriority(WorldConstRef world, const CommandContext& context, const Command& command) const override;

        bool ExecuteOrder(WorldRef world, const UnitId& unit, const Order& order) const override;

        bool InterruptOrder(WorldRef world, const UnitId& unit, const Order& order) const override;

        uint32 AcquireOrder(WorldRef world, const UnitId& unit, const Order& order) const override;

        bool SupportsMagicBox(const Order& order) const override;

    private:

        static uint32 GetAcquireCommandPriority(
            WorldConstRef world,
            const CommandContext& context,
            const Command& command);

        static uint32 GetSmartCommandPriority(
            WorldConstRef world,
            const CommandContext& context,
            const Command& command);

        static bool ExecuteAttackTargetOrder(
            WorldRef world,
            const UnitId& unit,
            const UnitId& target,
            const Data::AttackAbilityPtr& attackAbility,
            AttackAbilityComponent& attackComp);

        static bool ExecuteAttackMoveOrder(
            WorldRef world,
            const UnitId& unit,
            const Vec2& target,
            const Data::AttackAbilityPtr& attackAbility,
            AttackAbilityComponent& attackComp);

        static bool ExecuteAttackGroundOrder(
            WorldRef world,
            const UnitId& unit,
            const Vec2& target,
            const Data::AttackAbilityPtr& attackAbility,
            AttackAbilityComponent& attackComp);

        TSharedPtr<AttackAbilitySystem> System;
    };
}
