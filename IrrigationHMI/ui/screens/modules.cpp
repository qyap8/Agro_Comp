#include "../ui_app.h"

namespace ui::screens {

static lv_obj_t* g_modulesList = nullptr;
static core::EventBus* g_modulesBus = nullptr;

static void modulesRescanCb(lv_event_t* e) {
  (void)e;
  core::Event ev;
  ev.type = core::EventType::DISCOVER;
  if (g_modulesBus) g_modulesBus->publish(ev);
}

void buildModules(lv_obj_t* parent, core::EventBus* bus) {
  g_modulesBus = bus;

  lv_obj_t* rescan = lv_btn_create(parent);
  lv_obj_set_size(rescan, 180, 56);
  lv_obj_align(rescan, LV_ALIGN_TOP_RIGHT, -16, 8);
  lv_obj_add_event_cb(rescan, modulesRescanCb, LV_EVENT_CLICKED, nullptr);
  lv_obj_t* lr = lv_label_create(rescan);
  lv_label_set_text(lr, "Rescan");
  lv_obj_center(lr);

  g_modulesList = lv_list_create(parent);
  lv_obj_set_size(g_modulesList, 780, 360);
  lv_obj_align(g_modulesList, LV_ALIGN_BOTTOM_MID, 0, -8);
}

void refreshModules(const core::SystemState& state) {
  if (!g_modulesList) return;
  lv_obj_clean(g_modulesList);
  for (const auto& m : state.modules) {
    char line[140];
    snprintf(line, sizeof(line), "UID:%02X%02X%02X%02X%02X%02X A:%u FW:%u Last:%lus %s", m.uid[0], m.uid[1],
             m.uid[2], m.uid[3], m.uid[4], m.uid[5], m.addr, m.fwVer, m.lastSeenMs / 1000UL,
             m.online ? "ON" : "OFF");
    lv_list_add_text(g_modulesList, line);
  }
}

}  // namespace ui::screens
