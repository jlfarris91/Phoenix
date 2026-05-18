
#pragma once

#include "Phoenix.Sim/LDS/ObjectModel/LDSObjectPtr.h"

namespace Phoenix::LDS
{
    template <class>
    struct LDSObjectSerializer { };

    template <class T>
    struct LDSObjectSerializer<TLDSObjectPtr<T>>
    {
        static bool Read(const LDSReadObjectArgs& context, TLDSObjectPtr<T>& outItem)
        {
            return context.GetQueryContext().QueryRecordValueAs<FName>(outItem.ObjectId, FName::None);
        }
    };
}
