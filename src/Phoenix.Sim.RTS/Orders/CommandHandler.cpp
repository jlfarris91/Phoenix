#include "Phoenix.Sim.RTS/Orders/CommandHandler.h"

#include "Phoenix.Sim/Session.h"

#include "Phoenix.Sim.RTS/Orders/FeatureOrders.h"

Phoenix::FName Phoenix::RTS::ICommandHandler::GetCommandId() const
{
    return FName::None;
}

void Phoenix::RTS::ICommandHandler::Initialize(const std::shared_ptr<Phoenix::Session>& session)
{
    ISessionService::Initialize(session);
    OrdersFeature = session->GetFeature<FeatureOrders>();
}

void Phoenix::RTS::ICommandHandler::Shutdown()
{
    OrdersFeature.reset();
    ISessionService::Shutdown();
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
