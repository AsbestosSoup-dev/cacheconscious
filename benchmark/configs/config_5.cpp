// Config 5: SoA | Virtual dispatch | Archetype grouping OFF
#include "config_result.h"
#include "../common/rng.h"
#include "../common/timer.h"
#include "../common/signpost.h"
#include "../ecs/world_soa.h"

#ifndef BUILD_FLAGS_STR
#  define BUILD_FLAGS_STR "unknown"
#endif

ConfigResult runConfig5(std::size_t N) {
    SpawnData spawn = generateSpawnData(N);
    WorldSoAVirtual world(spawn, /*grouped=*/false);

    SignpostInterval sp(5, N, BUILD_FLAGS_STR);
    TimingResult timing = runBenchmark([&]{ world.tick(); });

    return ConfigResult{
        5, "SoA Virtual Ungrouped", "SoA", "Virtual", "Off",
        timing.avg_ms, timing.stddev_ms, N,
        world.finalPositions()
    };
}
