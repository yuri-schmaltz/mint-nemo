# SYSTEM_MAP

## Componentes
- `meson.build`: orquestra build, dependências GTK3/Cinnamon/XApp e subdiretórios (`meson.build:72` a `meson.build:85`, `meson.build:179` a `meson.build:192`).
- `src/meson.build`: define executáveis principais (`src/meson.build:116`, `src/meson.build:124`, `src/meson.build:132`, `src/meson.build:140`).
- `libnemo-private/`: biblioteca interna com ações, busca, operações de arquivo e integração UI.
- `eel/`: utilitários/abstrações GTK/GDK e teste unitário (`eel/meson.build:40`).
- `gresources/`: assets UI (Glade/UI/CSS) carregados pela aplicação.
- `debian/`: empacotamento e flags de configuração (`debian/rules:6` a `debian/rules:11`).

## Fluxos E2E críticos (3-5)
1. Início do gerenciador de arquivos:
- Entrada: `nemo` (`src/meson.build:116`).
- Caminho: inicialização app/janela (`src/nemo-main.c`, `src/nemo-window.c`).
- Saída: janela principal com navegação e painéis.

2. Modo desktop:
- Entrada: `nemo-desktop` (`src/meson.build:124`).
- Caminho: app desktop + overlay (`src/nemo-desktop-main.c`, `src/nemo-desktop-overlay.c`).
- Saída: ícones e overlay no desktop.

3. Conectar a servidor remoto:
- Entrada: `nemo-connect-server` (`src/meson.build:140`).
- Caminho: diálogo e operação de conexão (`src/nemo-connect-server-dialog.c`, `src/nemo-connect-server-operation.c`).
- Saída: montagem/abertura de localização remota.

4. Ações customizadas (Nemo Actions):
- Entrada: carregamento/layout de ações (`libnemo-private/nemo-action-manager.c`).
- Caminho: parsing/configuração e integração em UI manager (`libnemo-private/nemo-ui-utilities.c`).
- Saída: ações em menu/toolbar/contexto.

5. Busca avançada e helpers:
- Entrada: query no editor (`src/nemo-query-editor.c`).
- Caminho: engine de busca e subprocessos auxiliares (`libnemo-private/nemo-search-engine-advanced.c`).
- Saída: resultados de busca em view.

## Integrações externas
- GTK3/GIO/GLib, Cinnamon Desktop, XApp (`meson.build:72` a `meson.build:85`).
- DBus codegen para interfaces freedesktop/cinnamon (`src/meson.build:1` a `src/meson.build:15`).
- Empacotamento Debian (`debian/control`, `debian/rules`).
