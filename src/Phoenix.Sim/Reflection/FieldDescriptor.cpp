#include "FieldDescriptor.h"

#include "FieldAccessor.h"

const Phoenix::TypeDescriptor* Phoenix::FieldDescriptor::GetType() const
{
    return Type;
}

Phoenix::uint32 Phoenix::FieldDescriptor::GetOffset() const
{
    return Accessor->GetOffset();
}

bool Phoenix::FieldDescriptor::GetPointer(void* object, void** outValue) const
{
    return Accessor->GetPtr(object, outValue);
}

bool Phoenix::FieldDescriptor::GetPointer(const void* object, const void** outValue) const
{
    return Accessor->GetPtr(object, outValue);
}

void Phoenix::FieldDescriptor::Get(const void* obj, void* value) const
{
    return Accessor->Get(obj, value);
}

void Phoenix::FieldDescriptor::Set(void* obj, void* value) const
{
    return Accessor->Set(obj, value);
}

void Phoenix::FieldDescriptor::Set(void* obj, const void* value) const
{
    return Accessor->Set(obj, value);
}
