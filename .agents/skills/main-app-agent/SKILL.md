---
name: main-app-agent
description: Agente responsável pela orquestração principal da aplicação ESP32 em main.c, inicialização do LVGL, callbacks e tarefas FreeRTOS.
---

# Agente Orquestrador & Main App (`main-app-agent`)

Este agente é responsável por conectar todos os componentes da aplicação (driver de tela, biblioteca LVGL e interface gráfica UI) e gerenciar a execução contínua no ESP32.

## 🛠️ Arquivos sob Responsabilidade
- `main/main.c`

## 📋 Diretrizes de Implementação e Manutenção
1. **Inicialização do Sistema**:
   - `app_main(void)` deve inicializar o hardware na seguinte ordem rigorosa:
     1. Driver ST7735 (`tft_display_init`).
     2. Biblioteca LVGL (`lv_init`).
     3. Alocação de memória DMA para os buffers do LVGL (`heap_caps_malloc`).
     4. Objeto de display LVGL (`lv_display_create`).
     5. Timer periódico de hardware (`esp_timer`) de 1ms para `lv_tick_inc(1)`.
     6. Componentes da UI (`ui_init(NULL)`).
     7. Instanciação e carregamento da tela principal (`preview_home_create`).

2. **Gerenciamento de Recursos e Tarefas**:
   - Executar o loop `lv_timer_handler()` com atrasos apropriados do FreeRTOS (`vTaskDelay(pdMS_TO_TICKS(5))`).

3. **Documentação**:
   - Manter comentários extensos e detalhados em **Português (Brasil)** em cada etapa do ciclo de vida da aplicação.
