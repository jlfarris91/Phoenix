#pragma once

#include "LDSRecordStore.h"

namespace Phoenix::LDS
{
    struct LDSObjectQueryScope
    {
        FName ObjectId;
        TArray2<LDSObjectRun> Runs;
    };
}
