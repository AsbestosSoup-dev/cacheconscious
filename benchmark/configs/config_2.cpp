// Config 2: AoS | Virtual dispatch | Archetype grouping ON
#include "config_result.h"
#include "../common/rng.h"
#include "../common/timer.h"
#include "../common/signpost.h"
#include "../oop/world_aos.h"

#ifndef BUILD_FLAGS_STR
#  define BUILD_FLAGS_STR "unknown"
#endif

ConfigResult runConfig2(std::size_t N) {
    SpawnData spawn = generateSpawnData(N);
    WorldAoS world(spawn, /*grouped=*/true);

    SignpostInterval sp(2, N, BUILD_FLAGS_STR);
    TimingResult timing = runBenchmark([&]{ world.tickVirtual(); });

    return ConfigResult{
        2, "AoS Virtual Grouped", "AoS", "Virtual", "On",
        timing.avg_ms, timing.stddev_ms, N,
        world.finalPositions()
    };
}
