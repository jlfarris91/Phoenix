
#include "Phoenix.Sim.RTS/Orders/Orders.h"

bool Phoenix::RTS::Order::operator==(const Order& other) const
{
    return OrderId == other.OrderId &&
           OrderIndex == other.OrderIndex &&
           TargetEntity == other.TargetEntity &&
           Vec2::Equals(TargetLocation, other.TargetLocation) &&
           Flags == other.Flags;
}

bool Phoenix::RTS::Order::operator!=(const Order& other) const
{
    return !operator==(other);
}