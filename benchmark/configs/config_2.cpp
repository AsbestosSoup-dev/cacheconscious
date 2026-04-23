// Config 2: AoS | Virtual dispatch | Archetype grouping ON
#include "config_result.h"
#include "../common/rng.h"
#include "../common/timer.h"
#include "../oop/world_aos.h"

ConfigResult runConfig2(std::size_t N) {
    SpawnData spawn = generateSpawnData(N);
    WorldAoS world(spawn, /*grouped=*/true);

    TimingResult timing = runBenchmark([&]{ world.tickVirtual(); });

    return ConfigResult{
        2, "AoS Virtual Grouped", "AoS", "Virtual", "On",
        timing.avg_ms, timing.stddev_ms, N,
        world.finalPositions()
    };
}
