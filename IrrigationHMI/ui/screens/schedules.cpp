#include "../ui_app.h"

namespace ui::screens {

static lv_obj_t* sList = nullptr;

void buildSchedules(lv_obj_t* parent, core::EventBus* bus) {
  (void)bus;
  sList = lv_list_create(parent);
  lv_obj_set_size(sList, 780, 380);
  lv_obj_align(sList, LV_ALIGN_TOP_MID, 0, 8);

  lv_obj_t* hint = lv_label_create(parent);
  lv_label_set_text(hint, "Add/Edit/Delete dialogs: TODO hooks in this scaffold");
  lv_obj_align(hint, LV_ALIGN_BOTTOM_LEFT, 16, -12);
}

void refreshSchedules(const core::SystemState& state) {
  if (!sList) return;
  lv_obj_clean(sList);
  for (const auto& s : state.schedules) {
    char line[120];
    snprintf(line, sizeof(line), "ID:%u D:%02X %02u:%02u dur:%umin zones:%04X %s", s.id, s.daysMask, s.hour,
             s.minute, s.durationMin, s.zoneMask, s.enabled ? "EN" : "DIS");
    lv_list_add_text(sList, line);
  }
}

}  // namespace ui::screens
