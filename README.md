# Waveshare ESP32-S3-Touch-LCD-7 — Arduino проект полива

Этот репозиторий содержит модульный проект под **Arduino framework** для платы
**Waveshare ESP32-S3-Touch-LCD-7 (800x480 RGB, GT911, RS-485)**.

## 1) Что нужно установить

### Arduino IDE
- Arduino IDE 2.x (рекомендуется актуальная стабильная версия).

### Пакет плат ESP32
1. Откройте **File → Preferences**.
2. В поле **Additional Boards Manager URLs** добавьте:
   - `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
3. Откройте **Tools → Board → Boards Manager**.
4. Найдите **esp32 by Espressif Systems** и установите версию **2.0.14+**.

### Библиотеки
Через **Library Manager** установите:
- **lvgl** (ветка v8.x)

> `esp_lcd` и большая часть нужного HAL уже идут внутри ESP32 Arduino core.

---

## 2) Подготовка проекта

1. Откройте папку проекта в Arduino IDE.
2. Убедитесь, что основной файл — `src/main.cpp`.
3. Проверьте структуру каталогов `src/` (core/ui/comm/drivers).

---

## 3) Настройки платы в Arduino IDE

Откройте **Tools** и выставьте:

- **Board**: `ESP32S3 Dev Module` (или профиль Waveshare, если присутствует)
- **PSRAM**: `OPI PSRAM` / `Enabled`
- **Flash Size**: `8MB` (или штатное значение для вашей ревизии)
- **USB CDC On Boot**: по необходимости (обычно `Enabled` удобно для логов)
- **Upload Speed**: 460800 или 921600 (если кабель/ПК стабильны)

> Если прошивка нестабильна, уменьшите Upload Speed до 115200.

---

## 4) Настройка RS-485, Mock Mode и пинов

### Где настраивать
Откройте `src/main.cpp`, функция:
- `makeRs485Config()`

Там задаются:
- `cfg.uartNum` (по умолчанию 2)
- `cfg.txPin`, `cfg.rxPin`
- `cfg.dePin` (`-1`, если управление направлением делает железо)
- `cfg.baud`
- `cfg.mockMode`

### Mock Mode
- По умолчанию mock включен через состояние (`settings.mockMode = true` в NVS по умолчанию).
- Для реальной шины RS-485 поставьте `cfg.mockMode = false` и задайте реальные пины.

### Важно по RS-485
- Проверьте физический переключатель UART на плате (должен соответствовать UART, выбранному в коде).
- Если сеть длинная/шумная — при необходимости включите терминацию 120 Ом джампером.

---

## 5) Настройка дисплея и тачскрина

### RGB LCD
Файл: `src/drivers/display_driver.cpp`
- Используется `esp_lcd_rgb_panel` + flush в LVGL.
- Буферы LVGL выделяются в PSRAM (double buffering).
- Параметр строк буфера: `DisplayConfig::bufferLines` (по умолчанию 60).

Если изображение некорректно:
1. Проверьте `panel_config.data_gpio_nums[]`, `pclk_gpio_num`, `hsync/vsync/de`.
2. Проверьте тайминги `hsync_*`, `vsync_*`, `pclk_hz`.
3. Проверьте полярность `pclk_active_neg`.

### GT911
Файл: `src/drivers/touch_driver.cpp`
- I2C адрес сначала `0x5D`, fallback `0x14`.
- Убедитесь, что уровень I2C на плате выставлен в **3.3V**.

---

## 6) Загрузка прошивки в контроллер

1. Подключите плату по USB.
2. Выберите корректный **Port** в Arduino IDE.
3. Нажмите **Verify** (сборка).
4. Если сборка успешна — нажмите **Upload**.
5. Откройте **Serial Monitor** на 115200 бод.

Если не прошивается:
- Попробуйте другой USB-кабель (с поддержкой data).
- Нажмите/удерживайте BOOT при старте загрузки (зависит от ревизии).
- Снизьте Upload Speed.

---

## 7) Что должно произойти после старта

1. Инициализируется LVGL + дисплей.
2. Инициализируется GT911.
3. Стартуют FreeRTOS задачи:
   - `ui_task` (core 1)
   - `logic_task` (core 0)
   - `comm_task` (core 0)
4. Включаются правила безопасности:
   - насосы в OFF,
   - при `closeAllOnBoot=true` отправляются последовательные команды закрытия зон.

---

## 8) Базовая проверка функционала

### В Mock Mode
- Экран должен открываться, вкладки переключаться.
- В **Modules** кнопка Rescan должна давать mock-обнаружение модуля.
- В **Manual** можно нажимать зоны и `Stop All`.

### В реальном режиме RS-485
- Выключите mock.
- Проверьте TX/RX/DE пины и UART switch.
- Нажмите Rescan и проверьте обнаружение модулей.

---

## 9) Полезные места в коде

- Точка входа/таски: `src/main.cpp`
- Логика и безопасность: `src/core/logic.cpp`
- Состояние и NVS: `src/core/state.cpp`
- Протокол/CRC: `src/comm/protocol.cpp`
- RS-485 транспорт: `src/comm/rs485_transport.cpp`
- Драйвер RGB дисплея: `src/drivers/display_driver.cpp`
- Драйвер GT911: `src/drivers/touch_driver.cpp`
- UI экраны: `src/ui/screens/*`

---

## 10) Примечание

Проект является рабочим каркасом для платы Waveshare, но конкретные GPIO/тайминги RGB и
RS-485 wiring зависят от ревизии вашей платы и схемы подключения внешних модулей.
Сначала верифицируйте пины и только затем переходите к полевым тестам полива.
