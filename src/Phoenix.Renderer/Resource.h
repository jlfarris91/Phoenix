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

        HResource GetHandle() const;

        FName GetFullName() const;

        virtual FName GetResourceType() const = 0;

        virtual void ReleaseResources() {}

    private:

        friend struct ResourceInit;

        uint32_t Id = 0;
        FName FullName;
    };

    struct ResourceInit
    {
        static void Init(IResource& resource, uint32_t id, FName fullName);
    };
}
