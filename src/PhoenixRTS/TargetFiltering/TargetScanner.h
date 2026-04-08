#pragma once

#include "PhoenixSim/Containers/Optional.h"
#include "PhoenixSim/FixedPoint/FixedVector.h"
#include "PhoenixSim/Name.h"
#include "PhoenixSim/WorldsFwd.h"

#include "PhoenixRTS/Orders/Commands.h"
#include "PhoenixRTS/TargetFiltering/TargetScanLevel.h"
#include "PhoenixRTS/Units/UnitId.h"

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
        std::shared_ptr<const LDS::ILDSQueryContext> LdsQueryContext;
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
