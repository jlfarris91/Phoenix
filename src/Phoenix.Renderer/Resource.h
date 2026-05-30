#pragma once

#include <memory>

#include "ResourceHandle.h"
#include "Phoenix/Reflection/Registration.h"

namespace Phoenix::Renderer
{
    class IResource : public std::enable_shared_from_this<IResource>
    {
        PHX_DECLARE_TYPE_INTERFACE(IResource)
    public:
        virtual ~IResource() = default;
        virtual void SetId(uint32_t id) = 0;
        virtual HResource GetHandle() const = 0;
        virtual FName GetResourceType() const = 0;
        virtual void ReleaseResources() {}
    };

    class Resource : public IResource
    {
        PHX_DECLARE_TYPE_DERIVED(Resource, IResource);
    public:
        void SetId(uint32_t id) override;
        HResource GetHandle() const override;
    private:
        uint32_t Id;
    };
}
