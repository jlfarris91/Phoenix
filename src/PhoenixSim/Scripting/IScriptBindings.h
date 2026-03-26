#pragma once

#include "PhoenixSim/Services/Service.h"

namespace Phoenix
{
    class IScriptRuntime;

    // ── IScriptBindings ───────────────────────────────────────────────────────
    //
    // Manual escape hatch for script bindings that cannot be expressed
    // declaratively via PHX_SCRIPT_REGISTRATION.
    //
    // Examples of cases that benefit from manual bindings:
    //  • Functions whose C++ signatures include types not covered by GenericConverter.
    //  • Lambdas that capture session or feature state.
    //  • Complex multi-step registration logic.
    //
    // Register as IScriptBindings via ServiceContainerBuilder:
    //
    //   builder.RegisterService<MyBindings>().As<IScriptBindings>();
    //
    // FeatureLua discovers all IScriptBindings services at session init and calls
    // Register() after the declarative registry has been processed.

    class IScriptBindings : public IService
    {
        PHX_DECLARE_TYPE(IScriptBindings, IService)

    public:
        // Dot-separated namespace this class registers into (e.g. "Phoenix.Unit").
        // The runtime opens this namespace before calling Register and closes it after.
        virtual const char* GetNamespace() const = 0;

        // Called once at session init. Implementations call runtime.RegisterFunction(...).
        virtual void Register(IScriptRuntime& runtime) = 0;
    };
}
