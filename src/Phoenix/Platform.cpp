#include "Phoenix/Platform.h"

using namespace Phoenix;

thread_local uint32 gCurrentThreadIndex = 0;

uint32 Phoenix::GetCurrentThreadIndex()
{
    return gCurrentThreadIndex;
}

void Phoenix::SetCurrentThreadIndex(uint32 idx)
{
    gCurrentThreadIndex = idx;
}