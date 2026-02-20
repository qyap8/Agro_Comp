#include "settings.h"

namespace ui {

lv_obj_t* SettingsScreen::create(lv_obj_t* parent, core::EventBus* bus) {
  root_ = lv_obj_create(parent);
  bus_ = bus;
  lv_obj_set_size(root_, LV_PCT(100), LV_PCT(100));

  pulseLabel_ = lv_label_create(root_);
  lv_obj_align(pulseLabel_, LV_ALIGN_TOP_LEFT, 20, 20);

  pulseSlider_ = lv_slider_create(root_);
  lv_obj_set_width(pulseSlider_, 500);
  lv_obj_align(pulseSlider_, LV_ALIGN_TOP_LEFT, 20, 60);
  lv_slider_set_range(pulseSlider_, 150, 300);
  lv_obj_add_event_cb(pulseSlider_, pulseSliderCb, LV_EVENT_VALUE_CHANGED, this);

  return root_;
}

void SettingsScreen::pulseSliderCb(lv_event_t* e) {
  auto* self = static_cast<SettingsScreen*>(lv_event_get_user_data(e));
  auto* slider = (lv_obj_t*)lv_event_get_target(e);
  core::Event ev;
  ev.type = core::EventType::SETTINGS_CHANGED;
  ev.value = lv_slider_get_value(slider);
  self->bus_->publish(ev);
}

void SettingsScreen::refresh(const core::SystemState& state) {
  lv_slider_set_value(pulseSlider_, state.settings.pulseWidthMs, LV_ANIM_OFF);
  lv_label_set_text_fmt(pulseLabel_, "Pulse Width: %ums", state.settings.pulseWidthMs);
}

}  // namespace ui
