# World View Sync

The render thread needs a stable snapshot of simulation state to read while the sim thread advances to the next frame. This doc explores the problem, explains why the naive approaches fail, and documents the strategy we landed on.

---

## The Problem

PhoenixSim allocates world buffers at worst-case capacity. A world configured for 32k entities allocates enough memory for 32k entities — even if a typical match has 1k. This sparsity is intentional and correct: it eliminates mid-session allocation, avoids fragmentation, and keeps the block buffer model simple.

The consequence: the world buffer you need to hand to the render thread is large. For a mid-sized RTS configuration, the world buffer can be 50–150MB. The render thread must read through the same `GetBlockRef<T>()` APIs as the sim thread (requirement: the view must have the same shape as the live world, so all existing const APIs work without a second implementation).

At a sync point, the sim thread must produce a snapshot of that buffer while adding as little latency as possible. The target is **under 2ms of blocking time**.

---

## Why the Obvious Approaches Don't Work

### Full memcpy

The first instinct: `memcpy(view, sim, bufferSize)`. This is simple and correct. On hardware with DDR5 memory bandwidth (~40–80 GB/s), a 100MB copy takes roughly 2–4ms. That's right at or above budget, and it only gets worse as world capacity grows.

Parallelizing the memcpy with chunked workers doesn't help meaningfully. Memory bandwidth is the bottleneck — adding threads just adds scheduling overhead and contention on the same memory bus. Measured results: marginally *worse* than sequential memcpy.

### memcmp + memcpy to skip unchanged regions

Chunk the buffer into regions, compare each region first, skip the copy if unchanged. This sounds clever and fails for a specific reason: **memcmp reads bytes from both the source and destination**. You're now doing three passes over the unchanged data (one read from src, one read from dst to compare, then nothing) versus two passes for unchanged data in plain memcpy (one read from src, one write to dst). The total memory bandwidth consumed is higher, not lower.

Measured results: consistently slower than plain memcpy.

### Per-feature sparse copy

Have each feature write only its live data into the view (e.g., ECS only writes the 1k active entities rather than the 32k-capacity block). Smaller scenes improved, larger scenes got significantly worse. The overhead of the per-feature copy machinery + the loss of contiguous memory access patterns hurt more than the reduced byte count helped. Also, this approach breaks the "same shape" requirement unless the view is carefully constructed to match the live world's block offsets — which is complex.

### Explicit dirty marking

Add dirty flags to blocks or regions. Features mark themselves dirty when they write. At sync time, copy only dirty regions. This is semantically correct and architecturally appealing, but in practice it bleeds into every container and every write path. The developer experience degrades badly: it's easy to forget a dirty mark, and debugging incorrect renders due to stale view data is painful. Rejected for DX reasons.

---

## The Core Insight

All the approaches above share the same failure mode: **they touch all the bytes**. Either to copy them, or to compare them and decide not to copy them.

The only way to get under 2ms is to copy fewer bytes. For a sparse simulation, only a small fraction of the buffer's pages are actually written each frame. If we could know *which pages* were written without reading the whole buffer, we could copy just those.

It turns out the CPU's MMU already tracks this — for free, in hardware. Every page table entry has a dirty bit. The OS sets it when any write occurs to that page. No per-write software overhead, no instrumentation, no developer discipline required.

---

## The Strategy

### Windows: `MEM_WRITE_WATCH`

Windows exposes hardware dirty-page tracking through the `MEM_WRITE_WATCH` flag on `VirtualAlloc`. When a buffer is allocated with this flag, the kernel tracks which 4KB pages have been written. `GetWriteWatch` returns the list of dirty page addresses since the last reset. `WRITE_WATCH_FLAG_RESET` atomically gets and clears the list in a single call, eliminating any race between get and reset.

**Allocation change** — replace `_aligned_malloc` with `VirtualAlloc`:

```cpp
uint8* data = static_cast<uint8*>(VirtualAlloc(
    nullptr, size,
    MEM_RESERVE | MEM_COMMIT | MEM_WRITE_WATCH,
    PAGE_READWRITE));
```

**Deleter change** — replace `_aligned_free` with `VirtualFree`:

```cpp
VirtualFree(p, 0, MEM_RELEASE);
```

**Sync** — `SyncTo` replaces `CopyTo` at the sim→render handoff:

