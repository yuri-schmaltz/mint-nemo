# BASELINE

Data: 2026-02-15
Escopo: baseline automatizado antes de alterações de código (build/test/UI/perf mínima).

## Descoberto
- Stack principal C + Meson + GTK3 (`meson.build:4`, `meson.build:72`).
- Projeto usa GTK3 legado e componentes de desktop Cinnamon/XApp (`meson.build:81` a `meson.build:85`).
- Há um teste Meson explícito em `eel/meson.build:40`.
- CI GitHub Actions referencia workflow reutilizável de build (`.github/workflows/build.yml:20`).
- Baseline de ferramentas local disponível: Meson 1.3.2, Ninja 1.11.1, GCC 13.3.0 (`reports/EVIDENCE/10_env_toolchain.log`).
- Dependências pkg-config essenciais foram resolvidas localmente (`reports/EVIDENCE/11_pkg_config_baseline.log`).

## ASSUMIDO
- Defaults de opções Meson foram inferidos do empacotamento Debian quando necessário (`debian/rules:9` a `debian/rules:11`).
  - Justificativa: `meson_options.txt` não existe no checkout.
  - Risco: defaults inferidos podem divergir do upstream esperado para todos os ambientes.
- Fluxos E2E críticos foram definidos por executáveis/artefatos de UI declarados em Meson (`src/meson.build:116`, `src/meson.build:124`, `src/meson.build:140`).

## NÃO VERIFICADO
- Regressão visual pixel-perfect (AEGIS) por breakpoint: sem Xvfb no ambiente (`reports/EVIDENCE/20_xvfb_tools.log`, `reports/EVIDENCE/21_ui_validate_xvfb.log`).
- Métricas de CPU/RAM em idle do binário compilado localmente: build baseline falhou antes da compilação (`reports/EVIDENCE/14_meson_setup_baseline.log`).
- Screenshots GTK headless: bloqueado por ausência de Xvfb (`reports/EVIDENCE/23_xvfb_binary_probe.log`).

## Metodologia
1. Descoberta automática de stack/UI/testes/CI por varredura (`reports/EVIDENCE/01_build_gtk_scan.log`, `reports/EVIDENCE/02_tests_ci_scan.log`, `reports/EVIDENCE/03_ui_scan.log`).
2. Baseline de ambiente e dependências (`reports/EVIDENCE/10_env_toolchain.log`, `reports/EVIDENCE/11_pkg_config_baseline.log`, `reports/EVIDENCE/19_optional_dep_probe.log`).
3. Baseline funcional/build via Meson setup/compile/test (`reports/EVIDENCE/14_meson_setup_baseline.log`, `reports/EVIDENCE/15_meson_compile_baseline.log`, `reports/EVIDENCE/16_meson_test_baseline.log`).
4. Baseline de execução/superfície UI em ambiente sem display (`reports/EVIDENCE/12_system_nemo_baseline.log`, `reports/EVIDENCE/13_ui_validate_baseline.log`).

## Métricas Baseline (ANTES)
| Dimensão | Resultado | Evidência |
|---|---|---|
| Build configure | FAIL (`unknown option 'deprecated_warnings'`) | `reports/EVIDENCE/14_meson_setup_baseline.log` |
| Build compile | FAIL (sem build dir válido) | `reports/EVIDENCE/15_meson_compile_baseline.log` |
| Testes | FAIL (build não inicializado) | `reports/EVIDENCE/16_meson_test_baseline.log` |
| Startup probe (binário do sistema) | FAIL (`Cannot open display`) | `reports/EVIDENCE/12_system_nemo_baseline.log` |
| Validação GTK Builder | FAIL por display ausente | `reports/EVIDENCE/13_ui_validate_baseline.log` |
| Screenshot headless | NÃO VERIFICADO (sem Xvfb) | `reports/EVIDENCE/20_xvfb_tools.log` |

## Critérios PASS/FAIL adotados
- PASS Build: `meson setup` concluído com código 0.
- PASS Compile: `meson compile` concluído com código 0.
- PASS Test: `meson test` executado e sem falhas.
- PASS Visual/UI: validação de artefatos UI sem erro crítico e/ou screenshots headless por fluxo.
- PASS Performance mínima: tempo de startup mensurável sem erro fatal de ambiente.
