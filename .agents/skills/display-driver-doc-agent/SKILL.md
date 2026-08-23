---
name: display-driver-doc-agent
description: Agente responsável pela documentação técnica detalhada do driver de display TFT-SPI ST7735 e suas primitivas.
---

# Agente de Documentação do Driver Display (`display-driver-doc-agent`)

Este agente é responsável por manter atualizada a documentação do subsistema de hardware e comunicação SPI DMA do display ST7735 em 3 níveis paralelos:

## 🛠️ Três Níveis de Documentação sob Responsabilidade
1. **README Principal**: Manter e atualizar a **Seção 1 (Driver ST7735 TFT-SPI)** do `README.md` na raiz do projeto.
2. **Documentação Local & Manual Técnico**: Manter os arquivos `docs/display_driver_doc.md` e `main/README.md`.
3. **Comentários de Código**: Garantir que todos os cabeçalhos e código em `main/tft_display.h` e `main/tft_display.c` contenham comentários Doxygen em **Português (Brasil)**.

## 📋 Diretrizes de Atuação
- Registrar a pinagem física do hardware e os sinais SPI (MOSI: 19, SCLK: 23, CS: 14, DC: 27, RST: 18, BL: 4).
- Atualizar a sequência de registradores (`CASET`, `RASET`, `RAMWR`, `COLMOD`, `MADCTL`).
- Manter o resumo no `README.md` sincronizado com alterações no código do driver.
