#pragma once

#include "PhoenixSim/Reflection/BaseDescriptorBuilder.h"
#include "PhoenixSim/Reflection/EnumValueDescriptor.h"

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
