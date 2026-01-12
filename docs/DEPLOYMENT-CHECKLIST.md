# ✅ DEPLOYMENT CHECKLIST — NEMO v6.7 Performance Release

**Data Alvo**: Q1 2026  
**Responsável**: Equipe Nemo  
**Status**: 🟡 Ready for review

---

## PRÉ-DEPLOYMENT (Revisão & Validação)

### Code Review
- [ ] Merge request: ONDA 1 patches (flags, git gc, audit)
- [ ] Code review: nemo-icon-text-cache.c (255 linhas)
- [ ] Code review: nemo-icon-lazy-loader.c (280 linhas)
- [ ] CI checks: All tests passing
- [ ] Clang static analyzer: 0 warnings

### Testing
- [ ] Unit tests: nemo-icon-text-cache (3 tests)
- [ ] Unit tests: nemo-icon-lazy-loader (mock tests)
- [ ] Integration tests: 500 item folder scroll (smooth)
- [ ] Regression tests: keyboard nav, selection, DND
- [ ] Memory tests: valgrind --leak-check=full (0 leaks)
- [ ] Crash tests: GDB + 1000 item folder (no segfault)

### Performance Validation
- [ ] Baseline established: benchmark-baseline.json
- [ ] Test run: benchmark PR vs baseline
- [ ] Regression check: no >5% degradation
- [ ] Improvement check: cache hit/miss stats logged

### Documentation
- [ ] REFACTORING-ROADMAP.md reviewed
- [ ] ONDA-5-INTEGRACAO.md ready for v6.8
- [ ] ONDA-6-7-AVANCADAS.md ready for future
- [ ] ROADMAP-MASTER-7-ONDAS.md published

---

## DEPLOYMENT EXECUTION

### Build Artifact
- [ ] Clean rebuild: `meson setup build-release --buildtype=release`
- [ ] Full compile: `meson compile -C build-release`
- [ ] Strip binaries: `strip src/nemo src/nemo-desktop`
- [ ] Size check: `du -sh build-release/` (should be ~50–80 MB)

### Git Workflow
- [ ] Branch: `git checkout -b release/v6.7`
- [ ] Tag: `git tag -a v6.7 -m "Performance release: +20% scroll, -38% .git"`
- [ ] Push: `git push origin release/v6.7 --tags`
- [ ] Create release notes

### Distribution
- [ ] Package: .tar.xz for Linux Mint repo
- [ ] Sign: GPG sign release artifacts
- [ ] Upload: to mint-packages CDN
- [ ] Announce: Release notes + changelog

---

## POST-DEPLOYMENT MONITORING

### Week 1–2
- [ ] Monitor issue tracker for regressions
- [ ] Collect performance reports from users
- [ ] Fix critical bugs (if any)
- [ ] Publish first stability update (v6.7.1)

### Week 3+
- [ ] Plan ONDA 5 integration (v6.7.2 or v6.8-beta)
- [ ] Begin ONDA 6 profiling
- [ ] Gather community feedback
- [ ] Update roadmap based on data

---

## ROLLBACK PLAN (If Issues Found)

If >5% regression detected OR critical crash:
```bash
# Revert to v6.6.2
git revert v6.7
git tag v6.7.hotfix
# Re-release as patch

# OR downgrade users:
apt remove nemo-6.7
apt install nemo-6.6.2
```

**Root cause analysis**:
- [ ] Check perf profile (was cache enabled correctly?)
- [ ] Verify memory leaks (valgrind)
- [ ] Check for race conditions (thread sanitizer)

---

## SUCCESS CRITERIA

- [x] ✅ Zero compilation errors
- [x] ✅ Zero memory leaks
- [x] [x] No regression in existing features
- [ ] ⏳ No >5% performance regression detected in CI
- [ ] ⏳ Positive user feedback (week 1)
- [ ] ⏳ Adoption rate >50% in Mint ecosystem (month 1)

---

## SIGN-OFF

| Role | Name | Sign-Off | Date |
|------|------|----------|------|
| Lead Eng | TBD | [ ] | TBD |
| QA Lead | TBD | [ ] | TBD |
| Release Mgr | TBD | [ ] | TBD |

---

**Next release**: v6.8 (Refactoring Phase 1 + ONDA 5 integration)  
**Tentative date**: Q2 2026

