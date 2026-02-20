#include "schedules.h"

namespace ui {

lv_obj_t* SchedulesScreen::create(lv_obj_t* parent) {
  root_ = lv_obj_create(parent);
  lv_obj_set_size(root_, LV_PCT(100), LV_PCT(100));
  list_ = lv_list_create(root_);
  lv_obj_set_size(list_, 760, 400);
  lv_obj_center(list_);
  return root_;
}

void SchedulesScreen::refresh(const core::SystemState& state) {
  lv_obj_clean(list_);
  for (const auto& s : state.schedules) {
    lv_list_add_text(list_,
                     ("Z" + String(s.zoneId) + " " + String(s.hour) + ":" + String(s.minute) + " " +
                      String(s.durationMin) + "m")
                         .c_str());
  }
}

}  // namespace ui
