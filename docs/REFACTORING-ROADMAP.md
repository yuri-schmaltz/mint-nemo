# REFACTORING ROADMAP: nemo-view.c Modularization

**Arquivo**: `src/nemo-view.c` (11,263 linhas)  
**Status**: Planejamento para v6.8+  
**Prioridade**: Médio-Alto (refactor estrutural, pode aguardar próxima release)

## Motivação

`nemo-view.c` é o maior arquivo do projeto (11.2k linhas) e agrega múltiplas responsabilidades:
- Gerenciamento de seleção
- Renderização e layout de visualização
- Navegação por teclado/mouse
- Operações de drag & drop
- Manipulação de ações/menu contexto
- Gerenciamento de thumbnails

**Problemas atuais**:
1. Difícil de testar isoladamente
2. Alta complexidade cognitiva
3. Risco elevado em mudanças (afeta todas as views)
4. Difícil localizar bugs em view-specific

## Plano de Modularização

### Fase 1: Separar Gerenciamento de Seleção
**Arquivo novo**: `nemo-view-selection.c` / `nemo-view-selection.h`

**Funções a mover**:
- `nemo_view_select_file()`
- `nemo_view_selection_changed()`
- `nemo_view_select_all()`
- `nemo_view_invert_selection()`
- `nemo_view_*_selection()` — toda família

**Callbacks a mover**:
- `on_selection_changed_* ()`
- `handle_selection_* ()`

**Ganhos**:
- Reduz nemo-view.c em ~800 linhas
- Testável isoladamente
- Reutilizável em novos tipos de view

---

### Fase 2: Separar Navegação por Teclado
**Arquivo novo**: `nemo-view-navigation.c` / `nemo-view-navigation.h`

**Funções a mover**:
- `nemo_view_handle_* key*_* ()`
- `nemo_view_move_* ()`
- Trata Arrow keys, Tab, Return, Escape

**Ganhos**:
- Reduz complexidade de event handling
- Facilita testes de navegação

---

### Fase 3: Separar Drag & Drop
**Arquivo novo**: `nemo-view-dnd.c` (novo, reutiliza `nemo-icon-dnd.c`)

**Integração com existente**:
- `nemo-icon-dnd.c` já existe mas está acoplado
- Criar wrapper em nemo-view-dnd.c que orquestra

---

### Fase 4: Lazy-Load de Ícones (Performance)
**Arquivo**: `nemo-icon-container.c`

**Implementação**:
1. Renderizar apenas ícones visíveis no viewport
2. Enfileirar ícones off-screen
3. Usar thread pool para thumbnail loading assíncrono
4. Priorizar ícones próximos ao viewport durante scroll

**Ganho esperado**: 50–70% redução em "time to interact" para 1000+ arquivos

---

## Roadmap por Release

| Release | Fase | ETA | Risco |
|---------|------|-----|-------|
| 6.7 | Infrastructure (cache texto, git gc, audit) | ✅ DONE | Baixo |
| 6.8 | Phase 1 (Seleção) | Q2 2026 | Médio |
| 6.9 | Phase 2 (Navegação) | Q3 2026 | Médio |
| 7.0 | Phase 3 (DND) + Phase 4 (Lazy-load) | Q4 2026 | Alto |

---

## Testing Strategy

1. **Unit tests**: Funções isoladas (seleção, navegação)
2. **Integration tests**: Multi-view scenarios
3. **Regression tests**: Keyboard nav, DND, menu contexto
4. **Performance tests**: Scroll em 500, 1000, 5000 items
5. **UI tests**: Manual verificação após cada fase

---

## Nota de Implementação

Refactorings estruturais devem ser feitos com grande cuidado:
- Criar branch feature: `feature/nemo-view-refactoring`
- Cada fase em commit separado (facilita revert)
- Bisect-friendly (um feature por commit)
- Extensa suite de testes de regressão

---

FIM — Próximas ondas: Implementação incremental (Phase 1 em v6.8)
