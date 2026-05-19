#pragma once

#include <memory>
#include <vector>

namespace Phoenix::PropertyGrid
{
    class IModelDescriptor;

    class IPropertyGrid
    {
    public:

        void SetTargets(const std::vector<std::shared_ptr<IModelDescriptor>>& models);
    };
}
