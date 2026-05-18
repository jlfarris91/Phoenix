#include "PhoenixSim/Actions.h"

using namespace Phoenix;

bool Phoenix::operator==(const Data& a, const Data& b)
{
    return a.AsUInt32 == b.AsUInt32;
}

bool Action::operator==(const Action& other) const
{
    if (Verb != other.Verb && Sender != other.Sender)
    {
        return false;
    }
    for (uint32 i = 0; i < _countof(Args); ++i)
    {
        if (Args[i] != other.Args[i])
            return false;
    }
    return true;
}

bool Action::operator!=(const Action& other) const
{
    return !operator==(other);
}
