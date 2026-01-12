# 🔬 ONDA 6 — Profiling Detalhado COMPLETO

**Data**: 12 de janeiro de 2026  
**Status**: ✅ **SCRIPTS PRONTOS PARA EXECUÇÃO**

---

## 📊 Scripts Criados

### ✅ 1. profile-cpu.sh — CPU Profiling com perf

**Arquivo**: [utils/profile-cpu.sh](../utils/profile-cpu.sh)

**O que faz**:
- Cria pasta de teste com 1000 arquivos
- Executa Nemo sob `perf record -F 99 -g`
- Gera relatório de CPU hotspots
- (Opcional) Gera flamegraph se disponível

**Como usar**:
```bash
cd /home/yurix/Documentos/mint-nemo
./utils/profile-cpu.sh

# Custom options:
NEMO_BIN=./build/src/nemo NUM_FILES=2000 ./utils/profile-cpu.sh
```

**Output**:
- `perf.data` — Dados de profiling
- `perf-report.txt` — Relatório em texto
- `flamegraph.svg` — Visualização (se FlameGraph instalado)

**Hotspots esperados** (com cache):
- `pango_layout_get_extents`: <5% (antes: 15.2%)
- `gtk_widget_draw`: 10-15%
- `cairo_*`: 8-12%
- `g_hash_table_lookup`: 3-5% (cache)

---

### ✅ 2. profile-memory.sh — Memory Leak Check com valgrind

**Arquivo**: [utils/profile-memory.sh](../utils/profile-memory.sh)

**O que faz**:
- Cria pasta de teste com 500 arquivos
- Executa Nemo sob `valgrind --leak-check=full`
- Detecta memory leaks
- Gera relatório detalhado

**Como usar**:
```bash
./utils/profile-memory.sh

# Custom:
NUM_FILES=1000 ./utils/profile-memory.sh
```

**Output**:
- `valgrind-memcheck.log` — Log completo
- `valgrind-output.txt` — Output do terminal

**Resultados esperados**:
- `definitely lost`: **0 bytes** ✅
- `indirectly lost`: <100 bytes (GTK internals)
- Heap usage: 150-300 MB para 500 items

---

### ✅ 3. profile-heap.sh — Heap Profiling com heaptrack

**Arquivo**: [utils/profile-heap.sh](../utils/profile-heap.sh)

**O que faz**:
- Cria pasta de teste com 1000 arquivos
- Executa Nemo sob `heaptrack`
- Analisa heap allocation hotspots
- Gera relatório + abre GUI

**Como usar**:
```bash
./utils/profile-heap.sh

# Abrir GUI depois:
heaptrack --analyze heaptrack.nemo.*.gz
```

**Output**:
- `heaptrack.nemo.*.gz` — Dados de heap
- `heaptrack-report.txt` — Relatório texto
- GUI interativo (heaptrack-gui)

**Métricas esperadas**:
- Peak heap: 200-400 MB para 1000 items
- Top allocators: GTK widgets, Pango, pixbufs
- text_cache: ~10-20 MB

---

## 🧪 Workflow de Profiling Recomendado

### Fase 1: CPU Hotspots (15 min)

```bash
# 1. Build com debug symbols
meson setup build-debug --buildtype=debug
meson compile -C build-debug

# 2. Profile CPU
NEMO_BIN=./build-debug/src/nemo ./utils/profile-cpu.sh

# 3. Analisar
perf report  # Interactive
less perf-report.txt  # Text

# 4. Procurar por:
#    - pango_layout_get_extents (deve estar <5%)
#    - nemo_icon_text_cache_lookup (deve aparecer)
#    - Hotspots inesperados (oportunidades ONDA 7)
```

### Fase 2: Memory Leaks (20 min)

