#pragma once

#include "Phoenix.Sim/Reflection/BaseDescriptor.h"
#include "Phoenix.Sim/Reflection/Variant.h"

namespace Phoenix
{
    class PHOENIX_SIM_API EnumValueDescriptor : public BaseDescriptor
    {
    public:

        EnumValueDescriptor(uint32 index, const Variant& value)
            : Index(index)
            , Value(value)
        {
        }

        uint32 GetIndex() const { return Index; }
        const Variant& GetValue() const { return Value; }

    private:

        uint32 Index;
        Variant Value;
    };
}