```cpp
void BlockBuffer::SyncTo(BlockBuffer& view) const
{
    if (view.Size != Size)
    {
        // First sync: view is uninitialized, full copy to establish shape and data.
        CopyTo(view);
        ResetWriteWatch(Data.get(), Size);
        return;
    }

    ULONG_PTR pageCount = (Size / pageSize) + 1;
    // Use thread_local to avoid a heap allocation per sync.
    thread_local std::vector<void*> dirtyPages;
    dirtyPages.resize(pageCount);

    DWORD pageSizeBytes;
    GetWriteWatch(
        WRITE_WATCH_FLAG_RESET,  // atomic get-and-reset
        Data.get(), Size,
        dirtyPages.data(), &pageCount,
        &pageSizeBytes);

    const uint8* src = Data.get();
    uint8* dst = view.Data.get();

    for (ULONG_PTR i = 0; i < pageCount; ++i)
    {
        size_t offset = static_cast<uint8*>(dirtyPages[i]) - src;
        memcpy(dst + offset, src + offset, pageSizeBytes);
    }
}
```

**Correctness argument**: after the initial `CopyTo`, the view is a byte-for-byte copy of the sim buffer. On every subsequent `SyncTo`:

- Pages the sim wrote since the last sync → copied into view → view is current.
- Pages the sim did not write → not in the dirty list → view retains the value from the previous sync → which is correct, because those bytes didn't change.

The view is always a complete, coherent snapshot of the sim's last committed frame. No dirty page is ever missed because `WRITE_WATCH_FLAG_RESET` is atomic.

**Performance**: for a world with 1k active entities out of 32k capacity, only the pages containing live entity data, active physics bodies, live order queues, etc. are dirty. Typically 2–5% of the total buffer for a sparse simulation. Copying 2MB instead of 100MB takes ~0.05ms rather than ~4ms.

For a dense simulation (32k active entities), most pages are dirty and performance approaches plain memcpy — graceful degradation, no regression.

---

### Linux ≥ 5.18: Soft-Dirty Bits via `pagemap`

Linux maintains a "soft-dirty" bit in each page table entry alongside the hardware dirty bit. Unlike the hardware dirty bit (which the kernel clears for its own purposes), the soft-dirty bit is preserved until explicitly cleared by userspace. This makes it usable as a write-tracking mechanism.

**The mechanism:**

```
madvise(buffer, size, MADV_CLEAR_SOFT_DIRTY)   // clear soft-dirty bits for our buffer only
... sim frame ...
// read /proc/self/pagemap, 8 bytes per page, bit 55 = soft-dirty
pread(pagemap_fd, entries, (size / pageSize) * 8, (bufferVA / pageSize) * 8)
// copy pages where bit 55 is set
madvise(buffer, size, MADV_CLEAR_SOFT_DIRTY)   // reset for next frame
```

`MADV_CLEAR_SOFT_DIRTY` clears bits for a *specific address range* without touching the rest of the process. This is the key: earlier interfaces (writing `4` to `/proc/self/clear_refs`) cleared soft-dirty bits process-wide, which is unsafe for a library and would interfere with other address ranges. `MADV_CLEAR_SOFT_DIRTY` was added in Linux 5.18 (May 2022) precisely to fix this.

**pagemap read cost**: for a 100MB buffer at 4KB pages, the pagemap read is 25,600 entries × 8 bytes = 200KB. A sequential `pread` of 200KB is fast (sub-millisecond on any modern storage + kernel path). Parsing bit 55 of each entry is trivial.

The zero-per-write overhead property is the same as Windows: the CPU's MMU sets the soft-dirty bit in hardware when a page is written. No software trap, no signal handler.

**Requires Linux 5.18+.** For older kernels, fall back to block-level dirty marking (below).

---

### Linux < 5.18 / Cross-Platform: Block-Level Dirty Marking

When page-level hardware tracking isn't available, the next best option is to track dirtiness at block granularity. The existing API already expresses read vs. write intent through const vs. non-const `GetBlock`:

- `GetBlockRef<T>()` (non-const) → caller may write → mark dirty
- `GetBlockRef<T>() const` → caller only reads → no mark needed

Add a `mutable bool Dirty = false` to `BlockBuffer::Block`. Set it in the non-const `GetBlock(FName)` path. In `SyncTo`, skip blocks that aren't dirty and copy struct + alloc region for blocks that are.

**The limitation**: this works at block granularity, not page granularity. If the ECS block has an 80MB alloc region (32k entity capacity) but only 2.5MB is live (1k active entities), the entire block gets copied whenever any feature accesses it non-const — which is every frame. Block-level dirty marking helps when *entire features are idle* (no active projectiles → projectile block is clean), but it does not capture within-block sparsity.

