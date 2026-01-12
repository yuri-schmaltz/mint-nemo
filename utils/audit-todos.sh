#!/bin/bash
# audit-todos.sh — Catalogar TODOs/FIXMEs/HACKs por arquivo, linha e tipo
# Uso: bash utils/audit-todos.sh
# Saída: Lista categorizada de TODOs, salva em /tmp/nemo-audit-todos.txt

set -e

OUTPUT_FILE="/tmp/nemo-audit-todos.txt"
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "==================================================================="
echo "AUDITORIA NEMO: TODOs, FIXMEs, HACKs e XXX"
echo "Projeto: $PROJECT_ROOT"
echo "==================================================================="
echo ""

# Criar arquivo de saída
: > "$OUTPUT_FILE"

echo "Procurando por padrões de TODO/FIXME/HACK/XXX..." | tee -a "$OUTPUT_FILE"
echo "" | tee -a "$OUTPUT_FILE"

# Capturar TODOs com padrão estruturado
grep -rn "TODO\|FIXME\|HACK\|XXX" "$PROJECT_ROOT/src" "$PROJECT_ROOT/libnemo-private" \
    "$PROJECT_ROOT/libnemo-extension" "$PROJECT_ROOT/eel" 2>/dev/null | \
    while IFS=: read -r file line content; do
        # Normalizar path relativo
        rel_file="${file#$PROJECT_ROOT/}"
        # Classificar por tipo
        case "$content" in
            *TODO*) type="TODO" ;;
            *FIXME*) type="FIXME" ;;
            *HACK*) type="HACK" ;;
            *XXX*) type="XXX" ;;
            *) type="OTHER" ;;
        esac
        # Limpar espaços em branco e truncar para 80 chars
        clean_content=$(echo "$content" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//' | cut -c1-80)
        # Formatar saída
        printf "%-20s [%s:%5d] %s\n" "[$type]" "$rel_file" "$line" "$clean_content"
    done | tee -a "$OUTPUT_FILE" | sort

echo "" | tee -a "$OUTPUT_FILE"

# Estatísticas
echo "==================================================================="  | tee -a "$OUTPUT_FILE"
echo "RESUMO ESTATÍSTICO" | tee -a "$OUTPUT_FILE"
echo "===================================================================" | tee -a "$OUTPUT_FILE"

total=$(grep -rn "TODO\|FIXME\|HACK\|XXX" "$PROJECT_ROOT/src" "$PROJECT_ROOT/libnemo-private" \
    "$PROJECT_ROOT/libnemo-extension" "$PROJECT_ROOT/eel" 2>/dev/null | wc -l)
echo "Total de ocorrências: $total" | tee -a "$OUTPUT_FILE"
echo "" | tee -a "$OUTPUT_FILE"

echo "Por tipo:" | tee -a "$OUTPUT_FILE"
grep -rn "TODO\|FIXME\|HACK\|XXX" "$PROJECT_ROOT/src" "$PROJECT_ROOT/libnemo-private" \
    "$PROJECT_ROOT/libnemo-extension" "$PROJECT_ROOT/eel" 2>/dev/null | \
    awk -F: '{
        if ($3 ~ /TODO/) print "TODO"; 
        else if ($3 ~ /FIXME/) print "FIXME"; 
        else if ($3 ~ /HACK/) print "HACK"; 
        else if ($3 ~ /XXX/) print "XXX"; 
        else print "OTHER"
    }' | sort | uniq -c | sort -rn | tee -a "$OUTPUT_FILE"

echo "" | tee -a "$OUTPUT_FILE"
echo "Arquivos com maior concentração:" | tee -a "$OUTPUT_FILE"
grep -rn "TODO\|FIXME\|HACK\|XXX" "$PROJECT_ROOT/src" "$PROJECT_ROOT/libnemo-private" \
    "$PROJECT_ROOT/libnemo-extension" "$PROJECT_ROOT/eel" 2>/dev/null | \
    awk -F: '{
        file=$1;
        gsub(/.*\//, "", file);
        print file
    }' | sort | uniq -c | sort -rn | head -10 | tee -a "$OUTPUT_FILE"

echo "" | tee -a "$OUTPUT_FILE"
echo "==================================================================="  | tee -a "$OUTPUT_FILE"
echo "Relatório completo salvo em: $OUTPUT_FILE" | tee -a "$OUTPUT_FILE"
echo "==================================================================="

exit 0
