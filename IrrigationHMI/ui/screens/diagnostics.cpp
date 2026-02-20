#include "../ui_app.h"

namespace ui::screens {

static lv_obj_t* sCounters = nullptr;
static lv_obj_t* sList = nullptr;
static core::EventBus* sBus = nullptr;

static void clearLogCb(lv_event_t* e) {
  (void)e;
  core::Event ev;
  ev.type = core::EventType::LOG_CLEAR;
  if (sBus) sBus->publish(ev);
}

void buildDiagnostics(lv_obj_t* parent, core::EventBus* bus) {
  sBus = bus;

  sCounters = lv_label_create(parent);
  lv_obj_align(sCounters, LV_ALIGN_TOP_LEFT, 16, 12);

  lv_obj_t* clear = lv_btn_create(parent);
  lv_obj_set_size(clear, 150, 54);
  lv_obj_align(clear, LV_ALIGN_TOP_RIGHT, -16, 8);
  lv_obj_add_event_cb(clear, clearLogCb, LV_EVENT_CLICKED, nullptr);
  lv_obj_t* lc = lv_label_create(clear);
  lv_label_set_text(lc, "Clear log");
  lv_obj_center(lc);

  sList = lv_list_create(parent);
  lv_obj_set_size(sList, 780, 360);
  lv_obj_align(sList, LV_ALIGN_BOTTOM_MID, 0, -8);
}

void refreshDiagnostics(const core::SystemState& state) {
  if (!sCounters || !sList) return;
  lv_label_set_text_fmt(sCounters, "COMM ok:%lu crc:%lu to:%lu", state.comm.ok, state.comm.crcErr,
                        state.comm.timeouts);

  lv_obj_clean(sList);
  for (const auto& item : state.log) {
    lv_list_add_text(sList, (String(item.tsMs) + " " + item.text).c_str());
  }
}

}  // namespace ui::screens
