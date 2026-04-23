// Config 7: SoA | Direct dispatch | Archetype grouping OFF
#include "config_result.h"
#include "../common/rng.h"
#include "../common/timer.h"
#include "../ecs/world_soa.h"

ConfigResult runConfig7(std::size_t N) {
    SpawnData spawn = generateSpawnData(N);
    WorldSoADirect world(spawn, /*grouped=*/false);

    TimingResult timing = runBenchmark([&]{ world.tick(); });

    return ConfigResult{
        7, "SoA Direct Ungrouped", "SoA", "Direct", "Off",
        timing.avg_ms, timing.stddev_ms, N,
        world.finalPositions()
    };
}
