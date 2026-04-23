#pragma once
#include "components.h"

constexpr float WORLD_MIN = 0.0f;
constexpr float WORLD_MAX = 1000.0f;

// Single canonical bounce update — called identically by all 8 configs.
// Mutates pos and vel in place. No fast-math; IEEE-strict.
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
