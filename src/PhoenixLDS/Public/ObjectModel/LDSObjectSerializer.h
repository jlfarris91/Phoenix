
#pragma once

namespace Phoenix::LDS
{
    template <class>
    struct LDSObjectSerializer { };

    template <class T>
    struct LDSObjectSerializer<TLDSObjectPtr<T>>
    {
        static bool Read(const LDSReadObjectContext& context, TLDSObjectPtr<T>& outItem)
        {
            return context.Query->QueryObjectRecordValueAs<FName>(outItem.ObjectId, FName::None);
        }
    };
}