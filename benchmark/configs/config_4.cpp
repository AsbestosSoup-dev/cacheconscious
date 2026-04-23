// Config 4: AoS | Direct dispatch | Archetype grouping ON
#include "config_result.h"
#include "../common/rng.h"
#include "../common/timer.h"
#include "../oop/world_aos.h"

ConfigResult runConfig4(std::size_t N) {
    SpawnData spawn = generateSpawnData(N);
    WorldAoS world(spawn, /*grouped=*/true);

    TimingResult timing = runBenchmark([&]{ world.tickDirect(); });

    return ConfigResult{
        4, "AoS Direct Grouped", "AoS", "Direct", "On",
        timing.avg_ms, timing.stddev_ms, N,
        world.finalPositions()
    };
}
