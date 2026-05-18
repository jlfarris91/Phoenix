
#include "Phoenix.Sim.RTS/Data/DataResponseDamage.h"

using namespace Phoenix::RTS::Data;

bool ResponseDamage::Read(const LDS::LDSReadObjectArgs& args, ResponseDamage& outItem)
{
    bool success = Response::Read(args, outItem);

    const LDS::ILDSQueryContext& lds = args.GetQueryContext();

    ResponseDamagePtr dataPtr = args.CreatePtr<ResponseDamagePtr>();
    success = dataPtr.Amount().TryGetValue(lds, outItem.Amount) && success;
    success = dataPtr.AmountScalar().TryGetValue(lds, outItem.AmountScalar) && success;
    success = dataPtr.Clamp().TryGetValue(lds, outItem.Clamp) && success;
    success = dataPtr.ClampRemainderScalar().TryGetValue(lds, outItem.ClampRemainderScalar) && success;

    return success;
}

ResponseDamagePtr::ResponseDamagePtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : ResponsePtr(path, flags)
{
}

Phoenix::LDS::ValuePtr ResponseDamagePtr::Amount() const
{
    return Value<LDS::ValuePtr>("amount");
}

Phoenix::LDS::ValuePtr ResponseDamagePtr::AmountScalar() const
{
    return Value<LDS::ValuePtr>("amount_scalar");
}

Phoenix::LDS::ValuePtr ResponseDamagePtr::Clamp() const
{
    return Value<LDS::ValuePtr>("clamp");
}

Phoenix::LDS::ValuePtr ResponseDamagePtr::ClampRemainderScalar() const
{
    return Value<LDS::ValuePtr>("clamp_remainder_scalar");
}
