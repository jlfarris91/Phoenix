#include "PhoenixRTS/Orders/CommandHandler.h"

#include "PhoenixRTS/Orders/FeatureOrders.h"

Phoenix::FName Phoenix::RTS::ICommandHandler::GetCommandId() const
{
    return FName::None;
}

void Phoenix::RTS::ICommandHandler::Initialize(const TSharedPtr<Phoenix::Session>& session)
{
    IService::Initialize(session);
    OrdersFeature = session->GetFeature<FeatureOrders>();
}

void Phoenix::RTS::ICommandHandler::Shutdown()
{
    IService::Shutdown();
    OrdersFeature.reset();
}

void Phoenix::RTS::ICommandHandler::OnWorldInitialize(WorldRef world)
{
}

void Phoenix::RTS::ICommandHandler::OnWorldShutdown(WorldRef world)
{
}

bool Phoenix::RTS::ICommandHandler::IgnoreCommand(
    WorldConstRef world,
    const CommandContext& context,
    const Command& command) const
{
    return true;
}

Phoenix::uint32 Phoenix::RTS::ICommandHandler::GetCommandPriority(
    WorldConstRef world,
    const CommandContext& context,
    const Command& command) const
{
    return false;
}

bool Phoenix::RTS::ICommandHandler::IsTransient(WorldConstRef world, const Order& order) const
{
    return false;
}

bool Phoenix::RTS::ICommandHandler::ExecuteOrder(WorldRef world, const UnitId& unit, const Order& order) const
{
    return false;
}

bool Phoenix::RTS::ICommandHandler::InterruptOrder(WorldRef world, const UnitId& unit, const Order& order) const
{
    return false;
}

Phoenix::uint32 Phoenix::RTS::ICommandHandler::AcquireOrder(
    WorldRef world,
    const UnitId& unit,
    const Order& order) const
{
    return 0;
}

bool Phoenix::RTS::ICommandHandler::SupportsMagicBox(const Order& order) const
{
    return false;
}
