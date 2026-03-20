#include "FeatureSpawner.h"

#include "PhoenixRTS/Abilities/Attack/AttackAbilityHandler.h"
#include "PhoenixRTS/Orders/FeatureOrders.h"
#include "PhoenixRTS/Units/FeatureUnit.h"
#include "PhoenixSim/Random.h"

using namespace Phoenix;
using namespace Phoenix::RTS;

FeatureSpawner::FeatureSpawner()
{
    FEATURE_WORLD_BLOCK(FeatureSpawnerWorldBlock, EBufferBlockType::Dynamic)
    FEATURE_CHANNEL(FeatureChannels::WorldInitialize)
    FEATURE_CHANNEL(FeatureChannels::WorldUpdate)
}

bool FeatureSpawner::GetIsEnabled(WorldConstRef world)
{
    const FeatureSpawnerWorldBlock& block = world.GetBlockRef<FeatureSpawnerWorldBlock>();
    return block.SpawningEnabled;
}

void FeatureSpawner::SetIsEnabled(WorldRef world, const bool& enabled)
{
    FeatureSpawnerWorldBlock& block = world.GetBlockRef<FeatureSpawnerWorldBlock>();

    if (block.SpawningEnabled == enabled)
    {
        return;
    }

    block.SpawningEnabled = enabled;

    if (enabled)
    {
        block.NextSpawnTime = world.GetSimTime() + block.Random.RandomRange(block.SpawnCooldownMin, block.SpawnCooldownMax);
        block.NextWaveTime = world.GetSimTime() + block.WaveDuration;
    }
}

void FeatureSpawner::OnWorldInitialize(WorldRef world)
{
    IFeature::OnWorldInitialize(world);

    //Reset(world);

    SpawnTowerForPlayer(world, 0);
}

void FeatureSpawner::OnWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    IFeature::OnWorldUpdate(world, args);

    FeatureSpawnerWorldBlock& block = world.GetBlockRef<FeatureSpawnerWorldBlock>();
    if (block.SpawningEnabled)
    {
        Random& random = block.Random;

        while (world.GetSimTime() >= block.NextSpawnTime)
        {
            block.NextSpawnTime = world.GetSimTime() + random.RandomRange(block.SpawnCooldownMin, block.SpawnCooldownMax);

            SpawnUnit(world, block);
        }

        if (world.GetSimTime() >= block.NextWaveTime)
        {
            block.NextWaveTime += block.WaveDuration;
            ++block.WaveNum;
        }
    }
}

bool FeatureSpawner::OnHandleWorldAction(WorldRef world, const FeatureActionArgs& args)
{
    if (args.Action.Verb == "enable_spawning"_n)
    {
        SetIsEnabled(world, args.Action.Args[0].AsBool);
        return true;
    }

    return false;
}

void FeatureSpawner::Reset(WorldRef world)
{
    FeatureSpawnerWorldBlock& block = world.GetBlockRef<FeatureSpawnerWorldBlock>();

    while (!block.SpawnWaves.IsFull())
    {
        SpawnWave& spawnWave = block.SpawnWaves.PushBack_GetRef();
        spawnWave.UnitDataIds.EmplaceBack("Lancer"_n, 1.0);
        spawnWave.UnitDataIds.EmplaceBack("Archer"_n, 1.0);
    }

    block.SpawnCooldownMin = 1.0;
    block.SpawnCooldownMax = 3.0;
    block.WaveDuration = 30.0;
    block.WaveNum = 0;

    // This could alternatively be seeded using a value provided by the world feature config system
    block.Random = world.GetRandom();

    block.SpawningEnabled = true;
}

bool FeatureSpawner::SpawnUnit(WorldRef world, FeatureSpawnerWorldBlock& block)
{
    SpawnWave& spawnWave = block.SpawnWaves[block.WaveNum % block.SpawnWaves.GetNum()];

    Random& random = block.Random;

    FName unitDataId;
    if (!spawnWave.UnitDataIds.GetRandom(random, unitDataId))
    {
        return false;
    }

    Vec2 pos = random.RandomPointOnCircle<Distance>(10.0);
    Angle facing = -pos.AsDegrees();

    uint8 owner = world.GetRandom().RandomRange<uint8>(1, 10);

    UnitId unit = FeatureUnit::SpawnUnit(world, unitDataId, owner, pos, facing);
    if (unit == UnitId::Invalid)
    {
        return false;
    }

    Command command;
    command.Sender = 1;
    command.CommandId = AttackAbilityHandler::StaticGetCommandId();
    command.CommandIndex = AttackAbilityHandler::Commands::Attack;
    command.TargetLocation = Vec2::Zero;

    return FeatureOrders::StaticIssueCommand(world, unit, command);
}

UnitId FeatureSpawner::SpawnTowerForPlayer(WorldRef world, uint8_t player)
{
    uint32 maxPlayers = 9;

    // TODO (jfarris): ideally we pull this location from some map data?
    Vec2 towerPos = Vec2::One * 10.0f * player;
    towerPos.X = Wrap<Distance>(towerPos.X, 0.0f, 10.0f * Sqrt(Value(maxPlayers)));

    return FeatureUnit::SpawnUnit(world, "Tower"_n, player, towerPos, 0);
}
