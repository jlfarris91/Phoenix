
#include "ObjectModel/LDSObjectRefPtr.h"

#include "LDSRecordStore.h"
#include "ObjectModel/LDSObjectPtr.h"
#include "ObjectModel/LDSQueryContext.h"

using namespace Phoenix::LDS;

LDSObjectRefPtr::LDSObjectRefPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags)
    : LDSRecordPtr(path, flags)
{
}

LDSObjectRefPtr::LDSObjectRefPtr(const LDSRecordPtr& other)
    : LDSRecordPtr(other)
{
}

LDSObjectPtr LDSObjectRefPtr::ResolveObject(const ILDSQueryContext& context) const
{
    LDSObjectPtr result;
    (void)TryResolveObject(context, result);
    return result;
}

bool LDSObjectRefPtr::TryResolveObject(const ILDSQueryContext& context, LDSObjectPtr& outObjectPtr) const
{
    if (const LDSRecord* record = context.QueryObjectRecord(Path, Flags))
    {
        FName objectId = record->GetValueAs<FName>();
        outObjectPtr = LDSObjectPtr(LDSRecordPath(objectId), Flags);
        return true;
    }
    return false;
}