#pragma once

#include <string>
#include <vector>

#include "Phoenix.Sim/Name.h"
#include "Phoenix/Reflection/MethodDescriptor.h"
#include "Phoenix/Reflection/MethodDescriptorString.h"
#include "Phoenix/Reflection/TypeDescriptorBuilder.h"
#include "Phoenix/Reflection/TypeRegistry.h"
#include "Phoenix.Sim/Scripting/ScriptOptional.h"

namespace Phoenix
{
    // ── TypePack ──────────────────────────────────────────────────────────────
    //
    // Empty tag type that carries a list of C++ types for use with
    // ScriptClassBuilder::TemplateMethod().
    //
    // Usage:
    //   using BBTypes = TypePack<int32, EntityId, FName>;
    //   builder.Class<EntityId>("Phoenix.Entity")
    //       .TEMPLATE_METHOD("GetBlackboardValue(world, entity, key, defaultValue)",
    //                        &FeatureECS::GetBlackboardValue, BBTypes)
    //       .End();
    //
    // This registers one Lua method per type:
    //   GetBlackboardValue_int32, GetBlackboardValue_EntityId, ...

    template<class... TTypes>
    struct TypePack {};

    // Returns a script-safe suffix for T: alias if registered, otherwise the
    // last :: component of the C++ type name with non-alphanumerics stripped.
    template<class T>
    std::string ScriptTypeSuffix()
    {
        const TypeDescriptor& td = TypeRegistry::Get<T>();
        const std::string& alias = td.GetAlias();
        if (!alias.empty())
            return alias;
        const std::string& name = td.GetName();
        const size_t colon = name.rfind(':');
        std::string base = (colon != std::string::npos) ? name.substr(colon + 1) : name;
        for (char& c : base)
            if (!std::isalnum(static_cast<unsigned char>(c))) c = '_';
        while (!base.empty() && base.back() == '_') base.pop_back();
        return base;
    }

    // ── DeduceFnReturnType ────────────────────────────────────────────────────
    //
    // Deduces the return type of a function pointer via overload resolution.
    // Usage:  using R = decltype(DeduceFnReturnType(fn));

    template<class TRet, class... TArgs>
    TRet DeduceFnReturnType(TRet(*)(TArgs...));

    // ── StringLiteral ─────────────────────────────────────────────────────────
    //
    // A structural class-type non-type template parameter (C++20) that stores a
    // string literal.  Used to pass declaration strings through TemplateMethod /
    // TemplateFunction so they arrive as compile-time constants at the point where
    // TArgs... is known, enabling MethodDeclarationString validation.
    //
    // Usage:  .TemplateMethod<"GetValue(world, entity, key)">(TypePack<...>{}, factory)
    //   or via the TEMPLATE_METHOD / TEMPLATE_FUNCTION macros.

    template<size_t N>
    struct StringLiteral
    {
        char data[N];
        consteval StringLiteral(const char (&s)[N])
        {
            for (size_t i = 0; i < N; ++i) data[i] = s[i];
        }
    };

    // ── BuildTemplateDescriptor ───────────────────────────────────────────────
    //
    // Shared helper used by both ScriptNamespaceBuilder and ScriptClassBuilder.
    // Constructs a MethodDescriptor from a compile-time declaration string (NTTP)
    // at the point where TArgs... is known, appending "_<suffix>" to the name.

    template<StringLiteral Decl, class TRet, class... TArgs, size_t... I>
    static MethodDescriptor BuildTemplateDescriptorImpl(
        TRet(*fn)(TArgs...), const std::string& suffix, std::index_sequence<I...>)
    {
        constexpr MethodDeclarationString<TArgs...> decl(Decl.data);
        MethodDescriptor d;
        d.Function   = MakeGenericFunction(fn);
        d.Name       = std::string(decl.Name) + "_" + suffix;
        d.ReturnType = &TypeRegistry::Get<TRet>();
        d.Params     = { ParamDescriptor{ std::string(decl.ParamNames[I]), &TypeRegistry::Get<TArgs>() }... };
        return d;
    }

    template<StringLiteral Decl, class TRet, class... TArgs>
    static MethodDescriptor BuildTemplateDescriptor(TRet(*fn)(TArgs...), const std::string& suffix)
    {
        return BuildTemplateDescriptorImpl<Decl>(fn, suffix, std::index_sequence_for<TArgs...>{});
    }

    // ── ScriptFunctionEntry ───────────────────────────────────────────────────

    struct ScriptFunctionEntry
    {
        std::string      Namespace;
        MethodDescriptor Method;
    };

    // ── ScriptClassEntry ──────────────────────────────────────────────────────

    struct ScriptClassEntry
    {
        std::string              Namespace;
        std::string              BaseNamespace;
        FName                    HandleTypeId;
        std::vector<MethodDescriptor> Methods;
        std::vector<MethodDescriptor> Statics;
    };

