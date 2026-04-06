#pragma once

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/SessionFwd.h"
#include "PhoenixSim/WorldsFwd.h"
#include "PhoenixSim/FixedPoint/FixedVector.h"

#include "PhoenixRTS/Orders/CommandHandler.h"
#include "PhoenixRTS/TargetFiltering/TargetScanner.h"
#include "PhoenixRTS/Units/UnitId.h"

namespace Phoenix::LDS
{
    class ILDSQueryContext;
}

namespace Phoenix::ECS
{
    struct EntityId;
}

namespace Phoenix::RTS
{
    struct Command;
    struct UnitId;
    class FeatureAbilities;

    struct PHOENIX_RTS_API AbilityPriority
    {
        static uint32 All();
        static uint32 SelfTargetOrAll(const UnitId& unit, ECS::EntityId target);
        static uint32 Closest(WorldConstRef world, const UnitId& unit, const Vec2& target);
    };

    struct PHOENIX_RTS_API AbilityTargetScanArgs
    {
        UnitId Unit;
        FName AbilityId;
        TargetScanArgs ScanArgs;
    };

    class PHOENIX_RTS_API IAbilityHandler : public ICommandHandler
    {
        PHX_DECLARE_TYPE(IAbilityHandler, ICommandHandler)

    public:

        void Initialize(const std::shared_ptr<Phoenix::Session>& session) override;
        void Shutdown() override;

        virtual bool AddAbility(WorldRef world, const UnitId& unit) const;

        virtual bool RemoveAbility(WorldRef world, const UnitId& unit) const;

        virtual bool HasAbility(WorldConstRef world, const UnitId& unit) const;

        virtual ECS::EntityId ScanForTarget(WorldConstRef world, const AbilityTargetScanArgs& args) const;

    protected:

        std::shared_ptr<FeatureAbilities> AbilitiesFeature;
    };
}
