
#include "PhoenixRTS/Data/DataTagBonus.h"

using namespace Phoenix::RTS::Data;

bool TagBonus::Read(const LDS::LDSReadObjectArgs& args, TagBonus& outItem)
{
    const LDS::ILDSQueryContext& queryContext = args.GetQueryContext();

    bool success = true;

    TagBonusPtr dataPtr = args.CreatePtr<TagBonusPtr>();
    success = dataPtr.Amount().TryGetValue(queryContext, outItem.Amount) && success;
    success = dataPtr.Flags().TryGetValue(queryContext, outItem.Flags) && success;
    success = dataPtr.Tag().TryResolveObject(queryContext, outItem.Tag) && success;

    return success;
}

TagBonusPtr::TagBonusPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

Phoenix::LDS::ValuePtr TagBonusPtr::Amount() const
{
    return Value<LDS::ValuePtr>("amount");
}

Phoenix::LDS::TLDSEnumFlagsPtr<ETagBonusFlags> TagBonusPtr::Flags() const
{
    return EnumFlags<ETagBonusFlags>("flags");
}

TagRefPtr TagBonusPtr::Tag() const
{
    return ObjectRef<TagRefPtr>("tag");
}
