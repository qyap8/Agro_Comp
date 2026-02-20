#include "manual.h"

namespace ui {

lv_obj_t* ManualScreen::create(lv_obj_t* parent, core::EventBus* bus) {
  root_ = lv_obj_create(parent);
  bus_ = bus;
  lv_obj_set_size(root_, LV_PCT(100), LV_PCT(100));

  lv_obj_t* grid = lv_obj_create(root_);
  lv_obj_set_size(grid, 760, 360);
  lv_obj_align(grid, LV_ALIGN_TOP_MID, 0, 10);
  lv_obj_set_layout(grid, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);

  for (uint8_t i = 0; i < core::kMaxZones; ++i) {
    zoneBtns_[i] = lv_btn_create(grid);
    lv_obj_set_size(zoneBtns_[i], 170, 70);
    lv_obj_add_event_cb(zoneBtns_[i], zoneBtnCb, LV_EVENT_CLICKED, this);
    auto* lbl = lv_label_create(zoneBtns_[i]);
    lv_label_set_text_fmt(lbl, "Zone %u", i + 1);
    lv_obj_center(lbl);
  }

  lv_obj_t* stop = lv_btn_create(root_);
  lv_obj_set_size(stop, 220, 60);
  lv_obj_align(stop, LV_ALIGN_BOTTOM_LEFT, 20, -20);
  lv_obj_add_event_cb(stop, stopAllCb, LV_EVENT_CLICKED, this);
  auto* stopLbl = lv_label_create(stop);
  lv_label_set_text(stopLbl, "Stop All");
  lv_obj_center(stopLbl);

  return root_;
}

void ManualScreen::zoneBtnCb(lv_event_t* e) {
  auto* self = static_cast<ManualScreen*>(lv_event_get_user_data(e));
  auto* obj = lv_event_get_target(e);
  uint8_t zone = 0;
  for (uint8_t i = 0; i < core::kMaxZones; ++i) {
    if (self->zoneBtns_[i] == obj) {
      zone = i + 1;
      break;
    }
  }
  if (zone == 0) return;

  core::Event ev;
  ev.type = core::EventType::ZONE_SET;
  ev.a = zone;
  ev.b = lv_obj_has_state(obj, LV_STATE_CHECKED) ? 0 : 1;
  self->bus_->publish(ev);
}

void ManualScreen::stopAllCb(lv_event_t* e) {
  auto* self = static_cast<ManualScreen*>(lv_event_get_user_data(e));
  core::Event ev;
  ev.type = core::EventType::STOP_ALL;
  self->bus_->publish(ev);
}

void ManualScreen::refresh(const core::SystemState& state) {
  for (uint8_t i = 0; i < core::kMaxZones; ++i) {
    if (state.zones[i].isOpen) {
      lv_obj_add_state(zoneBtns_[i], LV_STATE_CHECKED);
    } else {
      lv_obj_clear_state(zoneBtns_[i], LV_STATE_CHECKED);
    }
  }
}

}  // namespace ui
