#pragma once

#include "PhoenixSim/ECS/System.h"
#include "PhoenixRTS/Abilities/Ability.h"
#include "PhoenixRTS/Abilities/AbilityStates.h"
#include "PhoenixRTS/Abilities/WeaponAbilityStates.h"

namespace Phoenix::RTS
{
    class PHOENIX_RTS_API AttackAbilitySystem : public ECS::ISystem
    {
        PHX_ECS_DECLARE_SYSTEM_BEGIN(AttackAbilitySystem)
        PHX_ECS_DECLARE_SYSTEM_END()

        void OnWorldUpdate(WorldRef world, const ECS::SystemUpdateArgs& args) override;
    };

    enum class PHOENIX_RTS_API EAttackAbilityState
    {
        Idle,
        MoveToPosition,
        Face,
        WeaponPreSwing,
        WeaponCooldown,
        WeaponSwing,
        WeaponChannel,
        WeaponBackswing
    };

    struct PHOENIX_RTS_API AttackTargetState
    {
        enum class EActiveState
        {
            None,
            MoveToEntity,
            FaceEntity,
            PreSwing,
            Cooldown,
            Swing,
            Channel,
            BackSwing
        } ActiveState = EActiveState::None;

        MoveToEntityState MoveToEntity;
        FaceEntityState FaceEntity;
        WeaponPreSwingState PreSwing;
        WeaponCooldownState Cooldown;
        WeaponSwingState Swing;
        WeaponExecuteState Channel;
        WeaponBackSwingState BackSwing;
    };

    struct PHOENIX_RTS_API AttackAbilityComponent : IAbilityComponent
    {
        PHX_ECS_DECLARE_COMPONENT_BEGIN(AttackAbilityComponent)
        PHX_ECS_DECLARE_COMPONENT_END()

        AttackAbilityComponent();

        union
        {
            MoveToLocationState MoveToPosition;
        } ActiveState;

        EAttackAbilityState State = EAttackAbilityState::Idle;
    };

    class PHOENIX_RTS_API AttackAbilityHandler : public AbilityHandlerBase
    {
        PHX_DECLARE_DERIVED_TYPE_BEGIN(AttackAbilityHandler, AbilityHandlerBase)
        PHX_DECLARE_DERIVED_TYPE_END()

    public:

        struct Commands
        {
            static constexpr uint8 Attack = 0;
            static constexpr uint8 AttackGround = 1;
            static constexpr uint8 AttackMove = 2;
            static constexpr uint8 AttackHold = 3;
        };

        AttackAbilityHandler();

        void Initialize(SessionRef session) override;

        void Shutdown(SessionRef session) override;

        void OnWorldInitialize(WorldRef world) override;

        void OnWorldShutdown(WorldRef world) override;

        bool AddAbility(WorldRef world, const UnitId& unit) const override;

        bool RemoveAbility(WorldRef world, const UnitId& unit) const override;

        bool HasAbility(WorldConstRef world, const UnitId& unit) const override;

        uint32 GetCommandPriority(WorldConstRef world, const AbilityCommandContext& context, const Command& command) const override;

        uint32 GetSmartCommandPriority(WorldConstRef world, const AbilityCommandContext& context, const Command& command) const override;

        bool ExecuteOrder(WorldRef world, const UnitId& unit, const Order& order) const override;

        bool InterruptOrder(WorldRef world, const UnitId& unit, const Order& order) const override;

        uint32 Acquire(const Order& order) const override;

        bool SupportsMagicBox(const Order& order) const override;

    private:

        TSharedPtr<AttackAbilitySystem> System;
    };
}