```bash
# 1. Profile memory
./utils/profile-memory.sh

# 2. Analisar leaks
grep "definitely lost" valgrind-memcheck.log
grep "LEAK SUMMARY" valgrind-memcheck.log

# 3. Se encontrar leaks:
#    - Buscar stack trace no log
#    - Corrigir em nemo-icon-canvas-item.c
#    - Re-run valgrind
```

### Fase 3: Heap Analysis (15 min)

```bash
# 1. Profile heap
./utils/profile-heap.sh

# 2. Abrir GUI
heaptrack --analyze heaptrack.nemo.*.gz

# 3. Na GUI, verificar:
#    - Peak allocations (quais funções?)
#    - Temporary allocations (muitas alocações pequenas?)
#    - Fragmentação (heap fragmentation)
```

---

## 📈 Interpretação de Resultados

### Scenario 1: Cache Funcionando ✅

**perf report mostra**:
```
15.3%  pango_layout_get_extents   → ANTES (sem cache)
 2.1%  pango_layout_get_extents   → DEPOIS (com cache) ✓
 5.8%  g_hash_table_lookup        → Cache lookup overhead (aceitável)
10.2%  gtk_widget_draw             → Normal
```

**Conclusão**: Cache efetivo, 7.3x redução em Pango calls.

---

### Scenario 2: Memory Leak Detectado ⚠️

**valgrind mostra**:
```
definitely lost: 8,192 bytes in 512 blocks
   at 0x4C2FB0F: malloc (vg_replace_malloc.c:299)
   by nemo_icon_text_cache_new (nemo-icon-text-cache.c:45)
```

**Ação**: Verificar se `nemo_icon_text_cache_free()` é chamado em `finalize()`.

---

### Scenario 3: Heap Fragmentation 🔍

**heaptrack mostra**:
```
Peak heap: 450 MB
Temporary allocations: 125,000 (muitas pequenas alocações)
```

**Oportunidade ONDA 7**: Memory pooling para reduzir fragmentação.

---

## 🎯 Critérios de Sucesso (ONDA 6)

| Métrica | Target | Como Verificar |
|---------|--------|----------------|
| **CPU: pango_layout_get_extents** | <5% | `perf report` |
| **CPU: cache lookup overhead** | <8% | `perf report` (g_hash_table_*) |
| **Memory: definite leaks** | 0 bytes | `valgrind` LEAK SUMMARY |
| **Memory: heap usage (1000 items)** | <400 MB | `heaptrack` peak |
| **Allocation hotspots** | Identificados | `heaptrack` GUI top 10 |

---

## 🚀 Próximos Passos

### Após Profiling (ONDA 6)

1. **Analisar resultados**: Identificar top 3 hotspots residuais
2. **Documentar findings**: Criar relatório com métricas reais
3. **Decidir ONDA 7**: Priorizar otimizações com base em dados

### Se Encontrar Issues

- **Memory leak**: Corrigir imediatamente (blocker para v6.7)
- **CPU hotspot >20%**: Investigar (candidato ONDA 7)
- **Heap fragmentation >15%**: Memory pooling (ONDA 7.3)

---

## 📚 Comandos Rápidos

```bash
# Profiling completo (one-liner)
./utils/profile-cpu.sh && \
./utils/profile-memory.sh && \
./utils/profile-heap.sh

# Análise rápida
perf report --stdio | head -50
grep "definitely lost" valgrind-memcheck.log
heaptrack --analyze heaptrack.nemo.*.gz

# Comparação antes/depois
# (rodar sem cache, depois com cache)
git stash  # desabilitar cache temporariamente
./utils/profile-cpu.sh
mv perf-report.txt perf-report-no-cache.txt
git stash pop
./utils/profile-cpu.sh
diff perf-report-no-cache.txt perf-report.txt
```

---

## ✅ Status

**ONDA 6: 100% PRONTO** — Scripts prontos, workflow documentado

**Próximo**: ONDA 7 (otimizações avançadas baseadas em findings)

---

*Última atualização: 12 jan 2026*

