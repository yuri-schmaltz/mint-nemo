# ONDA 6 — PROFILING DETALHADO (Opcional — Alto Impacto)

**Objetivo**: Usar ferramentas profissionais (perf, valgrind, gprof) para identificar hotspots reais.

## P6.1: Profiling com perf

```bash
# Criar pasta com 500 arquivos
mkdir -p /tmp/test-perf
for i in {1..500}; do touch /tmp/test-perf/file-$i.txt; done

# Profile CPU com perf (requer compilação com debug symbols)
cd /home/yurix/Documentos/mint-nemo
CFLAGS="-g -fno-omit-frame-pointer" meson setup build-perf
meson compile -C build-perf

perf record -g -o /tmp/nemo.perf ./build-perf/src/nemo /tmp/test-perf
# (deixar rodando 30 segundos, scroll bastante, depois fechar)

perf report -i /tmp/nemo.perf
# Ver top functions by CPU%
```

**Saída esperada**:
```
   Overhead  Command     Shared Object       Symbol
   ========  =======     =============       ======
     15.2%   nemo        libpango            pango_layout_get_extents
      8.3%   nemo        libpango            pango_layout_set_text
      7.1%   nemo        libgdk              gdk_cairo_region_create_from_path
      5.4%   nemo        nemo                measure_label_text
      3.2%   nemo        nemo                draw_or_measure_label_text
      ...
```

---

## P6.2: Memory Profiling com Valgrind

```bash
# Verificar memory leaks
valgrind --leak-check=full --show-leak-kinds=all \
         --track-origins=yes \
         --log-file=/tmp/valgrind.log \
         ./build/src/nemo /tmp/test-perf &
sleep 60
killall nemo
cat /tmp/valgrind.log | grep "LEAK SUMMARY" -A 20
```

---

## P6.3: Heap Profiling com heaptrack

```bash
heaptrack ./build/src/nemo /tmp/test-perf
# (deixar rodando 30 segundos)
killall nemo

heaptrack_gui heaptrack.nemo.*
# Abre GUI com visualização de alocações
```

---

## Ganho Esperado
- Identificar top 3 hotspots
- Reduzir memory fragmentation
- Detectar vazamentos

---

# ONDA 7 — OTIMIZAÇÕES SECUNDÁRIAS (Avançado)

## P7.1: Lazy-Load de Thumbnails

```c
// Estender nemo-icon-lazy-loader.c para thumbnails
// Ao invés de carregar ALL thumbnails na memória,
// carregar apenas os do viewport + buffer de 200px

// Ganho: -50% peak RAM para pastas com 1000+ imagens
```

## P7.2: Batching de I/O

```c
// nemo-file-operations.c
// Agrupar operações de copy/delete em batches
// Ganho: 30–40% mais rápido em lotes de 100+ arquivos
```

## P7.3: Memory Pooling

```c
// Pré-alocar pools de estruturas comuns
// Ex: GList, GHashTable entries
// Ganho: -10% fragmentation, -5% allocation overhead
```

---

## Resumo Ondas 5–7

| Onda | Escopo | Risco | Ganho | Tempo |
|------|--------|-------|-------|-------|
| **5** | Integração + testes | MÉDIO | +20–30% scroll | 4–6h |
| **6** | Profiling detalhado | BAIXO | Identificar hotspots | 2–3h |
| **7** | Otimizações secundárias | ALTO | Variável (5–50%) | 16–40h |

---

## RECOMENDAÇÃO FINAL

✅ **Executar**: ONDA 5 (integração de cache + testes)  
⚠️ **Considerar**: ONDA 6 (profiling sempre útil)  
🔮 **Futuro**: ONDA 7 (se houver time disponível)

---

FIM — Todas as ondas 1–7 mapeadas e documentadas.
