#include "WasmUtility.h"

#include <ranges>

#include "Phoenix.Sim/Logging.h"
#include "Phoenix.Sim/Worlds.h"
#include "Phoenix/Reflection/TypeDescriptor.h"

using namespace Phoenix;

bool Script::ToWasmTypeChar(const TypeDescriptor& type, char& outChar)
{
    if (type.GetTypeId() == StaticTypeName<void>::TypeId)
    {
        outChar = 'v';
        return true;
    }

    // TFixed stores int32_t internally, not IEEE 754 float
    if (type.IsTemplate("Phoenix::TFixed"))
    {
        if (type.GetSize() == sizeof(int32))
        {
            outChar = 'i';
            return true;
        }
        if (type.GetSize() == sizeof(int64))
        {
            outChar = 'I';
            return true;
        }
        return false;
    }

    switch (type.GetTypeId())
    {
        case StaticTypeName<bool>::TypeId:
        case StaticTypeName<int8>::TypeId:
        case StaticTypeName<uint8>::TypeId:
        case StaticTypeName<int16>::TypeId:
        case StaticTypeName<uint16>::TypeId:
        case StaticTypeName<int32>::TypeId:
        case StaticTypeName<uint32>::TypeId:
        case StaticTypeName<FName>::TypeId:
            {
                outChar = 'i';
                return true;
            }
        case StaticTypeName<int64>::TypeId:
        case StaticTypeName<uint64>::TypeId:
            {
                outChar = 'I';
                return true;
            }
        case StaticTypeName<float>::TypeId:
            {
                outChar = 'f';
                return true;
            }
        case StaticTypeName<double>::TypeId:
            {
                outChar = 'F';
                return true;
            }
        default:
            break;
    }

    // Fallback: opaque 32-bit handle type (e.g. EntityId, UnitId) with no registered
    // fields. Treat as i32 so it round-trips through WASM. Intentionally limited to
    // 4-byte types — 8-byte types without fields are likely multi-field structs with
    // padding (e.g. SpawnUnitArgs) and should stay Unknown so their default values apply.
    if (!IsExpandableStruct(type) && type.GetSize() == sizeof(int32))
    {
        outChar = 'i';
        return true;
    }

    return false;
}

char Script::ToWasmTypeChar(const TypeDescriptor& type)
{
    char c;
    return ToWasmTypeChar(type, c) ? c : 'v';
}

bool Script::IsSupportedWasmType(const TypeDescriptor& type)
{
    char c;
    return ToWasmTypeChar(type, c);
}

Variant Script::ReadWasmArg(uint64_t*& sp, const TypeDescriptor& type)
{
    const uint64_t slot = *sp++;

    if (type.IsTemplate("Phoenix::TFixed"))
    {
        // TFixed stores a raw int32_t. Pass it through as-is.
        Variant gv(type);
        const int32_t raw = static_cast<int32_t>(slot);
        std::memcpy(gv.GetData(), &raw, type.GetSize());
        return gv;
    }

    switch (type.GetTypeId())
    {
        case StaticTypeName<bool>::TypeId:      return Variant(static_cast<bool>(slot != 0ull));
        case StaticTypeName<int8>::TypeId:      return Variant(static_cast<int8>(static_cast<int32>(slot)));
        case StaticTypeName<uint8>::TypeId:     return Variant(static_cast<uint8>(slot));
        case StaticTypeName<int16>::TypeId:     return Variant(static_cast<int16>(static_cast<int32>(slot)));
        case StaticTypeName<uint16>::TypeId:    return Variant(static_cast<uint16>(slot));
        case StaticTypeName<int32>::TypeId:     return Variant(static_cast<int32>(static_cast<int32>(slot)));
        case StaticTypeName<uint32>::TypeId:    return Variant(static_cast<uint32>(slot));
        case StaticTypeName<int64>::TypeId:     return Variant(static_cast<int64>(static_cast<int64>(slot)));
        case StaticTypeName<uint64>::TypeId:    return Variant(static_cast<uint64>(slot));
        case StaticTypeName<float>::TypeId:
        {
            float v;
            std::memcpy(&v, &slot, sizeof(float));
            return Variant(v);
        }
        case StaticTypeName<double>::TypeId:
        {
            double v;
            std::memcpy(&v, &slot, sizeof(double));
            return Variant(v);
        }
        case StaticTypeName<FName>::TypeId:
        {
            const FName name(static_cast<hash32_t>(slot));
            return Variant(name);
        }
        default:
        {
            // Opaque value type (e.g. EntityId) — copy raw bits into a typed Variant.
            if (type.GetSize() == sizeof(int32))
            {
                Variant v(type);
                const uint32_t raw = static_cast<uint32_t>(slot);
                std::memcpy(v.GetData(), &raw, sizeof(uint32_t));
                return v;
            }
            if (type.GetSize() == sizeof(int64))
            {
                Variant v(type);
                std::memcpy(v.GetData(), &slot, sizeof(uint64_t));
                return v;
            }
            return Variant::Void();
        }
    }
}

