#pragma once

#include "Phoenix.Sim/ECS/System.h"

#include "Phoenix.Sim.RTS/Abilities/AbilityHandler.h"
#include "Phoenix.Sim.RTS/Abilities/Attack/AttackAbilityStates.h"

namespace Phoenix::RTS
{
    struct AttackAbilityTask;

    namespace Data
    {
        struct AttackAbilityPtr;
    }

    class PHOENIX_RTS_API AttackAbilityHandler : public IAbilityHandler
    {
        PHX_DECLARE_TYPE(AttackAbilityHandler, IAbilityHandler)

    public:

        struct Commands
        {
            static constexpr uint8 Attack = 0;
            static constexpr uint8 AttackGround = 1;
            static constexpr uint8 AttackHold = 2;
        };

        static FName StaticGetCommandId();
        FName GetCommandId() const override;

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
    };
}
