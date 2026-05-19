#pragma once

#include "JsonModelDescriptor.h"
#include "JsonPropertyDescriptor.h"
#include "History/SendActionMessage.h"
#include "Model/PropertyValueAccessor.h"

namespace Phoenix::PropertyGrid
{
    template <class T>
    class JsonPrimitivePropertyDescriptor : public JsonPropertyDescriptor
                                          , public IPropertyValueAccessor<T>
    {
    public:

        std::optional<T> GetValue() const override
        {
            auto modelDescriptor = GetModelDescriptor();
            assert(modelDescriptor);

            auto jsonModelDescriptor = Phoenix::Cast<JsonModelDescriptor>(modelDescriptor);
            assert(jsonModelDescriptor);

            std::optional<const nlohmann::json*> targetJson = jsonModelDescriptor->GetTargetJson(Pointer);
            if (!targetJson.has_value())
            {
                return {};
            }

            return targetJson.value()->get<T>();
        }

        void SetValue(const T& value) override
        {
            auto currentValue = GetValue();
            if (currentValue.has_value() && currentValue.value() == value)
            {
                return;
            }

            if (auto action = CreateSetValueAction(Pointer, value))
            {
                Phoenix::SendActionMessage message;
                message.Action = action;
                SendMessage(message);
            }
        }
    };
}
