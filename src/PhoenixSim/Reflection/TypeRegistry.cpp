#include "TypeRegistry.h"
#include "Reflection.h"

namespace Phoenix
{
    std::unordered_map<hash32_t, std::unique_ptr<TypeDescriptor>>& TypeRegistry::GetMap()
    {
        // Meyers singleton — safe for use during static initialisation across TUs.
        static std::unordered_map<hash32_t, std::unique_ptr<TypeDescriptor>> map;
        return map;
    }

    const TypeDescriptor* TypeRegistry::Get(FName name)
    {
        const auto& map = GetMap();
        const auto it = map.find((hash32_t)name);
        return it != map.end() ? it->second.get() : nullptr;
    }

    const std::unordered_map<hash32_t, std::unique_ptr<TypeDescriptor>>& TypeRegistry::GetAll()
    {
        return GetMap();
    }
}
