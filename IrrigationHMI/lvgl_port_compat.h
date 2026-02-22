#pragma once

// Совместимый слой подключения LVGL Port для разных версий ESP32_Display_Panel.
// В некоторых установках функции lvgl_port_* не подтягиваются через esp_display_panel.hpp автоматически.

#if __has_include(<lvgl_v8_port.h>)
#include <lvgl_v8_port.h>
#elif __has_include("lvgl_v8_port.h")
#include "lvgl_v8_port.h"
#elif __has_include(<esp_lvgl_port.h>)
#include <esp_lvgl_port.h>
#elif __has_include("esp_lvgl_port.h")
#include "esp_lvgl_port.h"
#else
// Fallback-декларации: позволяют скомпилировать скетч, если заголовок порта лежит
// в другой папке, но символы доступны на этапе линковки через библиотеку.
extern "C" {
bool lvgl_port_init(void *lcd, void *touch);
bool lvgl_port_lock(int timeout_ms);
void lvgl_port_unlock(void);
}
#endif

