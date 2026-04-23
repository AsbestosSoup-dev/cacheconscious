#pragma once
#include <cstdint>

struct Position { float x, y, z; };  // 12 bytes
struct Velocity { float x, y, z; };  // 12 bytes
struct Tag      { uint8_t value; };   //  1 byte  — pure archetype splitter
struct UUID     { uint64_t id; };     //  8 bytes — dead weight / cache polluter

enum class Archetype : uint8_t {
    PV   = 0,  // Position + Velocity              (50%)
    PVT  = 1,  // Position + Velocity + Tag        (30%)
    PVTU = 2   // Position + Velocity + Tag + UUID (20%)
};
