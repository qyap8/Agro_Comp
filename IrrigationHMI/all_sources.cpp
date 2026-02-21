// Этот файл нужен для Arduino IDE: в некоторых конфигурациях IDE не компилирует
// .cpp из вложенных подпапок скетча автоматически.
// Подключаем все реализации явно, чтобы избежать undefined reference на этапе линковки.

#include "core/state.cpp"
#include "core/event_bus.cpp"
#include "core/logic.cpp"

#include "comm/protocol.cpp"
#include "comm/rs485_transport.cpp"

#include "ui/ui_app.cpp"
#include "ui/screens/dashboard.cpp"
#include "ui/screens/manual.cpp"
#include "ui/screens/schedules.cpp"
#include "ui/screens/modules.cpp"
#include "ui/screens/settings.cpp"
#include "ui/screens/diagnostics.cpp"
