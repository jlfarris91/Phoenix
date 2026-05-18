#pragma once

#include "Phoenix/Delegates.h"

#include "Phoenix.Sim/BlockBuffer/BlockBufferAllocator.h"
#include "Phoenix.Sim/BlockBuffer/BlockBufferLayout.h"

namespace Phoenix
{
    class TypeDescriptor;

    PHX_DECLARE_DELEGATE_RET(FBufferBlockLayout, BlockBufferLayout);
    PHX_DECLARE_DELEGATE(FBufferBlockConstruct, void*, BlockBufferAllocator& allocator);

    // A definition for a block in a block buffer, which includes the block's type name, layout, sort order,
    // and optional factory delegates for layout and construction.
    struct PHOENIX_SIM_API BufferBlockDefinition
    {
        // The type name of the block.
        FName TypeName;

        // The size of the block structure.
        BlockBufferLayout Layout;

        // The sort order of the block.
        uint8 SortOrder = 0;

        // A pointer to the block's type descriptor.
        const TypeDescriptor* Type = nullptr;

        // An optional factory delegate to control construction.
        FBufferBlockLayout LayoutFn;

        // An optional factory delegate to control construction.
        FBufferBlockConstruct ConstructFn;
    };
}