    // ── ScriptNamespaceBuilder ────────────────────────────────────────────────

    class ScriptModuleBuilder;

    class ScriptNamespaceBuilder
    {
    public:
        ScriptNamespaceBuilder(ScriptModuleBuilder& module, std::string ns)
            : Module(module), Ns(std::move(ns))
        {}

        // Register a free function under this namespace.
        template <class TRet, class... TArgs>
        ScriptNamespaceBuilder& Function(
            const std::type_identity_t<MethodDeclarationString<TArgs...>>& name,
            TRet(*fn)(TArgs...))
        {
            ScriptFunctionEntry entry;
            entry.Namespace = Ns;
            entry.Method    = BuildMethod(name, fn);
            SetFlagRef(entry.Method.Flags, EMemberDescriptorFlags::Static);
            Emit(std::move(entry));
            return *this;
        }

        // Register one free-function variant per type in TypePack.
        // The declaration string is validated at compile time against each
        // variant's actual argument types — same as Function().
        // Method name = baseName + "_" + TypeDescriptor alias.
        template<StringLiteral Decl, class... TTypes, class TFactory>
        ScriptNamespaceBuilder& TemplateFunction(TypePack<TTypes...>, TFactory factory)
        {
            (RegisterTemplateFunctionVariant<TTypes, Decl>(factory), ...);
            return *this;
        }

        ScriptModuleBuilder& End() const { return Module; }

    private:
        template<class T, StringLiteral Decl, class TFactory>
        void RegisterTemplateFunctionVariant(TFactory& factory)
        {
            auto fn = factory.template operator()<T>();

            using RetType = decltype(DeduceFnReturnType(fn));
            if constexpr (IsScriptOptional<RetType>::value)
                EnsureScriptOptionalRegistered<typename IsScriptOptional<RetType>::Inner>(
                    ScriptTypeSuffix<typename IsScriptOptional<RetType>::Inner>());

            ScriptFunctionEntry entry;
            entry.Namespace = Ns;
            entry.Method    = BuildTemplateDescriptor<Decl>(fn, ScriptTypeSuffix<T>());
            SetFlagRef(entry.Method.Flags, EMemberDescriptorFlags::Static);
            Emit(std::move(entry));
        }

        template <class TRet, class... TArgs, size_t... I>
        static MethodDescriptor FillParams(
            MethodDescriptor& d,
            const std::type_identity_t<MethodDeclarationString<TArgs...>>& decl,
            std::index_sequence<I...>)
        {
            d.Name       = std::string(decl.Name);
            d.Params     = { ParamDescriptor{ std::string(decl.ParamNames[I]), &TypeRegistry::Get<TArgs>() }... };
            d.ReturnType = &TypeRegistry::Get<TRet>();
            return d;
        }

        template <class TRet, class... TArgs>
        static MethodDescriptor BuildMethod(
            const std::type_identity_t<MethodDeclarationString<TArgs...>>& decl,
            TRet(*fn)(TArgs...))
        {
            MethodDescriptor d;
            d.Function = MakeGenericFunction(fn);
            FillParams<TRet, TArgs...>(d, decl, std::index_sequence_for<TArgs...>{});
            return d;
        }

        void Emit(ScriptFunctionEntry entry) const;

        ScriptModuleBuilder& Module;
        std::string          Ns;
    };

    // ── ScriptClassBuilder ────────────────────────────────────────────────────

    template <class THandle>
    class ScriptClassBuilder
    {
    public:
        ScriptClassBuilder(ScriptModuleBuilder& module, std::string ns);

        template <class TRet, class... TArgs>
        ScriptClassBuilder& Method(
            const std::type_identity_t<MethodDeclarationString<TArgs...>>& name,
            TRet(*fn)(TArgs...))
        {
            MethodDescriptor d;
            d.Function = MakeGenericFunction(fn);
            FillParams<TRet, TArgs...>(d, name, std::index_sequence_for<TArgs...>{});
            Entry->Methods.push_back(std::move(d));
            return *this;
        }

        template <class TRet, class... TArgs>
        ScriptClassBuilder& StaticMethod(
            const std::type_identity_t<MethodDeclarationString<TArgs...>>& name,
            TRet(*fn)(TArgs...))
        {
            MethodDescriptor d;
            d.Function = MakeGenericFunction(fn);
            FillParams<TRet, TArgs...>(d, name, std::index_sequence_for<TArgs...>{});
            SetFlagRef(d.Flags, EMemberDescriptorFlags::Static);
            Entry->Statics.push_back(std::move(d));
            return *this;
        }

        ScriptClassBuilder& Inherits(const char* baseNamespace)
        {
            Entry->BaseNamespace = baseNamespace;
            return *this;
        }

