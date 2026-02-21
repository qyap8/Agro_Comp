# IrrigationHMI (Waveshare ESP32-S3-Touch-LCD-7, 800x480)

Полноценный Arduino IDE проект HMI для системы полива на базе:

- **Waveshare ESP32-S3-Touch-LCD-7**
- **ESP32_Display_Panel**
- **LVGL v8.4.x**
- **Arduino ESP32 core >= 3.1.0**

Проект разделён на слои:

- `ui/` — интерфейс (экраны и виджеты LVGL)
- `core/` — состояние системы, логика, очередь команд, event bus
- `comm/` — RS485 транспорт и протокол (CRC16 Modbus-like)

---

## 1) Важные предпосылки (обязательно)

1. В папке скетча должен быть файл `esp_panel_board_supported_conf.h`.
2. В нём должен быть включён `BOARD_WAVESHARE_ESP32_S3_TOUCH_LCD_7`.
3. Установлен `ESP32_Display_Panel`.
4. Установлен `lvgl` версии **8.4.x**.
5. В Arduino IDE выбран ESP32 core версии **3.1.0+**.
6. **PSRAM = Enabled**.

---

## 2) Структура проекта

```text
IrrigationHMI/
│
├── IrrigationHMI.ino
├── lvgl_port_compat.h
├── README.md
├── all_sources.cpp
├── ui/
│   ├── ui_app.h
│   ├── ui_app.cpp
│   ├── screens/
│       ├── dashboard.cpp
│       ├── manual.cpp
│       ├── schedules.cpp
│       ├── modules.cpp
│       ├── settings.cpp
│       ├── diagnostics.cpp
│
├── core/
│   ├── state.h
│   ├── state.cpp
│   ├── logic.h
│   ├── logic.cpp
│   ├── event_bus.h
│   ├── event_bus.cpp
│
├── comm/
│   ├── rs485_transport.h
│   ├── rs485_transport.cpp
│   ├── protocol.h
│   ├── protocol.cpp
│
└── config.h
```

---


## 2.1) Почему нужен `all_sources.cpp`

Arduino IDE иногда не линкует `.cpp` из вложенных подпапок скетча как отдельные единицы компиляции.
Из-за этого появляются ошибки `undefined reference` на функции из `core/`, `comm/`, `ui/`.

Файл `all_sources.cpp` явно подключает все `.cpp` проекта, чтобы линковка была стабильной в Arduino IDE.

---

## 3) Инициализация дисплея (как требуется)

В `setup()` используется строго такой путь:

```cpp
#include "esp_display_panel.hpp"
using namespace esp_panel::board;

Board *board = new Board();
board->init();
board->begin();
lvgl_port_init(board->getLCD(), board->getTouch());
```

В проекте это уже реализовано в `IrrigationHMI.ino`.

---

## 4) Почему добавлен `lvgl_port_compat.h`

На некоторых установках Arduino IDE функции `lvgl_port_init/lvgl_port_lock/lvgl_port_unlock`
не видны только через `esp_display_panel.hpp`.

Чтобы убрать ошибку вида:

- `'lvgl_port_init' was not declared in this scope`

добавлен файл `lvgl_port_compat.h`, который:

1. Пытается подключить подходящий заголовок LVGL порта (`lvgl_v8_port.h` или `esp_lvgl_port.h`).
2. В крайнем случае содержит `extern "C"` декларации функций.

Это решает проблему видимости символов в разных версиях библиотеки/ядра.

---

## 5) RS485 настройки

- UART: `HardwareSerial(2)`
- `TX = GPIO15`
- `RX = GPIO16`
- Скорость по умолчанию: `115200`
- Ручной DE не используется (предполагается автопереключение железом/платой)

CRC16 реализован в `comm/protocol.cpp` (полином `0xA001`).

---

## 6) Логика полива

- 16 зон
- 2 состояния насоса (`pumpRelay`, `pumpDc`)
- Очередь клапанов **последовательная** (никаких одновременных импульсов)
- На старте:
  - насосы выключаются
  - если `closeAllOnBoot == true` — очередь закрытия всех 16 зон

---

## 7) Экраны UI

1. **Dashboard**: статус, активные зоны, насосы, переходы
2. **Manual**: сетка 4x4 зон, Stop All, Pump Test
3. **Schedules**: список расписаний + Add/Edit (через event flow)
4. **Modules**: Rescan + список модулей
5. **Settings**: Pulse width, CloseAllOnBoot, Brightness (stub), Language (stub)
6. **Diagnostics**: event log (200), RS485 stats

---

## 8) Потоки FreeRTOS

Создаются только:

- `logic_task`
- `comm_task`

Отдельный LVGL task вручную **не создаётся**.

---

## 9) Как открыть в Arduino IDE

1. Скопируйте папку `IrrigationHMI` в папку Arduino sketches.
2. Откройте `IrrigationHMI/IrrigationHMI.ino`.
3. Выберите плату ESP32-S3 (под вашу конфигурацию Waveshare).
4. Проверьте PSRAM = Enabled.
5. Проверьте наличие `esp_panel_board_supported_conf.h` и макроса `BOARD_WAVESHARE_ESP32_S3_TOUCH_LCD_7`.
6. Нажмите **Verify/Upload**.

---

## 10) Что проверить, если снова ошибка по `lvgl_port_*`

1. Обновите `ESP32_Display_Panel` до актуальной версии.
2. Убедитесь, что в установке присутствует `lvgl_v8_port.h` или `esp_lvgl_port.h`.
3. Проверьте, что `lvgl` именно ветки v8.
4. Перезапустите Arduino IDE после установки/обновления библиотек.

---

## 11) Примечание

Проект является рабочим каркасом HMI/логики/связи, который легко расширять:

- реальная логика Modbus-команд
- полноценные диалоги редактирования расписаний
- реальное сканирование и адресация модулей
- локализация UI

