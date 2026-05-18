#pragma once

#include "Phoenix.Sim.LDS/LDSObjectModel.h"

#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim.RTS/Data/DataBuff.h"
#include "Phoenix.Sim.RTS/Data/DataCommand.h"
#include "Phoenix.Sim.RTS/Data/DataComponent.h"
#include "Phoenix.Sim.RTS/Data/DataFaction.h"
#include "Phoenix.Sim.RTS/Data/DataFogVisibility.h"
#include "Phoenix.Sim.RTS/Data/DataTag.h"
#include "Phoenix.Sim.RTS/Data/DataUnitActor.h"
#include "Phoenix.Sim.RTS/Data/DataUnitArmor.h"
#include "Phoenix.Sim.RTS/Data/DataUnitBuild.h"
#include "Phoenix.Sim.RTS/Data/DataUnitCargo.h"
#include "Phoenix.Sim.RTS/Data/DataUnitDeath.h"
#include "Phoenix.Sim.RTS/Data/DataUnitEffects.h"
#include "Phoenix.Sim.RTS/Data/DataUnitFlags.h"
#include "Phoenix.Sim.RTS/Data/DataUnitInfo.h"
#include "Phoenix.Sim.RTS/Data/DataUnitMovement.h"
#include "Phoenix.Sim.RTS/Data/DataUnitPlacement.h"
#include "Phoenix.Sim.RTS/Data/DataUnitSupply.h"
#include "Phoenix.Sim.RTS/Data/DataUnitVision.h"
#include "Phoenix.Sim.RTS/Data/DataVitalStatsPair.h"
#include "Phoenix.Sim.RTS/Data/DataWeapon.h"

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
        std::vector<BuffPtr> Buffs;
        UnitBuild BuildStats;
        UnitCargo CargoStats;
        ECollisionFlags CollisionFlags;
        std::vector<Command> Commands;
        std::vector<Component> Components;
        UnitDeath DeathStats;
        UnitEffects Effects;
        FactionPtr Faction;
        UnitFlags Flags;
        FogVisibility Fog;
        UnitInfo Info;
        UnitMovement Movement;
        UnitPlacement Placement;
        std::vector<TagPtr> Tags;
        Value SelectionCircleScale = 1.0;
        UnitSupply Supply;
        UnitVision Vision;
        std::vector<VitalStatsPair> Vitals;
        std::vector<WeaponPtr> Weapons;

        static bool Read(const LDS::LDSReadObjectArgs& args, Unit& outItem);
    };

    struct PHOENIX_RTS_API UnitPtr : LDS::TLDSObjectPtr<Unit>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Unit)

        UnitActorRefPtr Actor() const;
        UnitArmorPtr Armor() const;
        BuffRefArrayPtr Buffs() const;
        UnitBuildPtr BuildStats() const;
        UnitCargoPtr CargoStats() const;
        LDS::TLDSEnumFlagsPtr<ECollisionFlags> CollisionFlags() const;
        CommandArrayPtr Commands() const;
        ComponentArrayPtr Components() const;
        UnitDeathPtr DeathStats() const;
        UnitEffectsPtr Effects() const;
        FactionRefPtr Faction() const;
        UnitFlagsPtr Flags() const;
        FogVisibilityPtr Fog() const;
        UnitInfoPtr Info() const;
        UnitMovementPtr Movement() const;
        UnitPlacementPtr Placement() const;
        TagRefArrayPtr Tags() const;
        LDS::ValuePtr SelectionCircleScale() const;
        UnitSupplyPtr Supply() const;
        UnitVisionPtr Vision() const;
        VitalStatsPairArrayPtr Vitals() const;
        WeaponRefArrayPtr Weapons() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(Unit)
}