This is still worth implementing as the fallback. A projectile block, an ability block, and an effects block can all be clean in a quiescent simulation, and skipping them is free.

---

### Emscripten: Accept Full Copy

WebAssembly runs in a sandboxed linear memory model with no virtual memory control. There is no `mprotect`, no `userfaultfd`, no pagemap, no equivalent mechanism. World buffer tracking isn't possible at the OS level.

Two mitigating factors:

1. **World sizes are constrained by the browser target.** A WebAssembly game can't configure 32k entities with full physics — the memory limits and performance budget of a browser tab are much smaller. The buffer being copied is likely 5–20MB, not 100MB.

2. **Threading is constrained.** WebAssembly threads require `SharedArrayBuffer` and cross-origin isolation headers. Many deployments are effectively single-threaded. In a single-threaded build, the render "copy" is a simple swap of which buffer the renderer reads from, with no concurrent access to race against.

Use `CopyTo` (plain memcpy) and block-level dirty marking as a partial improvement. If memory allows, the async overlap pattern (described below) is applicable.

---

## Async Overlap (Complementary Technique)

For dense simulations where most pages are dirty, even the dirty-page approach approaches full-copy cost. The async overlap pattern hides this cost behind the next sim frame's computation time.

The pattern requires a third buffer (two render buffers alternating):

```
Sim works on A (frame N)
At sync:
    - hand A to copy thread: copy A → render buffer C (async, non-blocking to sim)
    - sim switches to B, begins frame N+1
    - render is reading C (frame N-1) while copy of A fills C (frame N)
    - when copy finishes, render switches to C (frame N)
    - B (now holding frame N+1 when it's done) becomes the next copy source
```

The sim never waits for the copy. The render reads 1 frame behind (acceptable in most RTS contexts). Copy time is effectively free as long as the render frame budget exceeds the copy time — which it does (render frames at 16ms+ vs. copy at 4ms).

The cost: 3× world buffer memory. For an 80MB world, this is 240MB just for the sim/render buffers — significant on constrained platforms.

On Windows with `SyncTo`, the copy thread uses dirty-page tracking, so even the async copy benefits from the page-level optimization.

---

## Platform Summary

| Platform | Mechanism | Granularity | Per-write overhead |
|---|---|---|---|
| Windows | `MEM_WRITE_WATCH` + `GetWriteWatch` | 4KB page | Zero (hardware MMU) |
| Linux ≥ 5.18 | `MADV_CLEAR_SOFT_DIRTY` + `pagemap` | 4KB page | Zero (hardware) + ~200KB `pread` at sync |
| Linux < 5.18 | Block-level dirty via non-const `GetBlock` | Per block | Zero (flag set on access) |
| Emscripten | Full memcpy (+ async overlap if memory permits) | Full buffer | N/A |

---

## What Was Rejected and Why

| Approach | Result | Why Dropped |
|---|---|---|
| Full memcpy | ~4ms | At or above budget; scales with capacity not usage |
| Parallel memcpy | ~4ms+ | Bandwidth-bound; parallelism adds overhead on same bus |
| memcmp + memcpy (parallel) | >4ms | 3× memory bandwidth vs. 2× for plain memcpy; comparison reads both buffers |
| Per-feature sparse copy | Mixed | Better for tiny scenes, worse for larger; breaks shape invariant without complexity |
| Explicit dirty marking | Rejected | Dev experience: easy to forget, painful to debug stale render artifacts |

---

## Implementation Notes

- `BlockBuffer::SyncTo` is the new method. `CopyTo` is retained for the initial view initialization and for code paths where write tracking isn't available.
- The `thread_local` dirty page vector in `SyncTo` avoids a heap allocation on the hot path. Size it once based on `maxBufferSize / pageSize` and it will never reallocate.
- `WRITE_WATCH_FLAG_RESET` is critical. Without it, you'd need a separate `ResetWriteWatch` call, and any writes that occur between the two calls are silently lost from the next frame's dirty list.
- `VirtualAlloc` with `MEM_WRITE_WATCH` requires the buffer to stay within a single `VirtualAlloc` region. The current `AllocateMemory` allocates the full buffer in one call, so this is naturally satisfied.
- If `AllocateMemory` is ever called with a larger size (growing the buffer), the old `VirtualFree` + new `VirtualAlloc` with `MEM_WRITE_WATCH` correctly resets tracking. The view will detect the size mismatch and do a full `CopyTo` on the next sync.
