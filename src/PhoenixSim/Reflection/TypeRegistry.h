#pragma once

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Name.h"

#include <memory>
#include <unordered_map>

namespace Phoenix
{
    struct TypeDescriptor;

    // Global registry that owns every reflected TypeDescriptor.
    // Types are created on first call to GetOrCreate<T>() and live for the
    // lifetime of the program (Meyer-singleton storage inside GetMap()).
    class PHOENIX_SIM_API TypeRegistry
    {
    public:
        // Creates (or retrieves) the TypeDescriptor for T.
        // On first call the descriptor is constructed from T's static type info
        // and each TBase is registered as a base class.
        // Subsequent calls are a no-op and just return the existing descriptor.
        // Full template definition lives in Reflection.h (needs complete TypeDescriptor).
        template <class T, class... TBases>
        static TypeDescriptor& GetOrCreate();

        // Returns the descriptor for the given name, or nullptr if not registered.
        static const TypeDescriptor* Get(FName name);

        // Read-only view of all registered descriptors.
        static const std::unordered_map<hash32_t, std::unique_ptr<TypeDescriptor>>& GetAll();

    private:
        static std::unordered_map<hash32_t, std::unique_ptr<TypeDescriptor>>& GetMap();
    };
}