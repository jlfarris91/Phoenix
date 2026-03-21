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
        PHX_ECS_DECLARE_SYSTEM(AttackAbilitySystem)

        void OnWorldUpdate(WorldRef world, const ECS::SystemUpdateArgs& args) override;
    };

    struct PHOENIX_RTS_API AttackAbilityComponent : IAbilityComponent
    {
        PHX_ECS_DECLARE_COMPONENT(AttackAbilityComponent)

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
        PHX_ENABLE_TYPE(AttackAbilityHandler, IAbilityHandler)

    public:

        struct Commands
        {
            static constexpr uint8 Attack = 0;
            static constexpr uint8 AttackGround = 1;
            static constexpr uint8 AttackHold = 2;
        };

        AttackAbilityHandler();

        static FName StaticGetCommandId();
        FName GetCommandId() const override;

        void Initialize(const std::shared_ptr<Phoenix::Session>& session) override;
        void Shutdown() override;

        void OnWorldInitialize(WorldRef world) override;
        void OnWorldShutdown(WorldRef world) override;

        bool AddAbility(WorldRef world, const UnitId& unit) const override;
        bool RemoveAbility(WorldRef world, const UnitId& unit) const override;
        bool HasAbility(WorldConstRef world, const UnitId& unit) const override;

        uint32 GetCommandPriority(WorldConstRef world, const CommandContext& context, const Command& command) const override;

        AcquireResult AcquireOrder(
            WorldConstRef world,
            const AcquireContext& context,
            const AcquireRequest& request) const override;

        bool ExecuteOrder(WorldRef world, const UnitId& unit, const Order& order) const override;

        bool InterruptOrder(WorldRef world, const UnitId& unit, const Order& order) const override;

        bool SupportsMagicBox(const Order& order) const override;

    private:

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

        std::shared_ptr<AttackAbilitySystem> System;
    };
}
