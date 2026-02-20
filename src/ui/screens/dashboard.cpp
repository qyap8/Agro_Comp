#include "dashboard.h"

namespace ui {

lv_obj_t* DashboardScreen::create(lv_obj_t* parent) {
  root_ = lv_obj_create(parent);
  lv_obj_set_size(root_, LV_PCT(100), LV_PCT(100));

  statusLabel_ = lv_label_create(root_);
  lv_obj_align(statusLabel_, LV_ALIGN_TOP_LEFT, 20, 20);

  zonesLabel_ = lv_label_create(root_);
  lv_obj_align(zonesLabel_, LV_ALIGN_TOP_LEFT, 20, 80);

  pumpLabel_ = lv_label_create(root_);
  lv_obj_align(pumpLabel_, LV_ALIGN_TOP_LEFT, 20, 140);

  footerLabel_ = lv_label_create(root_);
  lv_obj_align(footerLabel_, LV_ALIGN_BOTTOM_LEFT, 20, -20);

  return root_;
}

void DashboardScreen::refresh(const core::SystemState& state) {
  lv_label_set_text_fmt(statusLabel_, "Status: %s", state.runStatus.c_str());

  String active = "Active Zones: ";
  int count = 0;
  for (const auto& z : state.zones) {
    if (z.isOpen) {
      active += String(z.id) + " ";
      if (++count >= 4) break;
    }
  }
  if (count == 0) active += "none";

  lv_label_set_text(zonesLabel_, active.c_str());
  lv_label_set_text_fmt(pumpLabel_, "Pump Relay:%s  Pump DC:%s", state.pumpRelayOn ? "ON" : "OFF",
                        state.pumpDcOn ? "ON" : "OFF");
  lv_label_set_text_fmt(footerLabel_, "t=%lus RS485 ok:%lu", millis() / 1000, state.commStats.ok);
}

}  // namespace ui
