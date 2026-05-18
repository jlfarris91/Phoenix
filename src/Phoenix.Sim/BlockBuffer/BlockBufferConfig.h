#pragma once

#include "PhoenixSim/BlockBuffer/BlockBufferDefinition.h"
#include "PhoenixSim/BlockBuffer/BlockBufferEnums.h"

namespace Phoenix
{
    // A configuration for a block buffer, which consists of a list of block definitions.
    struct PHOENIX_SIM_API BlockBufferConfig
    {
        BufferBlockDefinition& RegisterBlock(const BufferBlockDefinition& def)
        {
            Definitions.push_back(def);
            return Definitions.back();
        }

        template <class TBlock>
        BufferBlockDefinition& RegisterBlock(EBufferBlockType type)
        {
            const TypeDescriptor& typeDescriptor = TypeRegistry::Get<TBlock>();
            return Definitions.emplace_back(
                typeDescriptor.GetTypeId(),
                BlockBufferLayout::For<TBlock>(),
                (uint8)type,
                &typeDescriptor);
        }

        template <class TBlock, class ...TVars>
        BufferBlockDefinition& RegisterBlockWithAlloc(EBufferBlockType type, TVars&&... vars)
        {
            BufferBlockDefinition& registration = RegisterBlock<TBlock>(type);
            registration.LayoutFn.BindStatic(&TBlock::StaticLayout, std::forward<TVars>(vars)...);
            registration.ConstructFn.BindStatic(&TBlock::StaticConstruct, std::forward<TVars>(vars)...);
            return registration;
        }

        std::vector<BufferBlockDefinition> Definitions;
    };

    // A builder for a block buffer configuration, which provides a fluent interface for registering blocks and building the final layout.
    struct PHOENIX_SIM_API BlockBufferConfigBuilder
    {
        BufferBlockDefinition& RegisterBlock(const BufferBlockDefinition& definition)
        {
            return Config.RegisterBlock(definition);
        }

        template <class TBlock>
        BufferBlockDefinition& RegisterBlock(EBufferBlockType type)
        {
            return Config.RegisterBlock<TBlock>(type);
        }

        template <class TBlock, class ...TVars>
        BufferBlockDefinition& RegisterBlockWithAlloc(EBufferBlockType type, TVars&&... vars)
        {
            return Config.RegisterBlockWithAlloc<TBlock>(type, std::forward<TVars>(vars)...);
        }

        const BlockBufferConfig& GetLayout() const
        {
            return Config;
        }

    private:
        BlockBufferConfig Config;
    };
}
