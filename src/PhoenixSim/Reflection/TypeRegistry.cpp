#include "TypeRegistry.h"

namespace Phoenix
{
    std::unordered_map<hash32_t, std::unique_ptr<TypeDescriptor>>& TypeRegistry::GetMap()
    {
        // Heap-allocated and never freed: Variant::DestructActive calls TypeRegistry::Get()
        // during its own destruction, which can re-enter GetMap() while the Meyers singleton
        // is mid-teardown. An immortal singleton avoids that use-after-destruction crash.
        static auto* map = new std::unordered_map<hash32_t, std::unique_ptr<TypeDescriptor>>();
        return *map;
    }

    std::unordered_map<hash32_t, TypeDescriptor*>& TypeRegistry::GetAliasMap()
    {
        static auto* aliasMap = new std::unordered_map<hash32_t, TypeDescriptor*>();
        return *aliasMap;
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
