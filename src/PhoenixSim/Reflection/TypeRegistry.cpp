#include "TypeRegistry.h"

namespace Phoenix
{
    std::unordered_map<hash32_t, std::unique_ptr<TypeDescriptor>>& TypeRegistry::GetMap()
    {
        // Meyers singleton — safe for use during static initialization across TUs.
        static std::unordered_map<hash32_t, std::unique_ptr<TypeDescriptor>> map;
        return map;
    }

    std::unordered_map<hash32_t, TypeDescriptor*>& TypeRegistry::GetAliasMap()
    {
        static std::unordered_map<hash32_t, TypeDescriptor*> aliasMap;
        return aliasMap;
    }

    const TypeDescriptor* TypeRegistry::Get(FName typeId)
    {
        const auto& map = GetMap();
        const auto mapIter = map.find(typeId);
        if (mapIter != map.end())
        {
            return mapIter->second.get();
        }
        const auto& aliasMap = GetAliasMap();
        const auto aliasMapIter = aliasMap.find(typeId);
        return aliasMapIter != aliasMap.end() ? aliasMapIter->second : nullptr;
    }

    const std::unordered_map<hash32_t, std::unique_ptr<TypeDescriptor>>& TypeRegistry::GetAll()
    {
        return GetMap();
    }
}
