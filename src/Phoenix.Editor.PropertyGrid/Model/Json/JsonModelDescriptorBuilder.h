#pragma once

#include <nlohmann/json.hpp>

#include "Model/ModelDescriptorBuilder.h"

namespace Phoenix::PropertyGrid
{
    class ModelDescriptor;
    class PropertyDescriptor;
    class JsonModelObject;

    class JsonModelDescriptorBuilder : public IModelDescriptorBuilder
    {
    public:

        bool CanBuild(const std::shared_ptr<Phoenix::IObject>& object) const override;

        std::shared_ptr<IModelDescriptor> Build(const std::shared_ptr<Phoenix::IObject>& object) override;

    private:

        void ProcessJson(
            const std::shared_ptr<ModelDescriptor>& modelDescriptor,
            const std::shared_ptr<PropertyDescriptor>& parentDescriptor,
            const std::shared_ptr<JsonModelObject>& object,
            const nlohmann::json::json_pointer& pointer);
    };
}
