# CHANGES

## Onda 1
- Commit: `ec36feb`
- Arquivos: `reports/BASELINE.md`, `reports/SYSTEM_MAP.md`, `reports/BACKLOG.md`, `reports/VALIDATION.md`, `reports/CHANGES.md`, `reports/EXEC_SUMMARY.md`, `reports/RELEASE_CHECKLIST.md`.
- Contexto ANTES: não existiam relatórios padronizados em `reports/`.
- Contexto DEPOIS: baseline, backlog, validação e checklist formalizados.
- Rollback: `git revert ec36feb`.

## Onda 2
- Commit: `b354c13`
- Arquivos: `meson_options.txt`.
- Contexto ANTES: `meson.build` usava `get_option(...)` sem arquivo de opções, quebrando configure.
- Contexto DEPOIS: opções declaradas (`deprecated_warnings`, `tracker`, `gtk_doc`, `exif`, `xmp`, `selinux`, `empty_view`).
- Evidência: `reports/EVIDENCE/14_meson_setup_baseline.log` (ANTES), `reports/EVIDENCE/40_meson_setup_wave2.log` (DEPOIS parcial).
- Rollback: `git revert b354c13`.

## Onda 3
- Commit: `bf67921`
- Arquivos: `meson.build`, `libnemo-private/meson.build`, `libnemo-private/nemo-icon-text-cache.c`, `libnemo-private/nemo-icon-text-cache.h`, `libnemo-private/nemo-lazy-thumbnail-loader.c`, `libnemo-private/nemo-lazy-thumbnail-loader.h`.
- Contexto ANTES:
  - `meson.build` referenciava subdirs ausentes sem fallback.
  - `libnemo-private/meson.build` listava fonte inexistente.
  - APIs usadas por `nemo-icon-canvas-item`/`nemo-icon-container` não estavam presentes no checkout.
- Contexto DEPOIS:
  - inclusão condicional de subdirs ausentes;
  - remoção de referência quebrada;
  - módulos fallback adicionados para restabelecer compilabilidade com comportamento seguro.
- Evidência: `reports/EVIDENCE/50_meson_setup_wave3.log`, `reports/EVIDENCE/54_meson_setup_wave3b.log`, `reports/EVIDENCE/57_meson_compile_wave3b_retry.log`.
- Rollback: `git revert bf67921`.

## Onda 4
- Commit: atual (neste ciclo)
- Arquivos: `meson_options.txt` (default `empty_view=false`) + atualização final de `reports/`.
- Contexto ANTES: compile quebrava com `empty_view=true` por símbolo ausente em `nemo-empty-view.c`.
- Contexto DEPOIS: configure+compile PASS em `build-wave4`; teste ainda FAIL por ausência de display.
- Evidência: `reports/EVIDENCE/57_meson_compile_wave3b_retry.log` (ANTES), `reports/EVIDENCE/61_meson_compile_wave4.log` e `reports/EVIDENCE/62_meson_test_wave4.log` (DEPOIS).
- Rollback: `git revert HEAD`.
