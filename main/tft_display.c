/**
 * @file tft_display.c
 * @brief Driver de Baixo Nível e Primitivas Gráficas para Display ST7735 TFT-SPI.
 * @details Este arquivo implementa a inicialização do SPI Master do ESP32 com DMA,
 *          o envio de comandos e dados para o controlador ST7735, a definição de janelas
 *          de escrita (CASET/RASET/RAMWR), o streaming de pixels RGB565 para o LVGL,
 *          e funções primitivas gráficas auxiliares.
 */

#include "tft_display.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "TFT_DISPLAY";

// ============================================================================
// COMANDOS ST7735 DO PROTOCOLO SPI
// ============================================================================
#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_SLPOUT  0x11
#define ST7735_NORON   0x13
#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_MADCTL  0x36
#define ST7735_COLMOD  0x3A
#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5
#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1


// ============================================================================
// VARIÁVEIS LOCAIS E MANIPULADORES SPI
// ============================================================================
static spi_device_handle_t spi_dev = NULL;

// Fonte interna 8x8 simples ASCII para renderização direta de texto (0x20 a 0x7E)
static const uint8_t font8x8_basic[95][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // ' '
    {0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00}, // '!'
    {0x36,0x36,0x00,0x00,0x00,0x00,0x00,0x00}, // '"'
    {0x66,0x66,0xFF,0x66,0xFF,0x66,0x66,0x00}, // '#'
    {0x18,0x3E,0x60,0x3C,0x06,0x7C,0x18,0x00}, // '$'
    {0x62,0x66,0x0C,0x18,0x30,0x66,0x46,0x00}, // '%'
    {0x3C,0x66,0x3C,0x38,0x67,0x66,0x3F,0x00}, // '&'
    {0x06,0x0C,0x18,0x00,0x00,0x00,0x00,0x00}, // '\''
    {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00}, // '('
    {0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00}, // ')'
    {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00}, // '*'
    {0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00}, // '+'
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x30}, // ','
    {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00}, // '-'
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00}, // '.'
    {0x06,0x0C,0x18,0x30,0x60,0x40,0x00,0x00}, // '/'
    {0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00}, // '0'
    {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00}, // '1'
    {0x3C,0x66,0x06,0x1C,0x30,0x60,0x7E,0x00}, // '2'
    {0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00}, // '3'
    {0x0E,0x1E,0x36,0x66,0x7F,0x06,0x06,0x00}, // '4'
    {0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00}, // '5'
    {0x1C,0x30,0x60,0x7C,0x66,0x66,0x3C,0x00}, // '6'
    {0x7E,0x06,0x0C,0x18,0x30,0x30,0x30,0x00}, // '7'
    {0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00}, // '8'
    {0x3C,0x66,0x66,0x3E,0x06,0x0C,0x38,0x00}, // '9'
    {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00}, // ':'
    {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x30}, // ';'
    {0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0x00}, // '<'
    {0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00}, // '='
    {0x30,0x18,0x0C,0x06,0x0C,0x18,0x30,0x00}, // '>'
    {0x3C,0x66,0x06,0x0C,0x18,0x00,0x18,0x00}, // '?'
    {0x3C,0x66,0x6E,0x6E,0x60,0x66,0x3C,0x00}, // '@'
    {0x18,0x3C,0x66,0x66,0x7E,0x66,0x66,0x00}, // 'A'
    {0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C,0x00}, // 'B'
    {0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0x00}, // 'C'
    {0x78,0x6C,0x66,0x66,0x66,0x6C,0x78,0x00}, // 'D'
    {0x7E,0x60,0x60,0x7C,0x60,0x60,0x7E,0x00}, // 'E'
    {0x7E,0x60,0x60,0x7C,0x60,0x60,0x60,0x00}, // 'F'
    {0x3C,0x66,0x60,0x6E,0x66,0x66,0x3C,0x00}, // 'G'
    {0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00}, // 'H'
    {0x3E,0x18,0x18,0x18,0x18,0x18,0x3E,0x00}, // 'I'
    {0x1E,0x0C,0x0C,0x0C,0x0C,0x6C,0x38,0x00}, // 'J'
    {0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x00}, // 'K'
    {0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00}, // 'L'
    {0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x00}, // 'M'
    {0x66,0x76,0x7E,0x7E,0x6E,0x66,0x66,0x00}, // 'N'
    {0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0x00}, // 'O'
    {0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x00}, // 'P'
    {0x3C,0x66,0x66,0x66,0x66,0x3C,0x0E,0x00}, // 'Q'
    {0x7C,0x66,0x66,0x7C,0x78,0x6C,0x66,0x00}, // 'R'
    {0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C,0x00}, // 'S'
    {0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00}, // 'T'
    {0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00}, // 'U'
    {0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00}, // 'V'
    {0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00}, // 'W'
    {0x66,0x66,0x3C,0x18,0x3C,0x66,0x66,0x00}, // 'X'
    {0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x00}, // 'Y'
    {0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00}, // 'Z'
    {0x3C,0x30,0x30,0x30,0x30,0x30,0x3C,0x00}, // '['
    {0x60,0x30,0x18,0x0C,0x06,0x03,0x01,0x00}, // '\'
    {0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00}, // ']'
    {0x18,0x3C,0x66,0x00,0x00,0x00,0x00,0x00}, // '^'
    {0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00}, // '_'
    {0x30,0x18,0x0C,0x00,0x00,0x00,0x00,0x00}, // '`'
    {0x00,0x00,0x3C,0x06,0x3E,0x66,0x3E,0x00}, // 'a'
    {0x60,0x60,0x7C,0x66,0x66,0x66,0x7C,0x00}, // 'b'
    {0x00,0x00,0x3C,0x60,0x60,0x66,0x3C,0x00}, // 'c'
    {0x06,0x06,0x3E,0x66,0x66,0x66,0x3E,0x00}, // 'd'
    {0x00,0x00,0x3C,0x66,0x7E,0x60,0x3C,0x00}, // 'e'
    {0x1C,0x30,0x7C,0x30,0x30,0x30,0x30,0x00}, // 'f'
    {0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x7C}, // 'g'
    {0x60,0x60,0x7C,0x66,0x66,0x66,0x66,0x00}, // 'h'
    {0x18,0x00,0x38,0x18,0x18,0x18,0x3E,0x00}, // 'i'
    {0x0C,0x00,0x1C,0x0C,0x0C,0x0C,0x4C,0x38}, // 'j'
    {0x60,0x60,0x66,0x6C,0x78,0x6C,0x66,0x00}, // 'k'
    {0x38,0x18,0x18,0x18,0x18,0x18,0x3E,0x00}, // 'l'
    {0x00,0x00,0x66,0x7F,0x7F,0x6B,0x63,0x00}, // 'm'
    {0x00,0x00,0x7C,0x66,0x66,0x66,0x66,0x00}, // 'n'
    {0x00,0x00,0x3C,0x66,0x66,0x66,0x3C,0x00}, // 'o'
    {0x00,0x00,0x7C,0x66,0x66,0x7C,0x60,0x60}, // 'p'
    {0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x06}, // 'q'
    {0x00,0x00,0x7C,0x66,0x60,0x60,0x60,0x00}, // 'r'
    {0x00,0x00,0x3E,0x60,0x3C,0x06,0x7C,0x00}, // 's'
    {0x18,0x18,0x7E,0x18,0x18,0x18,0x0E,0x00}, // 't'
    {0x00,0x00,0x66,0x66,0x66,0x66,0x3E,0x00}, // 'u'
    {0x00,0x00,0x66,0x66,0x66,0x3C,0x18,0x00}, // 'v'
    {0x00,0x00,0x63,0x6B,0x7F,0x3E,0x36,0x00}, // 'w'
    {0x00,0x00,0x66,0x3C,0x18,0x3C,0x66,0x00}, // 'x'
    {0x00,0x00,0x66,0x66,0x66,0x3E,0x06,0x7C}, // 'y'
    {0x00,0x00,0x7E,0x0C,0x18,0x30,0x7E,0x00}, // 'z'
    {0x0E,0x18,0x18,0x70,0x18,0x18,0x0E,0x00}, // '{'
    {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00}, // '|'
    {0x70,0x18,0x18,0x0E,0x18,0x18,0x70,0x00}, // '}'
    {0x3B,0x6E,0x00,0x00,0x00,0x00,0x00,0x00}  // '~'
};

