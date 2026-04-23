#pragma once
#include <vector>
#include <cstddef>
#include "../common/components.h"

// One contiguous SoA chunk for all entities of a single archetype.
// Arrays are parallel: positions[i], velocities[i], tags[i] all belong to the same entity.
// tags and uuids are only populated for archetypes that include them.
struct ArchetypeChunk {
    Archetype type;
    std::vector<Position> positions;
    std::vector<Velocity> velocities;
    std::vector<Tag>      tags;    // populated for PVT and PVTU; empty for PV
    std::vector<UUID>     uuids;   // populated for PVTU; empty for PV and PVT

    std::size_t size() const { return positions.size(); }

    void reserve(std::size_t n) {
        positions.reserve(n);
        velocities.reserve(n);
        if (type == Archetype::PVT || type == Archetype::PVTU) tags.reserve(n);
        if (type == Archetype::PVTU) uuids.reserve(n);
    }
};

// Maps a spawn-order entity index to its location in a grouped chunk.
struct ChunkRef {
    uint8_t     chunkId;  // 0=PV, 1=PVT, 2=PVTU
    std::size_t idx;      // index within the chunk
};
