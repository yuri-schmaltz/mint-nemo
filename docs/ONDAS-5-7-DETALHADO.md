# ONDAS 5-7: Integração, Profiling & Otimizações Avançadas

**Data**: Q2–Q4 2026  
**Duração**: 12–16 semanas (3 sprints)  
**Time**: 2–3 eng + 1 QA

---

## 🟡 ONDA 5: Integração & Validação (4–6 semanas)

Integrar os frameworks criados (cache, lazy-loader) ao código real.

### P5.1: Integração do Text Cache

**Arquivo alvo**: `libnemo-private/nemo-icon-canvas-item.c`  
**Função**: `measure_label_text()` (linha ~2100)

**Antes**:
```c
// Linha ~2100 em nemo-icon-canvas-item.c
static void
measure_label_text (NemoIconCanvasItem *item)
{
    PangoLayout *layout;
    int width, height;

    layout = pango_layout_new (item->details->font_context);
    pango_layout_set_text (layout, item->details->text, -1);
    pango_layout_get_extents (layout, NULL, &logical_rect);  // ← HOTSPOT
    
    item->details->text_width = PANGO_PIXELS (logical_rect.width);
    item->details->text_height = PANGO_PIXELS (logical_rect.height);
    
    g_object_unref (layout);
}
```

**Depois**:
```c
#include "nemo-icon-text-cache.h"

static void
measure_label_text (NemoIconCanvasItem *item)
{
    NemoIconTextMeasurement measurement;
    int max_width = item->details->max_line_width;  // A partir do zoom

    // Tentar cache primeiro
    if (nemo_icon_text_cache_lookup (item->details->cache, 
                                      item->details->text, 
                                      max_width, 
                                      &measurement))
    {
        // HIT: Usar valores em cache
        item->details->text_width = measurement.text_width;
        item->details->text_height = measurement.text_height;
        return;  // 15–20x mais rápido
    }

    // MISS: Calcular (caminho antigo)
    PangoLayout *layout = pango_layout_new (item->details->font_context);
    pango_layout_set_text (layout, item->details->text, -1);
    pango_layout_get_extents (layout, NULL, &logical_rect);
    
    measurement.text_width = PANGO_PIXELS (logical_rect.width);
    measurement.text_height = PANGO_PIXELS (logical_rect.height);
    measurement.timestamp = g_get_monotonic_time ();
    
    // Guardar em cache
    nemo_icon_text_cache_insert (item->details->cache, 
                                  item->details->text, 
                                  max_width, 
                                  &measurement);
    
    item->details->text_width = measurement.text_width;
    item->details->text_height = measurement.text_height;
    
    g_object_unref (layout);
}
```

**Passos**:
1. Adicionar `#include "nemo-icon-text-cache.h"` no topo
2. Em `nemo_icon_canvas_item_init()`, criar cache:
   ```c
   item->details->cache = nemo_icon_text_cache_new ();
   ```
3. Substituir body de `measure_label_text()` (copiar pseudocode acima)
4. Em `nemo_icon_canvas_item_dispose()`, cleanup:
   ```c
   nemo_icon_text_cache_unref (item->details->cache);
   ```
5. Adicionar invalidação em callbacks de rename/font change:
   ```c
   // Em nemo_icon_canvas_item_set_text():
   nemo_icon_text_cache_invalidate_filename (cache, old_filename);
   ```

**Validação**:
- Compilar: `meson compile -C build`
- Run unit tests: `g-test test-icon-text-cache --verbose`
- Scroll 500 items: deve ser 15–20x mais rápido no cache
- Check stats: `nemo_icon_text_cache_get_stats()` → hit_rate >90% esperado

---

### P5.2: Integração do Lazy-Loader

**Arquivo alvo**: `libnemo-private/nemo-view.c`  
**Função**: `on_viewport_scroll()` (linha ~3100)

**Pseudocode**:
```c
#include "nemo-icon-lazy-loader.h"

static void
on_viewport_scroll (GtkAdjustment *adjustment, NemoView *view)
{
    NemoIconLazyLoaderViewport vp = {
        .y_offset = gtk_adjustment_get_value (adjustment),
        .height = gtk_widget_get_allocated_height (view->viewport),
        .item_height = 80  // aproximado
    };
    
    // Atualizar viewport no lazy-loader
    nemo_icon_lazy_loader_update_viewport (view->lazy_loader, &vp);
}

// Em nemo_view_init():
view->lazy_loader = nemo_icon_lazy_loader_new ();
// Iniciar thread pool (4 threads)
nemo_icon_lazy_loader_start (view->lazy_loader, 4);
```

**Validação**:
- Scroll 1000 items: deve ter <100ms time-to-interact
- Thread pool: verify 4 threads spawned (ps aux | grep nemo)
- Memory: check não ultrapassa +50MB vs baseline

---

### P5.3: Integration Tests

**Arquivo**: `test/test-icon-integration.c`

