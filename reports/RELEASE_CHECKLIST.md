# RELEASE_CHECKLIST

## QA
- [x] `meson setup` PASS em ambiente limpo (`build-wave4`).
- [x] `meson compile` PASS (`build-wave4`).
- [ ] `meson test` PASS (bloqueado por display ausente; ver `reports/EVIDENCE/62_meson_test_wave4.log`).
- [x] Smoke de build executado (targets principais linkados).

## UI/AEGIS
- [ ] Sem clipping/overlap >2px nos fluxos críticos (NÃO VERIFICADO neste ambiente).
- [ ] Sem scroll horizontal indevido (NÃO VERIFICADO neste ambiente).
- [ ] Foco visível e navegação por teclado (NÃO VERIFICADO neste ambiente).
- [ ] Evidência de screenshots por breakpoint (sem Xvfb disponível).

## Segurança/Operação
- [ ] Pontos de `g_spawn_*` revisados com modelagem de ameaça completa (pendente).
- [x] Logs/evidências de erro preservados em `reports/EVIDENCE/`.

## Release
- [x] `reports/VALIDATION.md` atualizado com PASS/FAIL antes/depois.
- [x] `reports/CHANGES.md` atualizado com ondas e rollback.
- [x] `reports/EXEC_SUMMARY.md` final preenchido.

## Rollback global
1. `git revert <commit-onda-4>`
2. `git revert bf67921`
3. `git revert b354c13`
4. `git revert ec36feb`
