# ============================================================================
# CMakeLists.txt para o Componente de Interface Gráfica (UI LVGL) no ESP-IDF
# ============================================================================
# Este arquivo registra o componente `ui` gerado pelo editor LVGL / LVGL Flow,
# permitindo sua compilação transparente pelo sistema CMake do ESP-IDF.

if(COMMAND idf_component_register)
    # Busca recursiva de todos os arquivos C do componente da interface
    file(GLOB_RECURSE UI_SOURCES "${CMAKE_CURRENT_LIST_DIR}/*.c")

    # Registro do componente no ESP-IDF
    idf_component_register(
        SRCS ${UI_SOURCES}
        INCLUDE_DIRS 
            "."
            ".."
            "components"
            "fonts"
            "images"
            "screens"
            "widgets"

        REQUIRES lvgl
    )

    # Define a macro LV_LVGL_H_INCLUDE_SIMPLE para que os arquivos C/H gerados pela UI
    # façam o include direto de "lvgl.h" (padrão ESP-IDF / Component Manager) em vez de "lvgl/lvgl.h"
    target_compile_definitions(${COMPONENT_LIB} PUBLIC LV_LVGL_H_INCLUDE_SIMPLE)
else()
    # Compatibilidade mantida para compilação Desktop/Simulador caso necessário
    if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
      cmake_minimum_required(VERSION 3.10)
      project(LVGLProject)
    endif()

    if(DEFINED LVGL_COMPONENT_LIB_NAME)
      set(LIB_NAME ${LVGL_COMPONENT_LIB_NAME})
    else()
      set(LIB_NAME lib-ui)
    endif()

    include(${CMAKE_CURRENT_LIST_DIR}/user_config.cmake OPTIONAL)
    include(${CMAKE_CURRENT_LIST_DIR}/file_list_gen.cmake OPTIONAL)

    add_library(${LIB_NAME} ${LV_EDITOR_PROJECT_SOURCES})
    target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
endif()
