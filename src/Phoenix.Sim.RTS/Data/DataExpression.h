
#pragma once

#include "Phoenix.Sim.LDS/LDSObjectModel.h"

namespace Phoenix::RTS::Data
{
    template <class T>
    struct TExpression
    {
        T Execute() const { return {}; }
    };

    template <class T>
    struct TExpressionPtr : LDS::TLDSObjectPtr<TExpression<T>>
    {
        // PHX_LDS_DECLARE_OBJECT_PTR(TExpressionPtr)
    };
}
