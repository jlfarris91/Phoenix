#pragma once

#include "Phoenix.Sim/BlockBuffer/BlockBufferAllocator.h"
#include "Phoenix.Sim/BlockBuffer/BlockBufferBlock.h"
#include "Phoenix.Sim/BlockBuffer/BlockBufferConfig.h"
#include "Phoenix.Sim/BlockBuffer/BlockBufferLayout.h"

#define PHX_BLOCK_BUFFER_COMMON(type) \
    struct Config; \
    void Construct(Phoenix::BlockBufferAllocator& allocator, const Config& config); \
    template <class ...TArgs> \
        requires (!std::is_same_v<std::tuple<std::decay_t<TArgs>...>, std::tuple<Config>>) \
    void Construct(Phoenix::BlockBufferAllocator& allocator, TArgs&& ...args) \
    { \
        Construct(allocator, Config { std::forward<TArgs>(args)... }); \
    } \
    static void StaticConstruct(void *dest, Phoenix::BlockBufferAllocator& allocator, const Config& config) \
    { \
        static_cast<type*>(dest)->Construct(allocator, config); \
    } \
    static BlockBufferLayout StaticLayout(const Config& config); \
    struct Config

#define PHX_DECLARE_BLOCK(block) \
    PHX_DECLARE_TYPE(block, Phoenix::BlockBufferBlock)

#define PHX_DECLARE_BLOCK_WITH_ALLOC(block) \
    PHX_DECLARE_BLOCK(block) \
    PHX_BLOCK_BUFFER_COMMON(block)

#define PHX_DECLARE_BLOCK_CONTAINER(container) \
    PHX_BLOCK_BUFFER_COMMON(container)
