#pragma once

#include "PhoenixSim/Reflection/EnumValueDescriptorBuilder.h"
#include "PhoenixSim/Reflection/TypeRegistry.h"

namespace Phoenix
{
    template <class TValue>
    using EnumValueDescriptorBuilderFunc = std::function<void(EnumValueDescriptorBuilder<TValue>& builder)>;

    template <class T>
    class EnumDescriptorBuilder
    {
    public:
        explicit EnumDescriptorBuilder()
            : Descriptor(&TypeRegistry::Get<T>())
        {
        }

        EnumDescriptorBuilder& EnumFlags()
        {
            SetFlagRef(Descriptor->Flags, ETypeDescriptorFlags::EnumFlags);
            return *this;
        }

        template <class TValue>
        EnumDescriptorBuilder& Value(
            const char* name,
            TValue value,
            const std::type_identity_t<EnumValueDescriptorBuilderFunc<TValue>>& enumValue = {})
        {
            uint32 index = Descriptor->EnumValues.size();
            EnumValueDescriptor descriptor(index, Variant(static_cast<const T&>(value)));
            descriptor.Name = name;
            if (enumValue)
            {
                EnumValueDescriptorBuilder<TValue> builder(&descriptor);
                enumValue(builder);
            }
            Descriptor->EnumValues.push_back(descriptor);
            return *this;
        }

    private:

        TypeDescriptor* Descriptor;
    };
}
