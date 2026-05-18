#pragma once

#include "Phoenix.Sim/Reflection/MemberDescriptorBuilder.h"
#include "Phoenix.Sim/Reflection/FieldDescriptor.h"
#include "Phoenix.Sim/Utils.h"

namespace Phoenix
{
    template <class TValue>
    class FieldDescriptorBuilder : public MemberDescriptorBuilder<FieldDescriptorBuilder<TValue>>
    {
        using Super = MemberDescriptorBuilder<FieldDescriptorBuilder>;
    public:

        FieldDescriptorBuilder(FieldDescriptor* descriptor)
            : MemberDescriptorBuilder<FieldDescriptorBuilder>(descriptor)
        {}
    };

    template <IsNumerical TValue>
    class FieldDescriptorBuilder<TValue> : public MemberDescriptorBuilder<FieldDescriptorBuilder<TValue>>
    {
    public:

        FieldDescriptorBuilder(FieldDescriptor* descriptor)
            : MemberDescriptorBuilder<FieldDescriptorBuilder>(descriptor)
        {}

        FieldDescriptorBuilder& MinValue(TValue value)
        {
            this->Metadata("MinValue", std::to_string(value));
            return *this;
        }

        FieldDescriptorBuilder& MaxValue(TValue value)
        {
            this->Metadata("MaxValue", std::to_string(value));
            return *this;
        }

        FieldDescriptorBuilder& Step(TValue value)
        {
            this->Metadata("Step", std::to_string(value));
            return *this;
        }
    };
}
