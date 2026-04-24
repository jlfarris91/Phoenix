#include "Delegates.h"

using namespace Phoenix;

std::atomic<uint64> gHandleIdGen = 0;

bool DelegateHandle::operator==(const DelegateHandle& other) const
{
    return ID == other.ID;
}

bool DelegateHandle::operator!=(const DelegateHandle& other) const
{
    return ID != other.ID;
}

DelegateHandle DelegateHandle::GetNextHandle()
{
    return DelegateHandle{++gHandleIdGen};
}