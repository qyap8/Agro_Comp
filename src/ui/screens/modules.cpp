#include "modules.h"

namespace ui {

lv_obj_t* ModulesScreen::create(lv_obj_t* parent, core::EventBus* bus) {
  root_ = lv_obj_create(parent);
  bus_ = bus;
  lv_obj_set_size(root_, LV_PCT(100), LV_PCT(100));

  auto* btn = lv_btn_create(root_);
  lv_obj_set_size(btn, 160, 50);
  lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, -20, 20);
  lv_obj_add_event_cb(btn, rescanCb, LV_EVENT_CLICKED, this);
  auto* lbl = lv_label_create(btn);
  lv_label_set_text(lbl, "Rescan");
  lv_obj_center(lbl);

  list_ = lv_list_create(root_);
  lv_obj_set_size(list_, 760, 360);
  lv_obj_align(list_, LV_ALIGN_BOTTOM_MID, 0, -10);
  return root_;
}

void ModulesScreen::rescanCb(lv_event_t* e) {
  auto* self = static_cast<ModulesScreen*>(lv_event_get_user_data(e));
  core::Event ev;
  ev.type = core::EventType::DISCOVER;
  self->bus_->publish(ev);
}

void ModulesScreen::refresh(const core::SystemState& state) {
  lv_obj_clean(list_);
  for (const auto& m : state.modules) {
    char line[96];
    snprintf(line, sizeof(line), "Addr:%u UID:%02X%02X.. FW:%u %s", m.addr, m.uid[0], m.uid[1], m.fw,
             m.online ? "ON" : "OFF");
    lv_list_add_text(list_, line);
  }
}

}  // namespace ui
