#pragma once
#include <random>
#include <vector>
#include <cstdint>
#include <cmath>
#include "components.h"

constexpr uint32_t GLOBAL_SEED = 42;

// All per-entity spawn data, generated once from the canonical seed.
// Every config calls generateSpawnData(N) — same seed, same sequence.
struct SpawnData {
    std::vector<Archetype> archetypes;  // archetype for entity i
    std::vector<Velocity>  velocities;  // initial velocity for entity i
};

inline SpawnData generateSpawnData(std::size_t N) {
    std::mt19937 rng(GLOBAL_SEED);

    // Archetype assignment: 50% PV, 30% PVT, 20% PVTU
    std::discrete_distribution<int> archetypeDist({50.0, 30.0, 20.0});

    // Velocity magnitude: uniform [1.0, 10.0]
    std::uniform_real_distribution<float> magDist(1.0f, 10.0f);

    // Spherical coordinates for direction
    std::uniform_real_distribution<float> azimuthDist(0.0f, 6.283185307f);  // [0, 2pi)
    std::uniform_real_distribution<float> elevDist(-1.5707963f, 1.5707963f); // [-pi/2, pi/2]

    SpawnData data;
    data.archetypes.reserve(N);
    data.velocities.reserve(N);

    for (std::size_t i = 0; i < N; ++i) {
        // Archetype
        int a = archetypeDist(rng);
        data.archetypes.push_back(static_cast<Archetype>(a));

        // Velocity: spherical -> Cartesian, scaled by magnitude
        float azimuth = azimuthDist(rng);
        float elev    = elevDist(rng);
        float mag     = magDist(rng);

        float cosE = std::cos(elev);
        Velocity v{
            mag * cosE * std::cos(azimuth),
            mag * cosE * std::sin(azimuth),
            mag * std::sin(elev)
        };
        data.velocities.push_back(v);
    }

    return data;
}
