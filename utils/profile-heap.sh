#!/bin/bash
# profile-heap.sh — Heap profiling with heaptrack for Nemo
# Part of ONDA 6: Profiling

set -e

NEMO_BIN="${NEMO_BIN:-./build/src/nemo}"
TEST_DIR="${TEST_DIR:-/tmp/nemo-profile-test}"
NUM_FILES="${NUM_FILES:-1000}"

echo "=== Nemo Heap Profiling with heaptrack ==="
echo ""

# Check if heaptrack is available
if ! command -v heaptrack &> /dev/null; then
    echo "ERROR: heaptrack not found. Install with:"
    echo "  sudo apt install heaptrack heaptrack-gui"
    exit 1
fi

# Check if nemo binary exists
if [ ! -f "$NEMO_BIN" ]; then
    echo "ERROR: Nemo binary not found at $NEMO_BIN"
    echo "Build first with: meson compile -C build"
    exit 1
fi

# Create test directory
echo "[1/4] Creating test directory with $NUM_FILES files..."
mkdir -p "$TEST_DIR"
for i in $(seq 1 "$NUM_FILES"); do
    touch "$TEST_DIR/file_$(printf "%04d" $i).txt"
done
echo "  ✓ Test directory ready: $TEST_DIR"
echo ""

# Run heaptrack
echo "[2/4] Running Nemo under heaptrack..."
echo "  Command: heaptrack $NEMO_BIN $TEST_DIR"
echo ""
echo "  >>> MANUAL ACTION REQUIRED <<<"
echo "  1. Nemo will open (may be slightly slower)"
echo "  2. Scroll up and down for ~30 seconds"
echo "  3. Close Nemo (Ctrl+Q)"
echo ""
read -p "Press ENTER to start heaptrack..." 

heaptrack -o heaptrack.nemo "$NEMO_BIN" "$TEST_DIR" || true

echo ""
echo "[3/4] Analyzing heap data..."

# Find the most recent heaptrack file
HEAPTRACK_FILE=$(ls -t heaptrack.nemo.*.gz 2>/dev/null | head -1)

if [ -z "$HEAPTRACK_FILE" ]; then
    echo "ERROR: No heaptrack data file found"
    exit 1
fi

echo "  ✓ Heaptrack data: $HEAPTRACK_FILE ($(du -h "$HEAPTRACK_FILE" | cut -f1))"
echo ""

# Generate text report
echo "[4/4] Generating heap report..."
heaptrack --analyze "$HEAPTRACK_FILE" > heaptrack-report.txt 2>&1 || true

# Show summary
echo ""
echo "Heap Statistics:"
echo "―――――――――――――――――――――――――――――――――――――――――――――――"
grep -A 5 "peak heap" heaptrack-report.txt 2>/dev/null || echo "  (analysis in progress)"
echo "―――――――――――――――――――――――――――――――――――――――――――――――"

# Top allocators
echo ""
echo "Top 10 Allocation Sites:"
echo "―――――――――――――――――――――――――――――――――――――――――――――――"
grep -A 15 "MOST CALLS" heaptrack-report.txt 2>/dev/null | head -20 || echo "  See heaptrack-report.txt"
echo "―――――――――――――――――――――――――――――――――――――――――――――――"

echo ""
echo "=== Heap Profiling Complete ==="
echo ""
echo "Next steps:"
echo "  1. View GUI: heaptrack --analyze $HEAPTRACK_FILE (opens GUI)"
echo "  2. Check text report: less heaptrack-report.txt"
echo "  3. Look for allocation hotspots"
echo ""
echo "Expected results:"
echo "  • Peak heap: ~200-400 MB for 1000 items"
echo "  • Major allocators: GTK widgets, Pango layouts, pixbufs"
echo "  • text_cache: ~10-20 MB (1000 entries)"
echo ""
echo "To open GUI now:"
echo "  heaptrack --analyze $HEAPTRACK_FILE"
echo ""
