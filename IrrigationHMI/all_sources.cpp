// Этот файл нужен для Arduino IDE: принудительно собираем реализации из подпапок.

#include "core/state.cpp"
#include "core/event_bus.cpp"
#include "core/logic.cpp"

#include "comm/protocol.cpp"
#include "comm/rs485_transport.cpp"

#include "ui/ui_app.cpp"
#include "ui/screens/dashboard.cpp"
#include "ui/screens/manual.cpp"
#include "ui/screens/settings.cpp"
