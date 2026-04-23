#!/usr/bin/env bash
# collect_cache_stats.sh
#
# Records L1D_CACHE_MISS_LD, L2_TLB_MISS_LD, and INST_RETIRED for each
# benchmark binary using xctrace (Instruments CLI).
#
# Produces one .trace file per binary in ./traces/:
#   traces/cache_conscious_O0.trace
#   traces/cache_conscious_O2.trace
#
# Open any .trace file in Instruments.app to inspect per-signpost-interval
# counter breakdowns. In the CPU Counters timeline, filter by subsystem
# "dev.asbestossoup.cacheconscious" to see per-config slices.
#
# Usage:
#   chmod +x collect_cache_stats.sh
#   ./collect_cache_stats.sh [build_dir]
#
# build_dir defaults to ./build_test if not provided.

set -euo pipefail

BUILD_DIR="${1:-build_test}"
TRACE_DIR="traces"
mkdir -p "$TRACE_DIR"

# Counters to collect. Must match names accepted by the CPU Counters template.
# Verify available names on your machine with:
#   instruments -s counters
COUNTERS=(
    "L1D_CACHE_MISS_LD"
    "L2_TLB_MISS_LD"
    "INST_RETIRED"
)

# Build the --counters flags string
COUNTER_FLAGS=()
for c in "${COUNTERS[@]}"; do
    COUNTER_FLAGS+=("--counters" "$c")
done

BINARIES=(
    "cache_conscious_O0"
    "cache_conscious_O2"
)

for BIN in "${BINARIES[@]}"; do
    BINARY_PATH="${BUILD_DIR}/${BIN}"

    if [[ ! -x "$BINARY_PATH" ]]; then
        echo "ERROR: binary not found or not executable: $BINARY_PATH"
        echo "  Run: cmake --build ${BUILD_DIR}"
        exit 1
    fi

    TRACE_PATH="${TRACE_DIR}/${BIN}.trace"

    # Remove stale trace if present (xctrace will not overwrite)
    if [[ -e "$TRACE_PATH" ]]; then
        rm -rf "$TRACE_PATH"
    fi

    echo "==> Recording: $BIN"
    echo "    Binary:  $BINARY_PATH"
    echo "    Output:  $TRACE_PATH"
    echo "    Counters: ${COUNTERS[*]}"
    echo ""

    xctrace record \
        --template "CPU Counters" \
        "${COUNTER_FLAGS[@]}" \
        --output "$TRACE_PATH" \
        --launch -- "$BINARY_PATH"

    echo ""
    echo "    Done. Open in Instruments.app:"
    echo "    open \"$TRACE_PATH\""
    echo ""
done

echo "All traces written to ./${TRACE_DIR}/"
echo ""
echo "To view a trace:"
echo "  open traces/cache_conscious_O2.trace"
echo ""
echo "In Instruments.app:"
echo "  1. Select the 'CPU Counters' instrument"
echo "  2. In the timeline, look for signpost intervals labelled Config1..Config8"
echo "     (subsystem: dev.asbestossoup.cacheconscious)"
echo "  3. Click an interval to filter counter samples to that config's run"
