#include "Resource.h"

void Phoenix::Renderer::Resource::SetId(uint32_t id)
{
    PHX_ASSERT(Id == 0);
    Id = id;
}

Phoenix::Renderer::HResource Phoenix::Renderer::Resource::GetHandle() const
{
    return { Id, GetResourceType() };
}
