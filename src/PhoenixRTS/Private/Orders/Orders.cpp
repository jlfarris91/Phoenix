
#include "Orders/Orders.h"

Phoenix::RTS::Order::Order(const FName& abilityId, const Action& action, uint32 flags)
    : AbilityId(abilityId)
    , 
{
}

bool Phoenix::RTS::Order::operator==(const Order& other) const
{
    return AbilityId == other.AbilityId && Flags == other.Flags && Action == other.Action;
}

bool Phoenix::RTS::Order::operator!=(const Order& other) const
{
    return !operator==(other);
}
