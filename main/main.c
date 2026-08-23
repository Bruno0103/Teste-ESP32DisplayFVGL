/**
 * @file main.c
 * @brief Aplicação Principal ESP32 - Integração LVGL v9 + ST7735 TFT-SPI + Interface UI.
 * @details Este arquivo configura o sistema operacional FreeRTOS, inicializa o driver de hardware
 *          do display ST7735 TFT-SPI, configura os buffers DMA e callbacks de renderização do LVGL v9,
 *          estabelece o timer de hardware de 1ms para a contagem de ticks do LVGL, inicializa a
 *          interface gráfica exportada e gerencia o loop de eventos no ESP32.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_heap_caps.h"

// Driver do Display TFT-SPI ST7735
#include "tft_display.h"

// Biblioteca Gráfica LVGL v9
#include "lvgl.h"

// Interface de Usuário (UI) exportada
#include "ui.h"
#include "screens/preview_home_gen.h"


static const char *TAG = "MAIN_APP";

// ============================================================================
// CONFIGURAÇÕES DE BUFFER DO LVGL v9
// ============================================================================
// Tamanho do buffer de desenho parcial (128 pixels de largura x 20 linhas x 2 bytes por pixel RGB565)
#define DRAW_BUF_HEIGHT 20
#define DRAW_BUF_SIZE   (TFT_WIDTH * DRAW_BUF_HEIGHT * sizeof(uint16_t))

// ============================================================================
// CALLBACK DE FLUSH DO LVGL v9 (TRANSMISSÃO DE PIXELS PARA O DISPLAY)
// ============================================================================

/**
 * @brief Função chamada pelo LVGL para enviar os pixels renderizados para o ST7735 via TFT-SPI DMA.
 * @param disp Ponteiro para o objeto de display do LVGL.
 * @param area Área retangular da tela a ser atualizada (x1, y1 até x2, y2).
 * @param px_map Ponteiro para o buffer de memória contendo os pixels em RGB565.
 */
static void lvgl_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
    // Calcula largura e altura da região de renderização
    uint32_t width = (area->x2 - area->x1 + 1);
    uint32_t height = (area->y2 - area->y1 + 1);
    size_t len_bytes = width * height * sizeof(uint16_t);

    // Envia o buffer de pixels diretamente para o display ST7735 via SPI DMA
    tft_display_draw_raw_pixels(area->x1, area->y1, width, height, px_map, len_bytes);

    // Notifica a biblioteca LVGL que a transferência dos pixels desta área foi concluída
    lv_display_flush_ready(disp);
}

// ============================================================================
// CALLBACK DO TEMPORIZADOR DE TICKS DO LVGL (1 MS)
// ============================================================================

/**
 * @brief Callback de interrupção do esp_timer para manter o relógio interno do LVGL atualizado.
 * @param arg Parâmetro genérico do timer (não utilizado).
 */
static void lvgl_tick_timer_cb(void *arg)
{
    // Incrementa 1 milissegundo no contador interno de tempo do LVGL
    lv_tick_inc(1);
}

// ============================================================================
// PONTO DE ENTRADA PRINCIPAL DA APLICAÇÃO (ESP-IDF)
// ============================================================================

void app_main(void)
{
    ESP_LOGI(TAG, "=================================================");
    ESP_LOGI(TAG, " Iniciando Interface LVGL v9 no ESP32 (ST7735) ");
    ESP_LOGI(TAG, "=================================================");

    // 1. Inicializa o driver de baixo nível do display ST7735 TFT-SPI
    esp_err_t ret = tft_display_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Erro crítico ao inicializar o display ST7735 TFT-SPI!");
        return;
    }

    // 2. Inicializa o núcleo da biblioteca gráfica LVGL
    lv_init();
    ESP_LOGI(TAG, "LVGL v9 inicializado com sucesso.");

    // 3. Aloca memória interna com capacidade DMA para os buffers de desenho duplo do LVGL
    uint8_t *buf1 = heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    uint8_t *buf2 = heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);

    if (buf1 == NULL || buf2 == NULL) {
        ESP_LOGE(TAG, "Falha ao alocar memória DMA para os buffers do LVGL!");
        return;
    }

    // 4. Cria e configura o objeto de display no LVGL v9
    lv_display_t *disp = lv_display_create(TFT_WIDTH, TFT_HEIGHT);
    lv_display_set_buffers(disp, buf1, buf2, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(disp, lvgl_flush_cb);

    ESP_LOGI(TAG, "Display LVGL configurado (%dx%d pixels, Modo Parcial DMA).", TFT_WIDTH, TFT_HEIGHT);

    // 5. Configura o temporizador de hardware (esp_timer) para gerar o tick de 1ms do LVGL
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lvgl_tick_timer_cb,
        .name = "lvgl_tick_timer"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 1000)); // 1000 microsegundos = 1 ms

    ESP_LOGI(TAG, "Timer de hardware para ticks do LVGL ativo.");

    // 6. Inicializa os componentes e recursos da Interface de Usuário (UI) exportada
    ui_init(NULL);
    ESP_LOGI(TAG, "Componentes da UI inicializados.");

    // 7. Instancia e carrega a tela principal gerada
    lv_obj_t *home_screen = preview_home_create();
    if (home_screen != NULL) {
        lv_screen_load(home_screen);
        ESP_LOGI(TAG, "Tela 'preview_home' carregada com sucesso.");
    } else {
        ESP_LOGW(TAG, "Não foi possível carregar a tela 'preview_home'.");
    }

    // 8. Loop principal de eventos FreeRTOS (executa temporizadores, animações e atualizações do LVGL)
    ESP_LOGI(TAG, "Iniciando loop de tarefas do LVGL...");
    while (1) {
        // Pausa de 5 ms para ceder tempo de CPU a outras tarefas FreeRTOS
        vTaskDelay(pdMS_TO_TICKS(5));

        // Executa o manipulador de tarefas do LVGL
        lv_timer_handler();
    }
}
