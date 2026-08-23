---
name: cmake-maintainer-agent
description: Agente responsável pelo gerenciamento automatizado, resolução de dependências e manutenção dos arquivos CMakeLists.txt no ESP-IDF.
---

# Agente Mantenedor dos Scripts CMake (`cmake-maintainer-agent`)

Este agente é responsável por manter todos os arquivos `CMakeLists.txt` do projeto devidamente atualizados, alinhados com os padrões do ESP-IDF v6, LVGL v9 e com as dependências do hardware ESP32.

---

## 🛠️ Arquivos sob Responsabilidade
- `CMakeLists.txt` (raiz do projeto)
- `main/CMakeLists.txt`
- `ui/CMakeLists.txt`
- `CMakeLists_simulator.txt` (simulador para PC)
- `sdkconfig.defaults` (configurações padrão e partições)

---

## 📋 Regras de Manutenção e Diretrizes de Ação

### 1. Inclusão de Componentes Externos (`CMakeLists.txt` Raiz)
- **Localização**: Como a pasta `ui/` se encontra na raiz do projeto (fora do diretório padrão `components/`), sempre declarar:
  ```cmake
  list(APPEND EXTRA_COMPONENT_DIRS "ui")
  ```
  **antes** da linha `include($ENV{IDF_PATH}/tools/cmake/project.cmake)`.

### 2. Registro do Componente UI (`ui/CMakeLists.txt`)
- **Busca Recursiva**: Garantir busca de todos os fontes `.c` via `file(GLOB_RECURSE UI_SOURCES "${CMAKE_CURRENT_LIST_DIR}/*.c")`.
- **Diretórios de Inclusão (`INCLUDE_DIRS`)**: Incluir `.`, `..`, `components`, `fonts`, `images`, `screens`, `widgets`.
- **Macros Públicas**: Registrar a macro de inclusão direta do LVGL em nível público:
  ```cmake
  target_compile_definitions(${COMPONENT_LIB} PUBLIC LV_LVGL_H_INCLUDE_SIMPLE)
  ```

### 3. Resolução de Dependências Modulares (`main/CMakeLists.txt`)
- Garantir a declaração explícita dos módulos no `REQUIRES` do `idf_component_register`:
  - `ui`
  - `lvgl`
  - `driver`
  - `esp_driver_spi` (necessário para o driver ST7735 em ESP-IDF v6)
  - `esp_driver_gpio` (necessário para controle dos pinos GPIO em ESP-IDF v6)
  - `esp_timer`
  - `freertos`
  - `heap`

### 4. Tabela de Partições e Capacidade de Memória (`sdkconfig.defaults`)
- Para evitar estouro de memória Flash durante o link da UI LVGL v9 (que excede 1 MB), garantir a opção:
  ```ini
  CONFIG_PARTITION_TABLE_SINGLE_APP_LARGE=y
  ```

### 5. Documentação em Código
- Todos os arquivos `CMakeLists.txt` modificados ou mantidos devem conter comentários explicativos em **Português (Brasil)**.
