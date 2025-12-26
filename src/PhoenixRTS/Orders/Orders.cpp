
#include "PhoenixRTS/Orders/Orders.h"

bool Phoenix::RTS::Order::operator==(const Order& other) const
{
    return CommandId == other.CommandId &&
           CommandIndex == other.CommandIndex &&
           TargetEntity == other.TargetEntity &&
           Vec2::Equals(TargetLocation, other.TargetLocation) &&
           Flags == other.Flags;
}

bool Phoenix::RTS::Order::operator!=(const Order& other) const
{
    return !operator==(other);
}