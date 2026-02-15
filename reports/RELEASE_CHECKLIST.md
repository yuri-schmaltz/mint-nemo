# RELEASE_CHECKLIST

## QA
- [ ] `meson setup` PASS em ambiente limpo.
- [ ] `meson compile` PASS.
- [ ] `meson test` PASS (ou justificativa documentada).
- [ ] Smoke dos fluxos críticos executado.

## UI/AEGIS
- [ ] Sem clipping/overlap >2px nos fluxos críticos.
- [ ] Sem scroll horizontal indevido.
- [ ] Foco visível e navegação por teclado nos principais diálogos.
- [ ] Evidência de screenshots por breakpoint (quando Xvfb disponível).

## Segurança/Operação
- [ ] Pontos de `g_spawn_*` revisados em relação a entradas externas.
- [ ] Logs de erro/diagnóstico preservados.

## Release
- [ ] `reports/VALIDATION.md` atualizado com PASS/FAIL antes/depois.
- [ ] `reports/CHANGES.md` com commits e rollback de cada onda.
- [ ] `reports/EXEC_SUMMARY.md` final preenchido.

## Rollback global
- Reverter commits em ordem inversa das ondas:
  1. `git revert <onda-4>`
  2. `git revert <onda-3>`
  3. `git revert <onda-2>`
  4. `git revert <onda-1>`
