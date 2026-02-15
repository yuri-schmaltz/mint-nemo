# BACKLOG

Critério de score: `(Impacto x Probabilidade) / Esforço`.
Escala: 1 (baixo) a 5 (alto).

## Achados

### BLD-001
- Categoria: Build/Release
- Descrição: `meson setup` falha por opção inexistente `deprecated_warnings`.
- Evidência: `meson.build:46`; `reports/EVIDENCE/14_meson_setup_baseline.log`.
- Impacto: bloqueia build/test local e pipeline derivado.
- Causa provável: ausência de `meson_options.txt` apesar de múltiplos `get_option(...)`.
- Recomendação: criar `meson_options.txt` com opções usadas em `meson.build` e defaults conservadores.
- Validação atual: FAIL.
- Risco: baixo (arquivo de configuração).
- Mitigação: manter defaults alinhados ao empacotamento (`debian/rules:9` a `debian/rules:11`).
- Rollback: remover `meson_options.txt`.
- Priorização: Severidade 5; Impacto 5; Probabilidade 5; Esforço 1; Risco regressão 1; Urgência 5; Score 25.0.

### BLD-002
- Categoria: Build/Portabilidade
- Descrição: referências a `subdir('search-helpers')` e `subdir('test')` sem diretórios no checkout atual.
- Evidência: `meson.build:189`, `meson.build:190`; `reports/EVIDENCE/04_missing_dirs_check.log`.
- Impacto: alto risco de falha de configure após desbloquear opções.
- Causa provável: checkout parcial/variante de empacotamento.
- Recomendação: condicionar inclusão dos subdirs quando existirem.
- Validação atual: FAIL potencial (a confirmar após BLD-001).
- Risco: baixo-médio (pode mascarar ausência de conteúdo esperado).
- Mitigação: emitir `message()` explícita quando pular subdir.
- Rollback: reverter condição e restaurar `subdir()` direto.
- Priorização: Severidade 4; Impacto 4; Probabilidade 4; Esforço 2; Risco regressão 2; Urgência 4; Score 8.0.

### QA-001
- Categoria: QA/Release
- Descrição: cobertura de testes automatizados efetivos muito baixa no estado atual de checkout.
- Evidência: `eel/meson.build:40`; `debian/rules:23` a `debian/rules:25`; `reports/EVIDENCE/02_tests_ci_scan.log`.
- Impacto: regressões podem passar sem detecção.
- Causa provável: foco em build de distribuição e testes desativados no empacotamento Debian.
- Recomendação: habilitar smoke de configuração/build em CI local e registrar baseline mínimo.
- Validação atual: FAIL parcial.
- Risco: baixo (documentação/guardrail).
- Mitigação: começar por smoke não-interativo.
- Rollback: remover smoke local.
- Priorização: Severidade 4; Impacto 4; Probabilidade 4; Esforço 2; Risco regressão 1; Urgência 3; Score 8.0.

### UI-001
- Categoria: UI/AEGIS
- Descrição: validação visual headless bloqueada por ausência de Xvfb/display funcional.
- Evidência: `reports/EVIDENCE/13_ui_validate_baseline.log`, `reports/EVIDENCE/20_xvfb_tools.log`, `reports/EVIDENCE/23_xvfb_binary_probe.log`.
- Impacto: não há prova automática de pixel-perfect/reflow/a11y por breakpoint neste ambiente.
- Causa provável: ambiente de execução sem X server virtual instalado.
- Recomendação: adicionar etapa de CI com Xvfb para screenshots/validação UI.
- Validação atual: NÃO VERIFICADO.
- Risco: médio (defeitos visuais podem escapar).
- Mitigação: manter validação textual de UI e ampliar checks headless quando possível.
- Rollback: remover job de screenshot.
- Priorização: Severidade 3; Impacto 3; Probabilidade 4; Esforço 3; Risco regressão 1; Urgência 3; Score 4.0.

### GTK4-001
- Categoria: Migração GTK4
- Descrição: forte acoplamento a APIs GTK3 legadas (`GtkAction`, `GtkUIManager`, `GtkAlignment`, `GtkMisc`, `gtk_dialog_run`, `gtk_container_add`).
- Evidência: `reports/EVIDENCE/30_gtk3_api_scan.log`, `reports/EVIDENCE/31_glade_legacy_widgets.log`.
- Impacto: migração GTK4 exige onda dedicada e potencialmente extensa.
- Causa provável: arquitetura histórica GTK3 da base do Nemo.
- Recomendação: abordagem recomendada `GTK4 puro` em fases, com camada de compatibilidade por módulos.
- Validação atual: FAIL para prontidão GTK4.
- Risco: alto (quebra visual/comportamental).
- Mitigação: migração incremental por subsistemas + regressão visual e E2E por etapa.
- Rollback: manter branch de compatibilidade GTK3 até paridade validada.
- Priorização: Severidade 4; Impacto 5; Probabilidade 5; Esforço 5; Risco regressão 5; Urgência 3; Score 5.0.

### SEC-001
- Categoria: Segurança/Hardening
- Descrição: múltiplos pontos de execução de comandos externos (`g_spawn_*`, `g_shell_parse_argv`, `g_subprocess`).
- Evidência: `reports/EVIDENCE/32_security_calls_scan.log`.
- Impacto: risco depende da origem dos argumentos/comandos; superfície sensível existe.
- Causa provável: integração com ações customizadas, helpers e utilitários externos.
- Recomendação: revisar sanitização e trust boundary por ponto crítico.
- Validação atual: NÃO VERIFICADO (auditoria profunda não executada).
- Risco: médio.
- Mitigação: checklist de validação de entrada + testes de abuso em comandos.
- Rollback: reverter hardening pontual se regressão funcional ocorrer.
- Priorização: Severidade 4; Impacto 4; Probabilidade 3; Esforço 4; Risco regressão 3; Urgência 4; Score 3.0.
