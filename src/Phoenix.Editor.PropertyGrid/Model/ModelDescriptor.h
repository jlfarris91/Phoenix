#pragma once

#include <memory>
#include <vector>

#include "Phoenix/Reflection/Registration.h"
#include "Util/Attribute.h"
#include "Messaging/MessageRouter.h"
#include "Object.h"

namespace Phoenix::PropertyGrid
{
    class IPropertyDescriptor;

    class IModelDescriptor : public Phoenix::MessageRouter
    {
        PHX_DECLARE_TYPE_INTERFACE(IModelDescriptor)
    public:

        using Phoenix::MessageRouter::SendMessage;

        virtual std::string GetId() const = 0;

        virtual std::string GetDisplayName() const = 0;

        virtual std::string GetDescription() const = 0;

        virtual const std::shared_ptr<IModelDescriptor>& GetParent() const = 0;

        virtual const std::vector<std::shared_ptr<Phoenix::IObject>>& GetTargets() const = 0;

        virtual const std::vector<std::shared_ptr<IPropertyDescriptor>>& GetProperties() const = 0;

        virtual std::shared_ptr<IPropertyDescriptor> FindProperty(const std::string& id) const = 0;

        template <class T>
        uint32_t GetTargets(std::vector<std::shared_ptr<T>>& outTargets) const
        {
            uint32_t count = 0;
            for (const auto& target : GetTargets())
            {
                if (auto typedTarget = Phoenix::Cast<T>(target))
                {
                    outTargets.push_back(typedTarget);
                    ++count;
                }
            }
            return count;
        }
    };

    class ModelDescriptor : public IModelDescriptor
    {
        PHX_DECLARE_TYPE_DERIVED(ModelDescriptor, IModelDescriptor)
    public:

        using IModelDescriptor::GetTargets;

        std::string GetId() const override;
        void SetId(const std::string& id);

        std::string GetDisplayName() const override;
        void SetDisplayName(const Phoenix::Attribute<std::string>& displayName);

        std::string GetDescription() const override;
        void SetDescription(const Phoenix::Attribute<std::string>& description);

        const std::shared_ptr<IModelDescriptor>& GetParent() const override;
        void SetParent(const std::shared_ptr<IModelDescriptor>& parent);

        const std::vector<std::shared_ptr<Phoenix::IObject>>& GetTargets() const override;
        void SetTargets(const std::vector<std::shared_ptr<Phoenix::IObject>>& targets);

        const std::vector<std::shared_ptr<IPropertyDescriptor>>& GetProperties() const override;

        std::shared_ptr<IPropertyDescriptor> FindProperty(const std::string& id) const override;

    protected:

        friend class PropertyDescriptor;

        bool SendMessage(type_id typeId, const void* data) override;

        std::string Id;
        Phoenix::Attribute<std::string> DisplayName;
        Phoenix::Attribute<std::string> Description;
        std::shared_ptr<IModelDescriptor> Parent;
        std::vector<std::shared_ptr<Phoenix::IObject>> Targets;
        std::vector<std::shared_ptr<IPropertyDescriptor>> Properties;
    };
}
