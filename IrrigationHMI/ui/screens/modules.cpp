#include "../ui_app.h"

namespace ui::screens {

static lv_obj_t* sList = nullptr;
static core::EventBus* sBus = nullptr;

static void rescanCb(lv_event_t* e) {
  (void)e;
  core::Event ev;
  ev.type = core::EventType::DISCOVER;
  if (sBus) sBus->publish(ev);
}

void buildModules(lv_obj_t* parent, core::EventBus* bus) {
  sBus = bus;

  lv_obj_t* rescan = lv_btn_create(parent);
  lv_obj_set_size(rescan, 180, 56);
  lv_obj_align(rescan, LV_ALIGN_TOP_RIGHT, -16, 8);
  lv_obj_add_event_cb(rescan, rescanCb, LV_EVENT_CLICKED, nullptr);
  lv_obj_t* lr = lv_label_create(rescan);
  lv_label_set_text(lr, "Rescan");
  lv_obj_center(lr);

  sList = lv_list_create(parent);
  lv_obj_set_size(sList, 780, 360);
  lv_obj_align(sList, LV_ALIGN_BOTTOM_MID, 0, -8);
}

void refreshModules(const core::SystemState& state) {
  if (!sList) return;
  lv_obj_clean(sList);
  for (const auto& m : state.modules) {
    char line[140];
    snprintf(line, sizeof(line), "UID:%02X%02X%02X%02X%02X%02X A:%u FW:%u Last:%lus %s", m.uid[0], m.uid[1],
             m.uid[2], m.uid[3], m.uid[4], m.uid[5], m.addr, m.fwVer, m.lastSeenMs / 1000UL,
             m.online ? "ON" : "OFF");
    lv_list_add_text(sList, line);
  }
}

}  // namespace ui::screens