// ============================================================================
// FUNÇÕES AUXILIARES DE ENVIO SPI (COMANDO E DADOS)
// ============================================================================

/**
 * @brief Envia um byte de COMANDO via SPI para o ST7735 (pino DC em nível BAIXO).
 */
static void send_cmd(uint8_t cmd) {
    gpio_set_level(TFT_DC_IO, 0); // DC = 0 indica comando
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd,
    };
    spi_device_polling_transmit(spi_dev, &t);
}

/**
 * @brief Envia dados de 8 bits via SPI para o ST7735 (pino DC em nível ALTO).
 */
static void send_data(const uint8_t *data, size_t len) {
    if (len == 0) return;
    gpio_set_level(TFT_DC_IO, 1); // DC = 1 indica dados
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = data,
    };
    spi_device_polling_transmit(spi_dev, &t);
}

/**
 * @brief Envia um único byte de dado via SPI.
 */
static void send_data_byte(uint8_t data) {
    send_data(&data, 1);
}

// ============================================================================
// INICIALIZAÇÃO DO HARDWARE E DISPLAY
// ============================================================================

esp_err_t tft_display_init(void) {
    ESP_LOGI(TAG, "Inicializando o driver TFT-SPI ST7735 (128x160 RGB565)...");

    // 1. Configuração dos pinos GPIO de controle (DC, RST, BL)
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << TFT_DC_IO) | (1ULL << TFT_RST_IO) | (1ULL << TFT_BL_IO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    // Liga o backlight imediatamente
    gpio_set_level(TFT_BL_IO, 1);

    // 2. Hardware Reset no controlador ST7735
    gpio_set_level(TFT_RST_IO, 0);
    vTaskDelay(pdMS_TO_TICKS(50));
    gpio_set_level(TFT_RST_IO, 1);
    vTaskDelay(pdMS_TO_TICKS(120));

    // 3. Configuração do barramento SPI (MOSI e SCLK com suporte a DMA)
    spi_bus_config_t buscfg = {
        .mosi_io_num = TFT_MOSI_IO,
        .miso_io_num = -1,
        .sclk_io_num = TFT_SCLK_IO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = TFT_WIDTH * TFT_HEIGHT * 2 + 10,
    };
    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao inicializar o barramento SPI DMA: %s", esp_err_to_name(ret));
        return ret;
    }

    // 4. Adiciona o dispositivo ST7735 ao barramento SPI
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 26 * 1000 * 1000, // 26 MHz para máxima fluidez
        .mode = 0,                         // Modo SPI 0 (CPOL=0, CPHA=0)
        .spics_io_num = TFT_CS_IO,
        .queue_size = 7,
    };
    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi_dev);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao adicionar dispositivo SPI: %s", esp_err_to_name(ret));
        return ret;
    }

    // 5. Sequência completa de comandos de inicialização dos registradores do ST7735
    ESP_LOGI(TAG, "Enviando sequência completa de registradores (Power Control, Frame Rate, Gamma)...");

    send_cmd(ST7735_SWRESET); // Reset de software
    vTaskDelay(pdMS_TO_TICKS(150));

    send_cmd(ST7735_SLPOUT);  // Sair do modo Sleep
    vTaskDelay(pdMS_TO_TICKS(200));

    // Frame Rate Control (Modo normal, idle e parcial)
    send_cmd(ST7735_FRMCTR1);
    uint8_t frm1[3] = { 0x01, 0x2C, 0x2D };
    send_data(frm1, 3);
    send_cmd(ST7735_FRMCTR2);
    send_data(frm1, 3);
    send_cmd(ST7735_FRMCTR3);
    uint8_t frm3[6] = { 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D };
    send_data(frm3, 6);

    // Controle de Inversão de Display
    send_cmd(ST7735_INVCTR);
    send_data_byte(0x07);

    // Configurações de Energia e Gerador de Carga Interno (Charge Pump)
    send_cmd(ST7735_PWCTR1);
    uint8_t pw1[3] = { 0xA2, 0x02, 0x84 };
    send_data(pw1, 3);

    send_cmd(ST7735_PWCTR2);
    send_data_byte(0xC5);

    send_cmd(ST7735_PWCTR3);
    uint8_t pw3[2] = { 0x0A, 0x00 };
    send_data(pw3, 2);

    send_cmd(ST7735_PWCTR4);
    uint8_t pw4[2] = { 0x8A, 0x2A };
    send_data(pw4, 2);

    send_cmd(ST7735_PWCTR5);
    uint8_t pw5[2] = { 0x8A, 0xEE };
    send_data(pw5, 2);

    send_cmd(ST7735_VMCTR1);
    send_data_byte(0x0E);

    send_cmd(ST7735_INVOFF);  // Desativa inversão de cor (ST7735_BLACKTAB: 0x0000 = Preto puro, 0xFFFF = Branco)

    send_cmd(ST7735_COLMOD);  // Formato de cor do pixel: 16-bit RGB565
    send_data_byte(0x05);

    send_cmd(ST7735_MADCTL);  // Orientação de tela e ordem de cor (0xC0 = MY|MX|RGB)
    send_data_byte(0xC0);


    // Correção de Curva Gamma (Positiva e Negativa)
    send_cmd(ST7735_GMCTRP1);
    uint8_t gm_p[16] = { 0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2D, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10 };
    send_data(gm_p, 16);

    send_cmd(ST7735_GMCTRN1);
    uint8_t gm_n[16] = { 0x03, 0x1D, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10 };
    send_data(gm_n, 16);

    send_cmd(ST7735_NORON);   // Modo Normal
    vTaskDelay(pdMS_TO_TICKS(10));

    send_cmd(ST7735_DISPON);  // Liga exibição do display
    vTaskDelay(pdMS_TO_TICKS(100));

    // Teste visual de inicialização preenchendo a tela com cor azul por 100ms
    tft_display_fill_screen(TFT_BLUE);
    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "Display TFT-SPI ST7735 inicializado com sucesso!");
    return ESP_OK;
}


