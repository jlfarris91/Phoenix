#include "Actions.h"

using namespace Phoenix;

bool Phoenix::operator==(const Data& a, const Data& b)
{
    return a.UInt32 == b.UInt32;
}

bool Action::operator==(const Action& other) const
{
    if (Verb != other.Verb && Sender != other.Sender)
    {
        return false;
    }
    for (uint32 i = 0; i < _countof(Data); ++i)
    {
        if (Data[i] != other.Data[i])
            return false;
    }
    return true;
}

bool Action::operator!=(const Action& other) const
{
    return !operator==(other);
}
