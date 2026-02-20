#include "../ui_app.h"

namespace ui::screens {

static lv_obj_t* g_diagCounters = nullptr;
static lv_obj_t* g_diagList = nullptr;
static core::EventBus* g_diagBus = nullptr;

static void diagnosticsClearLogCb(lv_event_t* e) {
  (void)e;
  core::Event ev;
  ev.type = core::EventType::LOG_CLEAR;
  if (g_diagBus) g_diagBus->publish(ev);
}

void buildDiagnostics(lv_obj_t* parent, core::EventBus* bus) {
  g_diagBus = bus;

  g_diagCounters = lv_label_create(parent);
  lv_obj_align(g_diagCounters, LV_ALIGN_TOP_LEFT, 16, 12);

  lv_obj_t* clear = lv_btn_create(parent);
  lv_obj_set_size(clear, 150, 54);
  lv_obj_align(clear, LV_ALIGN_TOP_RIGHT, -16, 8);
  lv_obj_add_event_cb(clear, diagnosticsClearLogCb, LV_EVENT_CLICKED, nullptr);
  lv_obj_t* lc = lv_label_create(clear);
  lv_label_set_text(lc, "Clear log");
  lv_obj_center(lc);

  g_diagList = lv_list_create(parent);
  lv_obj_set_size(g_diagList, 780, 360);
  lv_obj_align(g_diagList, LV_ALIGN_BOTTOM_MID, 0, -8);
}

void refreshDiagnostics(const core::SystemState& state) {
  if (!g_diagCounters || !g_diagList) return;
  lv_label_set_text_fmt(g_diagCounters, "COMM ok:%lu crc:%lu to:%lu", state.comm.ok, state.comm.crcErr,
                        state.comm.timeouts);

  lv_obj_clean(g_diagList);
  for (const auto& item : state.log) {
    lv_list_add_text(g_diagList, (String(item.tsMs) + " " + item.text).c_str());
  }
}

}  // namespace ui::screens
