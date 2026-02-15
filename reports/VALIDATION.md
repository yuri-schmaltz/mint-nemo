# VALIDATION

## Antes (baseline)
| Item | Resultado | Evidência |
|---|---|---|
| `meson setup build-baseline` | FAIL | `reports/EVIDENCE/14_meson_setup_baseline.log` |
| `meson compile -C build-baseline` | FAIL | `reports/EVIDENCE/15_meson_compile_baseline.log` |
| `meson test -C build-baseline` | FAIL | `reports/EVIDENCE/16_meson_test_baseline.log` |
| `nemo --version` (ambiente atual) | FAIL (`Cannot open display`) | `reports/EVIDENCE/12_system_nemo_baseline.log` |
| `gtk-builder-tool validate` | FAIL (display ausente) | `reports/EVIDENCE/13_ui_validate_baseline.log` |
| Visual headless via Xvfb | NÃO VERIFICADO | `reports/EVIDENCE/21_ui_validate_xvfb.log` |

## Depois (pós-ondas)
- Pendente de execução nas ondas 2 a 4.