        // Register one instance-method variant per type in TypePack.
        // The declaration string is validated at compile time against each
        // variant's actual argument types — same as Method().
        // Method name = baseName + "_" + TypeDescriptor alias.
        template<StringLiteral Decl, class... TTypes, class TFactory>
        ScriptClassBuilder& TemplateMethod(TypePack<TTypes...>, TFactory factory)
        {
            (RegisterTemplateVariant<TTypes, Decl>(factory, false), ...);
            return *this;
        }

        template<StringLiteral Decl, class... TTypes, class TFactory>
        ScriptClassBuilder& TemplateStaticMethod(TypePack<TTypes...>, TFactory factory)
        {
            (RegisterTemplateVariant<TTypes, Decl>(factory, true), ...);
            return *this;
        }

        ScriptModuleBuilder& End() const { return Module; }

    private:
        template <class TRet, class... TArgs, size_t... I>
        static void FillParams(
            MethodDescriptor& d,
            const std::type_identity_t<MethodDeclarationString<TArgs...>>& decl,
            std::index_sequence<I...>)
        {
            d.Name       = std::string(decl.Name);
            d.Params     = { ParamDescriptor{ std::string(decl.ParamNames[I]), &TypeRegistry::Get<TArgs>() }... };
            d.ReturnType = &TypeRegistry::Get<TRet>();
        }

        template<class T, StringLiteral Decl, class TFactory>
        void RegisterTemplateVariant(TFactory& factory, bool isStatic)
        {
            auto fn = factory.template operator()<T>();

            using RetType = decltype(DeduceFnReturnType(fn));
            if constexpr (IsScriptOptional<RetType>::value)
            {
                EnsureScriptOptionalRegistered<typename IsScriptOptional<RetType>::Inner>(
                    ScriptTypeSuffix<typename IsScriptOptional<RetType>::Inner>());
            }

            MethodDescriptor d = BuildTemplateDescriptor<Decl>(fn, ScriptTypeSuffix<T>());
            if (isStatic)
            {
                SetFlagRef(d.Flags, EMemberDescriptorFlags::Static);
                Entry->Statics.push_back(std::move(d));
            }
            else
            {
                Entry->Methods.push_back(std::move(d));
            }
        }

        ScriptModuleBuilder& Module;
        ScriptClassEntry*    Entry;
    };

    // ── ScriptModuleBuilder ───────────────────────────────────────────────────

    class PHOENIX_SIM_API ScriptModuleBuilder
    {
    public:
        ScriptNamespaceBuilder Namespace(const char* ns)
        {
            return ScriptNamespaceBuilder(*this, ns);
        }

        template <class THandle>
        ScriptClassBuilder<THandle> Class(const char* ns)
        {
            return ScriptClassBuilder<THandle>(*this, ns);
        }

        void AddFunction(ScriptFunctionEntry entry)  { Functions.push_back(std::move(entry)); }
        void AddClass   (ScriptClassEntry    entry)  { Classes  .push_back(std::move(entry)); }

        ScriptClassEntry* FindClass(const std::string& ns)
        {
            for (ScriptClassEntry& entry : Classes)
                if (entry.Namespace == ns) return &entry;
            return nullptr;
        }

        const std::vector<ScriptFunctionEntry>& GetFunctions() const { return Functions; }
        const std::vector<ScriptClassEntry>&    GetClasses()   const { return Classes; }

    private:
        template <class T>
        friend class ScriptClassBuilder;

        std::vector<ScriptFunctionEntry> Functions;
        std::vector<ScriptClassEntry>    Classes;
    };

    // ── Out-of-line definitions ───────────────────────────────────────────────

    inline void ScriptNamespaceBuilder::Emit(ScriptFunctionEntry entry) const
    {
        Module.AddFunction(std::move(entry));
    }

    template <class THandle>
    ScriptClassBuilder<THandle>::ScriptClassBuilder(ScriptModuleBuilder& module, std::string ns)
        : Module(module)
    {
        Entry = Module.FindClass(ns);
        if (!Entry)
            Entry = &Module.Classes.emplace_back(std::move(ns));

        assert(FName::IsNoneOrEmpty(Entry->HandleTypeId) || Entry->HandleTypeId == StaticTypeName<THandle>::TypeId);
        Entry->HandleTypeId = StaticTypeName<THandle>::TypeId;
    }

} // namespace Phoenix

// ── Convenience macros ────────────────────────────────────────────────────────
//
// Pass the declaration string as a class NTTP so it arrives as a compile-time
// constant where TArgs... is known, enabling MethodDeclarationString validation.

#define TEMPLATE_FUNCTION(decl, fn, typePack) \
    TemplateFunction<decl>(typePack, []<class T>(){ return fn<T>; })

#define TEMPLATE_METHOD(decl, fn, typePack) \
    TemplateMethod<decl>(typePack, []<class T>(){ return fn<T>; })
