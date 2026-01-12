# 💡 EXEMPLO PRÁTICO: Integrando o Text Cache em Nemo

**Objetivo**: Demonstrar passo-a-passo como integrar `nemo-icon-text-cache.h/c` no código real.

---

## 📋 Passo 1: Identificar o Target

### Arquivo: `libnemo-private/nemo-icon-canvas-item.c`

**Localizar a função**:
```bash
grep -n "measure_label_text" libnemo-private/nemo-icon-canvas-item.c
# Output: ~2100:static void measure_label_text (NemoIconCanvasItem *item)
```

**Visualizar o código atual**:
```c
// Linhas ~2100-2120
static void
measure_label_text (NemoIconCanvasItem *item)
{
    PangoLayout *layout;
    PangoRectangle logical_rect;
    int width, height;

    g_return_if_fail (NEMO_IS_ICON_CANVAS_ITEM (item));

    if (item->details->text == NULL) {
        item->details->text_width = 0;
        item->details->text_height = 0;
        return;
    }

    width = item->details->max_text_width;

    layout = pango_layout_new (item->details->font_context);
    pango_layout_set_width (layout, width * PANGO_SCALE);
    pango_layout_set_text (layout, item->details->text, -1);
    pango_layout_get_extents (layout, NULL, &logical_rect);  // ← HOTSPOT

    item->details->text_width = PANGO_PIXELS (logical_rect.width);
    item->details->text_height = PANGO_PIXELS (logical_rect.height);

    g_object_unref (layout);
}
```

---

## 🔧 Passo 2: Modificar nemo-icon-canvas-item.h

Adicione um campo para a cache na struct `NemoIconCanvasItemDetails`:

```c
// File: libnemo-private/nemo-icon-canvas-item.h
// Procure: typedef struct NemoIconCanvasItemDetails { ... }

// Adicione ao final da struct, ANTES do fechamento '}':
struct {
    NemoIconTextCache *text_cache;  // Nova linha
} cache;
```

---

## 🔧 Passo 3: Incluir o Header

**No topo do arquivo**:
```c
// File: libnemo-private/nemo-icon-canvas-item.c

#include "nemo-icon-text-cache.h"  // ← Adicione esta linha
#include "nemo-icon-canvas-item.h"
// ... resto dos includes ...
```

---

## 🔧 Passo 4: Inicializar a Cache

**Localize a função**: `nemo_icon_canvas_item_init()` ou `nemo_icon_canvas_item_new()`

```c
// Procure esta função (tipicamente por volta da linha 400-500)
static void
nemo_icon_canvas_item_init (NemoIconCanvasItem *item)
{
    item->details = g_new0 (NemoIconCanvasItemDetails, 1);
    
    // ... inicializar outros campos ...
    
    // ADICIONE:
    item->details->cache.text_cache = nemo_icon_text_cache_new ();
    
    // ... resto da inicialização ...
}
```

---

## 🔧 Passo 5: Substituir a Função measure_label_text()

**Copie este código** e substitua a função inteira:

```c
static void
measure_label_text (NemoIconCanvasItem *item)
{
    NemoIconTextMeasurement measurement;
    int max_width;
    PangoLayout *layout;
    PangoRectangle logical_rect;

    g_return_if_fail (NEMO_IS_ICON_CANVAS_ITEM (item));

    if (item->details->text == NULL) {
        item->details->text_width = 0;
        item->details->text_height = 0;
        return;
    }

    max_width = item->details->max_text_width;

    // ===== NOVO: TENTAR CACHE PRIMEIRO =====
    if (item->details->cache.text_cache != NULL &&
        nemo_icon_text_cache_lookup (item->details->cache.text_cache,
                                      item->details->text,
                                      max_width,
                                      &measurement))
    {
        // HIT: Usar valores em cache
        item->details->text_width = measurement.text_width;
        item->details->text_height = measurement.text_height;
        // Cache hit contabilizado automaticamente na cache
        return;
    }

    // MISS: Calcular via Pango (caminho original)
    layout = pango_layout_new (item->details->font_context);
    pango_layout_set_width (layout, max_width * PANGO_SCALE);
    pango_layout_set_text (layout, item->details->text, -1);
    pango_layout_get_extents (layout, NULL, &logical_rect);

    measurement.text_width = PANGO_PIXELS (logical_rect.width);
    measurement.text_height = PANGO_PIXELS (logical_rect.height);
    measurement.timestamp = g_get_monotonic_time ();

    // Guardar em cache
    if (item->details->cache.text_cache != NULL) {
        nemo_icon_text_cache_insert (item->details->cache.text_cache,
                                      item->details->text,
                                      max_width,
                                      &measurement);
    }

    item->details->text_width = measurement.text_width;
    item->details->text_height = measurement.text_height;

    g_object_unref (layout);
}
```

