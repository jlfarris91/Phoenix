#include "PhoenixRTS/Orders/Commands.h"

#include "PhoenixSim/Flags.h"

bool Phoenix::RTS::FromVerb(const FName& verb, ECommandFlags& outFlags)
{
    if (verb == "command"_n)
    {
        outFlags = ECommandFlags::Replace;
        return true;
    }
    if (verb == "command_queued"_n)
    {
        outFlags = ECommandFlags::Queue;
        return true;
    }
    if (verb == "smart_command"_n)
    {
        outFlags = ECommandFlags::Smart;
        SetFlagRef(outFlags, ECommandFlags::Replace);
        return true;
    }
    if (verb == "smart_command_queued"_n)
    {
        outFlags = ECommandFlags::Smart;
        SetFlagRef(outFlags, ECommandFlags::Queue);
        return true;
    }
    outFlags = ECommandFlags::Invalid;
    return false;
}

Phoenix::FName Phoenix::RTS::ToVerb(ECommandFlags flags)
{
    if (HasAnyFlags(flags, ECommandFlags::Smart))
    {
        if (HasAnyFlags(flags, ECommandFlags::Queue))
        {
            return "smart_command_queued"_n;
        }
        return "smart_command"_n;
    }
    if (HasAnyFlags(flags, ECommandFlags::Queue))
    {
        return "command_queued"_n;
    }
    return "command"_n;
}

Phoenix::RTS::Command::Command(const Action& action)
    : Sender(action.Sender)
    , Kind(action.Data[0].UInt32)
    , CommandId(action.Data[1].Name)
    , CommandIndex(action.Data[2].UInt32)
    , TargetEntity(action.Data[3].UInt32)
    , TargetLocation(action.Data[4].Distance, action.Data[5].Distance)
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
    command.Data[0].UInt32 = Kind;
    command.Data[1].Name = CommandId;
    command.Data[2].UInt32 = CommandIndex;
    command.Data[3].UInt32 = TargetEntity;
    command.Data[4].Distance = TargetLocation.X;
    command.Data[5].Distance = TargetLocation.Y;
    return command;
}