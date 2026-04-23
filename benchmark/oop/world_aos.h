#pragma once
#include <vector>
#include <memory>
#include "../common/components.h"
#include "../common/rng.h"
#include "particle.h"

// Array-of-Structs world: one heap-allocated Particle object per entity.
// Supports both virtual and direct dispatch, and both grouping modes.
class WorldAoS {
public:
    // grouped=false: entities stored in spawn order (archetype grouping OFF)
    // grouped=true:  entities sorted by archetype after construction (grouping ON)
    explicit WorldAoS(const SpawnData& spawn, bool grouped);

    // Virtual dispatch: calls p->update() through the vtable
    void tickVirtual();

    // Direct dispatch: calls directUpdate(*p) as a free function
    void tickDirect();

    // Returns final positions in canonical spawn order for correctness validation.
    // When grouped=true, inverts the sort permutation to restore spawn order.
    std::vector<Position> finalPositions() const;

private:
    std::vector<std::unique_ptr<Particle>> entities_;

    // Permutation table: originalOrder_[i] = index in entities_ of spawn-entity i.
    // Identity when grouped=false.
    std::vector<std::size_t> originalOrder_;

    bool grouped_;
};
