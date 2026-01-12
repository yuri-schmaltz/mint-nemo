#!/bin/bash
# profile-cpu.sh — CPU profiling with perf for Nemo
# Part of ONDA 6: Profiling

set -e

NEMO_BIN="${NEMO_BIN:-./build/src/nemo}"
TEST_DIR="${TEST_DIR:-/tmp/nemo-profile-test}"
NUM_FILES="${NUM_FILES:-1000}"
DURATION="${DURATION:-30}"

echo "=== Nemo CPU Profiling with perf ==="
echo ""

# Check if perf is available
if ! command -v perf &> /dev/null; then
    echo "ERROR: perf not found. Install with:"
    echo "  sudo apt install linux-tools-common linux-tools-generic"
    exit 1
fi

# Check if nemo binary exists
if [ ! -f "$NEMO_BIN" ]; then
    echo "ERROR: Nemo binary not found at $NEMO_BIN"
    echo "Build first with: meson compile -C build"
    exit 1
fi

# Create test directory with files
echo "[1/5] Creating test directory with $NUM_FILES files..."
mkdir -p "$TEST_DIR"
for i in $(seq 1 "$NUM_FILES"); do
    touch "$TEST_DIR/file_$(printf "%04d" $i).txt"
done
echo "  ✓ Test directory ready: $TEST_DIR"
echo ""

# Record perf data
echo "[2/5] Starting Nemo with perf recording..."
echo "  Command: perf record -F 99 -g --call-graph=dwarf $NEMO_BIN $TEST_DIR"
echo "  Duration: ${DURATION}s (scroll up/down during this time)"
echo ""
echo "  >>> MANUAL ACTION REQUIRED <<<"
echo "  1. Nemo will open with the test folder"
echo "  2. Scroll up and down continuously for ${DURATION} seconds"
echo "  3. Close Nemo (Ctrl+Q or window close)"
echo ""
read -p "Press ENTER to start perf recording..." 

perf record -F 99 -g --call-graph=dwarf -o perf.data "$NEMO_BIN" "$TEST_DIR" &
NEMO_PID=$!

echo "  Nemo PID: $NEMO_PID"
echo "  Recording... (close Nemo when done)"
wait $NEMO_PID || true

echo ""
echo "[3/5] Analyzing perf data..."
if [ ! -f perf.data ]; then
    echo "ERROR: perf.data not found. Recording may have failed."
    exit 1
fi

echo "  ✓ perf.data captured ($(du -h perf.data | cut -f1))"
echo ""

# Generate report
echo "[4/5] Generating perf report..."
perf report --stdio > perf-report.txt 2>&1 || true
echo "  ✓ Report saved: perf-report.txt"
echo ""

# Show top functions
echo "[5/5] Top 10 CPU-consuming functions:"
echo "―――――――――――――――――――――――――――――――――――――――――――――――"
perf report --stdio --no-children 2>/dev/null | grep -A 15 "Overhead.*Command.*Shared Object.*Symbol" | head -20 || echo "  (run 'perf report' manually to view)"
echo "―――――――――――――――――――――――――――――――――――――――――――――――"
echo ""

# Flamegraph (optional)
if command -v flamegraph.pl &> /dev/null; then
    echo "[OPTIONAL] Generating flamegraph..."
    perf script | stackcollapse-perf.pl | flamegraph.pl > flamegraph.svg
    echo "  ✓ Flamegraph: flamegraph.svg"
else
    echo "[SKIP] Flamegraph generation (install FlameGraph toolkit)"
fi

echo ""
echo "=== Profiling Complete ==="
echo ""
echo "Next steps:"
echo "  1. View interactive report: perf report"
echo "  2. Check text report: less perf-report.txt"
echo "  3. Look for hotspots in pango_layout_get_extents, gtk_widget_draw, etc."
echo ""
echo "Expected hotspots:"
echo "  • pango_layout_get_extents (should be <5% with cache)"
echo "  • gtk_widget_draw (~10-15%)"
echo "  • cairo rendering (~8-12%)"
echo ""
