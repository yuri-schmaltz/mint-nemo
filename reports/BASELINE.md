# BASELINE

Data da execução: 2026-02-15
Escopo: Fase 0 (autodescoberta) + Fase 1 (baseline automático antes de mudanças).

## Descoberto
- Stack principal C + Meson (`meson.build:4`) com UI GTK3 (`meson.build:73`).
- Dependências desktop/X11/Cinnamon detectadas (`meson.build:82`, `meson.build:83`, `meson.build:84`, `meson.build:85`).
- Teste automatizado nativo detectado: `Eel test` (`eel/meson.build:40`).
- Workflow de CI usa reusable workflow externo (`.github/workflows/build.yml:20`).
- Recursos de UI em Glade/UI detectados (`reports/EVIDENCE/20260215_ui_files.log`).

## ASSUMIDO
- Opção conservadora: não alterar código funcional do produto nesta execução; somente orquestração, evidência e priorização em `reports/`.
Justificativa: pedido focado em fluxo completo + relatórios + commits incrementais.
Risco: backlog de correções técnicas fica planejado, não executado nesta rodada.

- Opção conservadora para visual headless: sem instalar pacotes no host e sem mudar ambiente do usuário.
Justificativa: `xvfb-run` ausente e política de execução sem intervenção.
Risco: validação visual pixel-perfect/a11y automatizada permanece pendente.

## NÃO VERIFICADO
- Regressão visual por breakpoint (AEGIS) com screenshots automatizadas.
Evidência: `reports/EVIDENCE/20260215_nemo_quit_xvfb.log`.

- Startup real de UI (janela aberta, idle CPU/RAM em sessão gráfica ativa).
Evidência: `reports/EVIDENCE/20260215_nemo_version_time.log`, `reports/EVIDENCE/20260215_nemo_quit_time.log`.

## Metodologia
1. Descoberta de stack/UI/testes/CI por varredura (`reports/EVIDENCE/20260215_analysis_results.txt`).
2. Baseline de build: `meson setup` e `meson compile` em `build-orch-baseline`.
3. Baseline funcional mínimo: `meson test` e smoke CLI dos binários principais.
4. Consolidação de evidências versionáveis em `.txt` dentro de `reports/EVIDENCE/`.

## Métricas Baseline (antes de mudanças)
| Item | Resultado | Evidência |
|---|---|---|
| `meson setup -C build-orch-baseline` | PASS | `reports/EVIDENCE/20260215_meson_setup_baseline.log` |
| `meson compile -C build-orch-baseline` | PASS | `reports/EVIDENCE/20260215_meson_compile_baseline.log` |
| `meson test -C build-orch-baseline` | FAIL (`cannot open display`) | `reports/EVIDENCE/20260215_meson_test_baseline.log` |
| `nemo --version` (sistema) | FAIL (`Cannot open display`) | `reports/EVIDENCE/20260215_nemo_version_time.log` |
| `nemo --quit` (sistema) | FAIL (`Cannot open display`) | `reports/EVIDENCE/20260215_nemo_quit_time.log` |
| `xvfb-run -a nemo --quit` | FAIL (`xvfb-run not available`) | `reports/EVIDENCE/20260215_nemo_quit_xvfb.log` |
| `build-orch-baseline/src/nemo --help` | PASS | `reports/EVIDENCE/20260215_smoke_nemo_help.log` |
| `build-orch-baseline/src/nemo-connect-server --help` | PASS | `reports/EVIDENCE/20260215_smoke_connect_server_help.log` |
| `build-orch-baseline/src/nemo-open-with --help` | PASS | `reports/EVIDENCE/20260215_smoke_open_with_help.log` |
