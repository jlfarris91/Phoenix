#include "Variant.h"

#include "TypeRegistry.h"

Phoenix::Variant::Variant() noexcept
{
    new(&Buffer) std::byte[40]{};
}

Phoenix::Variant::~Variant() noexcept
{
    DestructActive();
}

Phoenix::Variant::Variant(const TypeDescriptor& desc)
    : TypeId(desc.GetTypeId())
    , Flags(EVariantFlags::OwnsData)
    , Size(desc.GetSize())
{
    assert(desc.GetSize() < sizeof(Buffer));
    desc.DefaultConstruct(Buffer);
}

Phoenix::Variant::Variant(const Variant& other)
    : TypeId(other.TypeId)
    , Flags(other.Flags)
    , Size(other.Size)
{
    std::memcpy(Buffer, other.Buffer, sizeof(Buffer));
}

Phoenix::Variant::Variant(Variant&& other) noexcept
    : TypeId(other.TypeId)
    , Flags(other.Flags)
    , Size(other.Size)
{
    std::memcpy(Buffer, other.Buffer, sizeof(Buffer));
}

Phoenix::Variant Phoenix::Variant::Void()
{
    return {};
}

Phoenix::Variant& Phoenix::Variant::operator=(const Variant& other)
{
    if (this != &other)
    {
        DestructActive();
        TypeId = other.TypeId;
        Flags = other.Flags;
        Size = other.Size;
        std::memcpy(Buffer, other.Buffer, sizeof(Buffer));
    }
    return *this;
}

Phoenix::Variant& Phoenix::Variant::operator=(Variant&& other) noexcept
{
    if (this != &other)
    {
        DestructActive();
        TypeId = other.TypeId;
        Flags = other.Flags;
        Size = other.Size;
        std::memcpy(Buffer, other.Buffer, sizeof(Buffer));
    }
    return *this;
}

bool Phoenix::Variant::operator==(const Variant& other) const
{
    return TypeId == other.TypeId &&
           Flags == other.Flags &&
           Size == other.Size &&
           std::memcmp(Buffer, other.Buffer, Size) == 0;
}

bool Phoenix::Variant::operator!=(const Variant& other) const
{
    return !(*this == other);
}

bool Phoenix::Variant::OwnsData() const
{
    return HasAnyFlags(Flags, EVariantFlags::OwnsData);
}

bool Phoenix::Variant::IsRef() const
{
    return HasAnyFlags(Flags, EVariantFlags::Ref);
}

bool Phoenix::Variant::IsConstRef() const
{
    return HasAllFlags(Flags, EVariantFlags::ConstRef);
}

Phoenix::FName Phoenix::Variant::GetTypeId() const
{
    return TypeId;
}

const Phoenix::TypeDescriptor* Phoenix::Variant::GetType() const
{
    return TypeRegistry::Get(TypeId);
}

void* Phoenix::Variant::GetData()
{
    return &Buffer[0];
}

const void* Phoenix::Variant::GetData() const
{
    return &Buffer[0];
}

Phoenix::uint32 Phoenix::Variant::GetSize() const
{
    return Size;
}

void Phoenix::Variant::DestructActive()
{
    if (!OwnsData())
    {
        return;
    }

    if (auto descriptor = TypeRegistry::Get(TypeId))
    {
        descriptor->Destruct(&Buffer[0]);
    }
}
