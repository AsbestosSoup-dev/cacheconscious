#pragma once
#include "components.h"

constexpr float WORLD_MIN = 0.0f;
constexpr float WORLD_MAX = 1000.0f;

// PV update: bounce only.
inline void bounceUpdate(Position& pos, Velocity& vel) noexcept {
    pos.x += vel.x;
    pos.y += vel.y;
    pos.z += vel.z;

    auto bounce = [](float& p, float& v) noexcept {
        if      (p < WORLD_MIN) { p = WORLD_MIN; v = -v; }
        else if (p > WORLD_MAX) { p = WORLD_MAX; v = -v; }
    };
    bounce(pos.x, vel.x);
    bounce(pos.y, vel.y);
    bounce(pos.z, vel.z);
}

// PVT update: bounce + tag-value drag.
// tag.value drives a per-archetype drag coefficient so this component is
// actually read during the tick, making archetype grouping meaningful.
inline void bounceUpdateTagged(Position& pos, Velocity& vel, const Tag& tag) noexcept {
    bounceUpdate(pos, vel);
    float drag = 1.0f - tag.value * 0.00001f;
    vel.x *= drag;
    vel.y *= drag;
    vel.z *= drag;
}

// PVTU update: bounce + tag drag + UUID-seeded micro-perturbation.
// The UUID drives a tiny deterministic nudge so the uuid field is read
// each tick, giving the PVTU archetype genuinely distinct behaviour.
inline void bounceUpdateUUID(Position& pos, Velocity& vel,
                             const Tag& tag, const UUID& uuid) noexcept {
    bounceUpdateTagged(pos, vel, tag);
    // Tiny perturbation derived from uuid bits — magnitude ~1e-7, well within
    // correctness epsilon across configs.
    float nudge = static_cast<float>(uuid.id & 0xFFFF) * 1e-10f;
    vel.x += nudge;
    vel.y -= nudge;
}
