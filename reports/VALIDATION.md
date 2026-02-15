# VALIDATION

## Critérios
- PASS: comando executa com código 0 e resultado esperado.
- FAIL: comando retorna erro ou bloqueio ambiental.
- NÃO VERIFICADO: exige recurso indisponível no ambiente (ex.: display virtual).

## Antes (Baseline)
| Verificação | Status | Evidência |
|---|---|---|
| `meson setup -C build-orch-baseline` | PASS | `reports/EVIDENCE/20260215_meson_setup_baseline.log` |
| `meson compile -C build-orch-baseline` | PASS | `reports/EVIDENCE/20260215_meson_compile_baseline.log` |
| `meson test -C build-orch-baseline` | FAIL (`Gtk-WARNING cannot open display`) | `reports/EVIDENCE/20260215_meson_test_baseline.log` |
| `nemo --version` | FAIL (`Cannot open display`) | `reports/EVIDENCE/20260215_nemo_version_time.log` |
| `nemo --quit` | FAIL (`Cannot open display`) | `reports/EVIDENCE/20260215_nemo_quit_time.log` |
| `xvfb-run -a nemo --quit` | FAIL (`xvfb-run not available`) | `reports/EVIDENCE/20260215_nemo_quit_xvfb.log` |
| `build-orch-baseline/src/nemo --help` | PASS | `reports/EVIDENCE/20260215_smoke_nemo_help.log` |
| `build-orch-baseline/src/nemo-connect-server --help` | PASS | `reports/EVIDENCE/20260215_smoke_connect_server_help.log` |
| `build-orch-baseline/src/nemo-open-with --help` | PASS | `reports/EVIDENCE/20260215_smoke_open_with_help.log` |

## Depois (Onda 1)
- Sem alteração funcional de código nesta onda.
- Resultado esperado: paridade funcional 100% preservada.

| Verificação | Status | Evidência |
|---|---|---|
| `meson setup -C build-orch-baseline` | PASS | `reports/EVIDENCE/20260215_meson_setup_baseline.log` |
| `meson compile -C build-orch-baseline` | PASS | `reports/EVIDENCE/20260215_meson_compile_baseline.log` |
| `meson test -C build-orch-baseline` | FAIL (mesma limitação de display) | `reports/EVIDENCE/20260215_meson_test_baseline.log` |

## Depois (Onda 2)
- Onda documental (achados + priorização) sem alteração de binários.
- Regressão funcional: nenhuma observada.

| Verificação | Status | Evidência |
|---|---|---|
| Integridade de baseline mantida | PASS | `reports/EVIDENCE/20260215_baseline_results.txt` |
| Backlog com score e rollback por item | PASS | `reports/BACKLOG.md` |

## Depois (Onda 3)
- Onda documental para módulos complexos (AEGIS/GTK4/Release).

| Verificação | Status | Evidência |
|---|---|---|
| Checklist de release atualizado | PASS | `reports/RELEASE_CHECKLIST.md` |
| Critérios AEGIS explicitados | PASS (documentado) | `reports/RELEASE_CHECKLIST.md` |
| Validação visual headless | NÃO VERIFICADO | `reports/EVIDENCE/20260215_nemo_quit_xvfb.log` |
| Prontidão GTK4 imediata | FAIL (alto acoplamento GTK3) | `reports/BACKLOG.md` (`BKL-006`) |

Conclusão parcial: baseline técnico estável; bloqueio residual permanece ambiental (display virtual ausente).
