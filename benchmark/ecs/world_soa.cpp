#include "world_soa.h"
#include "../common/simulation.h"

// ---- IEntityUpdater implementations ----

void PVUpdater::update(Position& p, Velocity& v, Tag&, UUID&)         { bounceUpdate(p, v); }
void PVTUpdater::update(Position& p, Velocity& v, Tag& t, UUID&)      { bounceUpdateTagged(p, v, t); }
void PVTUUpdater::update(Position& p, Velocity& v, Tag& t, UUID& u)   { bounceUpdateUUID(p, v, t, u); }

// ---- Shared spawn helper ----

static constexpr Position CENTER{500.0f, 500.0f, 500.0f};

// Populates grouped chunks and builds the spawn-order mapping.
static void populateGrouped(
    const SpawnData& spawn,
    ArchetypeChunk& pvChunk,
    ArchetypeChunk& pvtChunk,
    ArchetypeChunk& pvtuChunk,
    std::vector<ChunkRef>& spawnOrder)
{
    // Pre-count to reserve
    std::size_t nPV = 0, nPVT = 0, nPVTU = 0;
    for (auto a : spawn.archetypes) {
        if      (a == Archetype::PV)   ++nPV;
        else if (a == Archetype::PVT)  ++nPVT;
        else                           ++nPVTU;
    }

    pvChunk.type   = Archetype::PV;
    pvtChunk.type  = Archetype::PVT;
    pvtuChunk.type = Archetype::PVTU;

    pvChunk.positions.reserve(nPV);   pvChunk.velocities.reserve(nPV);
    pvtChunk.positions.reserve(nPVT); pvtChunk.velocities.reserve(nPVT);
    pvtChunk.tags.reserve(nPVT);
    pvtuChunk.positions.reserve(nPVTU); pvtuChunk.velocities.reserve(nPVTU);
    pvtuChunk.tags.reserve(nPVTU); pvtuChunk.uuids.reserve(nPVTU);

    spawnOrder.resize(spawn.archetypes.size());

    for (std::size_t i = 0; i < spawn.archetypes.size(); ++i) {
        switch (spawn.archetypes[i]) {
            case Archetype::PV: {
                std::size_t idx = pvChunk.positions.size();
                pvChunk.positions.push_back(CENTER);
                pvChunk.velocities.push_back(spawn.velocities[i]);
                spawnOrder[i] = {0, idx};
                break;
            }
            case Archetype::PVT: {
                std::size_t idx = pvtChunk.positions.size();
                pvtChunk.positions.push_back(CENTER);
                pvtChunk.velocities.push_back(spawn.velocities[i]);
                pvtChunk.tags.push_back(Tag{1});
                spawnOrder[i] = {1, idx};
                break;
            }
            case Archetype::PVTU: {
                std::size_t idx = pvtuChunk.positions.size();
                pvtuChunk.positions.push_back(CENTER);
                pvtuChunk.velocities.push_back(spawn.velocities[i]);
                pvtuChunk.tags.push_back(Tag{2});
                pvtuChunk.uuids.push_back(UUID{i});
                spawnOrder[i] = {2, idx};
                break;
            }
        }
    }
}

// Populates flat striped arrays in spawn order.
static void populateFlat(
    const SpawnData& spawn,
    std::vector<Position>&  flatPos,
    std::vector<Velocity>&  flatVel,
    std::vector<Tag>&       flatTag,
    std::vector<UUID>&      flatUUID,
    std::vector<Archetype>& flatArchetype)
{
    const std::size_t N = spawn.archetypes.size();
    flatPos.reserve(N); flatVel.reserve(N);
    flatTag.reserve(N); flatUUID.reserve(N);
    flatArchetype.reserve(N);

    for (std::size_t i = 0; i < N; ++i) {
        flatPos.push_back(CENTER);
        flatVel.push_back(spawn.velocities[i]);
        flatArchetype.push_back(spawn.archetypes[i]);

        switch (spawn.archetypes[i]) {
            case Archetype::PV:
                flatTag.push_back(Tag{0});
                flatUUID.push_back(UUID{0});
                break;
            case Archetype::PVT:
                flatTag.push_back(Tag{1});
                flatUUID.push_back(UUID{0});
                break;
            case Archetype::PVTU:
                flatTag.push_back(Tag{2});
                flatUUID.push_back(UUID{i});
                break;
        }
    }
}

// ---- WorldSoADirect ----

// Hack: reuse ChunkRef type for both Direct and Virtual by making populateGrouped
// take a raw ChunkRef pointer. Instead, just duplicate the struct (they are identical).
// The shared populateGrouped above uses WorldSoADirect::ChunkRef — we add a
// compatible overload for WorldSoAVirtual::ChunkRef below.

WorldSoADirect::WorldSoADirect(const SpawnData& spawn, bool grouped)
    : grouped_(grouped)
{
    if (grouped_) {
        populateGrouped(spawn, pvChunk_, pvtChunk_, pvtuChunk_, spawnOrderGrouped_);
    } else {
        populateFlat(spawn, flatPos_, flatVel_, flatTag_, flatUUID_, flatArchetype_);
    }
}

