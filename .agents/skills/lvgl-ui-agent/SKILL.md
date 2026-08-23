---
name: lvgl-ui-agent
description: Agente responsável pela gestão da interface de usuário gráfica gerada pelo LVGL Editor / LVGL Flow na pasta ui/.
---

# Agente Interface Gráfica LVGL (`lvgl-ui-agent`)

Este agente é responsável pela integração, manutenção e expansão da interface de usuário (UI) construída para a biblioteca LVGL v9.

## 🛠️ Arquivos sob Responsabilidade
- Pasta `ui/` e suas subpastas:
  - `ui/screens/` (telas da aplicação, ex: `preview_home_gen.c`, `screen1_gen.c`)
  - `ui/components/` (componentes visuais reutilizáveis)
  - `ui/fonts/` e `ui/images/` (fontes C estáticas e matrizes de imagens RGB)
  - `ui/ui_gen.h` / `ui_gen.c` e `ui.h` / `ui.c`

## 📋 Diretrizes de Implementação e Manutenção
1. **Padrão de Inicialização**:
   - `ui_init(const char * asset_path)` inicializa fontes, imagens e estilos globais da biblioteca gerada.
   - Telas são instanciadas através das funções geradas (ex: `preview_home_create()`).

2. **Compatibilidade LVGL v9**:
   - Respeitar a API de Observadores e Tópicos (`lv_subject_t`) do LVGL v9.
   - Manter inclusão de cabeçalhos simples com `#include "lvgl.h"`.

3. **Documentação**:
   - Garantir que novas funções de tela ou manipuladores de eventos de botão e touch tenham descrições claras em **Português (Brasil)**.
