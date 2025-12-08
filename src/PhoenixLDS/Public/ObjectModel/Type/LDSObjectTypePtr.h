
#pragma once

#include "ObjectModel/LDSRecordPtr.h"

namespace Phoenix::LDS
{
    struct LDSObjectTypePtr : LDSRecordPtr
    {
        LDSObjectTypePtr() = default;
        LDSObjectTypePtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSObjectTypePtr(const LDSRecordPtr& other);

        

    private:
        void InitCommon();
    };
}