void Script::WriteWasmReturn(uint64_t* sp, const Variant& val)
{
    const TypeDescriptor* type = val.GetType();
    assert(type != nullptr);

    if (type->IsTemplate("Phoenix::TFixed"))
    {
        // Raw int32 bits — pass through without float reinterpretation.
        int32_t raw;
        std::memcpy(&raw, val.GetData(), sizeof(int32_t));
        *sp = static_cast<uint64_t>(static_cast<uint32_t>(raw));
    }

    switch (val.GetTypeId())
    {
    case StaticTypeName<bool>::TypeId:
        *sp = val.As<bool>() ? 1u : 0u;
        break;
    case StaticTypeName<int8>::TypeId:
        *sp = static_cast<uint64_t>(static_cast<int32_t>(val.As<int8_t>()));
        break;
    case StaticTypeName<uint8>::TypeId:
        *sp = static_cast<uint64_t>(val.As<uint8_t>());
        break;
    case StaticTypeName<int16>::TypeId:
        *sp = static_cast<uint64_t>(static_cast<int32_t>(val.As<int16_t>()));
        break;
    case StaticTypeName<uint16>::TypeId:
        *sp = static_cast<uint64_t>(val.As<uint16_t>());
        break;
    case StaticTypeName<int32>::TypeId:
        *sp = static_cast<uint64_t>(val.As<int32_t>());
        break;
    case StaticTypeName<uint32>::TypeId:
        *sp = static_cast<uint64_t>(val.As<uint32_t>());
        break;
    case StaticTypeName<int64>::TypeId:
        *sp = static_cast<uint64_t>(val.As<int64_t>());
        break;
    case StaticTypeName<uint64>::TypeId:
        *sp = val.As<uint64_t>();
        break;
    case StaticTypeName<float>::TypeId:
    {
        const float v = val.As<float>();
        uint32_t u;
        std::memcpy(&u, &v, sizeof(float));
        *sp = u;
        break;
    }
    case StaticTypeName<double>::TypeId:
    {
        const double v = val.As<double>();
        std::memcpy(sp, &v, sizeof(double));
        break;
    }
    case StaticTypeName<FName>::TypeId:
    {
        const FName n = val.As<FName>();
        *sp = static_cast<uint64_t>(static_cast<hash32_t>(n));
        break;
    }
    default:
    {
        // Opaque value type (e.g. EntityId) — copy raw bits to the WASM stack slot.
        if (type->GetSize() == sizeof(uint32))
        {
            uint32_t raw;
            std::memcpy(&raw, val.GetData(), sizeof(uint32_t));
            *sp = static_cast<uint64_t>(raw);
        }
        else if (type->GetSize() == sizeof(uint64))
        {
            std::memcpy(sp, val.GetData(), sizeof(uint64_t));
        }
        break;
    }
    }
}


// Returns true when a type is a struct small enough to marshal over WASM.
bool Script::IsExpandableStruct(const TypeDescriptor& type)
{
    return !type.GetFields().empty() && type.GetSize() <= sizeof(std::byte[32]);
}

// Appends flattened WASM type chars for all fields of a struct.
void Script::AppendStructFieldChars(std::string& sig, const TypeDescriptor& desc)
{
    for (const auto& field : desc.GetFields() | std::views::values)
    {
        if (field.IsStatic() || field.IsScriptHidden())
        {
            continue;
        }

        const TypeDescriptor* type = field.GetType();
        assert(type);

        if (type == &desc)
        {
            continue;
        }

        char c;
        if (Script::ToWasmTypeChar(*type, c))
        {
            sig += c;
        }
        else if (IsExpandableStruct(*type))
        {
            AppendStructFieldChars(sig, *type);
        }
    }
}

// Read struct fields from the WASM stack into a Variant.
Variant Script::ReadStructArg(uint64_t*& sp, const TypeDescriptor& desc)
{
    Variant result(desc);
    void* resultData = result.GetData();

    for (const auto& field : desc.GetFields() | std::views::values)
    {
        if (field.GetType() == &desc || field.IsStatic() || field.IsScriptHidden())
        {
            continue;
        }

        const TypeDescriptor* type = field.GetType();
        assert(type);

        if (Script::IsSupportedWasmType(*type))
        {
            Variant fieldGv = Script::ReadWasmArg(sp, *type);
            field.Set(resultData, fieldGv.GetData());
        }
        else if (IsExpandableStruct(*type))
        {
            // Nested struct — read its fields recursively, then copy bytes into parent.
            Variant nested = ReadStructArg(sp, *type);
            field.Set(resultData, nested.GetData());
        }
    }
    return result;
}

std::string Script::BuildWasmSignature(const MethodDescriptor& fn)
{
    const TypeDescriptor* worldDesc = &TypeRegistry::Get<World>();
    const TypeDescriptor& returnType = *fn.GetReturnType();
    const bool returnIsStruct = Script::IsExpandableStruct(returnType);

    std::string sig;
    // Struct return → void return + leading 'i' sret pointer.
    sig += returnIsStruct ? 'v' : Script::ToWasmTypeChar(returnType);
    sig += '(';
    if (returnIsStruct)
    {
        sig += 'i';  // sret: pointer into WASM linear memory
    }

    for (const auto& param : fn.GetParams())
    {
        if (param.Type == worldDesc)
        {
            continue;  // world is injected — not a WASM argument
        }

        LogInfo("[DBG] {}  param={} id={} desc={} alias={} numFields={} isExpandable={}",
            fn.GetName(),
            param.Name,
            (hash32_t)param.Type->GetTypeId(),
            param.Type ? param.Type->GetName().c_str() : "null",
            param.Type ? param.Type->GetAlias().c_str() : "null",
            param.Type ? (int)param.Type->GetProperties().size() : -1,
            (int)Script::IsExpandableStruct(*param.Type));

        char c;
        if (Script::ToWasmTypeChar(*param.Type, c))
        {
            sig += c;
        }
        else if (Script::IsExpandableStruct(*param.Type))
        {
            Script::AppendStructFieldChars(sig, *param.Type);
        }
    }
    sig += ')';
    return sig;
}