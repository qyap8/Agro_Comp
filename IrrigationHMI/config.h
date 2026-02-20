#pragma once

#include <Arduino.h>

// ================= Board / Feature Config =================
#define APP_NAME "IrrigationHMI"

// Display geometry
static constexpr int LCD_HRES = 800;
static constexpr int LCD_VRES = 480;
static constexpr int LVGL_BUF_LINES = 60;  // chunk height for double draw buffers

// Touch I2C (GT911)
static constexpr int TOUCH_I2C_SDA = 8;   // TODO: verify for your board revision
static constexpr int TOUCH_I2C_SCL = 9;   // TODO: verify for your board revision
static constexpr uint32_t TOUCH_I2C_FREQ = 400000;

// Backlight
static constexpr int LCD_BL_PIN = 2;      // TODO: verify for your board revision
static constexpr bool LCD_BL_ACTIVE_HIGH = true;

// RS485 default UART settings
static constexpr uint8_t RS485_UART_NUM = 2;
static constexpr int RS485_TX_PIN = 43;   // TODO: verify with board UART switch setting
static constexpr int RS485_RX_PIN = 44;   // TODO: verify with board UART switch setting
static constexpr int RS485_DE_PIN = -1;   // -1: auto-direction transceiver path
static constexpr uint32_t RS485_BAUD = 115200;

// App behavior
static constexpr bool MOCK_MODE_DEFAULT = true;  // false for real RS485 bus
static constexpr uint16_t PULSE_WIDTH_DEFAULT_MS = 200;
static constexpr bool CLOSE_ALL_ON_BOOT_DEFAULT = true;

// Task settings
static constexpr uint16_t UI_REFRESH_MS = 200;
static constexpr uint16_t LOGIC_TICK_MS = 20;
static constexpr uint16_t COMM_TICK_MS = 10;

