#pragma once
#include <string>
#include <vector>
#include "../common/components.h"

struct ConfigResult {
    int         config_id;
    std::string label;
    std::string layout;    // "AoS" or "SoA"
    std::string dispatch;  // "Virtual" or "Direct"
    std::string grouping;  // "On" or "Off"
    double      avg_tick_ms;
    double      stddev_tick_ms;
    std::size_t entity_count;
    std::vector<Position> finalPositions;  // in canonical spawn order
};

// Forward declarations for all 8 config runner functions
ConfigResult runConfig1(std::size_t N);
ConfigResult runConfig2(std::size_t N);
ConfigResult runConfig3(std::size_t N);
ConfigResult runConfig4(std::size_t N);
ConfigResult runConfig5(std::size_t N);
ConfigResult runConfig6(std::size_t N);
ConfigResult runConfig7(std::size_t N);
ConfigResult runConfig8(std::size_t N);
