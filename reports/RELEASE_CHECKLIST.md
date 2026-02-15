# RELEASE_CHECKLIST

## QA
- [x] `meson setup -C build-orch-baseline` PASS (`reports/EVIDENCE/20260215_meson_setup_baseline.log`).
- [x] `meson compile -C build-orch-baseline` PASS (`reports/EVIDENCE/20260215_meson_compile_baseline.log`).
- [ ] `meson test -C build-orch-baseline` PASS (bloqueado por display ausente: `reports/EVIDENCE/20260215_meson_test_baseline.log`).
- [x] Smoke CLI executado (`nemo --help`, `nemo-connect-server --help`, `nemo-open-with --help`).

## UI/AEGIS
- [ ] Baseline visual por breakpoint com screenshot automatizado.
- [ ] Confirmação objetiva de zero clipping/overlap > 2px.
- [ ] Confirmação de foco visível e navegação por teclado.
- [ ] Confirmação de ausência de scroll horizontal inesperado.
Observação: itens acima seguem NÃO VERIFICADO nesta execução por ausência de `xvfb-run`.

## GTK4 Readiness
- [x] Mapeamento de acoplamento GTK3 coletado (`reports/EVIDENCE/20260215_gtk3_api.log`).
- [x] Estratégia registrada com no máximo 2 opções no backlog (`reports/BACKLOG.md`, item `BKL-006`).
- [ ] Plano de migração por subsistema validado em branch dedicada.
- [ ] Suite visual/a11y comparativa GTK3 vs GTK4 estabelecida.

## Segurança/Operação
- [x] Inventário de pontos `g_spawn*` coletado com linhas exatas (`reports/EVIDENCE/20260215_security_exec_scan.log`).
- [ ] Endurecimento de validação de argumentos/origem aplicado em todos os pontos críticos.
- [x] Varredura de listeners de rede local sem achados (`reports/EVIDENCE/20260215_desktop_network_scan.log`).

## Release Docs
- [x] `reports/BASELINE.md` atualizado.
- [x] `reports/SYSTEM_MAP.md` atualizado.
- [x] `reports/BACKLOG.md` atualizado e priorizado.
- [x] `reports/CHANGES.md` finalizado com commits e rollback.
- [x] `reports/EXEC_SUMMARY.md` finalizado.

## Rollback global
1. `git revert HEAD`
2. `git revert 5ed25ac`
3. `git revert 9630d56`
4. `git revert a77819c`
