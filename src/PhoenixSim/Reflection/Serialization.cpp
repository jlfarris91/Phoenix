#include "Serialization.h"

#include <ranges>

#include "PhoenixSim/Name.h"
#include "PhoenixSim/Reflection/Reflection.h"
#include "PhoenixSim/FixedPoint/FixedPoint.h"
#include "PhoenixSim/FixedPoint/FixedVector.h"
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
            case EPropertyValueType::Int8:   return prop.PropertyAccessor->Get<int8>(accessObj);
            case EPropertyValueType::UInt8:  return prop.PropertyAccessor->Get<uint8>(accessObj);
            case EPropertyValueType::Int16:  return prop.PropertyAccessor->Get<int16>(accessObj);
            case EPropertyValueType::UInt16: return prop.PropertyAccessor->Get<uint16>(accessObj);
            case EPropertyValueType::Int32:  return prop.PropertyAccessor->Get<int32>(accessObj);
            case EPropertyValueType::UInt32: return prop.PropertyAccessor->Get<uint32>(accessObj);
            case EPropertyValueType::Int64:  return prop.PropertyAccessor->Get<int64>(accessObj);
            case EPropertyValueType::UInt64: return prop.PropertyAccessor->Get<uint64>(accessObj);
            case EPropertyValueType::Float:  return prop.PropertyAccessor->Get<float>(accessObj);
            case EPropertyValueType::Double: return prop.PropertyAccessor->Get<double>(accessObj);
            case EPropertyValueType::Bool:   return prop.PropertyAccessor->Get<bool>(accessObj);

            case EPropertyValueType::String:
                return prop.PropertyAccessor->Get<std::string>(accessObj);

            case EPropertyValueType::Name:
            {
                const FName name = prop.PropertyAccessor->Get<FName>(accessObj);
                if (const char* str = FName::GetNameEntry((hash32_t)name))
                    return str;
                return static_cast<uint32_t>((hash32_t)name);
            }

            case EPropertyValueType::FixedPoint:
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

            case EPropertyValueType::Vec2:
            {
                const Vec2 v = prop.PropertyAccessor->Get<Vec2>(accessObj);
                return nlohmann::json::array({ static_cast<double>(v.X), static_cast<double>(v.Y) });
            }

            case EPropertyValueType::Transform2D:
            {
                const Transform2D t = prop.PropertyAccessor->Get<Transform2D>(accessObj);
                return nlohmann::json{
                    { "x",        static_cast<double>(t.Position.X) },
                    { "y",        static_cast<double>(t.Position.Y) },
                    { "rotation", static_cast<double>(t.Rotation)   },
                    { "scale",    static_cast<double>(t.Scale)       },
                };
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
