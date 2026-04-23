#include "world_aos.h"
#include <algorithm>
#include <numeric>

WorldAoS::WorldAoS(const SpawnData& spawn, bool grouped)
    : grouped_(grouped)
{
    const std::size_t N = spawn.archetypes.size();
    entities_.reserve(N);
    originalOrder_.resize(N);

    // Spawn position: all entities start at world center
    constexpr Position CENTER{500.0f, 500.0f, 500.0f};

    for (std::size_t i = 0; i < N; ++i) {
        std::unique_ptr<Particle> p;
        switch (spawn.archetypes[i]) {
            case Archetype::PV: {
                auto pp = std::make_unique<PlainParticle>();
                pp->pos = CENTER;
                pp->vel = spawn.velocities[i];
                p = std::move(pp);
                break;
            }
            case Archetype::PVT: {
                auto pp = std::make_unique<TaggedParticle>();
                pp->pos = CENTER;
                pp->vel = spawn.velocities[i];
                pp->tag = Tag{1};
                p = std::move(pp);
                break;
            }
            case Archetype::PVTU: {
                auto pp = std::make_unique<SpecificParticle>();
                pp->pos = CENTER;
                pp->vel = spawn.velocities[i];
                pp->tag = Tag{2};
                pp->uuid = UUID{i};
                p = std::move(pp);
                break;
            }
        }
        entities_.push_back(std::move(p));
    }

    // Identity permutation before any sorting
    std::iota(originalOrder_.begin(), originalOrder_.end(), std::size_t{0});

    if (grouped_) {
        // Sort entities by archetype: PV < PVT < PVTU
        // Use stable_sort on index pairs to track the permutation.
        std::vector<std::size_t> indices(N);
        std::iota(indices.begin(), indices.end(), std::size_t{0});

        std::stable_sort(indices.begin(), indices.end(),
            [&](std::size_t a, std::size_t b) {
                return entities_[a]->archetypeId() < entities_[b]->archetypeId();
            });

        // Apply permutation to entities_
        std::vector<std::unique_ptr<Particle>> sorted(N);
        for (std::size_t newIdx = 0; newIdx < N; ++newIdx) {
            sorted[newIdx] = std::move(entities_[indices[newIdx]]);
        }
        entities_ = std::move(sorted);

        // Build inverse permutation: originalOrder_[spawnIdx] = storageIdx
        // indices[storageIdx] = spawnIdx  =>  originalOrder_[spawnIdx] = storageIdx
        for (std::size_t storageIdx = 0; storageIdx < N; ++storageIdx) {
            originalOrder_[indices[storageIdx]] = storageIdx;
        }
    }
}

void WorldAoS::tickVirtual() {
    for (auto& p : entities_) {
        p->update();
    }
}

void WorldAoS::tickDirect() {
    for (auto& p : entities_) {
        directUpdate(*p);
    }
}

std::vector<Position> WorldAoS::finalPositions() const {
    const std::size_t N = entities_.size();
    std::vector<Position> out(N);
    for (std::size_t spawnIdx = 0; spawnIdx < N; ++spawnIdx) {
        out[spawnIdx] = entities_[originalOrder_[spawnIdx]]->pos;
    }
    return out;
}
