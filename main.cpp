#include <array>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "benchmark/configs/config_result.h"

// BUILD_FLAGS_STR and BUILD_TAG are injected by CMake at compile time.
// e.g. BUILD_FLAGS_STR="-O0 -std=c++17", BUILD_TAG="O0"
#ifndef BUILD_FLAGS_STR
#  define BUILD_FLAGS_STR "unknown"
#endif
#ifndef BUILD_TAG
#  define BUILD_TAG "unknown"
#endif

// ---------------------------------------------------------------------------
// Correctness validation
// ---------------------------------------------------------------------------

// Returns the index of the first differing entity, or -1 if all match.
static long firstDivergence(const std::vector<Position>& ref,
                             const std::vector<Position>& cmp,
                             float epsilon = 1e-4f)
{
    if (ref.size() != cmp.size()) return 0L;
    for (std::size_t i = 0; i < ref.size(); ++i) {
        if (std::fabs(ref[i].x - cmp[i].x) > epsilon ||
            std::fabs(ref[i].y - cmp[i].y) > epsilon ||
            std::fabs(ref[i].z - cmp[i].z) > epsilon)
        {
            return static_cast<long>(i);
        }
    }
    return -1L;
}

// ---------------------------------------------------------------------------
// CSV helpers
// ---------------------------------------------------------------------------

static void writeHeader(std::ofstream& csv) {
    csv << "config_id,label,layout,dispatch,grouping,build_flags,"
           "entity_count,avg_tick_ms,stddev_tick_ms,notes\n";
}

static void writeRow(std::ofstream& csv, const ConfigResult& r,
                     const std::string& buildFlags)
{
    csv << r.config_id      << ","
        << r.label          << ","
        << r.layout         << ","
        << r.dispatch       << ","
        << r.grouping       << ","
        << buildFlags       << ","
        << r.entity_count   << ","
        << r.avg_tick_ms    << ","
        << r.stddev_tick_ms << ","
        << "cache stats require manual Instruments.app run"
        << "\n";
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main() {
    const std::vector<std::size_t> entityCounts = {10'000, 100'000, 1'000'000};
    const std::string buildFlags = BUILD_FLAGS_STR;
    const std::string buildTag   = BUILD_TAG;

    const std::string csvPath = std::string("results_") + buildTag + ".csv";
    std::ofstream csv(csvPath);
    if (!csv) {
        std::cerr << "ERROR: cannot open " << csvPath << " for writing\n";
        return 1;
    }
    writeHeader(csv);

    std::cout << "=== ECS vs OOP Ablation Benchmark (" << buildFlags << ") ===\n\n";

    bool anyDivergence = false;

    for (std::size_t N : entityCounts) {
        std::cout << "--- Entity count: " << N << " ---\n";
        std::cout << "  Running correctness validation and timing...\n";
        std::cout.flush();

        // Run all 8 configs
        std::array<ConfigResult, 8> results = {
            runConfig1(N), runConfig2(N), runConfig3(N), runConfig4(N),
            runConfig5(N), runConfig6(N), runConfig7(N), runConfig8(N)
        };

        // Correctness validation: all configs must match config 1 (reference)
        const std::vector<Position>& ref = results[0].finalPositions;
        bool entityDivergence = false;
        for (int k = 1; k < 8; ++k) {
            long idx = firstDivergence(ref, results[k].finalPositions);
            if (idx >= 0) {
                const Position& rp = ref[static_cast<std::size_t>(idx)];
                const Position& cp = results[k].finalPositions[static_cast<std::size_t>(idx)];
                std::cerr << "DIVERGENCE: config " << results[k].config_id
                          << " (" << results[k].label << ") vs config 1"
                          << " at N=" << N << " entity=" << idx << "\n"
                          << "  ref:    (" << rp.x << ", " << rp.y << ", " << rp.z << ")\n"
                          << "  config" << results[k].config_id
                          << ": (" << cp.x << ", " << cp.y << ", " << cp.z << ")\n"
                          << "  delta:  (" << std::fabs(rp.x - cp.x) << ", "
                                           << std::fabs(rp.y - cp.y) << ", "
                                           << std::fabs(rp.z - cp.z) << ")\n";
                entityDivergence = true;
                anyDivergence = true;
            }
        }

        if (entityDivergence) {
            std::cerr << "Aborting: correctness validation failed at N=" << N << "\n";
            return 1;
        }

        std::cout << "  Correctness: PASS (all 8 configs agree within epsilon=1e-4)\n";

        // Write CSV rows and print summary
        for (const auto& r : results) {
            writeRow(csv, r, buildFlags);
            std::cout << "  Config " << r.config_id
                      << " [" << r.label << "]"
                      << "  avg=" << r.avg_tick_ms << " ms"
                      << "  stddev=" << r.stddev_tick_ms << " ms\n";
        }
        std::cout << "\n";
    }

    csv.close();
    std::cout << "Results written to: " << csvPath << "\n";

    return anyDivergence ? 1 : 0;
}
