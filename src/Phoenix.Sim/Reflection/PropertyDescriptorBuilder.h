#pragma once

#include "Phoenix.Sim/Reflection/MemberDescriptorBuilder.h"
#include "Phoenix.Sim/Reflection/PropertyDescriptor.h"
#include "Phoenix.Sim/Utils.h"

namespace Phoenix
{
    template <class TValue>
    class PropertyDescriptorBuilder : public MemberDescriptorBuilder<PropertyDescriptorBuilder<TValue>>
    {
        using Super = MemberDescriptorBuilder<PropertyDescriptorBuilder>;
    public:

        PropertyDescriptorBuilder(PropertyDescriptor* descriptor)
            : MemberDescriptorBuilder<PropertyDescriptorBuilder>(descriptor)
        {}
    };

    template <IsNumerical TValue>
    class PropertyDescriptorBuilder<TValue> : public MemberDescriptorBuilder<PropertyDescriptorBuilder<TValue>>
    {
    public:

        PropertyDescriptorBuilder(PropertyDescriptor* descriptor)
            : MemberDescriptorBuilder<PropertyDescriptorBuilder>(descriptor)
        {}

        PropertyDescriptorBuilder& MinValue(TValue value)
        {
            this->Metadata("MinValue", std::to_string(value));
            return *this;
        }

        PropertyDescriptorBuilder& MaxValue(TValue value)
        {
            this->Metadata("MaxValue", std::to_string(value));
            return *this;
        }

        PropertyDescriptorBuilder& Step(TValue value)
        {
            this->Metadata("Step", std::to_string(value));
            return *this;
        }
    };
}
