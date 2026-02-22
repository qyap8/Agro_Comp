# IrrigationHMI (refactored architecture)

## Что изменено архитектурно

HMI больше **не управляет физическими выходами напрямую** (клапаны/насосы).
Теперь HMI выполняет роль:
- экран + UX,
- RS-485 master,
- Wi‑Fi manager,
- встроенный web server,
- discovery модулей и динамический UI каналов.

Команда управления только одна по смыслу: `SET_CHANNEL_STATE(moduleAddr, channelId, state)`.

## Навигация

Главные вкладки теперь только 3 (нижние):
- Home
- Zones
- Settings

## Dynamic Zones

Экран Zones строится динамически по инвентарю модулей:
- без hardcode 16 зон,
- каналы сгруппированы по модулю,
- при исчезновении модуля карточка удаляется.

## Wi‑Fi manager

- ввод SSID/Password с тачскрина (LVGL textarea + keyboard),
- сохранение в Preferences,
- автоподключение на старте,
- fallback AP: `Irrigation-HMI-Setup`.

## REST API

- `GET /api/state`
- `POST /api/channel`
- `POST /api/rescan`
- `POST /api/wifi`

Также есть простая встроенная web-страница из PROGMEM.

## Localization

Добавлен лёгкий слой локализации через `tr(KEY)`:
- English
- Español
- Русский
- Հայերեն

## Важно

- Инициализация дисплея/LVGL не изменялась: `Board->init(); Board->begin(); lvgl_port_init(...)`
- FreeRTOS задачи сохранены: `logic_task`, `comm_task`
- RS-485 слой сохранён
