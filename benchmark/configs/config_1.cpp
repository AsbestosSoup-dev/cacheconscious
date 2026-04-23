// Config 1: AoS | Virtual dispatch | Archetype grouping OFF  (Baseline OOP)
#include "config_result.h"
#include "../common/rng.h"
#include "../common/timer.h"
#include "../oop/world_aos.h"

ConfigResult runConfig1(std::size_t N) {
    SpawnData spawn = generateSpawnData(N);
    WorldAoS world(spawn, /*grouped=*/false);

    TimingResult timing = runBenchmark([&]{ world.tickVirtual(); });

    return ConfigResult{
        1, "Baseline OOP", "AoS", "Virtual", "Off",
        timing.avg_ms, timing.stddev_ms, N,
        world.finalPositions()
    };
}
