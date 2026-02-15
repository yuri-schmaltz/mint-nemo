# SYSTEM_MAP

## Componentes
- Build system: Meson/Ninja (`meson.build`, `reports/EVIDENCE/20260215_env_toolchain.log`).
- Core app: `src/nemo` (GtkApplication) (`reports/EVIDENCE/20260215_entrypoints.log`).
- Desktop mode: `src/nemo-desktop` (desktop window manager integrado ao Nemo).
- Bibliotecas internas: `libnemo-private/`, `eel/`, `libnemo-extension/`.
- UI declarativa: arquivos Glade/UI em `gresources/` (`reports/EVIDENCE/20260215_ui_files.log`).
- CI: workflow em `.github/workflows/build.yml` chamando reusable pipeline externo (`reports/EVIDENCE/20260215_build_workflow_lines.txt`).

## Fluxos E2E (automatizáveis nesta execução)
1. Configuração de build.
Comando: `meson setup build-orch-baseline`.
Evidência: `reports/EVIDENCE/20260215_meson_setup_baseline.log`.

2. Compilação completa de binários.
Comando: `meson compile -C build-orch-baseline`.
Evidência: `reports/EVIDENCE/20260215_meson_compile_baseline.log`.

3. Execução de suíte nativa (`Eel test`).
Comando: `meson test -C build-orch-baseline --print-errorlogs`.
Evidência: `reports/EVIDENCE/20260215_meson_test_baseline.log`.

4. Smoke CLI do binário principal.
Comando: `build-orch-baseline/src/nemo --help`.
Evidência: `reports/EVIDENCE/20260215_smoke_nemo_help.log`.

5. Smoke CLI de utilitários.
Comandos: `build-orch-baseline/src/nemo-connect-server --help`, `build-orch-baseline/src/nemo-open-with --help`.
Evidência: `reports/EVIDENCE/20260215_smoke_connect_server_help.log`, `reports/EVIDENCE/20260215_smoke_open_with_help.log`.

## Fluxos críticos não verificáveis no ambiente atual
- Fluxo visual com janela GTK renderizada e screenshot baseline.
Bloqueio: ausência de display + ausência de `xvfb-run`.
Evidência: `reports/EVIDENCE/20260215_nemo_quit_xvfb.log`.
