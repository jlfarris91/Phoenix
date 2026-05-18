#pragma once

#include "PhoenixSim/Flags.h"
#include "PhoenixSim/Reflection/BaseDescriptorBuilder.h"
#include "PhoenixSim/Reflection/MemberDescriptor.h"

namespace Phoenix
{
    template <class TBuilder>
    class MemberDescriptorBuilder : public BaseDescriptorBuilder<TBuilder>
    {
    public:
        MemberDescriptorBuilder(MemberDescriptor* descriptor)
            : BaseDescriptorBuilder<TBuilder>(descriptor)
            , Descriptor(descriptor)
        {}

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
