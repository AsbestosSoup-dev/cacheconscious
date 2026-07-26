#pragma once
#include <vector>
#include <memory>
#include "archetype.h"
#include "../common/rng.h"

// ---- IEntityUpdater: per-entity virtual dispatch interface for SoA configs 5/6 ----
// Three singleton instances (one per archetype) — compiler cannot devirtualize
// because call site holds IEntityUpdater*, proving per-entity vtable overhead.
class IEntityUpdater {
public:
    // tag and uuid are passed for all archetypes; PV updater ignores them.
    // Keeping a single virtual signature avoids per-archetype dispatch at the
    // call site, preserving the vtable overhead the benchmark is measuring.
    virtual void update(Position& pos, Velocity& vel, Tag& tag, UUID& uuid) = 0;
    virtual ~IEntityUpdater() = default;
};

class PVUpdater   : public IEntityUpdater { public: void update(Position& p, Velocity& v, Tag& t, UUID& u) override; };
class PVTUpdater  : public IEntityUpdater { public: void update(Position& p, Velocity& v, Tag& t, UUID& u) override; };
class PVTUUpdater : public IEntityUpdater { public: void update(Position& p, Velocity& v, Tag& t, UUID& u) override; };

// ---- WorldSoADirect: SoA storage, direct (non-virtual) dispatch — configs 7, 8 ----
class WorldSoADirect {
public:
    // grouped=false: flat arrays in spawn order (grouping OFF / "striped SoA")
    // grouped=true:  three ArchetypeChunks, one per archetype (grouping ON)
    explicit WorldSoADirect(const SpawnData& spawn, bool grouped);

    void tick();

    std::vector<Position> finalPositions() const;

private:
    bool grouped_;

    // --- Grouped storage (grouping=ON) ---
    ArchetypeChunk pvChunk_;
    ArchetypeChunk pvtChunk_;
    ArchetypeChunk pvtuChunk_;
    // Spawn-order mapping: spawnOrderGrouped_[i] = {chunkId, withinChunkIdx}
    std::vector<ChunkRef> spawnOrderGrouped_;

    // --- Flat / striped storage (grouping=OFF) ---
    // All N entities in spawn order. Tag/UUID arrays are zero-padded for
    // entities that don't have those components. This preserves SoA (separate
    // arrays per component type) while forcing iteration to walk all N entries.
    std::vector<Position>  flatPos_;
    std::vector<Velocity>  flatVel_;
    std::vector<Tag>       flatTag_;       // zero for PV entities
    std::vector<UUID>      flatUUID_;      // zero for PV and PVT entities
    std::vector<Archetype> flatArchetype_; // type tag per entity (not used in tick)
};

// ---- WorldSoAVirtual: SoA storage, per-entity virtual dispatch — configs 5, 6 ----
class WorldSoAVirtual {
public:
    explicit WorldSoAVirtual(const SpawnData& spawn, bool grouped);

    void tick();

    std::vector<Position> finalPositions() const;

private:
    bool grouped_;

    // Same grouped/flat storage as Direct variant
    ArchetypeChunk pvChunk_;
    ArchetypeChunk pvtChunk_;
    ArchetypeChunk pvtuChunk_;
    std::vector<ChunkRef> spawnOrderGrouped_;

    std::vector<Position>  flatPos_;
    std::vector<Velocity>  flatVel_;
    std::vector<Tag>       flatTag_;
    std::vector<UUID>      flatUUID_;
    std::vector<Archetype> flatArchetype_;

    // Per-entity updater pointers (one per entity, pointing to a singleton)
    std::vector<IEntityUpdater*> updaterPtrs_;

    // Singleton updater instances (owned by the world)
    std::unique_ptr<PVUpdater>   pvUpdater_;
    std::unique_ptr<PVTUpdater>  pvtUpdater_;
    std::unique_ptr<PVTUUpdater> pvtuUpdater_;
};
