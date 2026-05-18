
#include "PhoenixRTS/Data/DataValidator.h"


bool Phoenix::RTS::Data::Validator::Read(const LDS::LDSReadObjectArgs& args, Validator& outItem)
{
    const LDS::ILDSQueryContext& queryContext = args.GetQueryContext();

    bool success = true;

    ValidatorPtr dataPtr = args.CreatePtr<ValidatorPtr>();
    success = dataPtr.Expression().TryResolveObject(queryContext, outItem.Expression) && success;
    success = dataPtr.Error().TryGetValue(queryContext, outItem.Error) && success;

    return success;
}

Phoenix::RTS::Data::ValidatorPtr::ValidatorPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

Phoenix::LDS::TLDSObjectRefPtr<Phoenix::RTS::Data::TExpression<bool>> Phoenix::RTS::Data::ValidatorPtr::Expression() const
{
    return ObjectRef<TExpression<bool>>("execute");
}

Phoenix::LDS::TLDSValuePtr<Phoenix::FName> Phoenix::RTS::Data::ValidatorPtr::Error() const
{
    return Value<FName>("error");
}
