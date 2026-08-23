# ============================================================================
# CMakeLists.txt para o Componente Principal (main) no ESP-IDF
# ============================================================================
idf_component_register(
    SRCS 
        "main.c"
        "tft_display.c"
    INCLUDE_DIRS 
        "."
    REQUIRES 
        ui
        lvgl
        driver
        esp_driver_spi
        esp_driver_gpio
        esp_timer
        freertos
        heap

)
