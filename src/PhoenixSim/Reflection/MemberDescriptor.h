#pragma once

#include <string>
#include <unordered_map>

#include "PhoenixSim/Platform.h"

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

    class PHOENIX_SIM_API MemberDescriptor
    {
    public:
        virtual ~MemberDescriptor() = default;

        const std::string& GetName() const;

        const std::string& GetDisplayName() const;
        void SetDisplayName(const std::string& displayName);

        const std::string& GetCategory() const;
        void SetCategory(const std::string& category);

        int32 GetSortOrder() const;
        void SetSortOrder(int32 sortOrder);

        const std::unordered_map<std::string, std::string>& GetMetadata() const;

        EMemberDescriptorFlags GetFlags() const;

        bool IsStatic() const;

        bool IsReadOnly() const;

        bool IsScriptHidden() const;

    // protected:

        std::string Name;
        std::string DisplayName;
        std::string Category;
        int32 SortOrder = 0;
        std::unordered_map<std::string, std::string> Metadata;
        EMemberDescriptorFlags Flags = EMemberDescriptorFlags::None;
    };
}
