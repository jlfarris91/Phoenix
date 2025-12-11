#include "Commands/Commands.h"

bool Phoenix::RTS::FromVerb(const FName& verb, ECommandType& outType)
{
    if (verb == "command"_n)
    {
        outType = ECommandType::Order;
        return true;
    }
    if (verb == "command_queued"_n)
    {
        outType = ECommandType::Queued;
        return true;
    }
    outType = ECommandType::Invalid;
    return false;
}

Phoenix::FName Phoenix::RTS::ToVerb(ECommandType type)
{
    switch (type)
    {
        case ECommandType::Order: return "command"_n;
        case ECommandType::Queued: return "command_queued"_n;
        default: break;
    }
    return FName::None;
}

Phoenix::RTS::Command::Command(const Action& action)
    : Sender(action.Sender)
    , AbilityId(action.Data[0].Name)
    , CommandIndex(action.Data[1].UInt32)
    , TargetEntity(action.Data[2].UInt32)
    , TargetLocation(action.Data[3].Distance, action.Data[4].Distance)
{
    FromVerb(action.Verb, Type);
}

bool Phoenix::RTS::Command::IsValid() const
{
    return Type != ECommandType::Invalid;
}

Phoenix::RTS::Command::operator Phoenix::Action() const
{
    Action command;
    command.Sender = Sender;
    command.Verb = ToVerb(Type);
    command.Data[0].Name = AbilityId;
    command.Data[1].UInt32 = CommandIndex;
    command.Data[2].UInt32 = TargetEntity;
    command.Data[3].Distance = TargetLocation.X;
    command.Data[4].Distance = TargetLocation.Y;
    return command;
}
