
#pragma once

#include "LDSObjectModel.h"

namespace Phoenix::RTS::Data
{
    template <class T>
    struct PHOENIX_RTS_API TExpression
    {
        T Execute() const { return {}; }
    };

    template <class T>
    struct PHOENIX_RTS_API TExpressionPtr : LDS::TLDSObjectPtr<TExpression<T>>
    {
        // PHX_LDS_DECLARE_OBJECT_PTR(TExpressionPtr)
    };
}
