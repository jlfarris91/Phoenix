#pragma once

#include <string>
#include <unordered_map>

#include "Phoenix/Platform.h"

namespace Phoenix
{
    class PHOENIX_SIM_API BaseDescriptor
    {
    public:
        virtual ~BaseDescriptor() = default;

        const std::string& GetName() const;

        const std::string& GetDisplayName() const;
        void SetDisplayName(const std::string& displayName);

        const std::unordered_map<std::string, std::string>& GetMetadata() const;

    // protected:

        std::string Name;
        std::string DisplayName;
        std::unordered_map<std::string, std::string> Metadata;
    };
}
