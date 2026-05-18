
#include "PhoenixRTS/Data/DataSmartCast.h"

using namespace Phoenix::RTS::Data;

bool SmartCast::Read(const LDS::LDSReadObjectArgs& args, SmartCast& outItem)
{
    bool success = true;

    const LDS::ILDSQueryContext& lds = args.GetQueryContext();

    SmartCastPtr dataPtr = args.CreatePtr<SmartCastPtr>();
    success = dataPtr.Priority().TryGetValue(lds, outItem.Priority) && success;
    success = dataPtr.Filter().TryReadObject(lds, outItem.Filter) && success;

    return success;
}

SmartCastPtr::SmartCastPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

Phoenix::LDS::Int32Ptr SmartCastPtr::Priority() const
{
    return Value<LDS::Int32Ptr>("priority");
}

TargetFilterPtr SmartCastPtr::Filter() const
{
    return Object<TargetFilterPtr>("filter");
}