---

## 🔧 Passo 6: Invalidar Cache em Mudanças

**Quando o texto é renomeado**:
```c
// Procure: nemo_icon_canvas_item_set_text()
// Adicione antes de atualizar o texto:

if (item->details->cache.text_cache != NULL &&
    item->details->text != NULL) {
    // Invalidar entrada antiga se existir
    nemo_icon_text_cache_invalidate_filename (
        item->details->cache.text_cache,
        item->details->text);  // old text
}

// ... depois, atualizar item->details->text = g_strdup(new_text) ...
```

**Quando a fonte muda**:
```c
// Procure: signal handler para GTK_SETTINGS_FONT_NAME_CHANGED
// Ou onde a fonte é alterada

// Adicione:
if (item->details->cache.text_cache != NULL) {
    nemo_icon_text_cache_clear (item->details->cache.text_cache);
    // Força recalcular todas as medidas na próxima vez
}
```

---

## 🔧 Passo 7: Cleanup

**Na função dispose/finalize**:
```c
// Procure: nemo_icon_canvas_item_dispose() ou nemo_icon_canvas_item_finalize()

// Adicione:
if (item->details->cache.text_cache != NULL) {
    nemo_icon_text_cache_unref (item->details->cache.text_cache);
    item->details->cache.text_cache = NULL;
}

// ... resto do cleanup ...
```

---

## ✅ Passo 8: Verificar a Compilação

```bash
cd /home/yurix/Documentos/mint-nemo

# Limpar build anterior
rm -rf build

# Recompilar
meson setup build
meson compile -C build

# Esperado: ✅ Sem erros
# Se houver erro, verificar:
# 1. Syntax errors (parênteses, semicolons)
# 2. Tipos incompatíveis (int vs gint)
# 3. Headers não inclusos
```

---

## 🧪 Passo 9: Testar a Integração

### Teste 1: Cache Hits

```c
// Arquivo: test/test-cache-integration.c

#include <glib.h>
#include "nemo-icon-canvas-item.h"

static void
test_cache_hits (void)
{
    NemoIconCanvasItem *item = g_object_new (NEMO_TYPE_ICON_CANVAS_ITEM, NULL);
    item->details->text = g_strdup ("test_file.txt");
    item->details->max_text_width = 100;

    // Primeira medição (MISS)
    measure_label_text (item);
    NemoIconTextCacheStats stats1 = nemo_icon_text_cache_get_stats (item->details->cache.text_cache);
    g_assert_cmpint (stats1.miss_count, ==, 1);

    // Segunda medição (HIT)
    measure_label_text (item);
    NemoIconTextCacheStats stats2 = nemo_icon_text_cache_get_stats (item->details->cache.text_cache);
    g_assert_cmpint (stats2.hit_count, ==, 1);
    g_assert_cmpint (stats2.hit_rate, >, 50);  // >50% hit rate

    g_object_unref (item);
}

int main (int argc, char *argv[])
{
    g_test_init (&argc, &argv, NULL);
    g_test_add_func ("/nemo-icon/cache-integration", test_cache_hits);
    return g_test_run ();
}
```

