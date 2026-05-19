#pragma once

#include <memory>

#include "Object.h"

namespace Phoenix::PropertyGrid
{
    class IModelDescriptor;

    class IModelDescriptorBuilder
    {
    public:
        virtual ~IModelDescriptorBuilder() = default;

        virtual bool CanBuild(const std::shared_ptr<Phoenix::IObject>& object) const = 0;

        virtual std::shared_ptr<IModelDescriptor> Build(const std::shared_ptr<Phoenix::IObject>& object) = 0;
    };
}
