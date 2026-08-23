# 🎨 Documentação Técnica: Interface Gráfica LVGL v9

Esta documentação é mantida pelo **Agente de Documentação de Interface LVGL** (`lvgl-ui-doc-agent`).

---

## 📌 Visão Geral do Subsistema
A camada de interface visual (`ui/`) contém todos os elementos de interface gráfica exportados pelo editor visual LVGL (LVGL Editor / LVGL Flow). Ela é totalmente compatível com a API moderna do **LVGL v9**.

---

## 📂 Estrutura de Diretórios e Arquivos

```
ui/
├── components/          # Componentes visuais modulares reutilizáveis
│   ├── base/            # Widgets base (bar, column, container, image, label, panel, row)
│   ├── battery_body/    # Componente visual de indicador de bateria
│   └── infobar/         # Barra superior de status e informações
├── fonts/               # Fontes tipográficas convertidas para matrizes C
├── images/              # Matrizes de imagens C (ícones bluetooth, música, etc.)
├── screens/             # Telas principais da aplicação
│   ├── preview_home_gen # Tela principal de visualização da Home
│   └── screen1_gen      # Tela secundária configurada
├── ui_gen.h / ui_gen.c  # Código gerado automaticamente de ativos e observadores
└── ui.h / ui.c          # Ponto de entrada de inicialização da biblioteca UI
```

---

## 🔄 Padrão Observer & Tópicos (`lv_subject_t`)

O LVGL v9 introduz o padrão Observer para vínculo dinâmico de dados entre variáveis do sistema e widgets na tela. Os seguintes tópicos foram declarados em `ui_gen.h`:

| Tópico (`lv_subject_t`) | Descrição |
| :--- | :--- |
| `subject_battery_level` | Nível de carga percentual da bateria (0 a 100%) |
| `subject_consumo` | Medição de consumo elétrico ou corrente |
| `subject_home_artAtual` | Identificador da capa de álbum ou mídia em reprodução |

---

## 🚀 Ciclo de Vida e Carregamento de Telas

1. **Inicialização de Recursos**:
   ```c
   ui_init(NULL); // Inicializa fontes, estilos e tópicos globais
   ```

2. **Instanciação da Tela**:
   ```c
   lv_obj_t * screen = preview_home_create();
   ```

3. **Exibição Ativa**:
   ```c
   lv_screen_load(screen);
   ```
