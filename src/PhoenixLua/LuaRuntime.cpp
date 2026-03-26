#include "PhoenixLua/LuaRuntime.h"

#include <iostream>
#include <sstream>

#include "PhoenixSim/Reflection/Reflection.h"
#include "PhoenixSim/Session.h"
#include "PhoenixSim/Worlds.h"

using namespace Phoenix;

// ── helpers ───────────────────────────────────────────────────────────────────

static std::vector<std::string> SplitDot(const std::string& path)
{
    std::vector<std::string> parts;
    std::istringstream ss(path);
    std::string part;
    while (std::getline(ss, part, '.'))
        if (!part.empty())
            parts.push_back(part);
    return parts;
}

// ── LuaRuntime ────────────────────────────────────────────────────────────────

LuaRuntime::LuaRuntime(sol::state& state, Session* session)
    : m_state(state)
    , m_session(session)
{
}

// ── IScriptRuntime ─────────────────────────────────────────────────────────────

void LuaRuntime::RegisterType(const TypeDescriptor& desc)
{
    const auto& meta = desc.GetMetadata();
    const auto nsIt  = meta.find("Namespace");
    if (nsIt == meta.end() || nsIt->second.empty())
        return;

    sol::table ns = GetOrCreateTable(nsIt->second);
    for (const auto& [name, method] : desc.GetMethods())
        RegisterFunctionInTable(ns, method);
}

void LuaRuntime::OpenNamespace(const char* dotSeparatedPath)
{
    m_namespaceStack.push_back(GetOrCreateTable(dotSeparatedPath));
}

void LuaRuntime::CloseNamespace()
{
    if (!m_namespaceStack.empty())
        m_namespaceStack.pop_back();
}

void LuaRuntime::RegisterFunction(const MethodDescriptor& fn)
{
    if (m_namespaceStack.empty())
    {
        std::cerr << "[LuaRuntime] RegisterFunction called outside OpenNamespace/CloseNamespace\n";
        return;
    }
    RegisterFunctionInTable(m_namespaceStack.back(), fn);
}

bool LuaRuntime::LoadFile(const char* path)
{
    auto script = m_state.load_file(path);
    if (!script.valid())
    {
        sol::error err = script;
        std::cerr << "Error loading script '" << path << "': " << err.what() << "\n";
        return false;
    }

    auto result = script();
    if (!result.valid())
    {
        sol::error err = result;
        std::cerr << "Error executing script '" << path << "': " << err.what() << "\n";
        return false;
    }
    return true;
}

bool LuaRuntime::ExecString(const std::string& code)
{
    auto result = m_state.safe_script(code, sol::script_pass_on_error);
    if (!result.valid())
    {
        sol::error err = result;
        std::cerr << "Lua script error: " << err.what() << "\n";
        return false;
    }
    return true;
}

void LuaRuntime::SetCurrentWorld(World* world)
{
    m_currentWorld = world;
}

void LuaRuntime::OnBindingsComplete()
{
    // Nothing needed for sol2; all tables are already live in the state.
}

// ── Private helpers ────────────────────────────────────────────────────────────

sol::table LuaRuntime::GetOrCreateTable(const std::string& dotPath)
{
    const auto parts = SplitDot(dotPath);
    if (parts.empty())
        return m_state.globals();

    sol::table current = m_state.globals();
    for (const auto& part : parts)
        current = current[part].get_or_create<sol::table>();
    return current;
}

void LuaRuntime::RegisterFunctionInTable(sol::table& tbl, const MethodDescriptor& fn)
{
    const TypeDescriptor* worldDesc = &World::GetStaticTypeDescriptor();

    MethodDescriptor fnCopy = fn;
    tbl[fn.Name] = [this, fnCopy, worldDesc](sol::variadic_args va) mutable -> sol::object
    {
        std::vector<GenericValue> args;
        args.reserve(fnCopy.Params.size());

        size_t luaIdx = 0;
        for (const auto& param : fnCopy.Params)
        {
            if (param.Type.Descriptor == worldDesc)
            {
                // World param: inject the current world, not read from Lua.
                args.push_back(GenericConverter<World>::Borrow(*m_currentWorld));
            }
            else
            {
                if (luaIdx < static_cast<size_t>(va.size()))
                    args.push_back(SolToGenericValue(va[luaIdx], param.Type));
                else
                    args.push_back(GenericValue::Void());
                ++luaIdx;
            }
        }

        GenericValue result = fnCopy.Execute(nullptr, args);
        return GenericValueToSol(result);
    };
}

