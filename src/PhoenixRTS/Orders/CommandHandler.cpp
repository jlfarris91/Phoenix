#include "PhoenixRTS/Orders/CommandHandler.h"

#include "PhoenixSim/Session.h"

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

bool Phoenix::RTS::ICommandHandler::IgnoreCommand(
    WorldConstRef world,
    const CommandContext& context,
    const Command& command) const
{
    return false;
}

Phoenix::uint32 Phoenix::RTS::ICommandHandler::GetCommandPriority(
    WorldConstRef world,
    const CommandContext& context,
    const Command& command) const
{
    return false;
}

Phoenix::RTS::AcquireResult Phoenix::RTS::ICommandHandler::AcquireOrder(
    WorldConstRef world,
    const AcquireContext& context,
    const AcquireRequest& request) const
{
    return AcquireResult{};
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

bool Phoenix::RTS::ICommandHandler::SupportsMagicBox(const Order& order) const
{
    return false;
}
