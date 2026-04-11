#include "PropertyDescriptor.h"

#include "PropertyAccessor.h"

const Phoenix::TypeDescriptor* Phoenix::PropertyDescriptor::GetType() const
{
    return Type;
}

void Phoenix::PropertyDescriptor::Get(const void* obj, void* value) const
{
    return Accessor->Get(obj, value);
}

void Phoenix::PropertyDescriptor::Set(void* obj, const void* value) const
{
    Accessor->Set(obj, value);
}

void Phoenix::PropertyDescriptor::Get(const World& world, const void* obj, void* value) const
{
    Accessor->Get(world, obj, value);
}

void Phoenix::PropertyDescriptor::Set(World& world, void* obj, void* value) const
{
    Accessor->Set(world, obj, value);
}

void Phoenix::PropertyDescriptor::Set(World& world, void* obj, const void* value) const
{
    Accessor->Set(world, obj, value);
}
