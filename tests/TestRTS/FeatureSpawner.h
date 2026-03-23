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
    PHX_DECLARE_TYPE(FeatureSpawnerWorldBlock)

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
    PHX_DECLARE_FEATURE_TYPE(FeatureSpawner)

public:

    FeatureSpawner();

    static bool GetIsEnabled(Phoenix::WorldConstRef world);
    static void SetIsEnabled(Phoenix::WorldRef world, const bool& enabled);

private:
    void OnWorldInitialize(Phoenix::WorldRef world) override;
    void OnWorldUpdate(Phoenix::WorldRef world, const Phoenix::FeatureUpdateArgs& args) override;

    bool OnHandleWorldAction(Phoenix::WorldRef world, const Phoenix::FeatureActionArgs& args) override;

    static void Reset(Phoenix::WorldRef world);

    static bool SpawnUnit(Phoenix::WorldRef world, FeatureSpawnerWorldBlock& block);

    static Phoenix::RTS::UnitId SpawnTowerForPlayer(Phoenix::WorldRef world, uint8_t player);
};