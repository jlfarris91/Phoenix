
#include "Abilities/MoveAbility.h"

using namespace Phoenix;
using namespace Phoenix::RTS;


MoveAbility::MoveAbility()
    : AbilityBase("MoveAbility"_n)
{
}

uint32 MoveAbility::HandleOrder(EOrderType type, const Order& order)
{
    

    order.CommandIndex

    
}

uint32 MoveAbility::GetPriority(const Order& order)
{
}

uint32 MoveAbility::Acquire(const Order& order)
{
}

bool MoveAbility::SupportsMagicBox(const Order& order)
{
}
