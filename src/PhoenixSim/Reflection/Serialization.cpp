#include "Serialization.h"

#include <ranges>

#include "PhoenixSim/Name.h"
#include "PhoenixSim/Reflection/Reflection.h"
#include "PhoenixSim/FixedPoint/FixedPoint.h"
#include "PhoenixSim/FixedPoint/FixedTransform.h"

namespace Phoenix
{
    nlohmann::json PropertyToJson(const void* obj, const PropertyDescriptor& prop)
    {
        // World-context (static world properties) cannot be serialised without a World&.
        if (prop.PropertyAccessor->RequiresWorld())
            return nullptr;

        const void* accessObj = prop.PropertyAccessor->IsStatic() ? nullptr : obj;

        switch (prop.ValueType)
        {
            case EGenericValueType::Int8:   return prop.PropertyAccessor->Get<int8>(accessObj);
            case EGenericValueType::UInt8:  return prop.PropertyAccessor->Get<uint8>(accessObj);
            case EGenericValueType::Int16:  return prop.PropertyAccessor->Get<int16>(accessObj);
            case EGenericValueType::UInt16: return prop.PropertyAccessor->Get<uint16>(accessObj);
            case EGenericValueType::Int32:  return prop.PropertyAccessor->Get<int32>(accessObj);
            case EGenericValueType::UInt32: return prop.PropertyAccessor->Get<uint32>(accessObj);
            case EGenericValueType::Int64:  return prop.PropertyAccessor->Get<int64>(accessObj);
            case EGenericValueType::UInt64: return prop.PropertyAccessor->Get<uint64>(accessObj);
            case EGenericValueType::Float:  return prop.PropertyAccessor->Get<float>(accessObj);
            case EGenericValueType::Double: return prop.PropertyAccessor->Get<double>(accessObj);
            case EGenericValueType::Bool:   return prop.PropertyAccessor->Get<bool>(accessObj);

            case EGenericValueType::String:
                return prop.PropertyAccessor->Get<std::string>(accessObj);

            case EGenericValueType::Name:
            {
                const FName name = prop.PropertyAccessor->Get<FName>(accessObj);
                if (const char* str = FName::GetNameEntry((hash32_t)name))
                    return str;
                return static_cast<uint32_t>((hash32_t)name);
            }

            case EGenericValueType::FixedPoint:
            {
                // Read the raw Q-format integer using TFixed<1> (same layout as all TFixed<N>),
                // then convert to double using the fractional-bit count stored in metadata.
                // Mirrors the ImGuiPropertyGrid approach exactly.
                const auto it = prop.Metadata.find("FractionalBits");
                if (it != prop.Metadata.end())
                {
                    const TFixed<1> fp = prop.PropertyAccessor->Get<TFixed<1>>(accessObj);
                    const uint8 b = static_cast<uint8>(std::stoi(it->second));
                    return ConvertFromQ<int32, double>(fp.Value, b);
                }
                return nullptr;
            }

            case EGenericValueType::Struct:
            {
                if (prop.StructDescriptor)
                {
                    std::vector<std::byte> tmp(prop.StructDescriptor->GetSize());
                    prop.PropertyAccessor->Get(accessObj, tmp.data(), tmp.size());
                    return TypeToJson(tmp.data(), *prop.StructDescriptor);
                }
                return nullptr;
            }

            default:
                return nullptr;
        }
    }

    nlohmann::json TypeToJson(const void* obj, const TypeDescriptor& desc)
    {
        nlohmann::json result = nlohmann::json::object();
        for (const auto& propDesc : desc.GetProperties() | std::views::values)
        {
            result[propDesc.Name] = PropertyToJson(obj, propDesc);
        }
        return result;
    }
}
