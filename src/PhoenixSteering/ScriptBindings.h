#pragma once

#include "PhoenixSim/Reflection/Registration.h"
#include "PhoenixSim/Scripting/IScriptBindings.h"

namespace Phoenix::Steering
{
    class ScriptBindings : public IScriptBindings
    {
        PHX_DECLARE_TYPE_DERIVED(ScriptBindings, IScriptBindings)

    public:
        void Describe(ScriptModuleBuilder& builder) const override;
    };
}
