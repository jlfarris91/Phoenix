#pragma once

#include "PhoenixSim/Reflection/BaseDescriptor.h"

namespace Phoenix
{
    template <class TBuilder>
    class BaseDescriptorBuilder
    {
    public:
        BaseDescriptorBuilder(BaseDescriptor* descriptor)
            : Descriptor(descriptor)
        {}

        TBuilder& Metadata(const std::string& key, const std::string& value)
        {
            Descriptor->Metadata[key] = value;
            return *reinterpret_cast<TBuilder*>(this);
        }

        template <class TValue>
        TBuilder& Metadata(const std::string& key, const TValue& value)
        {
            Descriptor->Metadata[key] = std::to_string(value);
            return *reinterpret_cast<TBuilder*>(this);
        }

        TBuilder& DisplayName(const std::string& displayName)
        {
            Descriptor->DisplayName = displayName;
            return *reinterpret_cast<TBuilder*>(this);
        }

    private:
        BaseDescriptor* Descriptor = nullptr;
    };
}
