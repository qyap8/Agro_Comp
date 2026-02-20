#include "../ui_app.h"

namespace ui::screens {

static lv_obj_t* sPulseSlider = nullptr;
static lv_obj_t* sPulseLabel = nullptr;
static core::EventBus* sBus = nullptr;

static void pulseCb(lv_event_t* e) {
  auto* slider = (lv_obj_t*)lv_event_get_target(e);
  core::Event ev;
  ev.type = core::EventType::SETTINGS_PULSE_WIDTH;
  ev.v = (uint16_t)lv_slider_get_value(slider);
  if (sBus) sBus->publish(ev);
}

void buildSettings(lv_obj_t* parent, core::EventBus* bus) {
  sBus = bus;

  sPulseLabel = lv_label_create(parent);
  lv_obj_align(sPulseLabel, LV_ALIGN_TOP_LEFT, 16, 16);

  sPulseSlider = lv_slider_create(parent);
  lv_slider_set_range(sPulseSlider, 150, 300);
  lv_obj_set_width(sPulseSlider, 520);
  lv_obj_align(sPulseSlider, LV_ALIGN_TOP_LEFT, 16, 48);
  lv_obj_add_event_cb(sPulseSlider, pulseCb, LV_EVENT_VALUE_CHANGED, nullptr);

  lv_obj_t* lang = lv_label_create(parent);
  lv_label_set_text(lang, "Language RU/EN: stub");
  lv_obj_align(lang, LV_ALIGN_TOP_LEFT, 16, 100);

  lv_obj_t* bright = lv_label_create(parent);
  lv_label_set_text(bright, "Brightness slider: stub (BL PWM TODO)");
  lv_obj_align(bright, LV_ALIGN_TOP_LEFT, 16, 132);
}

void refreshSettings(const core::SystemState& state) {
  if (!sPulseSlider) return;
  lv_slider_set_value(sPulseSlider, state.settings.pulseWidthMs, LV_ANIM_OFF);
  lv_label_set_text_fmt(sPulseLabel, "Pulse width: %u ms", state.settings.pulseWidthMs);
}

}  // namespace ui::screens
