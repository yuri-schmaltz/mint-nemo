# EVIDENCE SUMMARY

## Baseline crítico (antes)
- `meson setup build-baseline` falhou com:
  - `ERROR: Tried to access unknown option 'deprecated_warnings'.`
  - Fonte: `reports/EVIDENCE/14_meson_setup_baseline.log`.

## Evolução por ondas
- Após Onda 2 (`meson_options.txt`):
  - `meson setup` avançou além da fase de opções e encontrou próximos bloqueios de árvore.
  - Fonte: `reports/EVIDENCE/40_meson_setup_wave2.log`.

- Após Onda 3 (fallback/estrutura):
  - `meson setup build-wave3b` passou (`Exit status: 0`).
  - Fonte: `reports/EVIDENCE/54_meson_setup_wave3b.log`.

- Após Onda 4 (ajuste `empty_view`):
  - `meson setup build-wave4` PASS.
  - `meson compile -C build-wave4` PASS.
  - `meson test -C build-wave4` FAIL por `cannot open display`.
  - Fontes: `reports/EVIDENCE/60_meson_setup_wave4.log`, `reports/EVIDENCE/61_meson_compile_wave4.log`, `reports/EVIDENCE/62_meson_test_wave4.log`.

## Ambiente
- Ferramentas detectadas: Meson 1.3.2, Ninja 1.11.1, GCC 13.3.0.
- Dependências GTK3/Cinnamon/XApp presentes via pkg-config.
- Fonte: `reports/EVIDENCE/10_env_toolchain.log`, `reports/EVIDENCE/11_pkg_config_baseline.log`.

## Limitações ambientais
- Sem `xvfb-run`/`Xvfb`; validações visuais headless e testes GUI ficam bloqueados.
- Fonte: `reports/EVIDENCE/20_xvfb_tools.log`, `reports/EVIDENCE/23_xvfb_binary_probe.log`.