void WorldSoADirect::tick() {
    if (grouped_) {
        for (std::size_t i = 0; i < pvChunk_.size(); ++i)
            bounceUpdate(pvChunk_.positions[i], pvChunk_.velocities[i]);
        for (std::size_t i = 0; i < pvtChunk_.size(); ++i)
            bounceUpdateTagged(pvtChunk_.positions[i], pvtChunk_.velocities[i], pvtChunk_.tags[i]);
        for (std::size_t i = 0; i < pvtuChunk_.size(); ++i)
            bounceUpdateUUID(pvtuChunk_.positions[i], pvtuChunk_.velocities[i],
                             pvtuChunk_.tags[i], pvtuChunk_.uuids[i]);
    } else {
        for (std::size_t i = 0; i < flatPos_.size(); ++i) {
            switch (flatArchetype_[i]) {
                case Archetype::PV:
                    bounceUpdate(flatPos_[i], flatVel_[i]);
                    break;
                case Archetype::PVT:
                    bounceUpdateTagged(flatPos_[i], flatVel_[i], flatTag_[i]);
                    break;
                case Archetype::PVTU:
                    bounceUpdateUUID(flatPos_[i], flatVel_[i], flatTag_[i], flatUUID_[i]);
                    break;
            }
        }
    }
}

std::vector<Position> WorldSoADirect::finalPositions() const {
    const std::size_t N = grouped_ ? spawnOrderGrouped_.size() : flatPos_.size();
    std::vector<Position> out(N);

    if (grouped_) {
        for (std::size_t i = 0; i < N; ++i) {
            const auto& ref = spawnOrderGrouped_[i];
            switch (ref.chunkId) {
                case 0: out[i] = pvChunk_.positions[ref.idx];   break;
                case 1: out[i] = pvtChunk_.positions[ref.idx];  break;
                case 2: out[i] = pvtuChunk_.positions[ref.idx]; break;
            }
        }
    } else {
        out = flatPos_;
    }
    return out;
}

// ---- WorldSoAVirtual ----

// WorldSoAVirtual::ChunkRef is identical in layout to WorldSoADirect::ChunkRef.
// We re-use populateGrouped by casting — both structs are {uint8_t, size_t}.
// To avoid the cast, we just duplicate the populate logic inline here.

WorldSoAVirtual::WorldSoAVirtual(const SpawnData& spawn, bool grouped)
    : grouped_(grouped)
    , pvUpdater_(std::make_unique<PVUpdater>())
    , pvtUpdater_(std::make_unique<PVTUpdater>())
    , pvtuUpdater_(std::make_unique<PVTUUpdater>())
{
    const std::size_t N = spawn.archetypes.size();

    if (grouped_) {
        // Populate chunks (reuse the Direct variant's helper via a temp)
        std::vector<ChunkRef> tmpOrder;
        populateGrouped(spawn, pvChunk_, pvtChunk_, pvtuChunk_, tmpOrder);
        spawnOrderGrouped_ = tmpOrder;

        // Build per-entity updater pointer array (in chunk iteration order)
        // grouped: PV chunk first, then PVT, then PVTU
        updaterPtrs_.reserve(N);
        for (std::size_t i = 0; i < pvChunk_.size(); ++i)
            updaterPtrs_.push_back(pvUpdater_.get());
        for (std::size_t i = 0; i < pvtChunk_.size(); ++i)
            updaterPtrs_.push_back(pvtUpdater_.get());
        for (std::size_t i = 0; i < pvtuChunk_.size(); ++i)
            updaterPtrs_.push_back(pvtuUpdater_.get());
    } else {
        populateFlat(spawn, flatPos_, flatVel_, flatTag_, flatUUID_, flatArchetype_);

        // Build per-entity updater pointer array in spawn order
        updaterPtrs_.reserve(N);
        for (std::size_t i = 0; i < N; ++i) {
            switch (spawn.archetypes[i]) {
                case Archetype::PV:   updaterPtrs_.push_back(pvUpdater_.get());   break;
                case Archetype::PVT:  updaterPtrs_.push_back(pvtUpdater_.get());  break;
                case Archetype::PVTU: updaterPtrs_.push_back(pvtuUpdater_.get()); break;
            }
        }
    }
}

void WorldSoAVirtual::tick() {
    if (grouped_) {
        // Chunks are iterated in order: PV, PVT, PVTU.
        // updaterPtrs_ is parallel to the concatenated chunk order.
        static Tag dummyTag{0};
        static UUID dummyUUID{0};
        std::size_t ptr = 0;
        for (std::size_t i = 0; i < pvChunk_.size(); ++i, ++ptr)
            updaterPtrs_[ptr]->update(pvChunk_.positions[i], pvChunk_.velocities[i], dummyTag, dummyUUID);
        for (std::size_t i = 0; i < pvtChunk_.size(); ++i, ++ptr)
            updaterPtrs_[ptr]->update(pvtChunk_.positions[i], pvtChunk_.velocities[i], pvtChunk_.tags[i], dummyUUID);
        for (std::size_t i = 0; i < pvtuChunk_.size(); ++i, ++ptr)
            updaterPtrs_[ptr]->update(pvtuChunk_.positions[i], pvtuChunk_.velocities[i], pvtuChunk_.tags[i], pvtuChunk_.uuids[i]);
    } else {
        for (std::size_t i = 0; i < flatPos_.size(); ++i)
            updaterPtrs_[i]->update(flatPos_[i], flatVel_[i], flatTag_[i], flatUUID_[i]);
    }
}

std::vector<Position> WorldSoAVirtual::finalPositions() const {
    const std::size_t N = grouped_ ? spawnOrderGrouped_.size() : flatPos_.size();
    std::vector<Position> out(N);

    if (grouped_) {
        for (std::size_t i = 0; i < N; ++i) {
            const auto& ref = spawnOrderGrouped_[i];
            switch (ref.chunkId) {
                case 0: out[i] = pvChunk_.positions[ref.idx];   break;
                case 1: out[i] = pvtChunk_.positions[ref.idx];  break;
                case 2: out[i] = pvtuChunk_.positions[ref.idx]; break;
            }
        }
    } else {
        out = flatPos_;
    }
    return out;
}
