#pragma once

#include "PhoenixSim/Flags.h"
#include "PhoenixSim/Reflection/MemberDescriptor.h"

namespace Phoenix
{
    template <class TBuilder>
    class MemberDescriptorBuilder
    {
    public:
        MemberDescriptorBuilder(MemberDescriptor* descriptor)
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

        TBuilder& Category(const std::string& category)
        {
            Descriptor->Category = category;
            return *reinterpret_cast<TBuilder*>(this);
        }

        TBuilder& SortOrder(int32 order)
        {
            Descriptor->SortOrder = order;
            return *reinterpret_cast<TBuilder*>(this);
        }

        TBuilder& ScriptHidden()
        {
            SetFlagRef(Descriptor->Flags, EMemberDescriptorFlags::ScriptHidden);
            return *reinterpret_cast<TBuilder*>(this);
        }

        template <class ...TFlag>
        TBuilder& Flags(TFlag&&... flags)
        {
            (SetFlagRef(Descriptor->Flags, flags), ...);
            return *reinterpret_cast<TBuilder*>(this);
        }

    private:
        MemberDescriptor* Descriptor = nullptr;
    };
}
