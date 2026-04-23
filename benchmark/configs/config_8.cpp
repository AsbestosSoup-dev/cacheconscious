// Config 8: SoA | Direct dispatch | Archetype grouping ON  (Full ECS)
#include "config_result.h"
#include "../common/rng.h"
#include "../common/timer.h"
#include "../ecs/world_soa.h"

ConfigResult runConfig8(std::size_t N) {
    SpawnData spawn = generateSpawnData(N);
    WorldSoADirect world(spawn, /*grouped=*/true);

    TimingResult timing = runBenchmark([&]{ world.tick(); });

    return ConfigResult{
        8, "Full ECS", "SoA", "Direct", "On",
        timing.avg_ms, timing.stddev_ms, N,
        world.finalPositions()
    };
}
