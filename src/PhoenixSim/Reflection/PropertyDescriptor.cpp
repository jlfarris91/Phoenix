#include "PropertyDescriptor.h"

#include "PropertyAccessor.h"

const Phoenix::TypeDescriptor* Phoenix::PropertyDescriptor::GetType() const
{
    return Type;
}

void Phoenix::PropertyDescriptor::Get(const void* obj, void* value, size_t len) const
{
    return Accessor->Get(obj, value, len);
}

void Phoenix::PropertyDescriptor::Set(void* obj, const void* value, size_t len) const
{
    Accessor->Set(obj, value, len);
}

void Phoenix::PropertyDescriptor::Get(const World& world, const void* obj, void* value, size_t len) const
{
    Accessor->Get(world, obj, value, len);
}

void Phoenix::PropertyDescriptor::Set(World& world, void* obj, const void* value, size_t len) const
{
    Accessor->Set(world, obj, value, len);
}
