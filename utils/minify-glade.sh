#!/bin/bash
# minify-glade.sh — Minificar arquivos .glade removendo espaços em branco
# Uso: bash minify-glade.sh input.glade output.glade.min

set -e

INPUT="$1"
OUTPUT="$2"

if [ -z "$INPUT" ] || [ -z "$OUTPUT" ]; then
    echo "Uso: $0 input.glade output.glade.min"
    exit 1
fi

if [ ! -f "$INPUT" ]; then
    echo "Erro: arquivo não encontrado: $INPUT"
    exit 1
fi

# Minificar XML: remover espaços entre tags, comentários vazios, e quebras desnecessárias
python3 - "$INPUT" "$OUTPUT" << 'PYTHON_EOF'
import sys
import xml.dom.minidom as minidom

input_file = sys.argv[1]
output_file = sys.argv[2]

try:
    # Parsing e minificação
    dom = minidom.parse(input_file)
    minified = dom.toxml()
    
    # Remove espaços entre tags (>  <)
    minified = minified.replace('>\n<', '><')
    minified = minified.replace('>  <', '><')
    
    # Salvar
    with open(output_file, 'w') as f:
        f.write(minified)
except Exception as e:
    # Fallback para sed se Python falhar
    import subprocess
    subprocess.run(f"sed -e 's/>[[:space:]]*</></g' < {input_file} > {output_file}", shell=True)
PYTHON_EOF

# Verificar resultado
if [ ! -f "$OUTPUT" ]; then
    echo "Erro ao minificar $INPUT"
    exit 1
fi

INPUT_SIZE=$(stat --format=%s "$INPUT")
OUTPUT_SIZE=$(stat --format=%s "$OUTPUT")
REDUCTION=$(( (INPUT_SIZE - OUTPUT_SIZE) * 100 / INPUT_SIZE ))

echo "✓ Minificado: $INPUT"
echo "  Antes: $INPUT_SIZE bytes"
echo "  Depois: $OUTPUT_SIZE bytes"
echo "  Redução: ${REDUCTION}%"

exit 0
