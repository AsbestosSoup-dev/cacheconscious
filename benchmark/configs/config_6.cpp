// Config 6: SoA | Virtual dispatch | Archetype grouping ON
#include "config_result.h"
#include "../common/rng.h"
#include "../common/timer.h"
#include "../common/signpost.h"
#include "../ecs/world_soa.h"

#ifndef BUILD_FLAGS_STR
#  define BUILD_FLAGS_STR "unknown"
#endif

ConfigResult runConfig6(std::size_t N) {
    SpawnData spawn = generateSpawnData(N);
    WorldSoAVirtual world(spawn, /*grouped=*/true);

    SignpostInterval sp(6, N, BUILD_FLAGS_STR);
    TimingResult timing = runBenchmark([&]{ world.tick(); });

    return ConfigResult{
        6, "SoA Virtual Grouped", "SoA", "Virtual", "On",
        timing.avg_ms, timing.stddev_ms, N,
        world.finalPositions()
    };
}
