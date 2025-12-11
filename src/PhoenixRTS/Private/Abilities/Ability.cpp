
#include "Abilities/Ability.h"

using namespace Phoenix;
using namespace Phoenix::RTS;

AbilityBase::AbilityBase(const FName& abilityId)
    : AbilityId(abilityId)
{
}

FName AbilityBase::GetAbilityId() const
{
    return AbilityId;
}