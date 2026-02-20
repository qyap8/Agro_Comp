#include "ui_app.h"

namespace ui {

void UiApp::begin(core::SystemState* state, core::EventBus* bus) {
  state_ = state;
  bus_ = bus;

  lv_obj_t* tabs = lv_tabview_create(lv_scr_act());
#if LVGL_VERSION_MAJOR >= 9
  lv_tabview_set_tab_bar_position(tabs, LV_DIR_TOP);
  lv_tabview_set_tab_bar_size(tabs, 52);
#else
  lv_tabview_set_tab_pos(tabs, LV_DIR_TOP);
  lv_tabview_set_tab_size(tabs, 52);
#endif

  lv_obj_t* tDash = lv_tabview_add_tab(tabs, "Dashboard");
  lv_obj_t* tManual = lv_tabview_add_tab(tabs, "Manual");
  lv_obj_t* tSch = lv_tabview_add_tab(tabs, "Schedules");
  lv_obj_t* tMod = lv_tabview_add_tab(tabs, "Modules");
  lv_obj_t* tSet = lv_tabview_add_tab(tabs, "Settings");
  lv_obj_t* tDiag = lv_tabview_add_tab(tabs, "Diagnostics");

  screens::buildDashboard(tDash, bus_);
  screens::buildManual(tManual, bus_);
  screens::buildSchedules(tSch, bus_);
  screens::buildModules(tMod, bus_);
  screens::buildSettings(tSet, bus_);
  screens::buildDiagnostics(tDiag, bus_);
}

void UiApp::refresh() {
  if (!state_ || !state_->mutex) return;
  xSemaphoreTake(state_->mutex, portMAX_DELAY);
  screens::refreshDashboard(*state_);
  screens::refreshManual(*state_);
  screens::refreshSchedules(*state_);
  screens::refreshModules(*state_);
  screens::refreshSettings(*state_);
  screens::refreshDiagnostics(*state_);
  xSemaphoreGive(state_->mutex);
}

}  // namespace ui
