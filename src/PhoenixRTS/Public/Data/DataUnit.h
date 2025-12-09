
#pragma once

#include "DLLExport.h"
#include "LDSObjectModel.h"

#include "DataBuff.h"
#include "DataCommand.h"
#include "DataComponent.h"
#include "DataFaction.h"
#include "DataFogVisibility.h"
#include "DataTag.h"
#include "DataUnitActor.h"
#include "DataUnitArmor.h"
#include "DataUnitBuild.h"
#include "DataUnitCargo.h"
#include "DataUnitDeath.h"
#include "DataUnitEffects.h"
#include "DataUnitFlags.h"
#include "DataUnitInfo.h"
#include "DataUnitPlacement.h"
#include "DataUnitSupply.h"
#include "DataUnitVision.h"
#include "DataUnitVitals.h"
#include "DataWeapon.h"

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
        UnitPlacement Placement;
        TArray2<TagPtr> Tags;
        UnitSupply Supply;
        UnitVision Vision;
        UnitVitals Vitals;
        TArray2<WeaponPtr> Weapons;

        static bool Read(const LDS::LDSReadObjectArgs& args, Unit& outItem);
    };

    struct PHOENIX_RTS_API UnitPtr : LDS::TLDSObjectPtr<Unit>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Unit)

        LDS::TLDSObjectRefPtr<UnitActorPtr> Actor;
        UnitArmorPtr Armor;
        LDS::TLDSObjectRefArrayPtr<BuffPtr> Buffs;
        UnitBuildPtr BuildStats;
        UnitCargoPtr CargoStats;
        LDS::TLDSEnumFlagsPtr<ECollisionFlags> CollisionFlags;
        LDS::TLDSObjectArrayPtr<Command> Commands;
        LDS::TLDSObjectArrayPtr<Component> Components;
        UnitDeathPtr DeathStats;
        UnitEffectsPtr Effects;
        LDS::TLDSObjectRefPtr<FactionPtr> Faction;
        UnitFlagsPtr Flags;
        FogVisibilityPtr Fog;
        UnitInfoPtr Info;
        UnitPlacementPtr Placement;
        LDS::TLDSObjectRefArrayPtr<TagPtr> Tags;
        UnitSupplyPtr Supply;
        UnitVisionPtr Vision;
        UnitVitalsPtr Vitals;
        LDS::TLDSObjectRefArrayPtr<WeaponPtr> Weapons;
    };
}
