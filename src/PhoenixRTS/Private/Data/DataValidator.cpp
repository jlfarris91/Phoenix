
#include "Data/DataValidator.h"

Phoenix::RTS::Data::ValidatorPtr::ValidatorPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
    , Expression(ObjectRef<TExpression<bool>>("execute"))
    , Error(Value<FName>("error"))
{
}

bool Phoenix::RTS::Data::Validator::Read(const LDS::LDSReadObjectContext& context, Validator& outItem)
{
    bool success = true;

    ValidatorPtr dataPtr(context.Path, context.Flags);
    success = dataPtr.Expression.TryResolveObject(context, outItem.Expression) && success;
    success = dataPtr.Error.TryGetValue(context, outItem.Error) && success;

    return success;
}
