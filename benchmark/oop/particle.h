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
    void update() override { bounceUpdate(pos, vel); }
    Archetype archetypeId() const override { return Archetype::PVT; }
};

class SpecificParticle : public TaggedParticle {
public:
    UUID uuid;
    void update() override { bounceUpdate(pos, vel); }
    Archetype archetypeId() const override { return Archetype::PVTU; }
};

// ---- Direct dispatch free function (configs 3, 4) ----
// Called without going through the vtable; update logic is identical.
inline void directUpdate(Particle& p) noexcept {
    bounceUpdate(p.pos, p.vel);
}
