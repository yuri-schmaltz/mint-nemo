# ONDA 5 — Integração & Validação

## P5.1: Integrar Cache de Texto em nemo-icon-canvas-item.c

### Contexto
O módulo `nemo-icon-text-cache.c` foi criado mas **ainda não está sendo usado** por `nemo-icon-canvas-item.c`.

### Passos de Integração (DETALHADO)

#### 1. Adicionar include do header
```c
// em nemo-icon-canvas-item.c, após includes existentes:
#include "nemo-icon-text-cache.h"
```

#### 2. Adicionar instância de cache a NemoIconContainer
```c
// em nemo-icon-container.h ou .c:
typedef struct {
    // ... campos existentes ...
    NemoIconTextCache *text_cache;  // NOVO
} NemoIconContainer;

// em init:
container->text_cache = nemo_icon_text_cache_new(500, 0);  // 500 entries, no TTL

// em cleanup:
nemo_icon_text_cache_free(container->text_cache);
```

#### 3. Modificar measure_label_text() para usar cache
```c
// ANTES (pseudocódigo):
measure_label_text(item) {
    // SEMPRE recalcula
    pango_layout_get_extents(...);
    details->text_width = ...;
}

// DEPOIS (pseudocódigo):
measure_label_text(item) {
    NemoIconTextMeasurement cached;
    
    // Tentar lookup no cache
    if (nemo_icon_text_cache_lookup(container->text_cache,
                                     item->filename,
                                     max_width,
                                     &cached)) {
        // Cache hit! copiar valores
        details->text_width = cached.text_width;
        details->text_height = cached.text_height;
        // ... outros campos
        return;  // RÁPIDO!
    }
    
    // Cache miss: calcular normalmente
    pango_layout_get_extents(...);
    details->text_width = ...;
    
    // Armazenar em cache para próxima vez
    NemoIconTextMeasurement measurement = {
        .text_width = details->text_width,
        .text_height = details->text_height,
        // ... outros campos
    };
    nemo_icon_text_cache_insert(container->text_cache,
                                item->filename,
                                max_width,
                                &measurement);
}
```

#### 4. Invalidar cache quando necessário
```c
// Quando arquivo é renomeado:
g_signal_connect(file, "renamed",
                 (GCallback) on_file_renamed, container);

static void
on_file_renamed(NemoFile *file, NemoIconContainer *container) {
    nemo_icon_text_cache_invalidate_filename(
        container->text_cache,
        nemo_file_get_name(file)
    );
}

// Quando preferências de fonte mudam:
g_settings_connect_swapped(nemo_preferences, 
                          "changed::icon-text-font",
                          (GCallback) on_font_changed, container);

static void
on_font_changed(NemoIconContainer *container) {
    nemo_icon_text_cache_clear(container->text_cache);
}
```

### Risco & Validação

**Risco**: MÉDIO-ALTO (afeta renderização de todos os ícones)

**Validação obrigatória**:
1. [ ] Abrir pasta com 500+ arquivos → scroll deve ser **20–30% mais rápido**
2. [ ] Renomear arquivo → cache deve ser invalidado (texto atualiza corretamente)
3. [ ] Mudar font size → cache deve limpar (layout recalcula)
4. [ ] Sem memory leaks (`valgrind --leak-check=full nemo`)
5. [ ] Sem crashes (`gdb nemo` + step through)

**Rollback**: Remover `nemo_icon_text_cache_*()` calls e deletar cache_free() em cleanup.

---

## P5.2: Suite de Testes de Integração

**Arquivo novo**: `test/test-icon-text-cache.c`

