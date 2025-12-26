#include "Teams.h"

Phoenix::RTS::TeamMask Phoenix::RTS::Teams::AlliesOf(WorldConstRef world, uint8 team)
{
    return Teams::All;
}

Phoenix::RTS::TeamMask Phoenix::RTS::Teams::EnemiesOf(WorldConstRef world, uint8 team)
{
    return Teams::All;
}
