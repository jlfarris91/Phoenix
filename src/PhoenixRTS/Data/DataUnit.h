
#pragma once

#include "PhoenixSim/LDS/LDSObjectModel.h"

#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Data/DataBuff.h"
#include "PhoenixRTS/Data/DataCommand.h"
#include "PhoenixRTS/Data/DataComponent.h"
#include "PhoenixRTS/Data/DataFaction.h"
#include "PhoenixRTS/Data/DataFogVisibility.h"
#include "PhoenixRTS/Data/DataTag.h"
#include "PhoenixRTS/Data/DataUnitActor.h"
#include "PhoenixRTS/Data/DataUnitArmor.h"
#include "PhoenixRTS/Data/DataUnitBuild.h"
#include "PhoenixRTS/Data/DataUnitCargo.h"
#include "PhoenixRTS/Data/DataUnitDeath.h"
#include "PhoenixRTS/Data/DataUnitEffects.h"
#include "PhoenixRTS/Data/DataUnitFlags.h"
#include "PhoenixRTS/Data/DataUnitInfo.h"
#include "PhoenixRTS/Data/DataUnitMovement.h"
#include "PhoenixRTS/Data/DataUnitPlacement.h"
#include "PhoenixRTS/Data/DataUnitSupply.h"
#include "PhoenixRTS/Data/DataUnitVision.h"
#include "PhoenixRTS/Data/DataVitalStatsPair.h"
#include "PhoenixRTS/Data/DataWeapon.h"

namespace Phoenix::RTS::Data
{
    enum class ECollisionFlags : uint8
    {
        None = 0,
        Ground = 1,
        Air = 2
    };

    struct PHOENIX_RTS_API Unit
    {
        UnitActorPtr Actor;
        UnitArmor Armor;
        TArray2<BuffPtr> Buffs;
        UnitBuild BuildStats;
        UnitCargo CargoStats;
        ECollisionFlags CollisionFlags;
        TArray2<Command> Commands;
        TArray2<Component> Components;
        UnitDeath DeathStats;
        UnitEffects Effects;
        FactionPtr Faction;
        UnitFlags Flags;
        FogVisibility Fog;
        UnitInfo Info;
        UnitMovement Movement;
        UnitPlacement Placement;
        TArray2<TagPtr> Tags;
        UnitSupply Supply;
        UnitVision Vision;
        TArray2<VitalStatsPair> Vitals;
        TArray2<WeaponPtr> Weapons;

        static bool Read(const LDS::LDSReadObjectArgs& args, Unit& outItem);
    };

    struct PHOENIX_RTS_API UnitPtr : LDS::TLDSObjectPtr<Unit>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Unit)

        LDS::TLDSObjectRefPtr<UnitActorPtr> Actor() const;
        UnitArmorPtr Armor() const;
        LDS::TLDSObjectRefArrayPtr<BuffPtr> Buffs() const;
        UnitBuildPtr BuildStats() const;
        UnitCargoPtr CargoStats() const;
        LDS::TLDSEnumFlagsPtr<ECollisionFlags> CollisionFlags() const;
        LDS::TLDSObjectArrayPtr<Command> Commands() const;
        LDS::TLDSObjectArrayPtr<Component> Components() const;
        UnitDeathPtr DeathStats() const;
        UnitEffectsPtr Effects() const;
        LDS::TLDSObjectRefPtr<FactionPtr> Faction() const;
        UnitFlagsPtr Flags() const;
        FogVisibilityPtr Fog() const;
        UnitInfoPtr Info() const;
        UnitMovementPtr Movement() const;
        UnitPlacementPtr Placement() const;
        LDS::TLDSObjectRefArrayPtr<TagPtr> Tags() const;
        UnitSupplyPtr Supply() const;
        UnitVisionPtr Vision() const;
        LDS::TLDSObjectArrayPtr<VitalStatsPair> Vitals() const;
        LDS::TLDSObjectRefArrayPtr<WeaponPtr> Weapons() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(Unit)
}
