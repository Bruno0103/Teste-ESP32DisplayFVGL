# ============================================================================
# CMakeLists.txt Principal do Projeto ESP-IDF para ESP32
# ============================================================================
# As linhas a seguir configuram o sistema de compilação padrão do ESP-IDF.
# Para utilizar o simulador no PC novamente, utilize o arquivo `CMakeLists_simulator.txt`.

cmake_minimum_required(VERSION 3.16)

# ============================================================================
# Configuração de Diretórios Adicionais de Componentes
# ============================================================================
# Como a pasta 'ui' está localizada na raiz do projeto (e não dentro de 'components/'),
# informamos explicitamente ao ESP-IDF para buscar componentes no diretório 'ui'.
list(APPEND EXTRA_COMPONENT_DIRS "ui")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(Teste-ESP32DisplayFVGL)

