#!/bin/bash
# profile-memory.sh — Memory profiling with valgrind for Nemo
# Part of ONDA 6: Profiling

set -e

NEMO_BIN="${NEMO_BIN:-./build/src/nemo}"
TEST_DIR="${TEST_DIR:-/tmp/nemo-profile-test}"
NUM_FILES="${NUM_FILES:-500}"

echo "=== Nemo Memory Profiling with Valgrind ==="
echo ""

# Check if valgrind is available
if ! command -v valgrind &> /dev/null; then
    echo "ERROR: valgrind not found. Install with:"
    echo "  sudo apt install valgrind"
    exit 1
fi

# Check if nemo binary exists
if [ ! -f "$NEMO_BIN" ]; then
    echo "ERROR: Nemo binary not found at $NEMO_BIN"
    echo "Build first with: meson compile -C build"
    exit 1
fi

# Create test directory
echo "[1/3] Creating test directory with $NUM_FILES files..."
mkdir -p "$TEST_DIR"
for i in $(seq 1 "$NUM_FILES"); do
    touch "$TEST_DIR/file_$(printf "%04d" $i).txt"
done
echo "  ✓ Test directory ready: $TEST_DIR"
echo ""

# Run valgrind
echo "[2/3] Running Nemo under valgrind..."
echo "  WARNING: This will be SLOW (10-50x slower than normal)"
echo "  Command: valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes"
echo ""
echo "  >>> MANUAL ACTION REQUIRED <<<"
echo "  1. Nemo will open VERY SLOWLY"
echo "  2. Scroll a bit (10-15 seconds)"
echo "  3. Close Nemo (Ctrl+Q)"
echo ""
read -p "Press ENTER to start valgrind..." 

valgrind \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --verbose \
    --log-file=valgrind-memcheck.log \
    "$NEMO_BIN" "$TEST_DIR" 2>&1 | tee valgrind-output.txt

echo ""
echo "[3/3] Analyzing memory report..."

if [ -f valgrind-memcheck.log ]; then
    echo "  ✓ Full log: valgrind-memcheck.log"
    
    # Extract summary
    echo ""
    echo "Memory Leak Summary:"
    echo "―――――――――――――――――――――――――――――――――――――――――――――――"
    grep -A 10 "LEAK SUMMARY" valgrind-memcheck.log || echo "  No leaks detected ✓"
    echo "―――――――――――――――――――――――――――――――――――――――――――――――"
    
    # Check for definite leaks
    DEFINITE_LEAKS=$(grep "definitely lost:" valgrind-memcheck.log | grep -oP '\d+(?= bytes)' | head -1)
    if [ -n "$DEFINITE_LEAKS" ] && [ "$DEFINITE_LEAKS" -gt 0 ]; then
        echo ""
        echo "⚠️  WARNING: $DEFINITE_LEAKS bytes definitely lost"
        echo "    Review valgrind-memcheck.log for details"
    else
        echo ""
        echo "✓ No definite memory leaks detected"
    fi
fi

echo ""
echo "=== Memory Profiling Complete ==="
echo ""
echo "Next steps:"
echo "  1. Review full log: less valgrind-memcheck.log"
echo "  2. Search for 'definitely lost' to find leaks"
echo "  3. Check 'HEAP SUMMARY' for memory usage"
echo ""
echo "Expected results (with cache):"
echo "  • definitely lost: 0 bytes"
echo "  • Heap usage: ~150-300 MB for 500 items"
echo "  • text_cache allocations should be freed"
echo ""