```c
#include <glib.h>
#include "nemo-icon-canvas-item.h"
#include "nemo-icon-text-cache.h"
#include "nemo-icon-lazy-loader.h"

// Test 1: Cache hit/miss ratio
static void
test_cache_integration (void)
{
    NemoIconCanvasItem *item = create_test_item ();
    NemoIconTextCache *cache = nemo_icon_text_cache_new ();
    
    // Simular scroll 500 items
    for (int i = 0; i < 500; i++) {
        nemo_icon_canvas_item_measure_text (item, "file_%d.txt", i);
    }
    
    NemoIconTextCacheStats stats = nemo_icon_text_cache_get_stats (cache);
    g_assert_cmpint (stats.hit_count, >, 400);  // Expect >80% hit rate
    
    g_object_unref (item);
    nemo_icon_text_cache_unref (cache);
}

// Test 2: No memory leaks during scroll
static void
test_scroll_no_leak (void)
{
    // Valgrind will check
    run_scroll_benchmark (500);
    
    // Should show:
    // ==12345== HEAP SUMMARY
    // ==12345==     in use at exit: 0 bytes
}

// Test 3: Lazy-loader prioritizes visible items
static void
test_lazy_loader_priority (void)
{
    NemoIconLazyLoader *loader = nemo_icon_lazy_loader_new ();
    NemoIconLazyLoaderViewport vp = {.y_offset = 0, .height = 600};
    
    nemo_icon_lazy_loader_update_viewport (loader, &vp);
    nemo_icon_lazy_loader_queue_batch (loader, 1000);  // 1000 items
    
    // First batch processed should contain visible items (0–7)
    // Not random items (999, 500, etc)
}
```

**Run**:
```bash
cd /home/yurix/Documentos/mint-nemo
meson test -C build test-icon-integration
```

---

### P5.4: Performance Benchmarks (Antes/Depois)

**Script**: `utils/benchmark-integration.sh`

```bash
#!/bin/bash

NEMO_DIR="/home/yurix/Documentos/mint-nemo"
BUILD_DIR="$NEMO_DIR/build"

# Compilar versão SEM cache (baseline)
echo "[*] Building BASELINE (without cache)..."
git checkout libnemo-private/nemo-icon-canvas-item.c  # versão original
meson setup build-baseline --buildtype=release -Dprefix=/tmp/nemo-baseline
meson compile -C build-baseline
meson install -C build-baseline --prefix=/tmp/nemo-baseline

# Compilar versão COM cache (PR)
echo "[*] Building PR (with cache)..."
git checkout -  # restaurar versão com cache
meson setup build-pr --buildtype=release -Dprefix=/tmp/nemo-pr
meson compile -C build-pr
meson install -C build-pr --prefix=/tmp/nemo-pr

# Testar ambas
echo "[*] Benchmarking..."
/tmp/nemo-baseline/bin/nemo-benchmark-scroll 500 > baseline.json
/tmp/nemo-pr/bin/nemo-benchmark-scroll 500 > pr.json

# Comparar
python3 utils/compare-benchmarks.py baseline.json pr.json
# Esperado: Baseline = 250ms, PR = 15ms (16.6x faster)
```

**Output esperado**:
```
Baseline (no cache):  250ms per scroll
PR (with cache):      15ms per scroll
Improvement:          94.0% ✅
Hit ratio:            92.5%
Memory delta:         +8MB (acceptable)
```

---

## 🟠 ONDA 6: Profiling Detalhado (2–3 semanas)

Usar ferramentas reais de profiling para identificar hotspots residuais.

### P6.1: CPU Profiling com `perf`

```bash
# 1. Build com símbolos de debug
meson setup build-debug --buildtype=debug
meson compile -C build-debug

# 2. Criar pasta com 1000 arquivos
mkdir -p /tmp/nemo-test-1000
touch /tmp/nemo-test-1000/file_{1..1000}.txt

# 3. Perfazer recording
perf record -F 99 -g /tmp/nemo-debug/bin/nemo /tmp/nemo-test-1000

# Dentro da GUI:
# - Scroll para cima e para baixo (30 segundos)
# - Feche nemo
# Ctrl+C para parar perf

# 4. Análise
perf report
# Esperado: No topo do hotspot deve estar em Pango (text measurement)
# Com cache: Hotspot muda para compositor/drawing (normal)

# 5. Flamegraph
perf script > out.perf
# ...converte para flamegraph (script separado)
```

**Resultado esperado**:
```
 15.3%  nemo  [.] pango_layout_get_extents  ← COM CACHE: <2%
  8.2%  nemo  [.] gdk_window_move_resize
  7.1%  nemo  [.] gtk_widget_draw
  5.8%  nemo  [.] g_hash_table_lookup     ← NOVO: cache lookups
  ...
```

---

### P6.2: Memory Profiling com `valgrind`

