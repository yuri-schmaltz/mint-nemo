# BACKLOG

Critério de score: `(Impacto x Probabilidade) / Esforço`.
Escala: 1 (baixo) a 5 (alto).

## BKL-001
- Categoria: QA/Release
- Descrição objetiva: `meson test` falha em ambiente headless por dependência de display GTK.
- Evidência: `reports/EVIDENCE/20260215_meson_test_baseline.log`, `reports/EVIDENCE/20260215_eel_meson_lines.txt`.
- Impacto: cobertura de regressão automática fica incompleta em ambientes sem display.
- Causa provável: teste `Eel test` executa binário GTK sem backend virtual.
- Recomendação (o que e onde): pipeline de teste com display virtual (Xvfb ou equivalente) no job de testes GTK.
- Validação PASS/FAIL: FAIL no ambiente atual.
- Risco + mitigação: risco médio; mitigar separando smoke CLI (sempre) e testes GUI (headless virtual).
- Rollback: remover etapa headless do pipeline (`git revert <commit-da-implementacao>`).
- Priorização: Severidade 5; Impacto 5; Probabilidade 5; Esforço 2; Risco de regressão 1; Urgência 5; Score 12.5.

## BKL-002
- Categoria: Observabilidade/Diagnóstico
- Descrição objetiva: há logging extenso (`g_warning`, `g_message`, `g_critical`), mas sem plano explícito de coleta padronizada por cenário de release.
- Evidência: `reports/EVIDENCE/20260215_observability_scan.log`.
- Impacto: diagnóstico pós-incidente mais lento e inconsistente entre ambientes.
- Causa provável: instrumentação existe, mas sem runbook consolidado de coleta.
- Recomendação (o que e onde): padronizar comando de coleta de logs por fluxo crítico no checklist de release.
- Validação PASS/FAIL: FAIL parcial (instrumentação existe, runbook não formalizado no baseline).
- Risco + mitigação: baixo; manter mudanças restritas a documentação/processo.
- Rollback: reverter atualização de checklist/runbook (`git revert <commit-da-implementacao>`).
- Priorização: Severidade 3; Impacto 3; Probabilidade 4; Esforço 1; Risco de regressão 1; Urgência 3; Score 12.0.

## BKL-003
- Categoria: Segurança
- Descrição objetiva: execução de comandos externos sensíveis via spawn (`pkexec nemo --fix-cache`, terminal launcher, exec de extensões).
- Evidência: `reports/EVIDENCE/20260215_thumbnail_fixcache_lines.txt`, `reports/EVIDENCE/20260215_nemo_view_spawn_lines.txt`, `reports/EVIDENCE/20260215_window_menus_spawn_lines.txt`, `reports/EVIDENCE/20260215_extensions_list_spawn_lines.txt`.
- Impacto: superfície de risco em fluxos privilegiados/execução externa.
- Causa provável: requisitos funcionais de integração com terminal/admin e extensões.
- Recomendação (o que e onde): endurecer validação de origem e argumentos antes de spawn, com logs de auditoria mínimos.
- Validação PASS/FAIL: NÃO VERIFICADO (análise estática; sem exploração dinâmica nesta execução).
- Risco + mitigação: risco médio; mitigar com allowlist de binários e validação explícita de entradas.
- Rollback: reverter endurecimento específico de spawn (`git revert <commit-da-implementacao>`).
- Priorização: Severidade 4; Impacto 4; Probabilidade 3; Esforço 3; Risco de regressão 2; Urgência 4; Score 4.0.

## BKL-004
- Categoria: Desktop/Compatibilidade
- Descrição objetiva: não foram encontradas evidências de servidor de rede local embutido (`localhost`, `listen`, `socket`) no escopo varrido.
- Evidência: `reports/EVIDENCE/20260215_desktop_network_scan.log` (`NO_MATCH`).
- Impacto: positivo para superfície de ataque de rede local.
- Causa provável: arquitetura focada em desktop local/DBus/GIO, sem daemon HTTP local.
- Recomendação (o que e onde): manter regra de não abrir portas locais por padrão; validar em revisão de PRs.
- Validação PASS/FAIL: PASS.
- Risco + mitigação: baixo; risco de regressão caso novos módulos adicionem listener sem guardrail.
- Rollback: não aplicável (achado informativo).
- Priorização: Severidade 2; Impacto 2; Probabilidade 2; Esforço 1; Risco de regressão 1; Urgência 2; Score 4.0.

## BKL-005
- Categoria: UI/AEGIS
- Descrição objetiva: sem baseline visual automatizado por breakpoint, portanto clipping/overlap/foco não foram comprovados nesta execução.
- Evidência: `reports/EVIDENCE/20260215_nemo_quit_xvfb.log`, `reports/EVIDENCE/20260215_ui_files.log`.
- Impacto: regressões visuais/a11y podem passar sem detecção automática.
- Causa provável: ausência de backend gráfico virtual no ambiente.
- Recomendação (o que e onde): adicionar etapa de screenshot headless na infraestrutura de QA visual.
- Validação PASS/FAIL: NÃO VERIFICADO.
- Risco + mitigação: risco médio; mitigar com checklist manual até automação.
- Rollback: remover etapa visual de CI (`git revert <commit-da-implementacao>`).
- Priorização: Severidade 3; Impacto 4; Probabilidade 4; Esforço 3; Risco de regressão 1; Urgência 3; Score 5.3.

## BKL-006
- Categoria: Migração GTK4
- Descrição objetiva: codebase ainda fortemente acoplada a APIs GTK3 legadas (`gtk_container_add`, `gtk_box_pack_start`, `gtk_dialog_run`, `gtk_bin_get_child`, `GtkTreeView`).
- Evidência: `reports/EVIDENCE/20260215_gtk3_api.log`, `meson.build:73`.
- Impacto: migração GTK4 exige projeto dedicado com alto risco de regressão.
- Causa provável: histórico arquitetural e escopo amplo da aplicação desktop.
- Recomendação (máx. 2 opções; recomendada 1):
  Opção 1 (Recomendada): GTK4 puro incremental por subsistema e feature flags.
  Opção 2: GTK4 + libadwaita (GNOME-first), com validação explícita em ambientes não-GNOME.
- Validação PASS/FAIL: FAIL para prontidão imediata GTK4.
- Risco + mitigação: risco alto; mitigar com branch dedicada, suite visual e gates por onda.
- Rollback: manter branch GTK3 como fallback até paridade validada.
- Priorização: Severidade 4; Impacto 5; Probabilidade 5; Esforço 5; Risco de regressão 5; Urgência 3; Score 5.0.
