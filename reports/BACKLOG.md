# BACKLOG

Critério de score: `(Impacto x Probabilidade) / Esforço`.
Escala: 1 (baixo) a 5 (alto).

## Achados

### BLD-001
- Categoria: Build/Release
- Descrição objetiva: `meson setup` quebrava por opção inexistente (`deprecated_warnings`).
- Evidência: `meson.build:47`, `reports/EVIDENCE/14_meson_setup_baseline.log`.
- Impacto: bloqueio total de configure/build.
- Causa provável: `meson_options.txt` ausente no checkout.
- Recomendação (o que/onde): manter `meson_options.txt` com todas as opções usadas (`meson_options.txt:1` a `meson_options.txt:49`).
- Validação PASS/FAIL: PASS (`reports/EVIDENCE/60_meson_setup_wave4.log`).
- Risco + mitigação: baixo; defaults explícitos e rollback por commit.
- Rollback: `git revert b354c13`.
- Priorização: Severidade 5; Impacto 5; Probabilidade 5; Esforço 1; Risco regressão 1; Urgência 5; Score 25.0.

### BLD-002
- Categoria: Build/Portabilidade
- Descrição objetiva: `subdir('search-helpers')` e `subdir('test')` sem diretórios no checkout parcial.
- Evidência: `meson.build:190` a `meson.build:199`, `reports/EVIDENCE/04_missing_dirs_check.log`.
- Impacto: configure quebraria em árvores parciais.
- Causa provável: variação de layout do source tree.
- Recomendação (o que/onde): manter inclusão condicional com `fs.is_dir(...)` (`meson.build:190` a `meson.build:199`).
- Validação PASS/FAIL: PASS (mensagens explícitas em `reports/EVIDENCE/60_meson_setup_wave4.log`).
- Risco + mitigação: baixo; mensagem explícita evita ocultar ausência.
- Rollback: `git revert bf67921`.
- Priorização: Severidade 4; Impacto 4; Probabilidade 4; Esforço 2; Risco regressão 2; Urgência 4; Score 8.0.

### BLD-003
- Categoria: Build/Integridade de Código
- Descrição objetiva: módulos usados em runtime estavam ausentes (`nemo-icon-text-cache`, `nemo-lazy-thumbnail-loader`).
- Evidência: `reports/EVIDENCE/40_meson_setup_wave2.log`, `reports/EVIDENCE/50_meson_setup_wave3.log`, `libnemo-private/nemo-icon-canvas-item.c:42`, `libnemo-private/nemo-icon-container.c:41`.
- Impacto: quebra de setup/compile.
- Causa provável: checkout incompleto de arquivos auxiliares.
- Recomendação (o que/onde): manter implementações fallback mínimas em `libnemo-private/nemo-icon-text-cache.*` e `libnemo-private/nemo-lazy-thumbnail-loader.*`.
- Validação PASS/FAIL: PASS (`reports/EVIDENCE/61_meson_compile_wave4.log`).
- Risco + mitigação: médio; fallback é no-op/compatível, com foco em preservar fluxo funcional e compilabilidade.
- Rollback: `git revert bf67921`.
- Priorização: Severidade 5; Impacto 5; Probabilidade 4; Esforço 3; Risco regressão 2; Urgência 5; Score 6.7.

### BLD-004
- Categoria: Build/Configuração
- Descrição objetiva: `empty_view=true` aciona compilação de unidade com símbolo ausente neste checkout.
- Evidência: erro em `reports/EVIDENCE/57_meson_compile_wave3b_retry.log`; opção em `meson_options.txt:45` a `meson_options.txt:48`.
- Impacto: compile interrompe no fim do pipeline.
- Causa provável: descompasso entre feature flag e árvore de código disponível.
- Recomendação (o que/onde): manter `empty_view=false` por padrão (`meson_options.txt:47`).
- Validação PASS/FAIL: PASS (`reports/EVIDENCE/61_meson_compile_wave4.log`).
- Risco + mitigação: baixo-médio; mudança de default documentada e reversível.
- Rollback: `git revert <commit-onda-4>`.
- Priorização: Severidade 4; Impacto 4; Probabilidade 4; Esforço 1; Risco regressão 2; Urgência 4; Score 16.0.

### QA-001
- Categoria: QA/Release
- Descrição objetiva: suíte de teste depende de display e falha em headless.
- Evidência: `reports/EVIDENCE/62_meson_test_wave4.log`, `eel/meson.build:40`.
- Impacto: CI/headless sem display não valida regressões funcionais via teste atual.
- Causa provável: teste GTK sem backend virtual.
- Recomendação (o que/onde): executar testes com Xvfb no pipeline.
- Validação PASS/FAIL: FAIL (ambiente atual).
- Risco + mitigação: médio; adicionar job com Xvfb e separar smoke não-GUI.
- Rollback: remover ajuste de pipeline.
- Priorização: Severidade 4; Impacto 4; Probabilidade 4; Esforço 2; Risco regressão 1; Urgência 4; Score 8.0.

### UI-001
- Categoria: UI/AEGIS
- Descrição objetiva: sem evidência automática de regressão visual/a11y por breakpoint.
- Evidência: `reports/EVIDENCE/20_xvfb_tools.log`, `reports/EVIDENCE/21_ui_validate_xvfb.log`, `reports/EVIDENCE/62_meson_test_wave4.log`.
- Impacto: clipping/overlap e problemas de foco podem passar sem detecção automatizada.
- Causa provável: falta de ambiente gráfico virtual.
- Recomendação (o que/onde): job dedicado de screenshot/validação headless.
- Validação PASS/FAIL: NÃO VERIFICADO.
- Risco + mitigação: médio; manter checklist manual + automatizar quando Xvfb disponível.
- Rollback: remover job visual.
- Priorização: Severidade 3; Impacto 3; Probabilidade 4; Esforço 3; Risco regressão 1; Urgência 3; Score 4.0.

### GTK4-001
- Categoria: Migração GTK4
- Descrição objetiva: forte acoplamento a APIs GTK3 legadas (`GtkAction`, `GtkUIManager`, `GtkAlignment`, `GtkMisc`).
- Evidência: `reports/EVIDENCE/30_gtk3_api_scan.log`, `reports/EVIDENCE/31_glade_legacy_widgets.log`.
- Impacto: migração GTK4 requer projeto dedicado e regressão controlada.
- Causa provável: arquitetura histórica da base.
- Recomendação (máx. 2 opções, recomendada 1):
  - Opção 1 (Recomendada): GTK4 puro, migração incremental por subsistema.
  - Opção 2: GTK4 + libadwaita (GNOME-first, exigir validação cross-platform).
- Validação PASS/FAIL: FAIL para prontidão GTK4.
- Risco + mitigação: alto; executar em branch separado com suite visual/E2E.
- Rollback: manter branch GTK3 como fallback até paridade comprovada.
- Priorização: Severidade 4; Impacto 5; Probabilidade 5; Esforço 5; Risco regressão 5; Urgência 3; Score 5.0.
