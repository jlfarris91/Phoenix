#include "PhoenixRTS/Cooldown/Cooldown.h"

bool Phoenix::RTS::Cooldown::IsCooldownActive(
    const World& world,
    const ECS::EntityId& entityId,
    const FName& cooldownId,
    const Data::Cooldown& cooldown)
{
    // TODO (jfarris): how should we track cooldowns? Probably via timers?
    return false;
}
