#pragma once

#include "Phoenix/Reflection/BaseDescriptor.h"

namespace Phoenix
{
    enum class PHOENIX_SIM_API EMemberDescriptorFlags : uint8
    {
        None            = 0,
        Field           = 1,
        Property        = 2,
        Method          = 4,
        Static          = 8,
        ReadOnly        = 16,
        Constructor     = 32,
        ScriptHidden    = 64,
    };

    class PHOENIX_SIM_API MemberDescriptor : public BaseDescriptor
    {
    public:

        const std::string& GetCategory() const;
        void SetCategory(const std::string& category);

        int32 GetSortOrder() const;
        void SetSortOrder(int32 sortOrder);

        EMemberDescriptorFlags GetFlags() const;

        bool IsStatic() const;

        bool IsReadOnly() const;

        bool IsScriptHidden() const;

    // protected:

        std::string Category;
        int32 SortOrder = 0;
        EMemberDescriptorFlags Flags = EMemberDescriptorFlags::None;
    };
}
