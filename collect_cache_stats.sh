#!/usr/bin/env bash
# collect_cache_stats.sh
#
# Records CPU counter data for each benchmark binary using xctrace.
#
# xctrace does not accept --counters on the command line — counter selection
# is part of the Instruments template, not the record command. This script
# records using the built-in "CPU Counters" template; after opening the trace
# in Instruments.app you configure which counters to display (L1D_CACHE_MISS_LD,
# L2_TLB_MISS_LD, INST_RETIRED) in the instrument's configuration panel.
#
# Produces one .trace file per binary in ./traces/:
#   traces/cache_conscious_O0.trace
#   traces/cache_conscious_O2.trace
#
# Usage:
#   ./collect_cache_stats.sh [build_dir]
#
# build_dir defaults to ./build_test if not provided.

set -euo pipefail

BUILD_DIR="${1:-build_test}"
TRACE_DIR="traces"
mkdir -p "$TRACE_DIR"

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

    # xctrace will not overwrite an existing trace
    if [[ -e "$TRACE_PATH" ]]; then
        rm -rf "$TRACE_PATH"
    fi

    echo "==> Recording: $BIN"
    echo "    Binary:  $BINARY_PATH"
    echo "    Output:  $TRACE_PATH"
    echo ""

    xctrace record \
        --template "CPU Counters" \
        --output "$TRACE_PATH" \
        --launch -- "$BINARY_PATH"

    echo ""
    echo "    Done. Open with:"
    echo "    open \"$TRACE_PATH\""
    echo ""
done

echo "All traces written to ./${TRACE_DIR}/"
echo ""
echo "Next steps in Instruments.app:"
echo "  1. open traces/cache_conscious_O2.trace"
echo "  2. Click the CPU Counters instrument in the left panel"
echo "  3. Click the instrument configuration button (i) and add counters:"
echo "       L1D_CACHE_MISS_LD, L2_TLB_MISS_LD, INST_RETIRED"
echo "  4. In the timeline, find the signpost intervals labelled Config1..Config8"
echo "     (subsystem: dev.asbestossoup.cacheconscious)"
echo "  5. Click an interval to filter counter samples to that config's run"
echo ""
echo "To list all available counter names on this machine:"
echo "  instruments -s counters"
