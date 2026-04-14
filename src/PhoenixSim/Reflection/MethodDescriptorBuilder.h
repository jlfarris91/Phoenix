#pragma once

#include "MemberDescriptorBuilder.h"
#include "MethodDescriptor.h"

namespace Phoenix
{
    class MethodDescriptor;

    class PHOENIX_SIM_API MethodDescriptorBuilder : public MemberDescriptorBuilder<MethodDescriptorBuilder>
    {
    public:
        MethodDescriptorBuilder(MethodDescriptor* descriptor)
            : MemberDescriptorBuilder(descriptor)
        {}
    };
}
