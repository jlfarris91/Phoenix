#pragma once

#include "Phoenix.Sim/Reflection/BaseDescriptorBuilder.h"
#include "Phoenix.Sim/Reflection/EnumValueDescriptor.h"

namespace Phoenix
{
    // Leaving for future use.
    template <class TValue>
    class EnumValueDescriptorBuilder : public BaseDescriptorBuilder<EnumValueDescriptorBuilder<TValue>>
    {
    public:
        EnumValueDescriptorBuilder(EnumValueDescriptor* descriptor)
            : BaseDescriptorBuilder<EnumValueDescriptorBuilder>(descriptor)
        {
        }
    };
}
