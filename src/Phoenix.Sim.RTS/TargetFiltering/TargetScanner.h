#pragma once

#include "Phoenix/Containers/Optional.h"
#include "Phoenix/FixedPoint/FixedVector.h"
#include "Phoenix/Name.h"
#include "Phoenix.Sim/WorldsFwd.h"

#include "Phoenix.Sim.RTS/Orders/Commands.h"
#include "Phoenix.Sim.RTS/TargetFiltering/TargetScanLevel.h"
#include "Phoenix.Sim.RTS/Units/UnitId.h"

namespace Phoenix::LDS
{
    class ILDSQueryContext;
}

namespace Phoenix::RTS
{
    enum class PHOENIX_RTS_API ETargetScanFlags : uint8
    {
        None,
        UnitCannotMove = 1
    };

    struct PHOENIX_RTS_API TargetScanArgs
    {
        TOptional<FName> UnitDataId;
        TOptional<Vec2> Location;
        TOptional<ECS::EntityId> LastScanTarget;
        ETargetScanFlags Flags = ETargetScanFlags::None;
        ETargetScanLevel Level = ETargetScanLevel::None;
        const LDS::ILDSQueryContext* LdsQueryContext = nullptr;
    };

    struct PHOENIX_RTS_API TargetScanResult
    {
        ECS::EntityId Target = ECS::EntityId::Invalid;
        Vec2 TargetLocation;
        TOptional<FName> AbilityId;
        TOptional<FName> WeaponId;
        TOptional<AcquireRequest> AcquireRequest;

        bool IsValid() const;
    };

    struct PHOENIX_RTS_API TargetScanner
    {
        static TargetScanResult ScanForTarget(WorldConstRef world, UnitId unit, const TargetScanArgs& args);

        static TargetScanResult ScanForAbilityTarget(WorldConstRef world, UnitId unit, const TargetScanArgs& args);

        static TargetScanResult ScanForWeaponTarget(WorldConstRef world, UnitId unit, const TargetScanArgs& args);

    private:

        static void PopulateTargetScanArgs(WorldConstRef world, UnitId unit, TargetScanArgs& args);

        static TargetScanResult ScanForTargetInternal(WorldConstRef world, UnitId unit, const TargetScanArgs& args);

        static TargetScanResult ScanForAbilityTargetInternal(WorldConstRef world, UnitId unit, const TargetScanArgs& args);

        static TargetScanResult ScanForWeaponTargetInternal(WorldConstRef world, UnitId unit, const TargetScanArgs& args);
    };
}
