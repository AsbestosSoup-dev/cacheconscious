#pragma once
#include "../common/components.h"
#include "../common/simulation.h"

// ---- Virtual dispatch hierarchy (configs 1, 2, 5, 6) ----

class Particle {
public:
    Position pos;
    Velocity vel;
    virtual void update() = 0;
    virtual ~Particle() = default;

    // Archetype identity for sorting (grouping=ON)
    virtual Archetype archetypeId() const = 0;
};

class PlainParticle : public Particle {
public:
    void update() override { bounceUpdate(pos, vel); }
    Archetype archetypeId() const override { return Archetype::PV; }
};

class TaggedParticle : public Particle {
public:
    Tag tag;
    void update() override { bounceUpdateTagged(pos, vel, tag); }
    Archetype archetypeId() const override { return Archetype::PVT; }
};

class SpecificParticle : public TaggedParticle {
public:
    UUID uuid;
    void update() override { bounceUpdateUUID(pos, vel, tag, uuid); }
    Archetype archetypeId() const override { return Archetype::PVTU; }
};

// ---- Direct dispatch free function (configs 3, 4) ----
// Downcasts to concrete type so the right update logic runs without vtable.
inline void directUpdate(Particle& p) noexcept {
    switch (p.archetypeId()) {
        case Archetype::PV:
            bounceUpdate(p.pos, p.vel);
            break;
        case Archetype::PVT: {
            auto& tp = static_cast<TaggedParticle&>(p);
            bounceUpdateTagged(tp.pos, tp.vel, tp.tag);
            break;
        }
        case Archetype::PVTU: {
            auto& sp = static_cast<SpecificParticle&>(p);
            bounceUpdateUUID(sp.pos, sp.vel, sp.tag, sp.uuid);
            break;
        }
    }
}
