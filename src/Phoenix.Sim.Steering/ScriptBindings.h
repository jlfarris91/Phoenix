#pragma once

#include "Phoenix.Sim/Reflection/Registration.h"
#include "Phoenix.Sim/Scripting/IScriptBindings.h"

namespace Phoenix::Steering
{
    class ScriptBindings : public IScriptBindings
    {
        PHX_DECLARE_TYPE_DERIVED(ScriptBindings, IScriptBindings)

    public:
        void Describe(ScriptModuleBuilder& builder) const override;
    };
}
