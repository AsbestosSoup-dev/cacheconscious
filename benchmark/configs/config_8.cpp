// Config 8: SoA | Direct dispatch | Archetype grouping ON  (Full ECS)
#include "config_result.h"
#include "../common/rng.h"
#include "../common/timer.h"
#include "../common/signpost.h"
#include "../ecs/world_soa.h"

#ifndef BUILD_FLAGS_STR
#  define BUILD_FLAGS_STR "unknown"
#endif

ConfigResult runConfig8(std::size_t N) {
    SpawnData spawn = generateSpawnData(N);
    WorldSoADirect world(spawn, /*grouped=*/true);

    SignpostInterval sp(8, N, BUILD_FLAGS_STR);
    TimingResult timing = runBenchmark([&]{ world.tick(); });

    return ConfigResult{
        8, "Full ECS", "SoA", "Direct", "On",
        timing.avg_ms, timing.stddev_ms, N,
        world.finalPositions()
    };
}
