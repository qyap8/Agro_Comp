#include "../ui_app.h"

namespace ui::screens {

static lv_obj_t* sStatus = nullptr;
static lv_obj_t* sZones = nullptr;
static lv_obj_t* sPump = nullptr;
static lv_obj_t* sFooter = nullptr;

void buildDashboard(lv_obj_t* parent, core::EventBus* bus) {
  (void)bus;
  sStatus = lv_label_create(parent);
  lv_obj_align(sStatus, LV_ALIGN_TOP_LEFT, 16, 16);

  sZones = lv_label_create(parent);
  lv_obj_align(sZones, LV_ALIGN_TOP_LEFT, 16, 68);

  sPump = lv_label_create(parent);
  lv_obj_align(sPump, LV_ALIGN_TOP_LEFT, 16, 120);

  sFooter = lv_label_create(parent);
  lv_obj_align(sFooter, LV_ALIGN_BOTTOM_LEFT, 16, -16);
}

void refreshDashboard(const core::SystemState& state) {
  if (!sStatus) return;
  lv_label_set_text_fmt(sStatus, "Status: %s", state.runStatus.c_str());

  String active = "Active zones: ";
  uint8_t n = 0;
  for (const auto& z : state.zones) {
    if (z.isOpen) {
      active += String(z.id) + " ";
      if (++n >= 4) {
        active += "...";
        break;
      }
    }
  }
  if (n == 0) active += "none";
  lv_label_set_text(sZones, active.c_str());

  lv_label_set_text_fmt(sPump, "Pump relay: %s | Pump DC: %s", state.pumpRelayOn ? "ON" : "OFF",
                        state.pumpDcOn ? "ON" : "OFF");
  lv_label_set_text_fmt(sFooter, "t=%lu s | RS485 ok=%lu", millis() / 1000UL, state.comm.ok);
}

}  // namespace ui::screens