GenericValue LuaRuntime::SolToGenericValue(const sol::object& obj, const GenericValueTypeRef& type)
{
    if (type.IsVoid() || type.IsStruct())
        return GenericValue::Void();

    const bool hasNum = obj.is<lua_Integer>() || obj.is<double>();
    const auto asInt  = [&]() -> lua_Integer { return obj.is<lua_Integer>() ? obj.as<lua_Integer>() : static_cast<lua_Integer>(obj.as<double>()); };
    const auto asReal = [&]() -> double      { return obj.is<double>() ? obj.as<double>() : static_cast<double>(obj.as<lua_Integer>()); };

    switch (type.Primitive)
    {
        case EGenericValueType::Bool:
            if (obj.is<bool>()) return GenericConverter<bool>::Borrow(obj.as<bool>());
            break;

        case EGenericValueType::Int8:
            if (hasNum) return GenericConverter<int8_t>::Borrow(static_cast<int8_t>(asInt()));
            break;
        case EGenericValueType::Int16:
            if (hasNum) return GenericConverter<int16_t>::Borrow(static_cast<int16_t>(asInt()));
            break;
        case EGenericValueType::Int32:
            if (hasNum) return GenericConverter<int32_t>::Borrow(static_cast<int32_t>(asInt()));
            break;
        case EGenericValueType::Int64:
            if (hasNum) return GenericConverter<int64_t>::Borrow(static_cast<int64_t>(asInt()));
            break;
        case EGenericValueType::UInt8:
            if (hasNum) return GenericConverter<uint8_t>::Borrow(static_cast<uint8_t>(asInt()));
            break;
        case EGenericValueType::UInt16:
            if (hasNum) return GenericConverter<uint16_t>::Borrow(static_cast<uint16_t>(asInt()));
            break;
        case EGenericValueType::UInt32:
            if (hasNum) return GenericConverter<uint32_t>::Borrow(static_cast<uint32_t>(asInt()));
            break;
        case EGenericValueType::UInt64:
            if (hasNum) return GenericConverter<uint64_t>::Borrow(static_cast<uint64_t>(asInt()));
            break;

        case EGenericValueType::Name:
            // Accept integer hash or string (hashed at registration time)
            if (obj.is<lua_Integer>())
                return GenericConverter<FName>::Borrow(FName(static_cast<hash32_t>(obj.as<lua_Integer>())));
            if (obj.is<std::string>())
                return GenericConverter<FName>::Borrow(FName(obj.as<std::string>()));
            break;

        case EGenericValueType::Float:
            if (hasNum) return GenericConverter<float>::Borrow(static_cast<float>(asReal()));
            break;
        case EGenericValueType::Double:
            if (hasNum) return GenericConverter<double>::Borrow(asReal());
            break;

        case EGenericValueType::String:
            if (obj.is<std::string>()) return GenericConverter<std::string>::Borrow(obj.as<std::string>());
            break;

        default:
            break;
    }

    return GenericValue::Void();
}

sol::object LuaRuntime::GenericValueToSol(const GenericValue& val) const
{
    if (val.Type.IsVoid() || val.Type.IsStruct())
        return sol::make_object(m_state, sol::nil);

    switch (val.Type.Primitive)
    {
        case EGenericValueType::Bool:
            return sol::make_object(m_state, val.As<bool>());
        case EGenericValueType::Int8:
            return sol::make_object(m_state, static_cast<lua_Integer>(val.As<int8_t>()));
        case EGenericValueType::Int16:
            return sol::make_object(m_state, static_cast<lua_Integer>(val.As<int16_t>()));
        case EGenericValueType::Int32:
            return sol::make_object(m_state, static_cast<lua_Integer>(val.As<int32_t>()));
        case EGenericValueType::Int64:
            return sol::make_object(m_state, static_cast<lua_Integer>(val.As<int64_t>()));
        case EGenericValueType::UInt8:
            return sol::make_object(m_state, static_cast<lua_Integer>(val.As<uint8_t>()));
        case EGenericValueType::UInt16:
            return sol::make_object(m_state, static_cast<lua_Integer>(val.As<uint16_t>()));
        case EGenericValueType::UInt32:
            return sol::make_object(m_state, static_cast<lua_Integer>(val.As<uint32_t>()));
        case EGenericValueType::UInt64:
            return sol::make_object(m_state, static_cast<lua_Integer>(val.As<uint64_t>()));
        case EGenericValueType::Name:
            return sol::make_object(m_state, static_cast<lua_Integer>(static_cast<hash32_t>(val.As<FName>())));
        case EGenericValueType::Float:
            return sol::make_object(m_state, static_cast<double>(val.As<float>()));
        case EGenericValueType::Double:
            return sol::make_object(m_state, val.As<double>());
        case EGenericValueType::String:
            return sol::make_object(m_state, val.As<std::string>());
        default:
            return sol::make_object(m_state, sol::nil);
    }
}
