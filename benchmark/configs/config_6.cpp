// Config 6: SoA | Virtual dispatch | Archetype grouping ON
#include "config_result.h"
#include "../common/rng.h"
#include "../common/timer.h"
#include "../ecs/world_soa.h"

ConfigResult runConfig6(std::size_t N) {
    SpawnData spawn = generateSpawnData(N);
    WorldSoAVirtual world(spawn, /*grouped=*/true);

    TimingResult timing = runBenchmark([&]{ world.tick(); });

    return ConfigResult{
        6, "SoA Virtual Grouped", "SoA", "Virtual", "On",
        timing.avg_ms, timing.stddev_ms, N,
        world.finalPositions()
    };
}
