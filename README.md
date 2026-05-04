# ECS vs OOP Ablation Benchmark

A headless C++17 bouncing particle simulation measuring the isolated performance
contribution of three ECS architectural features against traditional OOP.

Built for a presentation to Dr. Asif Khan (ECE, Georgia Tech).

---

## Build Instructions

Requirements: CMake ≥ 3.20, a C++17-capable compiler (tested on AppleClang on macOS ARM64).

```bash
cmake -B build
cmake --build build
```

This produces two binaries in `build/`:

- `cache_conscious_O0` — compiled with `-O0` (naive build)
- `cache_conscious_O2` — compiled with `-O2` (production build)

---

## Running the Benchmark

```bash
./build/cache_conscious_O0
./build/cache_conscious_O2
```

Each binary:
1. Runs correctness validation first (aborts with exit code 1 on any divergence)
2. Times all 8 configs × 3 entity counts (10k / 100k / 1M)
3. Writes results to `results_O0.csv` or `results_O2.csv` in the current directory

Progress is printed to stdout. The full run takes several minutes at 1M entities.

---

## Output Format

`results_O0.csv` / `results_O2.csv`:

```
config_id,label,layout,dispatch,grouping,build_flags,entity_count,avg_tick_ms,stddev_tick_ms,notes
```

- `avg_tick_ms`: mean over 1000 measured ticks (100 warmup ticks excluded)
- `stddev_tick_ms`: population standard deviation over those 1000 ticks
- `notes`: reminder that cache miss rates require manual collection (see below)

---

## 8 Ablation Configurations

| Config | Layout | Dispatch | Archetype Grouping | Label                  |
|--------|--------|----------|--------------------|------------------------|
| 1      | AoS    | Virtual  | Off                | Baseline OOP           |
| 2      | AoS    | Virtual  | On                 | AoS Virtual Grouped    |
| 3      | AoS    | Direct   | Off                | AoS Direct Ungrouped   |
| 4      | AoS    | Direct   | On                 | AoS Direct Grouped     |
| 5      | SoA    | Virtual  | Off                | SoA Virtual Ungrouped  |
| 6      | SoA    | Virtual  | On                 | SoA Virtual Grouped    |
| 7      | SoA    | Direct   | Off                | SoA Direct Ungrouped   |
| 8      | SoA    | Direct   | On                 | Full ECS               |

**Definitions:**
- **AoS**: One heap-allocated object per entity (Array of Structs)
- **SoA**: Separate contiguous arrays per component type (Struct of Arrays)
- **Virtual dispatch**: `update()` called through base class pointer (vtable lookup per entity)
- **Direct dispatch**: Free function call — no vtable indirection
- **Archetype grouping ON**: Entities stored grouped by component composition (PV / PVT / PVTU chunks)
- **Archetype grouping OFF**: Entities in spawn order; iteration walks all N regardless of type

---

## Correctness Validation

Before any timing, all 8 configs run the same 1100 ticks (100 warmup + 1000 measured) from the
same seed. Final positions are compared pairwise against config 1 (reference).

All 8 must agree within `epsilon = 1e-4f` per component per entity. If any diverges:

```
DIVERGENCE: config N (label) vs config 1 at N=<entity_count> entity=<index>
  ref:    (x, y, z)
  configN: (x, y, z)
  delta:  (dx, dy, dz)
Aborting: correctness validation failed at N=<entity_count>
```

The binary exits with code 1. Fix the diverging config before proceeding.

---

## Fixed Seed and How to Change It

The RNG seed is `GLOBAL_SEED = 42` in `benchmark/common/rng.h`:

```cpp
constexpr uint32_t GLOBAL_SEED = 42;
```

Change this value, rebuild, and re-run. All 8 configs use `generateSpawnData(N)` which
calls `std::mt19937(GLOBAL_SEED)` — the sequence is fully deterministic.

---

## Collecting L1D Cache Miss Rates on macOS (Instruments.app)

`perf stat` is not available on macOS/ARM64. Cache miss rates are collected using
Apple Instruments. The binaries emit `os_signpost` intervals around each config's
measurement loop so Instruments can slice counter samples per config via the
Points of Interest track.

### Setup

1. Open Instruments.app and select the **CPU Counters** template
2. Set the target binary to `cache_conscious_O2`
3. Set **Working Directory** to the project root (not Automatic)
4. Set **Recording Mode** to Deferred
5. Under CPU Counters configuration, select **Guided** → **L1D Miss Sampling**
6. Record

### Reading results

After the run completes, open the **Points of Interest** track in the timeline.
Each config's measurement loop appears as a named interval (`BenchmarkConfig`).
Click an interval to filter the CPU Counters summary to that config's window —
the summary shows **Cycles**, **L1D Cache Load Misses**, **L1D Cache Store Misses**,
and **L1D TLB Misses** for that interval.

### Notes

- `os_signpost` intervals cover the 1000 measured ticks only, not the 100 warmup
  ticks or world construction.
- Profiling overhead dilates wall-clock duration (~3x on M2 Max). Use counter
  values normalized by entity count and tick count for cross-config comparison,
  not raw durations.
- On M2 Max, SIP does not block local Instruments.app profiling.
- Trace files are large; `traces/` is gitignored.

---

## M2 Max Unified Memory Architecture Note

The Apple M2 Max uses a **unified memory architecture (UMA)**: CPU and GPU share a
single physical memory pool. There is no discrete VRAM boundary or PCIe bus to cross.

Key differences from a typical x86 system with discrete memory:

| Property          | M2 Max (ARM64 UMA)            | x86 + discrete GPU            |
|-------------------|-------------------------------|-------------------------------|
| Memory bus        | Internal SoC fabric (~400 GB/s) | PCIe Gen4 (64 GB/s typical)  |
| CPU cache         | L1: 192 KB/P-core, L2: 12 MB (cluster) | L1: 32–64 KB, L2: 256 KB–1 MB, L3: 16–64 MB |
| DRAM latency      | ~80–100 ns                    | ~80–100 ns (similar)          |
| NUMA              | None (single memory pool)     | Often NUMA (multi-socket)     |

**Interpretation for this benchmark:** The SoA cache locality advantage (configs 7/8)
is real on M2 Max but may appear less dramatic than on x86 because M2's L2 is large
and shared within a core cluster — the L2 miss penalty is lower than on many x86 parts.
On a system with 32 KB L1 and a cold L3, the SoA vs AoS difference would likely be
more pronounced at smaller entity counts.

The IPC benefit of eliminating virtual dispatch (direct vs virtual) should be visible
at `-O2` because the M2's branch predictor handles indirect calls less efficiently
than direct calls — vtable dispatch adds an indirect branch per entity.