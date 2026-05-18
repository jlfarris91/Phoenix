#include "MethodDescriptor.h"

#include "GenericFunction.h"

bool Phoenix::MethodDescriptor::CanExecute(void* self) const
{
    return true;
}

Phoenix::Variant Phoenix::MethodDescriptor::Execute(void* self, const std::span<const Variant>& args) const
{
    return Function(self, args);
}

const std::vector<Phoenix::ParamDescriptor>& Phoenix::MethodDescriptor::GetParams() const
{
    return Params;
}

const Phoenix::TypeDescriptor* Phoenix::MethodDescriptor::GetReturnType() const
{
    return ReturnType;
}

const Phoenix::GenericFunction& Phoenix::MethodDescriptor::GetFunction() const
{
    return Function;
}
