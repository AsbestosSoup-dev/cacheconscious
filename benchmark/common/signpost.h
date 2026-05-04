#pragma once
// os_signpost instrumentation for xctrace / Instruments.app
//
// Wraps os_signpost_interval_begin / os_signpost_interval_end so that each
// config's run appears as a named interval in the CPU Counters instrument.
// xctrace slices sampled hardware counter values (L1D_CACHE_MISS_LD,
// L2_TLB_MISS_LD, INST_RETIRED) by these intervals, giving per-config
// attribution.
//
// NOTE: os_signpost is a sampling-based annotation — xctrace attributes
// counter *samples* that fall inside each interval, not exact PMU deltas.
// For exact cache miss counts, use the interval boundaries as a filter in
// Instruments.app's CPU Counters timeline.
//
// This header is a no-op when compiled outside macOS (SIGNPOST_AVAILABLE=0).

#if defined(__APPLE__)
#  include <os/signpost.h>
#  include <os/log.h>
#  define SIGNPOST_AVAILABLE 1
#else
#  define SIGNPOST_AVAILABLE 0
#endif

#include <cstdint>

// Subsystem and category visible in Instruments.app
#define CC_SIGNPOST_SUBSYSTEM "com.cacheconscious.benchmark"

#if SIGNPOST_AVAILABLE

// Lazily initialised log object — one per process.
inline os_log_t ccSignpostLog() {
    static os_log_t log = os_log_create(CC_SIGNPOST_SUBSYSTEM,
                                        OS_LOG_CATEGORY_POINTS_OF_INTEREST);
    return log;
}

// RAII interval identifying config number, entity count, and build flags.
// Begins on construction, ends on destruction — place around runBenchmark()
// in each config file so the interval covers the measurement loop only
// (not construction, warmup, or correctness phases).
//
// In Instruments.app CPU Counters timeline, each interval appears labelled
// e.g. "Config3 N=1000000 -O2 -std=c++17" under subsystem
// com.cacheconscious.benchmark / category config.
class SignpostInterval {
public:
    SignpostInterval(int configId, std::size_t entityCount, const char* buildFlags)
        : id_(os_signpost_id_generate(ccSignpostLog()))
    {
        os_signpost_interval_begin(ccSignpostLog(), id_, "BenchmarkConfig",
                                   "Config%d N=%zu %s",
                                   configId,
                                   entityCount,
                                   buildFlags);
    }

    ~SignpostInterval() {
        os_signpost_interval_end(ccSignpostLog(), id_, "BenchmarkConfig", "");
    }

    // Non-copyable
    SignpostInterval(const SignpostInterval&)            = delete;
    SignpostInterval& operator=(const SignpostInterval&) = delete;

private:
    os_signpost_id_t id_;
};

#else  // !SIGNPOST_AVAILABLE

// Stubs for non-Apple platforms
class SignpostInterval {
public:
    SignpostInterval(int, std::size_t, const char*) {}
};

#endif  // SIGNPOST_AVAILABLE
