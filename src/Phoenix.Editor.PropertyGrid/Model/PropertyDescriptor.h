#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "Phoenix/Reflection/Registration.h"
#include "Phoenix/Delegates.h"
#include "Util/Attribute.h"
#include "Messaging/MessageRouter.h"

namespace Phoenix::PropertyGrid
{
    class ModelDescriptor;
    class IModelDescriptor;
    class IPropertyDescriptor;

    PHX_DECLARE_MULTICAST_DELEGATE(PropertySetValueEvent, const std::shared_ptr<IPropertyDescriptor>&);

    class IPropertyDescriptor : public Phoenix::MessageRouter
    {
        PHX_DECLARE_TYPE_INTERFACE(IPropertyDescriptor)
    public:

        using Phoenix::MessageRouter::SendMessage;

        virtual std::string GetId() const = 0;

        virtual std::string GetDisplayName() const = 0;

        virtual std::string GetDescription() const = 0;

        virtual std::string GetType() const = 0;

        virtual bool IsArray() const = 0;

        virtual int32_t GetArrayIndex() const = 0;

        virtual std::shared_ptr<IModelDescriptor> GetModelDescriptor() const = 0;

        virtual std::shared_ptr<IPropertyDescriptor> GetParentProperty() const = 0;

        virtual const std::vector<std::shared_ptr<IPropertyDescriptor>>& GetChildProperties() const = 0;

        virtual PropertySetValueEvent& OnPropertySetEvent() = 0;
    };

    class PropertyDescriptor : public IPropertyDescriptor
                             , public std::enable_shared_from_this<PropertyDescriptor>
    {
        PHX_DECLARE_TYPE_DERIVED(PropertyDescriptor, IPropertyDescriptor)
    public:

        using Phoenix::MessageRouter::SendMessage;

        std::string GetId() const override;
        void SetId(const std::string& id);

        std::string GetDisplayName() const override;
        void SetDisplayName(const Phoenix::Attribute<std::string>& name);

        std::string GetDescription() const override;
        void SetDescription(const std::string& description);

        std::string GetType() const override;
        void SetType(const std::string& type);

        bool IsArray() const override;
        void SetIsArray(bool isArray);

        int32_t GetArrayIndex() const override;
        void SetArrayIndex(int32_t index);

        std::shared_ptr<IModelDescriptor> GetModelDescriptor() const override;
        void SetModelDescriptor(const std::shared_ptr<ModelDescriptor>& model);

        std::shared_ptr<IPropertyDescriptor> GetParentProperty() const override;
        void SetParentProperty(const std::shared_ptr<PropertyDescriptor>& parent);

        const std::vector<std::shared_ptr<IPropertyDescriptor>>& GetChildProperties() const override;

        PropertySetValueEvent& OnPropertySetEvent() override;

    protected:

        friend class ModelDescriptor;

        bool SendMessage(type_id typeId, const void* data) override;

        std::string Id;
        std::string Type;
        Phoenix::Attribute<std::string> DisplayName;
        Phoenix::Attribute<std::string> Description;
        bool bIsArray = false;
        int32_t ArrayIndex = -1;
        std::shared_ptr<ModelDescriptor> Model;
        std::shared_ptr<PropertyDescriptor> Parent;
        std::vector<std::shared_ptr<IPropertyDescriptor>> ChildProperties;
        PropertySetValueEvent PropertySetValueEvent;
    };
}
