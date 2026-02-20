#pragma once

#include <Arduino.h>
#include <lvgl.h>

#include "../core/event_bus.h"
#include "../core/state.h"

namespace ui {

namespace screens {
void buildDashboard(lv_obj_t* parent, core::EventBus* bus);
void refreshDashboard(const core::SystemState& state);

void buildManual(lv_obj_t* parent, core::EventBus* bus);
void refreshManual(const core::SystemState& state);

void buildSchedules(lv_obj_t* parent, core::EventBus* bus);
void refreshSchedules(const core::SystemState& state);

void buildModules(lv_obj_t* parent, core::EventBus* bus);
void refreshModules(const core::SystemState& state);

void buildSettings(lv_obj_t* parent, core::EventBus* bus);
void refreshSettings(const core::SystemState& state);

void buildDiagnostics(lv_obj_t* parent, core::EventBus* bus);
void refreshDiagnostics(const core::SystemState& state);
}  // namespace screens

class UiApp {
 public:
  void begin(core::SystemState* state, core::EventBus* bus);
  void refresh();

 private:
  core::SystemState* state_ = nullptr;
  core::EventBus* bus_ = nullptr;
};

}  // namespace ui
