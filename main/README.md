# 📂 Módulo Main (Aplicação Principal e Driver TFT-SPI)

Este módulo é gerenciado em paralelo pelos agentes **`main-app-doc-agent`** e **`display-driver-doc-agent`**.

---

## 📄 Conteúdo do Módulo

1. **`main.c`** (Gerido por `main-app-doc-agent`)
   - Ponto de entrada `app_main()`.
   - Configuração de buffers DMA para LVGL v9.
   - Timer de interrupção de 1ms para `lv_tick_inc`.
   - Loop FreeRTOS para `lv_timer_handler()`.

2. **`tft_display.h` & `tft_display.c`** (Gerido por `display-driver-doc-agent`)
   - Driver de baixo nível do display ST7735 via TFT-SPI (`SPI2_HOST` com DMA).
   - Sequência de inicialização de registradores (`CASET`, `RASET`, `RAMWR`).
   - Primitivas gráficas e streaming de pixels `tft_display_draw_raw_pixels`.

3. **`idf_component.yml`** (Gerido por `esp-idf-build-doc-agent`)
   - Manifesto de dependência do pacote `lvgl/lvgl: "^9.2.2"`.

---

## 🔗 Links de Documentação
- Manual do Driver: [docs/display_driver_doc.md](file:///a:/BackupDesktopFileBruno/Estudos/LVGL/lvgl_vscode_project_teste/Teste-ESP32DisplayFVGL/docs/display_driver_doc.md)
- Manual do Orquestrador: [docs/main_app_doc.md](file:///a:/BackupDesktopFileBruno/Estudos/LVGL/lvgl_vscode_project_teste/Teste-ESP32DisplayFVGL/docs/main_app_doc.md)
