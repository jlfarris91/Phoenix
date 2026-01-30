#pragma once

#include "PhoenixRTS/Units/UnitId.h"
#include "PhoenixSim/Features.h"
#include "PhoenixSim/Containers/FixedWeightedSet.h"

struct SpawnWave
{
    Phoenix::TInlineWeightedSet<Phoenix::FName, 6> UnitDataIds;
};

struct FeatureSpawnerWorldBlock : Phoenix::BufferBlockBase
{
    PHX_DECLARE_BLOCK_BEGIN(FeatureSpawnerWorldBlock)
        PHX_REGISTER_FIELD(Phoenix::Time, SpawnCooldownMin)
        PHX_REGISTER_FIELD(Phoenix::Time, SpawnCooldownMax)
        PHX_REGISTER_FIELD(Phoenix::Time, NextSpawnTime)
        PHX_REGISTER_FIELD(Phoenix::Time, WaveDuration)
        PHX_REGISTER_FIELD(Phoenix::Time, NextWaveTime)
        PHX_REGISTER_FIELD(Phoenix::uint32, WaveNum)
        PHX_REGISTER_FIELD(bool, SpawningEnabled)
    PHX_DECLARE_BLOCK_END()

    Phoenix::Time SpawnCooldownMin;
    Phoenix::Time SpawnCooldownMax;
    Phoenix::Time NextSpawnTime;

    Phoenix::Time WaveDuration;
    Phoenix::Time NextWaveTime;

    Phoenix::uint32 WaveNum = 0;

    Phoenix::TInlineArray<SpawnWave, 30> SpawnWaves;
    bool SpawningEnabled = false;

    Phoenix::Random Random;
};

class FeatureSpawner : public Phoenix::IFeature
{
    PHX_DECLARE_FEATURE_TYPE_BEGIN(FeatureSpawner)
        PHX_REGISTER_STATIC_PROPERTY(bool, IsEnabled)
    PHX_DECLARE_FEATURE_TYPE_END()

public:

    FeatureSpawner();

    static bool GetIsEnabled(Phoenix::WorldConstRef world);
    static void SetIsEnabled(Phoenix::WorldRef world, const bool& enabled);

private:
    void OnWorldInitialize(Phoenix::WorldRef world) override;
    void OnWorldUpdate(Phoenix::WorldRef world, const Phoenix::FeatureUpdateArgs& args) override;

    static void Reset(Phoenix::WorldRef world);

    static bool SpawnUnit(Phoenix::WorldRef world, FeatureSpawnerWorldBlock& block);

    static Phoenix::RTS::UnitId SpawnTowerForPlayer(Phoenix::WorldRef world, uint8_t player);
};