```bash
# Checar leaks
valgrind --leak-check=full --show-leak-kinds=all \
  /tmp/nemo-pr/bin/nemo /tmp/nemo-test-1000 2>&1 | tee valgrind.log

# Feche nemo após 30s de uso
# Esperado: "definitely lost: 0 bytes"

# Se houver leak:
grep "LEAK SUMMARY" valgrind.log
grep "at 0x" valgrind.log | head -5  # top 5 leak sites
```

---

### P6.3: Heap Profiling com `heaptrack`

```bash
# 1. Instalar
sudo apt install heaptrack heaptrack-gui

# 2. Profile
heaptrack /tmp/nemo-pr/bin/nemo /tmp/nemo-test-1000
# GUI se abrirá automaticamente

# Esperado:
# - Peak memory: <300MB (large folder)
# - Fragmentation: <5%
# - Top allocators: pango, gtk, glib (normal)
```

---

### P6.4: Flame Graph

```bash
# Gerar flamegraph de todo scrolling
perf record -F 99 -g --call-graph=dwarf ./nemo /tmp/test-1000
perf script | stackcollapse-perf.pl | flamegraph.pl > out.svg
# Abrir em navegador: firefox out.svg
```

---

## 🟢 ONDA 7: Otimizações Secundárias (4–8 semanas)

Baseado em dados de ONDA 6, implementar 3 otimizações avançadas.

### P7.1: Lazy-Load de Thumbnails

**Problema**: Gerar thumbnails para 1000+ imagens gasta 500MB+ RAM.

**Solução**: Gerar thumbnails apenas para items visíveis + off-screen buffer.

```c
// nemo-icon-lazy-loader.c: P7.1

static void
load_thumbnail_for_item (NemoIconCanvasItem *item, void *user_data)
{
    // Verificar se visível
    if (!is_visible_in_viewport (item)) {
        return;  // Skip
    }
    
    // Gerar thumbnail assincronamente
    g_thread_pool_push (thumbnail_pool, item, NULL);
}

// Pseudocode: thumbnail_worker
static void
thumbnail_worker (gpointer item_p, gpointer user_data)
{
    NemoIconCanvasItem *item = item_p;
    GdkPixbuf *thumbnail;
    
    // Gerar thumbnail (pode levar 50–200ms)
    thumbnail = generate_thumbnail (item->filename, THUMB_SIZE_96);
    
    // Callback (thread-safe)
    g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
                    apply_thumbnail_callback,
                    thumbnail,
                    g_object_unref);  // cleanup
}
```

**Ganho esperado**: -50% pico de memória para 1000+ imagens.

---

### P7.2: Batching de I/O

**Problema**: Cada icon call faz `stat()` separado (context switch).

**Solução**: Batch `stat()` calls com `nftw()`.

```c
// nemo-icon-canvas.c: P7.2

// Antes (N chamadas stat):
for (item in items) {
    file_size = stat (item->path).st_size;  // 1000x stat() syscalls
}

// Depois (1 walk):
void batch_stat_callback (const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    // Store size in cache
    cache_file_stat (fpath, sb->st_size);
}

nftw (root_dir, batch_stat_callback, 16, FTW_ACTIONRETVAL);
// 1000 items → 1 syscall batch (50–70x faster)
```

**Ganho esperado**: +30–40% throughput em listagem inicial.

---

### P7.3: Memory Pooling

**Problema**: GTK allocation/deallocation cria fragmentation (10–15%).

**Solução**: Object pool para estruturas frequentes.

```c
// nemo-icon-pool.h: P7.3

typedef struct {
    GPtrArray *items;      // pool de NemoIconCanvasItem
    GMutex lock;
    guint free_count;
} NemoIconPool;

NemoIconCanvasItem *
nemo_icon_pool_get (NemoIconPool *pool)
{
    g_mutex_lock (&pool->lock);
    NemoIconCanvasItem *item = g_ptr_array_pop (pool->items);
    g_mutex_unlock (&pool->lock);
    
    if (item) {
        return item;  // reuse
    }
    
    return g_new0 (NemoIconCanvasItem, 1);  // allocate
}

void
nemo_icon_pool_return (NemoIconPool *pool, NemoIconCanvasItem *item)
{
    // Limpar e retornar ao pool
    memset (item, 0, sizeof (*item));
    g_ptr_array_push (pool->items, item);
}
```

**Ganho esperado**: -10% fragmentação, +5–8% cache locality.

---

## Timeline Recomendado

| Onda | Período | Duração | Risk | Ganho |
|------|---------|---------|------|-------|
| 5 | Semanas 1–6 | 4–6w | 🟠 MÉDIO | +20–30% scroll |
| 6 | Semanas 3–5 | 2–3w | 🟢 BAIXO | Data-driven |
| 7 | Semanas 7–14 | 4–8w | 🟠 MÉDIO | +10–50% várias |

---

## Sign-Off

- [ ] ONDA 5: Code review + tests passing
- [ ] ONDA 6: Profiling data collected + hotspots identified
- [ ] ONDA 7: 3 patches implemented + validated

**Entrega**: v6.8 (Q2 2026) + v6.9 (Q3 2026) + v7.0 (Q4 2026)

