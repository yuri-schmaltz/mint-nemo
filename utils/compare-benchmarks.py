#!/usr/bin/env python3
# compare-benchmarks.py — Compare baseline vs PR benchmarks
# Uso: python3 utils/compare-benchmarks.py baseline.json pr.json

import json
import sys

def load_benchmark(filepath):
    with open(filepath, 'r') as f:
        return json.load(f)

def compare_benchmarks(baseline_file, pr_file):
    baseline = load_benchmark(baseline_file)
    pr = load_benchmark(pr_file)
    
    print("\n" + "="*70)
    print("BENCHMARK COMPARISON: Baseline vs PR")
    print("="*70)
    print(f"Baseline:    {baseline['timestamp']}")
    print(f"PR:          {pr['timestamp']}")
    print()
    
    baseline_results = {r['item_count']: r for r in baseline['results']}
    pr_results = {r['item_count']: r for r in pr['results']}
    
    regressions = []
    improvements = []
    
    for count in sorted(baseline_results.keys()):
        baseline_time = baseline_results[count]['startup_time_ms']
        pr_time = pr_results[count]['startup_time_ms']
        
        delta = pr_time - baseline_time
        delta_pct = (delta / baseline_time) * 100 if baseline_time > 0 else 0
        
        print(f"Items: {count:4d} | Baseline: {baseline_time:6d} ms | PR: {pr_time:6d} ms | "
              f"Delta: {delta:+6d} ms ({delta_pct:+6.1f}%)")
        
        if delta_pct > 5.0:
            regressions.append((count, delta_pct))
        elif delta_pct < -5.0:
            improvements.append((count, -delta_pct))
    
    print()
    print("="*70)
    
    if improvements:
        print("✓ IMPROVEMENTS:")
        for count, improvement in improvements:
            print(f"  - {count} items: {improvement:.1f}% faster")
    
    if regressions:
        print("✗ REGRESSIONS (>5%):")
        for count, regression in regressions:
            print(f"  - {count} items: {regression:.1f}% slower")
        return False  # Fail if regressions detected
    else:
        print("✓ No significant regressions detected")
        return True

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage: python3 compare-benchmarks.py baseline.json pr.json")
        sys.exit(1)
    
    success = compare_benchmarks(sys.argv[1], sys.argv[2])
    sys.exit(0 if success else 1)
