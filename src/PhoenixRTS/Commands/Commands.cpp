#include "PhoenixRTS/Commands/Commands.h"

#include "PhoenixSim/Flags.h"

bool Phoenix::RTS::FromVerb(const FName& verb, ECommandFlags& outType)
{
    if (verb == "command"_n)
    {
        outType = ECommandFlags::Replace;
        return true;
    }
    if (verb == "command_queued"_n)
    {
        outType = ECommandFlags::Queued;
        return true;
    }
    if (verb == "smart_command"_n)
    {
        SetFlagRef(outType, ECommandFlags::Smart);
        SetFlagRef(outType, ECommandFlags::Replace);
        return true;
    }
    if (verb == "smart_command_queued"_n)
    {
        SetFlagRef(outType, ECommandFlags::Smart);
        SetFlagRef(outType, ECommandFlags::Queued);
        return true;
    }
    outType = ECommandFlags::Invalid;
    return false;
}

Phoenix::FName Phoenix::RTS::ToVerb(ECommandFlags flags)
{
    if (HasAnyFlags(flags, ECommandFlags::Smart))
    {
        if (HasAnyFlags(flags, ECommandFlags::Queued))
        {
            return "smart_command_queued"_n;
        }
        return "smart_command"_n;
    }
    if (HasAnyFlags(flags, ECommandFlags::Queued))
    {
        return "command_queued"_n;
    }
    return "command"_n;
}

Phoenix::RTS::Command::Command(const Action& action)
    : Sender(action.Sender)
    , AbilityId(action.Data[0].Name)
    , CommandIndex(action.Data[1].UInt32)
    , TargetEntity(action.Data[2].UInt32)
    , TargetLocation(action.Data[3].Distance, action.Data[4].Distance)
{
    FromVerb(action.Verb, Flags);
}

bool Phoenix::RTS::Command::IsValid() const
{
    return Flags != ECommandFlags::Invalid;
}

Phoenix::RTS::Command::operator Phoenix::Action() const
{
    Action command;
    command.Sender = Sender;
    command.Verb = ToVerb(Flags);
    command.Data[0].Name = AbilityId;
    command.Data[1].UInt32 = CommandIndex;
    command.Data[2].UInt32 = TargetEntity;
    command.Data[3].Distance = TargetLocation.X;
    command.Data[4].Distance = TargetLocation.Y;
    return command;
}