**Rodar**:
```bash
gcc -o test-cache-integration test/test-cache-integration.c \
    libnemo-private/nemo-icon-text-cache.c \
    libnemo-private/nemo-icon-canvas-item.c \
    `pkg-config --cflags --libs gtk+-3.0 glib-2.0`

./test-cache-integration
# Esperado: ✅ Todos testes passando
```

### Teste 2: Performance

```bash
# Criar pasta teste
mkdir -p /tmp/nemo-test-500
touch /tmp/nemo-test-500/file_{1..500}.txt

# Medir scroll
time nemo /tmp/nemo-test-500

# Scroll manualmente 3-4x
# Esperado: Scroll fluido, sem travamentos
```

### Teste 3: Memory Leak Check

```bash
valgrind --leak-check=full --show-leak-kinds=all \
    nemo /tmp/nemo-test-500

# Fechar nemo após 30s
# Esperado: "definitely lost: 0 bytes"
```

---

## 📊 Passo 10: Medir Melhoria

```bash
#!/bin/bash

NEMO_DIR="/home/yurix/Documentos/mint-nemo"

# Sem cache (revert changes)
git stash
meson setup build-old
meson compile -C build-old
echo "Benchmark SEM cache..."
time /tmp/nemo-old/bin/nemo /tmp/nemo-test-500 &
PID=$!
sleep 15
# Scroll 10x durante esse tempo
kill $PID

# Com cache (restore changes)
git stash pop
meson setup build-new
meson compile -C build-new
echo "Benchmark COM cache..."
time /tmp/nemo-new/bin/nemo /tmp/nemo-test-500 &
PID=$!
sleep 15
# Scroll 10x (deve ser muito mais rápido!)
kill $PID

# Resultado esperado:
# SEM cache: ~250ms por scroll
# COM cache: ~15ms por scroll (16.6x faster)
```

---

## 🎯 Checklist de Integração

- [ ] Header nemo-icon-canvas-item.h modificado (cache field added)
- [ ] Include adicionado (nemo-icon-text-cache.h)
- [ ] Cache inicializada em nemo_icon_canvas_item_init()
- [ ] measure_label_text() substituída com lógica de cache
- [ ] Invalidação adicionada (rename, font change)
- [ ] Cleanup em dispose/finalize
- [ ] Compilação bem-sucedida (zero erros)
- [ ] Unit tests passando
- [ ] Valgrind: zero leaks
- [ ] Performance teste: >10x improvement
- [ ] Code review aprovado

---

## 🚨 Troubleshooting

### Erro: `undefined reference to 'nemo_icon_text_cache_lookup'`
**Solução**: Verificar se `nemo-icon-text-cache.c` está linkado em meson.build

```bash
# Em libnemo-private/meson.build, procure:
sources = [...] 
# Adicione se faltar:
sources += ['nemo-icon-text-cache.c']
```

### Erro: `field 'cache' has incomplete type`
**Solução**: Verificar se #include está correto
```c
// Deve estar ACIMA de:
#include "nemo-icon-text-cache.h"
```

### Scroll ainda lento
**Solução**: Verificar hit rate
```c
// Adicione log temporário:
NemoIconTextCacheStats stats = nemo_icon_text_cache_get_stats (cache);
g_message ("Cache hits: %lu, misses: %lu, rate: %.1f%%",
           stats.hit_count, stats.miss_count, stats.hit_rate * 100);
```

---

## 📚 Próximos Passos

✅ **Agora que a cache está integrada**:

1. **ONDA 5.2**: Integrar lazy-loader (nemo-icon-lazy-loader.c)
2. **ONDA 5.3**: Executar suite de testes completa
3. **ONDA 5.4**: Benchmark antes/depois
4. **ONDA 6**: Profiling com perf/valgrind
5. **ONDA 7**: Otimizações avançadas (thumbnails, I/O batching)

---

**Referência**: [ONDAS-5-7-DETALHADO.md](ONDAS-5-7-DETALHADO.md)

