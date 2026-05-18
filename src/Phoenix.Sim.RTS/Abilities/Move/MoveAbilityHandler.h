#pragma once

#include "Phoenix.Sim.ECS/System.h"

#include "Phoenix.Sim.RTS/Abilities/AbilityComponent.h"
#include "Phoenix.Sim.RTS/Abilities/AbilityHandler.h"
#include "Phoenix.Sim.RTS/Abilities/States/CommonAbilityStates.h"

namespace Phoenix::RTS
{
    class PHOENIX_RTS_API MoveAbilityHandler : public IAbilityHandler
    {
        PHX_DECLARE_TYPE(MoveAbilityHandler, IAbilityHandler)

    public:

        struct Commands
        {
            static constexpr uint8 Move = 0;
            static constexpr uint8 Patrol = 1;
        };

        FName GetCommandId() const override;

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
    };
}
