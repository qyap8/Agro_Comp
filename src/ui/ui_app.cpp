#include "ui_app.h"

namespace ui {

void UiApp::begin(core::SystemState* state, core::EventBus* bus) {
  state_ = state;
  bus_ = bus;

  tabview_ = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 48);
  auto* tDash = lv_tabview_add_tab(tabview_, "Dashboard");
  auto* tManual = lv_tabview_add_tab(tabview_, "Manual");
  auto* tSchedules = lv_tabview_add_tab(tabview_, "Schedules");
  auto* tModules = lv_tabview_add_tab(tabview_, "Modules");
  auto* tSettings = lv_tabview_add_tab(tabview_, "Settings");
  auto* tDiag = lv_tabview_add_tab(tabview_, "Diagnostics");

  dashboard_.create(tDash);
  manual_.create(tManual, bus_);
  schedules_.create(tSchedules);
  modules_.create(tModules, bus_);
  settings_.create(tSettings, bus_);
  diagnostics_.create(tDiag, bus_);
}

void UiApp::refresh() {
  if (!state_ || !state_->mutex) return;
  xSemaphoreTake(state_->mutex, portMAX_DELAY);
  dashboard_.refresh(*state_);
  manual_.refresh(*state_);
  schedules_.refresh(*state_);
  modules_.refresh(*state_);
  settings_.refresh(*state_);
  diagnostics_.refresh(*state_);
  xSemaphoreGive(state_->mutex);
}

}  // namespace ui
