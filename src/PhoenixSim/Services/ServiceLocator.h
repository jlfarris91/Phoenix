#pragma once

#include "PhoenixSim/Containers/Array.h"
#include "PhoenixSim/Services/Service.h"

namespace Phoenix
{
    class IService;

    class PHOENIX_SIM_API IServiceLocator
    {
    public:
        virtual ~IServiceLocator() = default;

        // Get a service that was registered with the given type id.
        virtual TSharedPtr<IService> GetService(const FName& typeId) const = 0;

        // Get all services that were registered with a given type id.
        virtual uint32 GetServices(const FName& typeId, TArray2<TSharedPtr<IService>>& outServices) const = 0;

        // Get the list of all services registered.
        virtual const TArray2<TSharedPtr<IService>>& GetServices() const = 0;
    };

    template <class T>
    class PHOENIX_SIM_API ServiceLocator : public IServiceLocator
    {
    public:

        TSharedPtr<IService> GetService(const FName& typeId) const override
        {
            return ThisAsT()->GetService(typeId);
        }

        template <class TService>
        TSharedPtr<TService> GetService(const FName& typeId = TService::StaticTypeName) const
        {
            return std::const_pointer_cast<TService>(GetService(typeId));
        }

        uint32 GetServices(const FName& typeId, TArray2<TSharedPtr<IService>>& outServices) const override
        {
            return ThisAsT()->GetServices(typeId, outServices);
        }

        template <class TService>
        uint32 GetServices2(TArray2<TSharedPtr<TService>>& outServices) const
        {
            TArray2<TSharedPtr<IService>> services;
            GetServices(TService::StaticTypeName, services);
            for (const TSharedPtr<IService>& service : services)
            {
                outServices.PushBack(std::static_pointer_cast<TService>(service));
            }
            return static_cast<uint32>(services.Num());
        }

        const TArray2<TSharedPtr<IService>>& GetServices() const override
        {
            return ThisAsT()->GetServices();
        }

    private:

        friend T;

        PHX_FORCE_INLINE constexpr T* ThisAsT()
        {
            return static_cast<T*>(this);
        }

        PHX_FORCE_INLINE constexpr const T* ThisAsT() const
        {
            return static_cast<const T*>(this);
        }
    };
}