// ============================================================================
// DEFINIÇÃO DA JANELA ATIVA (CASET / RASET / RAMWR)
// ============================================================================

void tft_display_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    // Comando CASET (Column Address Set - 0x2A)
    send_cmd(ST7735_CASET);
    uint8_t data_x[] = { (x0 >> 8) & 0xFF, x0 & 0xFF, (x1 >> 8) & 0xFF, x1 & 0xFF };
    send_data(data_x, 4);

    // Comando RASET (Row Address Set - 0x2B)
    send_cmd(ST7735_RASET);
    uint8_t data_y[] = { (y0 >> 8) & 0xFF, y0 & 0xFF, (y1 >> 8) & 0xFF, y1 & 0xFF };
    send_data(data_y, 4);

    // Comando RAMWR (Memory Write - 0x2C) para preparar envio de pixels
    send_cmd(ST7735_RAMWR);
}

// ============================================================================
// STREAMING DE PIXELS BRUTOS (CALLBACK DO LVGL)
// ============================================================================

void tft_display_draw_raw_pixels(int x, int y, int w, int h, const uint8_t *pixels_rgb565, size_t len) {
    if (w <= 0 || h <= 0 || pixels_rgb565 == NULL || len == 0) return;

    uint16_t x1 = x + w - 1;
    uint16_t y1 = y + h - 1;

    // Define a janela no ST7735 e inicia a transmissão por DMA
    tft_display_set_window(x, y, x1, y1);
    send_data(pixels_rgb565, len);
}

