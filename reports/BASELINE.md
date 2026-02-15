# BASELINE

Data: 2026-02-15
Escopo: baseline automatizado antes e depois das ondas incrementais.

## Descoberto
- Stack principal C + Meson + GTK3 (`meson.build:4`, `meson.build:73`).
- Dependências desktop: Cinnamon/XApp/X11 (`meson.build:82` a `meson.build:85`).
- Teste Meson detectado: `Eel test` (`eel/meson.build:40`).
- CI build detectado em workflow reutilizável (`.github/workflows/build.yml:20`).
- Ambiente local com toolchain completo (`reports/EVIDENCE/10_env_toolchain.log`).

## ASSUMIDO
- Defaults de opções Meson inferidos de empacotamento e histórico local quando ausentes (`debian/rules:9` a `debian/rules:11`, `debian/changelog` em `reports/EVIDENCE/18_changelog_options_context.log`).
  - Justificativa: checkout sem `meson_options.txt` inicialmente.
  - Risco: divergência pontual de defaults em forks/branches alternativos.
- `empty_view=false` adotado como escolha conservadora para manter build funcional neste checkout (evita compilar unidade com símbolo ausente observado em `reports/EVIDENCE/57_meson_compile_wave3b_retry.log`).

## NÃO VERIFICADO
- Regressão visual pixel-perfect e a11y por screenshot headless: sem Xvfb (`reports/EVIDENCE/20_xvfb_tools.log`, `reports/EVIDENCE/21_ui_validate_xvfb.log`).
- Startup/idle metrics com UI real: ambiente sem display (`reports/EVIDENCE/12_system_nemo_baseline.log`).

## Metodologia
1. Fase 0: autodescoberta de stack/build/UI/testes/CI (`reports/EVIDENCE/01_build_gtk_scan.log`, `reports/EVIDENCE/02_tests_ci_scan.log`, `reports/EVIDENCE/03_ui_scan.log`).
2. Fase 1: baseline pré-mudança (`reports/EVIDENCE/14_meson_setup_baseline.log`, `reports/EVIDENCE/15_meson_compile_baseline.log`, `reports/EVIDENCE/16_meson_test_baseline.log`).
3. Fases 2-4: execução incremental, revalidação e comparação (`reports/EVIDENCE/40_meson_setup_wave2.log`, `reports/EVIDENCE/54_meson_setup_wave3b.log`, `reports/EVIDENCE/60_meson_setup_wave4.log`, `reports/EVIDENCE/61_meson_compile_wave4.log`, `reports/EVIDENCE/62_meson_test_wave4.log`).

## Métricas (ANTES vs DEPOIS)
| Dimensão | Antes | Depois |
|---|---|---|
| `meson setup` | FAIL (`unknown option 'deprecated_warnings'`) | PASS (`Exit status: 0`) |
| `meson compile` | FAIL (sem build válido) | PASS (`Exit status: 0`) |
| `meson test` | FAIL (sem build válido) | FAIL (`cannot open display`) |
| Startup probe (`nemo --version`) | FAIL (`Cannot open display`) | FAIL (mesma limitação ambiental) |
| UI headless/screenshot | NÃO VERIFICADO | NÃO VERIFICADO |

Evidências principais:
- Antes: `reports/EVIDENCE/14_meson_setup_baseline.log`, `reports/EVIDENCE/15_meson_compile_baseline.log`, `reports/EVIDENCE/16_meson_test_baseline.log`.
- Depois: `reports/EVIDENCE/60_meson_setup_wave4.log`, `reports/EVIDENCE/61_meson_compile_wave4.log`, `reports/EVIDENCE/62_meson_test_wave4.log`.
