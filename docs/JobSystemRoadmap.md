---
title: Job System Roadmap
---

# PhoenixSim Job System — Gap Analysis & Roadmap

This document captures a gap analysis between PhoenixSim's current job/threading stack and the modern work-stealing, fiber-aware design described in *Job Systems for Game Engines* (Mighty Professional, 2026). It is the planning artifact for a series of staged improvements; each section ends with a concrete next step.

## Current architecture (baseline)

| Concern | Implementation |
|---|---|
| Worker pool | `Phoenix::ThreadPool` — `min(hw_concurrency, 8) - 1` OS threads, one global instance, multiplexed across all worlds (`src/PhoenixSim/Parallel.h:62`, `tests/TestRTS/app.cpp:313`). |
| Submission queue | Single Vyukov bounded MPMC ring buffer (`src/PhoenixSim/Containers/MPMCQueue.h`). Capacity defaults to 1024. |
| Task payload | `Phoenix::Task` wrapping `std::function<void()>` + `std::shared_ptr<TaskHandle>` (`Parallel.h:30`). |
| DAG scheduler | `ECS::JobScheduler` with `RemainingPredecessors` atomic per node; auto-derives implicit edges from component Read/Write conflicts (`src/PhoenixSim/ECS/JobScheduler.h`, `SystemJob.h:32-65`). |
| Idle strategy | Exponential-backoff `PHX_THREAD_PAUSE()` on the worker side, plain `std::this_thread::yield()` on caller-side waits. No condition variables, no futex. |
| Range parallelism | `ParallelForEach(n, fn)` and `ParallelRange(n, minRange, fn)` (`Parallel.h:123-185`). |
| Fibers / mid-job wait | None. Waits are expressed by splitting into two jobs with a dependency edge. |

## What the design gets right

- One worker per core minus one, OS threads only — matches the "threads about the machine, tasks about the work" rule.
- Vyukov MPMC is textbook lock-free and ABA-safe via per-cell sequence numbers.
- `PHX_THREAD_PAUSE()` exponential backoff on the worker idle path is correct.
- Component-access dependency derivation in the ECS scheduler is *better* than Unity's runtime `AtomicSafetyHandle` — it's a schedule-time guarantee, not an editor-only check.
- `ParallelRange` enforces a minimum granularity, the right way to express data-parallel work per the Cilk-5 work-first principle.

## Gaps, ranked by performance impact

### 1. False sharing on the MPMC hotspot

`Containers/MPMCQueue.h:101-102`:

```cpp
std::atomic<size_t> EnqueuePos;
std::atomic<size_t> DequeuePos;
```

Adjacent 8-byte atomics; every producer writes the first, every consumer writes the second. The cache line ping-pongs between cores on every operation. The same pattern exists on the `ThreadPool` hot atomics (`Done`, `ActiveWorkerCount`, `SpinningWorkerCount` — `Parallel.h:85-88`).

**Fix:** `alignas(64)` on each hot atomic. Pad `Cell` to a full line if `sizeof(T) + sizeof(atomic<size_t>) < 64`. Expect 1.5–3× throughput on the queue itself on x86; more on Apple Silicon (128-byte lines).

### 2. No work stealing — single global queue is the contention point

Every submit and every dequeue races on the same `EnqueuePos`/`DequeuePos`. The Chase-Lev pattern replaces this with N per-worker deques where the owner uses near-zero-cost push/pop (relaxed bottom counter) and only thieves do CAS. Owner-side cost drops to "tens of nanoseconds" vs. "hundreds" for steals.

For PhoenixSim's typical millisecond-scale physics batches, the gap is hidden. It becomes visible with:
- `ParallelForEach(N, fn)` and small-N fan-outs from one big archetype.
- Scaling beyond 8–16 cores.

**Fix path:** port a Chase-Lev deque (use the Lê et al. 2013 weak-memory-model variant — important for Switch/ARM targets). Keep the global MPMC as the submission inbox; drain it into per-worker deques.

### 3. `ParallelForEach` violates the granularity rule

`Parallel.h:123-131` submits one task per element. The Cilk-5 work-first heuristic: each job should do 10–100× more work than the queue operation costs. Push/pop here is ~100 ns minimum (CAS + sequence store + `shared_ptr` alloc), so per-element jobs need ≥1 µs of real work to break even.

**Fix:** make `ParallelForEach` a shim that forwards to `ParallelRange` with a default `minRange`. Better yet, deprecate it in favor of `ParallelRange` everywhere.

### 4. `std::function` + `std::shared_ptr<TaskHandle>` per task

