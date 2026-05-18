#include "Delegates.h"

#include <atomic>

using namespace Phoenix;

std::atomic<uint64_t> gHandleIdGen = 0;

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