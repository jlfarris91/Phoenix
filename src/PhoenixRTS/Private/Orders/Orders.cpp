
#include "Orders/Orders.h"

bool Phoenix::RTS::Order::operator==(const Order& other) const
{
    return AbilityId == other.AbilityId && Flags == other.Flags && Action == other.Action;
}

bool Phoenix::RTS::Order::operator!=(const Order& other) const
{
    return !operator==(other);
}
