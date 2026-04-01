#pragma once

#include <span>

#include "GenericFunction.h"
#include "PhoenixSim/Reflection/MemberDescriptor.h"
#include "PhoenixSim/Reflection/Variant.h"

namespace Phoenix
{
    class TypeDescriptor;
    class Variant;
    class GenericFunction;

    // ── ParamDescriptor ───────────────────────────────────────────────────────
    //
    // Describes one parameter (or the return value) of a MethodDescriptor.
    // Belongs to the reflection layer — GenericFunction itself carries no names.

    struct PHOENIX_SIM_API ParamDescriptor
    {
        std::string             Name;
        const TypeDescriptor*   Type = nullptr;
        bool                    IsOptional = false;
    };

    // ── MethodDescriptor ──────────────────────────────────────────────────────
    //
    // Full reflection descriptor for a method or function registered on a type.
    // Owns:
    //   • Params / Return  — parameter and return type metadata (for UI / docs /
    //                        marshaling by consumers such as IScriptRuntime)
    //   • Function         — slim type-erased callable (GenericFunction)
    //   • CanExecutePredicate — optional runtime gate (e.g. CanStartAbility())
    //
    // Constructor convention:
    //   Entries in TypeDescriptor::Constructors use HasSelfParam = true with
    //   'self' pointing at pre-allocated memory.  Params describes only the
    //   user-visible constructor arguments, not the memory pointer.

    class PHOENIX_SIM_API MethodDescriptor : public MemberDescriptor
    {
    public:

        bool CanExecute(void* self) const;

        Variant Execute(void* self = nullptr, const std::span<const Variant>& args = {}) const;

        const std::vector<ParamDescriptor>& GetParams() const;

        const TypeDescriptor* GetReturnType() const;

        const GenericFunction& GetFunction() const;

    private:

        template <class T>
        friend class TypeDescriptorBuilder;
        friend class TypeRegistry;

        std::vector<ParamDescriptor>    Params;
        const TypeDescriptor*           ReturnType;
        GenericFunction                 Function;
    };
}
