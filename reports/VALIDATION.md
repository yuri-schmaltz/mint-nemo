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
- Revalidação rápida:

| Verificação | Status | Evidência |
|---|---|---|
| `meson setup -C build-orch-baseline` | PASS | `reports/EVIDENCE/20260215_meson_setup_baseline.log` |
| `meson compile -C build-orch-baseline` | PASS | `reports/EVIDENCE/20260215_meson_compile_baseline.log` |
| `meson test -C build-orch-baseline` | FAIL (mesma limitação de display) | `reports/EVIDENCE/20260215_meson_test_baseline.log` |

Conclusão Onda 1: PASS para objetivo de baseline/guardrails; FAIL conhecido restrito a display headless.
