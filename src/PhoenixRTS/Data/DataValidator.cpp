
#include "PhoenixRTS/Data/DataValidator.h"

Phoenix::RTS::Data::ValidatorPtr::ValidatorPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
    , Expression(ObjectRef<TExpression<bool>>("execute"))
    , Error(Value<FName>("error"))
{
}

bool Phoenix::RTS::Data::Validator::Read(const LDS::LDSReadObjectArgs& args, Validator& outItem)
{
    const LDS::ILDSQueryContext& queryContext = args.GetQueryContext();

    bool success = true;

    ValidatorPtr dataPtr = args.CreatePtr<ValidatorPtr>();
    success = dataPtr.Expression.TryResolveObject(queryContext, outItem.Expression) && success;
    success = dataPtr.Error.TryGetValue(queryContext, outItem.Error) && success;

    return success;
}
