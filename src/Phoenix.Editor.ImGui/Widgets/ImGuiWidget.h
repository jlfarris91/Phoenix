#pragma once

#include <assert.h>
#include "Phoenix/Reflection/Registration.h"
#include "Phoenix/Reflection/TypeName.h"

#include "ImGuiWidgetContext.h"

namespace Phoenix
{
    class IObject;
}

namespace Phoenix::UI
{
    class ImGuiWidget
    {
        PHX_DECLARE_TYPE_INTERFACE(ImGuiWidget)
    public:

        virtual ~ImGuiWidget() = default;

        const ImGuiWidgetContext& GetContext() const;
        void SetContext(const ImGuiWidgetContext& context);

        std::shared_ptr<IObject> GetContextObject(const std::string& typeId) const;

        template <class T>
        std::shared_ptr<T> GetContextObject() const
        {
            auto typeId = std::string(Phoenix::StaticTypeName<T>::GetQualifiedName());
            auto object = this->GetContextObject(typeId);
            return std::static_pointer_cast<T>(object);
        }

        template <class T>
        T& GetContextObjectRef() const
        {
            auto object = GetContextObject<T>();
            assert(object != nullptr);
            return *object;
        }

        virtual void Render() = 0;

    private:

        ImGuiWidgetContext Context;
    };
}