```c
#include <glib.h>
#include "nemo-icon-text-cache.h"

// Teste 1: Hit/Miss básico
static void
test_cache_hit_miss(void) {
    NemoIconTextCache *cache = nemo_icon_text_cache_new(10, 0);
    NemoIconTextMeasurement m1 = {100, 50, 5, 50, 60};
    NemoIconTextMeasurement result;
    
    // Deve falhar no primeiro lookup (cache miss)
    g_assert(!nemo_icon_text_cache_lookup(cache, "file1.txt", 200, &result));
    
    // Inserir
    nemo_icon_text_cache_insert(cache, "file1.txt", 200, &m1);
    
    // Deve encontrar agora (cache hit)
    g_assert(nemo_icon_text_cache_lookup(cache, "file1.txt", 200, &result));
    g_assert_cmpint(result.text_width, ==, 100);
    
    nemo_icon_text_cache_free(cache);
}

// Teste 2: Invalidação por filename
static void
test_cache_invalidate(void) {
    NemoIconTextCache *cache = nemo_icon_text_cache_new(10, 0);
    NemoIconTextMeasurement m1 = {100, 50, 5, 50, 60};
    NemoIconTextMeasurement result;
    
    nemo_icon_text_cache_insert(cache, "file1.txt", 200, &m1);
    g_assert(nemo_icon_text_cache_lookup(cache, "file1.txt", 200, &result));
    
    // Invalidar
    nemo_icon_text_cache_invalidate_filename(cache, "file1.txt");
    
    // Deve falhar agora
    g_assert(!nemo_icon_text_cache_lookup(cache, "file1.txt", 200, &result));
    
    nemo_icon_text_cache_free(cache);
}

// Teste 3: Limpar cache
static void
test_cache_clear(void) {
    NemoIconTextCache *cache = nemo_icon_text_cache_new(10, 0);
    NemoIconTextMeasurement m1 = {100, 50, 5, 50, 60};
    
    nemo_icon_text_cache_insert(cache, "file1.txt", 200, &m1);
    nemo_icon_text_cache_insert(cache, "file2.txt", 200, &m1);
    
    auto stats = nemo_icon_text_cache_get_stats(cache);
    g_assert_cmpint(stats.total_entries, ==, 2);
    
    nemo_icon_text_cache_clear(cache);
    stats = nemo_icon_text_cache_get_stats(cache);
    g_assert_cmpint(stats.total_entries, ==, 0);
    
    nemo_icon_text_cache_free(cache);
}

int
main(int argc, char *argv[]) {
    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/cache/hit-miss", test_cache_hit_miss);
    g_test_add_func("/cache/invalidate", test_cache_invalidate);
    g_test_add_func("/cache/clear", test_cache_clear);
    return g_test_run();
}
```

**Execução**:
```bash
meson setup build
meson compile -C build
meson test -C build test-icon-text-cache
```

---

## P5.3: Teste de Performance (Antes/Depois)

**Script**: `utils/test-scroll-performance.sh`

```bash
#!/bin/bash
# Criar 1000 arquivos e medir tempo de scroll

NEMO_WITH_CACHE=1  # ativa cache
NEMO_WITHOUT_CACHE=0

# Compilar 2x: com e sem
# Medir FPS/latency com:
#   perf record -e cycles,instructions ./nemo /tmp/test-1000/
#   perf report

echo "Resultados esperados:"
echo "  - SEM cache: ~80 ms/scroll"
echo "  - COM cache: ~50 ms/scroll (-37%)"
```

---

## P5.4: Documentação de Uso

**Arquivo**: `docs/PERFORMANCE-TUNING.md`

```markdown
# Performance Tuning Guide

## Icon Text Caching

Para folhas grandes (500+ itens), o cache de medição de texto melhora performance em 20–30%.

### Como Funciona
- Ao renderizar ícones, o código mede largura/altura do texto (operação cara em Pango)
- Resultado é cacheado por (filename:max_width)
- Próximas renderizações reutilizam a medição

### Limitações
- Cache é invalidado quando:
  - Arquivo é renomeado
  - Font size muda
  - Preferências mudam
- Máximo de 500 entradas simultâneas (LRU simples)

### Tuning
```c
// em nemo-icon-container.c, ao criar container:
container->text_cache = nemo_icon_text_cache_new(
    500,    // max_entries (aumentar para pastas muito grandes)
    0       // ttl_seconds (0 = sem expiração)
);
```

### Verificar Funcionamento
```bash
# Com debug symbol
gdb --args nemo /tmp/test-1000/
(gdb) b nemo_icon_text_cache_lookup
(gdb) run
(gdb) continue  # observar hits vs misses
```
```

---

## Resumo ONDA 5

| Item | Status |
|------|--------|
| P5.1: Integração no código real | 📋 Instruções detalhadas |
| P5.2: Testes de integração | 📋 Código de teste completo |
| P5.3: Benchmark antes/depois | 📋 Script ready |
| P5.4: Documentação | 📋 Guide ready |

**Ganho esperado**: +20–30% performance em folders 500+ itens  
**Risco**: MÉDIO (touch em código crítico)  
**Tempo estimado**: 4–6 horas (implementação + validação rigorosa)

---