Every `Submit()` performs:
1. `make_shared<TaskHandle>()` — heap alloc + atomic refcount init.
2. `std::function` move — heap alloc if capture exceeds the SBO (typically 16–32 B).
3. `TryEnqueue(Task)` — copies the Task (incl. another refcount bump).
4. Return shared_ptr — refcount bump.

Three atomic ops and 1–2 allocations per task before any work runs. Reinalter's Molecular Matters reference design uses per-thread linear allocators, intrusive sibling lists for dependents, and raw pointers tracked via a generational pool.

**Fix path:** per-worker linear allocator for `Task` instances, reset between frames. Replace `std::function` with an in-place 48-byte callable buffer (`Delegates.h` is the natural extension point). Replace `shared_ptr<TaskHandle>` with `TaskHandle*` + generation counter.

### 5. No fibers / no inline `WaitForCounter`

`TaskHandle::WaitForCompleted` (`Parallel.cpp:22-34`) is a spin with `yield()`. If a worker thread enters it from inside a job body, the worker stalls — exactly the Naughty Dog pain point. PhoenixSim mitigates by forcing continuation-passing style via the `JobScheduler` DAG: split into two jobs joined by a dependency edge.

That's a defensible choice. But it forces every "wait for X" pattern into two-job form, which is invasive for:
- PhysicsSystem's iterative solver (currently uses bespoke scheduler instances per phase).
- Scripting (PhoenixLua) calling into engine code that wants to wait.

**Fix:** introduce stackful fibers (per Naughty Dog GDC 2015 / Marl) so `WaitForCounter` can park a fiber and switch to another runnable fiber on the same worker.

### 6. Caller-side waits don't use PAUSE

The worker idle path uses `PHX_THREAD_PAUSE()`. The caller-side waits don't:

- `TaskHandle::WaitForCompleted` (`Parallel.cpp:22-34`)
- `ThreadPool::WaitIdle` (`Parallel.cpp:194-205`)
- `JobScheduler::Execute` final drain (`JobScheduler.cpp:282-283`)

All are `yield()`-only. Adding a 64-iter PAUSE loop before each `yield()` cuts wake latency from microseconds to nanoseconds when the wait is short. Matters most at end-of-frame fences.

### 7. Memory ordering review for weak memory models

The MPMC is correct (textbook Vyukov). `JobScheduler` mixes orderings — `RemainingPredecessors.store(..., memory_order_relaxed)` on init and `fetch_sub(..., memory_order_acq_rel)` on decrement. Should be fine because the init store happens-before any worker observes the node via the queue, but worth a TSan run and (ideally) a test on an Apple M-series box before any Switch-class deployment.

### Smaller items

- No priority tiers (Naughty Dog uses low/normal/high).
- No Pipe-equivalent for arbitrary work (Unreal's serialization primitive for non-thread-safe APIs).
- No per-worker stats exposed (counters for "jobs completed", "jobs stolen") — Tracy zones only.
- No `std::execution`-style sender/receiver vocabulary (long-term consideration only).

## Implementation order

1. **#1 — Cache-line padding** (low risk, ~10 lines, immediate measurable win).
2. **#6 — PAUSE in caller-side waits** (small, isolated, predictable).
3. **#4 — Task pool / kill `std::function` + `shared_ptr` on hot path** (medium; touches `Submit()` API).
4. **#3 — `ParallelForEach` → `ParallelRange` shim** (small; one helper).
5. **#2 — Chase-Lev per-worker deque** (large; reshapes the scheduler internals).
6. **#5 — Fibers** (large + invasive; depends on #2 being stable). Justified specifically by PhysicsSystem's iterative solver and PhoenixLua reentrancy.

Each step lands behind pinning tests (`tests/unit/PhoenixSim/test_parallel.cpp`) that capture the existing observable behavior so regressions are caught regardless of how the internals are reorganized.

## References

- Mighty Professional (2026). *Job Systems for Game Engines.* (Source diagnosis blog.)
- Gyrling, C. (2015). *Parallelizing the Naughty Dog Engine Using Fibers.* GDC.
- Chase, D., Lev, Y. (2005). *Dynamic Circular Work-Stealing Deque.* SPAA.
- Lê et al. (2013). *Correct and Efficient Work-Stealing for Weak Memory Models.* PPoPP.
- Frigo, Leiserson, Randall (1998). *The Implementation of the Cilk-5 Multithreaded Language.* PLDI.
- Reinalter, S. (2015–2016). *Job System 2.0: Lock-Free Work Stealing*, parts 1–5. Molecular Musings.
