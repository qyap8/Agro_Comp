#include "../ui_app.h"

namespace ui::screens {

static lv_obj_t* g_setPulseSlider = nullptr;
static lv_obj_t* g_setPulseLabel = nullptr;
static core::EventBus* g_setBus = nullptr;

static void settingsPulseCb(lv_event_t* e) {
  auto* slider = (lv_obj_t*)lv_event_get_target(e);
  core::Event ev;
  ev.type = core::EventType::SETTINGS_PULSE_WIDTH;
  ev.v = (uint16_t)lv_slider_get_value(slider);
  if (g_setBus) g_setBus->publish(ev);
}

void buildSettings(lv_obj_t* parent, core::EventBus* bus) {
  g_setBus = bus;

  g_setPulseLabel = lv_label_create(parent);
  lv_obj_align(g_setPulseLabel, LV_ALIGN_TOP_LEFT, 16, 16);

  g_setPulseSlider = lv_slider_create(parent);
  lv_slider_set_range(g_setPulseSlider, 150, 300);
  lv_obj_set_width(g_setPulseSlider, 520);
  lv_obj_align(g_setPulseSlider, LV_ALIGN_TOP_LEFT, 16, 48);
  lv_obj_add_event_cb(g_setPulseSlider, settingsPulseCb, LV_EVENT_VALUE_CHANGED, nullptr);

  lv_obj_t* lang = lv_label_create(parent);
  lv_label_set_text(lang, "Language RU/EN: stub");
  lv_obj_align(lang, LV_ALIGN_TOP_LEFT, 16, 100);

  lv_obj_t* bright = lv_label_create(parent);
  lv_label_set_text(bright, "Brightness slider: stub (BL PWM TODO)");
  lv_obj_align(bright, LV_ALIGN_TOP_LEFT, 16, 132);
}

void refreshSettings(const core::SystemState& state) {
  if (!g_setPulseSlider) return;
  lv_slider_set_value(g_setPulseSlider, state.settings.pulseWidthMs, LV_ANIM_OFF);
  lv_label_set_text_fmt(g_setPulseLabel, "Pulse width: %u ms", state.settings.pulseWidthMs);
}

}  // namespace ui::screens