// ============================================================================
// PRIMITIVAS GRÁFICAS DE BAIXO NÍVEL
// ============================================================================

void tft_display_fill_screen(uint16_t color) {
    tft_display_fill_rect(0, 0, TFT_WIDTH, TFT_HEIGHT, color);
}

void tft_display_draw_pixel(int x, int y, uint16_t color) {
    if (x < 0 || x >= TFT_WIDTH || y < 0 || y >= TFT_HEIGHT) return;
    tft_display_set_window(x, y, x, y);
    uint8_t data[] = { (color >> 8) & 0xFF, color & 0xFF };
    send_data(data, 2);
}

void tft_display_fill_rect(int x, int y, int w, int h, uint16_t color) {
    if (x >= TFT_WIDTH || y >= TFT_HEIGHT || w <= 0 || h <= 0) return;
    if (x + w > TFT_WIDTH)  w = TFT_WIDTH - x;
    if (y + h > TFT_HEIGHT) h = TFT_HEIGHT - y;

    tft_display_set_window(x, y, x + w - 1, y + h - 1);

    size_t total_pixels = w * h;
    size_t chunk_pixels = (total_pixels > 256) ? 256 : total_pixels;
    uint8_t *buffer = heap_caps_malloc(chunk_pixels * 2, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (!buffer) return;

    uint8_t high = (color >> 8) & 0xFF;
    uint8_t low  = color & 0xFF;

    for (size_t i = 0; i < chunk_pixels; i++) {
        buffer[i * 2]     = high;
        buffer[i * 2 + 1] = low;
    }

    size_t remaining = total_pixels;
    while (remaining > 0) {
        size_t current_chunk = (remaining > chunk_pixels) ? chunk_pixels : remaining;
        send_data(buffer, current_chunk * 2);
        remaining -= current_chunk;
    }

    free(buffer);
}

void tft_display_draw_rect(int x, int y, int w, int h, uint16_t color) {
    tft_display_draw_line(x, y, x + w - 1, y, color);
    tft_display_draw_line(x, y + h - 1, x + w - 1, y + h - 1, color);
    tft_display_draw_line(x, y, x, y + h - 1, color);
    tft_display_draw_line(x + w - 1, y, x + w - 1, y + h - 1, color);
}

void tft_display_draw_line(int x0, int y0, int x1, int y1, uint16_t color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        tft_display_draw_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void tft_display_draw_circle(int x0, int y0, int r, uint16_t color) {
    int f = 1 - r;
    int ddF_x = 1;
    int ddF_y = -2 * r;
    int x = 0;
    int y = r;

    tft_display_draw_pixel(x0, y0 + r, color);
    tft_display_draw_pixel(x0, y0 - r, color);
    tft_display_draw_pixel(x0 + r, y0, color);
    tft_display_draw_pixel(x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        tft_display_draw_pixel(x0 + x, y0 + y, color);
        tft_display_draw_pixel(x0 - x, y0 + y, color);
        tft_display_draw_pixel(x0 + x, y0 - y, color);
        tft_display_draw_pixel(x0 - x, y0 - y, color);
        tft_display_draw_pixel(x0 + y, y0 + x, color);
        tft_display_draw_pixel(x0 - y, y0 + x, color);
        tft_display_draw_pixel(x0 + y, y0 - x, color);
        tft_display_draw_pixel(x0 - y, y0 - x, color);
    }
}

void tft_display_fill_circle(int x0, int y0, int r, uint16_t color) {
    tft_display_draw_line(x0, y0 - r, x0, y0 + r, color);
    int f = 1 - r;
    int ddF_x = 1;
    int ddF_y = -2 * r;
    int x = 0;
    int y = r;

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        tft_display_draw_line(x0 + x, y0 - y, x0 + x, y0 + y, color);
        tft_display_draw_line(x0 - x, y0 - y, x0 - x, y0 + y, color);
        tft_display_draw_line(x0 + y, y0 - x, x0 + y, y0 + x, color);
        tft_display_draw_line(x0 - y, y0 - x, x0 - y, y0 + x, color);
    }
}

void tft_display_draw_round_rect(int x, int y, int w, int h, int r, uint16_t color) {
    tft_display_draw_line(x + r, y, x + w - r - 1, y, color);
    tft_display_draw_line(x + r, y + h - 1, x + w - r - 1, y + h - 1, color);
    tft_display_draw_line(x, y + r, x, y + h - r - 1, color);
    tft_display_draw_line(x + w - 1, y + r, x + w - 1, y + h - r - 1, color);
}

void tft_display_fill_round_rect(int x, int y, int w, int h, int r, uint16_t color) {
    tft_display_fill_rect(x + r, y, w - 2 * r, h, color);
    tft_display_fill_rect(x, y + r, r, h - 2 * r, color);
    tft_display_fill_rect(x + w - r, y + r, r, h - 2 * r, color);
}

// ============================================================================
// RENDERIZAÇÃO DE TEXTOS E MARQUEE
// ============================================================================

void tft_display_draw_string(int x, int y, const char *str, uint16_t fg_color, uint16_t bg_color) {
    tft_display_draw_string_scaled(x, y, str, fg_color, bg_color, 1);
}

void tft_display_draw_string_scaled(int x, int y, const char *str, uint16_t fg_color, uint16_t bg_color, uint8_t scale) {
    if (str == NULL || scale == 0) return;
    int curr_x = x;
    while (*str) {
        char c = *str;
        if (c >= 0x20 && c <= 0x7E) {
            uint8_t idx = c - 0x20;
            for (int col = 0; col < 8; col++) {
                uint8_t line = font8x8_basic[idx][col];
                for (int row = 0; row < 8; row++) {
                    uint16_t color = (line & (1 << row)) ? fg_color : bg_color;
                    if (scale == 1) {
                        tft_display_draw_pixel(curr_x + col, y + row, color);
                    } else {
                        tft_display_fill_rect(curr_x + col * scale, y + row * scale, scale, scale, color);
                    }
                }
            }
        }
        curr_x += 8 * scale;
        str++;
    }
}

void tft_display_draw_string_truncated(int x, int y, const char *str, int max_chars, uint16_t fg_color, uint16_t bg_color, uint8_t scale) {
    if (str == NULL || max_chars <= 0) return;
    char buf[128];
    size_t len = strlen(str);
    if ((int)len > max_chars && max_chars > 3) {
        strncpy(buf, str, max_chars - 3);
        buf[max_chars - 3] = '\0';
        strcat(buf, "...");
        tft_display_draw_string_scaled(x, y, buf, fg_color, bg_color, scale);
    } else {
        tft_display_draw_string_scaled(x, y, str, fg_color, bg_color, scale);
    }
}

void tft_display_draw_string_marquee(int x, int y, const char *str, int visible_chars, int scroll_offset, uint16_t fg_color, uint16_t bg_color, uint8_t scale) {
    if (str == NULL || visible_chars <= 0) return;
    size_t len = strlen(str);
    if (len == 0) return;

    char buf[128];
    int start_idx = scroll_offset % len;
    for (int i = 0; i < visible_chars; i++) {
        buf[i] = str[(start_idx + i) % len];
    }
    buf[visible_chars] = '\0';
    tft_display_draw_string_scaled(x, y, buf, fg_color, bg_color, scale);
}

void tft_display_draw_bitmap_icon(int x, int y, int w, int h, const uint8_t *bitmap, uint16_t fg_color, uint16_t bg_color) {
    if (bitmap == NULL) return;
    int byte_width = (w + 7) / 8;
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            uint8_t byte = bitmap[j * byte_width + i / 8];
            uint16_t color = (byte & (128 >> (i & 7))) ? fg_color : bg_color;
            tft_display_draw_pixel(x + i, y + j, color);
        }
    }
}
