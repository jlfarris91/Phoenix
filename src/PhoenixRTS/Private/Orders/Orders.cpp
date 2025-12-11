
#include "Orders/Orders.h"

bool Phoenix::RTS::Order::operator==(const Order& other) const
{
    return AbilityId == other.AbilityId &&
           CommandIndex == other.CommandIndex &&
           Target == other.Target &&
           Vec2::Equals(Location, other.Location) &&
           Flags == other.Flags;
}

bool Phoenix::RTS::Order::operator!=(const Order& other) const
{
    return !operator==(other);
}
