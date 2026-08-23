#ifndef TFT_DISPLAY_H
#define TFT_DISPLAY_H


/**
 * @file tft_display.h
 * @brief Driver de Baixo Nível e Primitivas Gráficas do Display ST7735 (128x160
 * RGB565) via TFT-SPI.
 * @details Este módulo é estritamente responsável pela configuração do hardware
 * SPI DMA do ESP32, inicialização do controlador ST7735 via
 * registradores/comandos SPI, e provimento de primitivas gráficas aceleradas
 * (pontos, linhas, retângulos, círculos, retângulos arredondados, fontes
 * escaladas, tratamento de textos longos e streaming).
 */

#include "esp_err.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// ============================================================================
// ESPECIFICAÇÕES DA TELA ST7735 (128x160 RGB565)
// ============================================================================
#define TFT_WIDTH 128  /**< Largura da tela em pixels */
#define TFT_HEIGHT 160 /**< Altura da tela em pixels */

// ============================================================================
// PALETA DE CORES EM FORMATO RGB565 (16 BITS)
// ============================================================================
#define TFT_BLACK 0x0000     /**< Preto */
#define TFT_WHITE 0xFFFF     /**< Branco */
#define TFT_RED 0xF800       /**< Vermelho */
#define TFT_GREEN 0x07E0     /**< Verde */
#define TFT_BLUE 0x001F      /**< Azul */
#define TFT_YELLOW 0xFFE0    /**< Amarelo */
#define TFT_CYAN 0x07FF      /**< Ciano */
#define TFT_MAGENTA 0xF81F   /**< Magenta */
#define TFT_ORANGE 0xFD20    /**< Laranja */
#define TFT_DARKGRAY 0x2104  /**< Cinza Escuro */
#define TFT_LIGHTGRAY 0xC618 /**< Cinza Claro */
#define TFT_NAVY 0x000F      /**< Azul Marinho */
#define TFT_PURPLE 0x780F    /**< Roxo */
#define TFT_DARKBLUE 0x0810  /**< Azul Escuro */
#define TFT_DARKCYAN 0x03E0  /**< Ciano Escuro */
#define TFT_DARKGREEN 0x0320 /**< Verde Escuro */
#define TFT_CARD_BG 0x10A2   /**< Cor de Fundo de Card (Modo Escuro) */

// ============================================================================
// PINAGEM DO DISPLAY TFT-SPI (SEM CONFLITO COM WM8960 I2C SDA 21 / SCL 22)
// ============================================================================
#define TFT_MOSI_IO 19 /**< GPIO Master Out Slave In (SPI MOSI) */
#define TFT_SCLK_IO 23 /**< GPIO Clock SPI (SCLK) */
#define TFT_CS_IO 14   /**< GPIO Chip Select (CS) */
#define TFT_DC_IO 27   /**< GPIO Data/Command Control (DC) */
#define TFT_RST_IO 18  /**< GPIO Reset de Hardware (RST) */
#define TFT_BL_IO 4    /**< GPIO Backlight / Controle de Brilho (BL) */

    // ============================================================================
    // PROTÓTIPOS DE FUNÇÕES DE HARDWARE E INICIALIZAÇÃO
    // ============================================================================

    /**
     * @brief Inicializa o periférico SPI DMA, pinos de controle (RST, BL) e o
     * controlador ST7735.
     * @return esp_err_t ESP_OK em caso de sucesso na inicialização.
     */
        esp_err_t tft_display_init(void);

/**
 * @brief Configura a janela ativa de exibição no ST7735 (Comandos CASET 0x2A e
 * RASET 0x2B).
 * @param x0 Coluna inicial (0 a 127).
 * @param y0 Linha inicial (0 a 159).
 * @param x1 Coluna final (0 a 127).
 * @param y1 Linha final (0 a 159).
 */
void tft_display_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

// ============================================================================
// PRIMITIVAS GRÁFICAS DE BAIXO NÍVEL
// ============================================================================

/**
 * @brief Preenche toda a tela de 128x160 com uma cor sólida em formato RGB565.
 * @param color Cor 16-bit RGB565.
 */
void tft_display_fill_screen(uint16_t color);

/**
 * @brief Desenha um único pixel na coordenada (x, y).
 * @param x Posição X.
 * @param y Posição Y.
 * @param color Cor em formato RGB565.
 */
void tft_display_draw_pixel(int x, int y, uint16_t color);

/**
 * @brief Desenha uma linha utilizando o algoritmo de Bresenham.
 * @param x0 Coordenada X inicial.
 * @param y0 Coordenada Y inicial.
 * @param x1 Coordenada X final.
 * @param y1 Coordenada Y final.
 * @param color Cor da linha em RGB565.
 */
void tft_display_draw_line(int x0, int y0, int x1, int y1, uint16_t color);

/**
 * @brief Desenha o contorno de um retângulo.
 * @param x Posição X do canto superior esquerdo.
 * @param y Posição Y do canto superior esquerdo.
 * @param w Largura em pixels.
 * @param h Altura em pixels.
 * @param color Cor do contorno em RGB565.
 */
void tft_display_draw_rect(int x, int y, int w, int h, uint16_t color);

/**
 * @brief Desenha um retângulo totalmente preenchido.
 * @param x Posição X do canto superior esquerdo.
 * @param y Posição Y do canto superior esquerdo.
 * @param w Largura em pixels.
 * @param h Altura em pixels.
 * @param color Cor do preenchimento em RGB565.
 */
