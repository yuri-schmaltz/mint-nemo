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
| Item | Resultado | Evidência |
|---|---|---|
| `meson setup build-wave4` | PASS | `reports/EVIDENCE/60_meson_setup_wave4.log` |
| `meson compile -C build-wave4` | PASS | `reports/EVIDENCE/61_meson_compile_wave4.log` |
| `meson test -C build-wave4 --print-errorlogs` | FAIL (`Gtk-WARNING cannot open display`) | `reports/EVIDENCE/62_meson_test_wave4.log` |
| Smoke funcional de build (targets `nemo`/`nemo-desktop`) | PASS (targets linkados) | `reports/EVIDENCE/61_meson_compile_wave4.log` |
| UI headless / screenshots AEGIS | NÃO VERIFICADO | `reports/EVIDENCE/20_xvfb_tools.log` |

## Delta objetivo
- Configure/build evoluiu de FAIL para PASS.
- Falha residual está concentrada em testes GUI sem display virtual.
