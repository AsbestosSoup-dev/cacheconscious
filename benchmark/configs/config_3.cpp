// Config 3: AoS | Direct dispatch | Archetype grouping OFF
#include "config_result.h"
#include "../common/rng.h"
#include "../common/timer.h"
#include "../oop/world_aos.h"

ConfigResult runConfig3(std::size_t N) {
    SpawnData spawn = generateSpawnData(N);
    WorldAoS world(spawn, /*grouped=*/false);

    TimingResult timing = runBenchmark([&]{ world.tickDirect(); });

    return ConfigResult{
        3, "AoS Direct Ungrouped", "AoS", "Direct", "Off",
        timing.avg_ms, timing.stddev_ms, N,
        world.finalPositions()
    };
}
