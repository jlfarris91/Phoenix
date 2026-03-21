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
    for (const auto& [name, method] : desc.GetMethods())
    {
        if (method.ScriptNamespace.empty())
            continue;
        sol::table ns = GetOrCreateTable(method.ScriptNamespace);
        RegisterFunctionInTable(ns, method);
    }
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

    // Snapshot what we need from the descriptor before capturing in the lambda.
    MethodDescriptor fnCopy = fn;

    tbl[fn.Name] = [this, fnCopy, worldDesc](sol::variadic_args va) mutable -> sol::object
    {
        // Build the full args array, injecting World params from context.
        std::vector<GenericValue> args;
        args.reserve(fnCopy.Params.size());

        size_t luaArgIdx = 0;
        for (const auto& param : fnCopy.Params)
        {
            if (param.Type.IsStruct() && param.Type.Descriptor == worldDesc)
            {
                // WorldRef — inject from current context rather than the Lua arg list.
                GenericValue gv;
                gv.Type.Descriptor = worldDesc;
                gv.Ptr = m_currentWorld;
                args.push_back(gv);
            }
            else if (luaArgIdx < static_cast<size_t>(va.size()))
            {
                args.push_back(SolToGenericValue(va[luaArgIdx++], param.Type));
            }
            else
            {
                args.push_back(GenericValue::Void());
            }
        }

        GenericValue result = fnCopy.Execute(nullptr, args);
        return GenericValueToSol(result);
    };
}

GenericValue LuaRuntime::SolToGenericValue(const sol::object& obj, const ParamTypeRef& type)
{
    if (type.IsVoid())
        return GenericValue::Void();

    // Struct types other than World are not currently marshalled from Lua.
    if (type.IsStruct())
        return GenericValue::Void();

    switch (type.Primitive)
    {
        case EPropertyValueType::Bool:
            if (obj.is<bool>())
                return GenericConverter<bool>::Borrow(obj.as<bool>());
            break;

        case EPropertyValueType::Int8:
        case EPropertyValueType::Int16:
        case EPropertyValueType::Int32:
        case EPropertyValueType::Int64:
        case EPropertyValueType::UInt8:
        case EPropertyValueType::UInt16:
        case EPropertyValueType::UInt32:
        case EPropertyValueType::UInt64:
        case EPropertyValueType::Name:   // FName stored as integer hash
        {
            GenericValue gv;
            gv.Type = type;
            if (obj.is<lua_Integer>())
                gv.I = static_cast<int64_t>(obj.as<lua_Integer>());
            else if (obj.is<double>())
                gv.I = static_cast<int64_t>(obj.as<double>());
            return gv;
        }

        case EPropertyValueType::Float:
        case EPropertyValueType::Double:
        case EPropertyValueType::FixedPoint:
        {
            GenericValue gv;
            gv.Type = type;
            if (obj.is<double>())
                gv.N = obj.as<double>();
            else if (obj.is<lua_Integer>())
                gv.N = static_cast<double>(obj.as<lua_Integer>());
            return gv;
        }

        case EPropertyValueType::String:
            if (obj.is<std::string>())
                return GenericConverter<std::string>::Borrow(obj.as<std::string>());
            break;

        default:
            break;
    }

    return GenericValue::Void();
}

sol::object LuaRuntime::GenericValueToSol(const GenericValue& val) const
{
    if (val.Type.IsVoid())
        return sol::make_object(m_state, sol::nil);

    if (val.Type.IsStruct())
        return sol::make_object(m_state, sol::nil);  // struct returns not yet exposed to Lua

    switch (val.Type.Primitive)
    {
        case EPropertyValueType::Bool:
            return sol::make_object(m_state, val.B);

        case EPropertyValueType::Int8:
        case EPropertyValueType::Int16:
        case EPropertyValueType::Int32:
        case EPropertyValueType::Int64:
        case EPropertyValueType::UInt8:
        case EPropertyValueType::UInt16:
        case EPropertyValueType::UInt32:
        case EPropertyValueType::UInt64:
        case EPropertyValueType::Name:
            return sol::make_object(m_state, static_cast<lua_Integer>(val.I));

        case EPropertyValueType::Float:
        case EPropertyValueType::Double:
        case EPropertyValueType::FixedPoint:
            return sol::make_object(m_state, val.N);

        case EPropertyValueType::String:
            return sol::make_object(m_state, val.Str);

        default:
            return sol::make_object(m_state, sol::nil);
    }
}
