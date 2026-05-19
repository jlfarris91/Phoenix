#pragma once

#include <memory>
#include <string>

namespace Phoenix::PropertyGrid
{
    class IObject;
    class IModelDescriptor;
    class IPropertyDescriptor;

    class IPropertyDescriptorBuilder
    {
    public:
        virtual ~IPropertyDescriptorBuilder() = default;

        virtual bool CanBuild(
            const std::shared_ptr<IModelDescriptor>& model,
            const std::string& propertyId) const = 0;

        virtual std::shared_ptr<IPropertyDescriptor> Build(
            const std::shared_ptr<IModelDescriptor>& model,
            const std::string& propertyId) = 0;
    };
}
