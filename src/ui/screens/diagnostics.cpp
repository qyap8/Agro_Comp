#include "diagnostics.h"

namespace ui {

lv_obj_t* DiagnosticsScreen::create(lv_obj_t* parent, core::EventBus* bus) {
  root_ = lv_obj_create(parent);
  bus_ = bus;
  lv_obj_set_size(root_, LV_PCT(100), LV_PCT(100));

  counters_ = lv_label_create(root_);
  lv_obj_align(counters_, LV_ALIGN_TOP_LEFT, 20, 20);

  auto* clear = lv_btn_create(root_);
  lv_obj_set_size(clear, 140, 50);
  lv_obj_align(clear, LV_ALIGN_TOP_RIGHT, -20, 20);
  lv_obj_add_event_cb(clear, clearCb, LV_EVENT_CLICKED, this);
  auto* clearLbl = lv_label_create(clear);
  lv_label_set_text(clearLbl, "Clear Log");
  lv_obj_center(clearLbl);

  logList_ = lv_list_create(root_);
  lv_obj_set_size(logList_, 760, 360);
  lv_obj_align(logList_, LV_ALIGN_BOTTOM_MID, 0, -10);

  return root_;
}

void DiagnosticsScreen::clearCb(lv_event_t* e) {
  auto* self = static_cast<DiagnosticsScreen*>(lv_event_get_user_data(e));
  core::Event ev;
  ev.type = core::EventType::LOG_CLEAR;
  self->bus_->publish(ev);
}

void DiagnosticsScreen::refresh(const core::SystemState& state) {
  lv_label_set_text_fmt(counters_, "OK:%lu CRC:%lu TO:%lu", state.commStats.ok, state.commStats.crcErr,
                        state.commStats.timeouts);
  lv_obj_clean(logList_);
  for (const auto& item : state.eventLog) {
    lv_list_add_text(logList_, (String(item.timestampMs) + " " + item.text).c_str());
  }
}

}  // namespace ui
