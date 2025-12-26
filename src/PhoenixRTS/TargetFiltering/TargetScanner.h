#pragma once

#include "PhoenixSim/Worlds.h"
#include "PhoenixSim/FixedPoint/FixedVector.h"

#include "PhoenixRTS/DLLExport.h"
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
        UnitCannotMove = 1,
        AutoAcquire = 2,
    };

    struct PHOENIX_RTS_API TargetScanArgs
    {
        TOptional<FName> UnitDataId;
        TOptional<Vec2> Location;
        TOptional<ECS::EntityId> LastScanTarget;
        ETargetScanFlags Flags = ETargetScanFlags::None;
        TSharedPtr<const LDS::ILDSQueryContext> LdsQueryContext;
    };

    struct PHOENIX_RTS_API TargetScanResult
    {
        ECS::EntityId Target = ECS::EntityId::Invalid;
        TOptional<FName> AbilityId;
        TOptional<FName> WeaponId;

        bool IsValid() const;
    };

    struct PHOENIX_RTS_API TargetScanner
    {
        static TargetScanResult ScanForTarget(WorldRef world, UnitId unit, const TargetScanArgs& args);

        static TargetScanResult ScanForAbilityTarget(WorldRef world, UnitId unit, const TargetScanArgs& args);

        static TargetScanResult ScanForWeaponTarget(WorldRef world, UnitId unit, const TargetScanArgs& args);

    private:

        static void PopulateTargetScanArgs(WorldConstRef world, UnitId unit, TargetScanArgs& args);
    };
}