void tft_display_fill_rect(int x, int y, int w, int h, uint16_t color);

/**
 * @brief Desenha o contorno de um círculo.
 * @param x0 Coordenada X do centro.
 * @param y0 Coordenada Y do centro.
 * @param r Raio do círculo em pixels.
 * @param color Cor do contorno em RGB565.
 */
void tft_display_draw_circle(int x0, int y0, int r, uint16_t color);

/**
 * @brief Desenha um círculo preenchido.
 * @param x0 Coordenada X do centro.
 * @param y0 Coordenada Y do centro.
 * @param r Raio do círculo em pixels.
 * @param color Cor de preenchimento em RGB565.
 */
void tft_display_fill_circle(int x0, int y0, int r, uint16_t color);

/**
 * @brief Desenha o contorno de um retângulo com cantos arredondados.
 * @param x Posição X.
 * @param y Posição Y.
 * @param w Largura.
 * @param h Altura.
 * @param r Raio das curvas dos cantos.
 * @param color Cor em RGB565.
 */
void tft_display_draw_round_rect(int x, int y, int w, int h, int r,
                                 uint16_t color);

/**
 * @brief Desenha um retângulo preenchido com cantos arredondados.
 * @param x Posição X.
 * @param y Posição Y.
 * @param w Largura.
 * @param h Altura.
 * @param r Raio das curvas dos cantos.
 * @param color Cor em RGB565.
 */
void tft_display_fill_round_rect(int x, int y, int w, int h, int r,
                                 uint16_t color);

// ============================================================================
// RENDERIZAÇÃO DE TEXTOS, TRUNCAÇÃO E ROLAGEM MARQUEE
// ============================================================================

/**
 * @brief Desenha uma string de texto na tela usando a fonte embutida 8x8.
 * @param x Posição X inicial.
 * @param y Posição Y inicial.
 * @param str Ponteiro para a string terminada em nulo.
 * @param fg_color Cor do texto (foreground).
 * @param bg_color Cor do fundo do texto (background).
 */
void tft_display_draw_string(int x, int y, const char *str, uint16_t fg_color,
                             uint16_t bg_color);

/**
 * @brief Desenha texto com fator de escala multiplicador (ex: escala 2 = 16x16
 * por caractere).
 * @param x Posição X inicial.
 * @param y Posição Y inicial.
 * @param str Texto a ser renderizado.
 * @param fg_color Cor do texto.
 * @param bg_color Cor de fundo.
 * @param scale Fator de escala (1 para 8x8, 2 para 16x16, etc.).
 */
void tft_display_draw_string_scaled(int x, int y, const char *str,
                                    uint16_t fg_color, uint16_t bg_color,
                                    uint8_t scale);

/**
 * @brief Desenha texto truncado com reticências (...) caso exceda o número
 * máximo de caracteres visíveis.
 * @param x Posição X.
 * @param y Posição Y.
 * @param str Texto original.
 * @param max_chars Número máximo de caracteres permitidos na linha.
 * @param fg_color Cor do texto.
 * @param bg_color Cor de fundo.
 * @param scale Fator de escala.
 */
void tft_display_draw_string_truncated(int x, int y, const char *str,
                                       int max_chars, uint16_t fg_color,
                                       uint16_t bg_color, uint8_t scale);

/**
 * @brief Desenha texto com efeito marquee (rolagem horizontal deslizante) para
 * strings longas.
 * @param x Posição X.
 * @param y Posição Y.
 * @param str Texto original.
 * @param visible_chars Quantidade de caracteres visíveis no container.
 * @param scroll_offset Offset da rolagem atual.
 * @param fg_color Cor do texto.
 * @param bg_color Cor de fundo.
 * @param scale Fator de escala.
 */
void tft_display_draw_string_marquee(int x, int y, const char *str,
                                     int visible_chars, int scroll_offset,
                                     uint16_t fg_color, uint16_t bg_color,
                                     uint8_t scale);

// ============================================================================
// RENDERIZAÇÃO DE BITMAPS E IMAGENS BRUTAS
// ============================================================================

/**
 * @brief Desenha um ícone bitmap monocromático de tamanho w x h pixels.
 * @param x Posição X inicial.
 * @param y Posição Y inicial.
 * @param w Largura do ícone.
 * @param h Altura do ícone.
 * @param bitmap Ponteiro para os dados da máscara bitmap de 1 bit por pixel.
 * @param fg_color Cor dos pixels ativos (1).
 * @param bg_color Cor dos pixels inativos (0).
 */
void tft_display_draw_bitmap_icon(int x, int y, int w, int h,
                                  const uint8_t *bitmap, uint16_t fg_color,
                                  uint16_t bg_color);

/**
 * @brief Envia um buffer de pixels em formato RGB565 diretamente para uma área
 * específica da tela.
 * @param x Posição X inicial.
 * @param y Posição Y inicial.
 * @param w Largura da área.
 * @param h Altura da área.
 * @param pixels_rgb565 Buffer contendo os pixels RGB565 (2 bytes por pixel).
 * @param len Tamanho total do buffer em bytes.
 */
void tft_display_draw_raw_pixels(int x, int y, int w, int h,
                                 const uint8_t *pixels_rgb565, size_t len);

#endif // TFT_DISPLAY_H
