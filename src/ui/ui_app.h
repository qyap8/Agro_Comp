#pragma once

#include <lvgl.h>

#include "core/event_bus.h"
#include "core/state.h"
#include "ui/screens/dashboard.h"
#include "ui/screens/diagnostics.h"
#include "ui/screens/manual.h"
#include "ui/screens/modules.h"
#include "ui/screens/schedules.h"
#include "ui/screens/settings.h"

namespace ui {

class UiApp {
 public:
  void begin(core::SystemState* state, core::EventBus* bus);
  void refresh();

 private:
  core::SystemState* state_ = nullptr;
  core::EventBus* bus_ = nullptr;

  lv_obj_t* tabview_ = nullptr;
  DashboardScreen dashboard_;
  ManualScreen manual_;
  SchedulesScreen schedules_;
  ModulesScreen modules_;
  SettingsScreen settings_;
  DiagnosticsScreen diagnostics_;
};

}  // namespace ui
