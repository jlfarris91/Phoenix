#pragma once

#include "PhoenixSim/Services/Service.h"

namespace Phoenix
{
    class PHOENIX_SIM_API IStringService : IService
    {
        PHX_DECLARE_INTERFACE_WITH_BASE(IStringService, IService)

    public:

        // Gets the string associated with the given name, or nullptr if no such string exists.
        virtual const char* Get(const FName& name) const = 0;

        // Stores the given string and returns a pointer to the stored string.
        // If the string already exists, it will not be stored again, and a pointer to the existing string will be returned.
        virtual const char* Store(const char* str, uint32 len) = 0;

        // Stores the given string with the given name and returns a pointer to the stored string.
        // If a string already exists with the given name, it will not be stored again, and a pointer to the existing string will be returned.
        virtual const char* StoreAs(const char* str, uint32 len, const FName& name) = 0;
    };

    PHOENIX_SIM_API IStringService& GetStringService();
    PHOENIX_SIM_API void SetStringService(const TSharedPtr<IStringService>& service);
}
