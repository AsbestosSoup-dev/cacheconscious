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
#define CC_SIGNPOST_SUBSYSTEM "dev.asbestossoup.cacheconscious"
#define CC_SIGNPOST_CATEGORY  "benchmark"

#if SIGNPOST_AVAILABLE

// Lazily initialised log object — one per process.
inline os_log_t ccSignpostLog() {
    static os_log_t log = os_log_create(CC_SIGNPOST_SUBSYSTEM, CC_SIGNPOST_CATEGORY);
    return log;
}

// RAII interval: begins on construction, ends on destruction.
// Usage:
//   {
//       SignpostInterval sp("Config 1 | N=10000");
//       runConfig1(N);
//   }  // interval ends here
class SignpostInterval {
public:
    explicit SignpostInterval(const char* name)
        : id_(os_signpost_id_generate(ccSignpostLog()))
        , name_(name)
    {
        os_signpost_interval_begin(ccSignpostLog(), id_, CC_SIGNPOST_CATEGORY,
                                   "begin: %s", name_);
    }

    ~SignpostInterval() {
        os_signpost_interval_end(ccSignpostLog(), id_, CC_SIGNPOST_CATEGORY,
                                 "end: %s", name_);
    }

    // Non-copyable
    SignpostInterval(const SignpostInterval&)            = delete;
    SignpostInterval& operator=(const SignpostInterval&) = delete;

private:
    os_signpost_id_t id_;
    const char*      name_;
};

// Convenience macro: opens a signpost interval for the remainder of the
// current scope. Name must be a string literal (os_signpost requirement).
#define CC_SIGNPOST(name) SignpostInterval _sp_##__LINE__(name)

#else  // !SIGNPOST_AVAILABLE

// Stubs for non-Apple platforms
class SignpostInterval {
public:
    explicit SignpostInterval(const char*) {}
};
#define CC_SIGNPOST(name) ((void)0)

#endif  // SIGNPOST_AVAILABLE
