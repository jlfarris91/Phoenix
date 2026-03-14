# Block Buffers

PhoenixSim avoids heap fragmentation and cache misses by storing all simulation data in pre-allocated contiguous `BlockBuffer`s. Understanding this is important when adding persistent state to a feature.

---

## The Two Buffers

### Session buffer

Allocated once when the session is created. Holds session-scoped data — things that are global across all worlds and persist for the entire session lifetime.

Access from anywhere you have a `SessionRef` or `Session*`:
```cpp
auto& block = session.GetBlockRef<MyFeatureSessionBlock>();
```

### World buffer

Each `World` has its own buffer. Holds per-world simulation state: ECS entity storage, order queues, physics contacts, steering state, etc.

Access from anywhere you have a `WorldRef`:
```cpp
auto& block = world.GetBlockRef<MyFeatureWorldBlock>();
```

---

## Registering Blocks

Features declare what they need during the **layout phase**, before any buffer is allocated. This happens in `OnSessionLayout` and `OnWorldLayout`.

### Static block (fixed size, known at layout time)

```cpp
void MyFeature::OnWorldLayout(
    const Phoenix::WorldLayoutContext&,
    Phoenix::BlockBufferLayoutBuilder& builder)
{
    builder.RegisterBlock<MyStaticBlock>(Phoenix::EBufferBlockType::Static);
}
```

`MyStaticBlock` must be a concrete struct — its `sizeof` is the block size. The block is default-constructed in place.

### Dynamic block (needs constructor args, may grow)

```cpp
struct MyDynamicBlock : Phoenix::BufferBlockBase
{
    explicit MyDynamicBlock(int maxEntities)
        : Entities(maxEntities) {}

    Phoenix::FixedBuffer<MyState> Entities;
};

void MyFeature::OnWorldLayout(
    const Phoenix::WorldLayoutContext& ctx,
    Phoenix::BlockBufferLayoutBuilder& builder)
{
    builder.RegisterBlockWithAlloc<MyDynamicBlock>(
        Phoenix::EBufferBlockType::Dynamic,
        ctx.Config.MaxEntities);   // forwarded to MyDynamicBlock's constructor
}
```

### Scratch block (cleared every frame)

Use scratch blocks for data that is rebuilt each tick — sorted entity lists, contact pairs, anything you compute fresh and don't need to persist.

```cpp
builder.RegisterBlock<MyScratchBlock>(Phoenix::EBufferBlockType::Scratch);
```

The buffer system zeros scratch blocks at the start of each world update. Don't store anything in a scratch block that needs to survive across frames.

---

## Why Blocks Instead of Heap Allocations?

1. **No fragmentation** — all simulation memory is in two contiguous allocations (session buffer, world buffer). No alloc/free churn per entity or per event.
2. **Cache coherency** — related data is adjacent in memory. Iterating entities or events doesn't scatter through heap pages.
3. **Deterministic sizing** — you know the max capacity up front. If you're over budget, you find out at session creation, not mid-simulation.
4. **Zero-cost access** — `GetBlockRef<T>()` is a single offset lookup into the buffer pointer. No map lookups, no indirection.

---

## Sizing Blocks Correctly

Blocks are sized at creation. Overflowing a `FixedBuffer` or `FixedMap` in a dynamic block will assert in debug. Size based on your expected worst-case per world:

```cpp
// In your world config or feature constructor args:
int maxUnits    = 1000;
int maxAbilities = maxUnits * 4;   // average 4 abilities per unit
```

Pass these through the `WorldLayoutContext::Config` if your feature needs configurable limits (add config fields to `WorldConfig`), or hardcode reasonable maximums as feature-level constants if the numbers are stable.

---

## Containers Available in Blocks

All containers in `src/PhoenixSim/Containers/` are designed to live inside blocks:

| Container | Use case |
|---|---|
| `FixedArray<T, N>` | Fixed-capacity array, size known at compile time |
| `FixedBuffer<T>` | Dynamic-capacity array, size set at construction |
| `FixedMap<K, V, N>` | Hash map, fixed capacity |
| `FixedSet<T, N>` | Hash set, fixed capacity |
| `FixedQueue<T, N>` | FIFO queue |
| `FixedCircularBuffer<T, N>` | Ring buffer |
| `MPMCQueue<T>` | Multi-producer multi-consumer lock-free queue |

Do **not** put `std::vector`, `std::map`, or other heap-allocating containers in a block struct — they will allocate outside the buffer and defeat the purpose.
