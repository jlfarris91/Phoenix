#pragma once

#include "PhoenixSim/Containers/Array.h"
#include "PhoenixSim/LDS/LDSRecordStore.h"

namespace Phoenix::LDS
{
    struct PHOENIX_SIM_API LDSObjectQueryScope
    {
        FName ObjectId;
        TArray2<LDSObjectRun> Runs;
    };
}
