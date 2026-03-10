#pragma once

#include "PhoenixSim/Services/Service.h"

namespace Phoenix
{
    class IService;

    class PHOENIX_SIM_API IServiceLocator
    {
    public:
        virtual ~IServiceLocator() = default;

        // Get a service that was registered with the given type id.
        virtual std::shared_ptr<IService> GetService(const FName& typeId) const = 0;

        // Get all services that were registered with a given type id.
        virtual uint32 GetServices(const FName& typeId, std::vector<std::shared_ptr<IService>>& outServices) const = 0;

        // Get the list of all services registered.
        virtual const std::vector<std::shared_ptr<IService>>& GetServices() const = 0;
    };

    template <class T>
    class PHOENIX_SIM_API ServiceLocator : public IServiceLocator
    {
    public:

        std::shared_ptr<IService> GetService(const FName& typeId) const override
        {
            return ThisAsT()->GetService(typeId);
        }

        template <class TService>
        std::shared_ptr<TService> GetServiceAs(const FName& typeId = TService::StaticTypeName) const
        {
            return std::static_pointer_cast<TService>(GetService(typeId));
        }

        uint32 GetServices(const FName& typeId, std::vector<std::shared_ptr<IService>>& outServices) const override
        {
            return ThisAsT()->GetServices(typeId, outServices);
        }

        template <class TService>
        uint32 GetServices2(std::vector<std::shared_ptr<TService>>& outServices) const
        {
            std::vector<std::shared_ptr<IService>> services;
            GetServices(TService::StaticTypeName, services);
            for (const std::shared_ptr<IService>& service : services)
            {
                outServices.push_back(std::static_pointer_cast<TService>(service));
            }
            return static_cast<uint32>(services.size());
        }

        const std::vector<std::shared_ptr<IService>>& GetServices() const override
        {
            return ThisAsT()->GetServices();
        }

    private:

        friend T;

        PHX_FORCEINLINE constexpr T* ThisAsT()
        {
            return static_cast<T*>(this);
        }

        PHX_FORCEINLINE constexpr const T* ThisAsT() const
        {
            return static_cast<const T*>(this);
        }
    };
}
