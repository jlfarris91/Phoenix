#pragma once

#include <string>
#include <unordered_map>

namespace Phoenix
{
    // An extension point for users to provide custom metadata for specific types.
    template <class T>
    struct TypeDescriptorMetadataProvider
    {
        static std::unordered_map<std::string, std::string> GetMetadata() { return {}; }
    };
}