#pragma once
#include <chrono>
#include <vector>
#include <cmath>

struct TimingResult {
    double avg_ms;
    double stddev_ms;
};

// Calls tickFn() (warmup_ticks + measured_ticks) times.
// Returns stats over the measured ticks only (warmup excluded).
template<typename Fn>
TimingResult runBenchmark(Fn tickFn,
                          int warmup_ticks  = 100,
                          int measured_ticks = 1000) {
    using Clock = std::chrono::high_resolution_clock;

    // Warmup
    for (int i = 0; i < warmup_ticks; ++i) {
        tickFn();
    }

    // Measurement
    std::vector<double> durations;
    durations.reserve(static_cast<std::size_t>(measured_ticks));

    for (int i = 0; i < measured_ticks; ++i) {
        auto t0 = Clock::now();
        tickFn();
        auto t1 = Clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        durations.push_back(ms);
    }

    // Mean
    double sum = 0.0;
    for (double d : durations) sum += d;
    double avg = sum / static_cast<double>(measured_ticks);

    // Population stddev
    double sq_sum = 0.0;
    for (double d : durations) {
        double diff = d - avg;
        sq_sum += diff * diff;
    }
    double stddev = std::sqrt(sq_sum / static_cast<double>(measured_ticks));

    return {avg, stddev};
}
