#include "../ui_app.h"

namespace ui::screens {

static lv_obj_t* g_manualBtns[core::kMaxZones] = {nullptr};
static core::EventBus* g_manualBus = nullptr;

static void manualZoneCb(lv_event_t* e) {
  uint32_t idx = (uint32_t)(uintptr_t)lv_event_get_user_data(e);
  core::Event ev;
  ev.type = core::EventType::ZONE_SET;
  ev.a = (uint8_t)(idx + 1);
  ev.b = lv_obj_has_state(g_manualBtns[idx], LV_STATE_CHECKED) ? 0 : 1;
  if (g_manualBus) g_manualBus->publish(ev);
}

static void manualStopAllCb(lv_event_t* e) {
  (void)e;
  core::Event ev;
  ev.type = core::EventType::STOP_ALL;
  if (g_manualBus) g_manualBus->publish(ev);
}

void buildManual(lv_obj_t* parent, core::EventBus* bus) {
  g_manualBus = bus;

  lv_obj_t* cont = lv_obj_create(parent);
  lv_obj_set_size(cont, 780, 390);
  lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 8);
  lv_obj_set_layout(cont, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);

  for (uint8_t i = 0; i < core::kMaxZones; ++i) {
    g_manualBtns[i] = lv_btn_create(cont);
    lv_obj_set_size(g_manualBtns[i], 180, 76);
    lv_obj_add_flag(g_manualBtns[i], LV_OBJ_FLAG_CHECKABLE);
    lv_obj_add_event_cb(g_manualBtns[i], manualZoneCb, LV_EVENT_CLICKED, (void*)(uintptr_t)i);

    lv_obj_t* l = lv_label_create(g_manualBtns[i]);
    lv_label_set_text_fmt(l, "Zone %u\nM%u", i + 1, (i / 4) + 1);
    lv_obj_center(l);
  }

  lv_obj_t* stop = lv_btn_create(parent);
  lv_obj_set_size(stop, 220, 60);
  lv_obj_align(stop, LV_ALIGN_BOTTOM_LEFT, 16, -12);
  lv_obj_add_event_cb(stop, manualStopAllCb, LV_EVENT_CLICKED, nullptr);
  lv_obj_t* ls = lv_label_create(stop);
  lv_label_set_text(ls, "Stop All");
  lv_obj_center(ls);

  lv_obj_t* pumpTest = lv_btn_create(parent);
  lv_obj_set_size(pumpTest, 220, 60);
  lv_obj_align(pumpTest, LV_ALIGN_BOTTOM_RIGHT, -16, -12);
  lv_obj_t* lp = lv_label_create(pumpTest);
  lv_label_set_text(lp, "Pump Test (stub)");
  lv_obj_center(lp);
}

void refreshManual(const core::SystemState& state) {
  for (uint8_t i = 0; i < core::kMaxZones; ++i) {
    if (!g_manualBtns[i]) continue;
    if (state.zones[i].isOpen)
      lv_obj_add_state(g_manualBtns[i], LV_STATE_CHECKED);
    else
      lv_obj_clear_state(g_manualBtns[i], LV_STATE_CHECKED);
  }
}

}  // namespace ui::screens
