#include "Teams.h"

Phoenix::RTS::TeamMask Phoenix::RTS::Teams::AlliesOf(WorldConstRef world, uint8 team)
{
    return All;
}

Phoenix::RTS::TeamMask Phoenix::RTS::Teams::EnemiesOf(WorldConstRef world, uint8 team)
{
    return All ^ (1ull << static_cast<uint64_t>(team));
}
