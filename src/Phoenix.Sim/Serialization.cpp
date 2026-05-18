#include "Serialization.h"

#include <ranges>

#include "Phoenix/Name.h"
#include "Phoenix/FixedPoint/FixedPoint.h"
#include "Phoenix/FixedPoint/FixedTransform.h"

namespace Phoenix
{
    nlohmann::json PropertyToJson(const void* obj, const PropertyDescriptor& prop)
    {
        // World-context (static world properties) cannot be serialised without a World&.
        // if (prop.RequiresWorld())
        //     return nullptr;

        auto type = prop.GetType();
        if (!type)
        {
            return {};
        }

        const void* accessObj = prop.IsStatic() ? nullptr : obj;

        switch (type->GetTypeId())
        {
            case StaticTypeName<int8>::TypeId:          return prop.Get<int8>(accessObj);
            case StaticTypeName<uint8>::TypeId:         return prop.Get<uint8>(accessObj);
            case StaticTypeName<int16>::TypeId:         return prop.Get<int16>(accessObj);
            case StaticTypeName<uint16>::TypeId:        return prop.Get<uint16>(accessObj);
            case StaticTypeName<int32>::TypeId:         return prop.Get<int32>(accessObj);
            case StaticTypeName<uint32>::TypeId:        return prop.Get<uint32>(accessObj);
            case StaticTypeName<int64>::TypeId:         return prop.Get<int64>(accessObj);
            case StaticTypeName<uint64>::TypeId:        return prop.Get<uint64>(accessObj);
            case StaticTypeName<float>::TypeId:         return prop.Get<float>(accessObj);
            case StaticTypeName<double>::TypeId:        return prop.Get<double>(accessObj);
            case StaticTypeName<bool>::TypeId:          return prop.Get<bool>(accessObj);
            case StaticTypeName<std::string>::TypeId:   return prop.Get<std::string>(accessObj);

            case StaticTypeName<FName>::TypeId:
            {
                const FName name = prop.Get<FName>(accessObj);
                if (const char* str = FName::GetNameEntry(name))
                {
                    return str;
                }
                return (hash32_t)name;
            }
        }

        const auto& metadata = type->GetMetadata();

        // FixedPoint
        {
            // Read the raw Q-format integer using TFixed<1> (same layout as all TFixed<N>),
            // then convert to double using the fractional-bit count stored in metadata.
            // Mirrors the ImGuiPropertyGrid approach exactly.
            const auto it = metadata.find("FractionalBits");
            if (it != metadata.end())
            {
                const TFixed<1> fp = prop.Get<TFixed<1>>(accessObj);
                const uint8 b = static_cast<uint8>(std::stoi(it->second));
                return ConvertFromQ<int32, double>(fp.Value, b);
            }
        }

        return {};
    }

    nlohmann::json TypeToJson(const void* obj, const TypeDescriptor& desc)
    {
        nlohmann::json result = nlohmann::json::object();
        for (const auto& propDesc : desc.GetProperties() | std::views::values)
        {
            result[propDesc.GetName()] = PropertyToJson(obj, propDesc);
        }
        return result;
    }
}
