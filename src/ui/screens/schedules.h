#pragma once

#include <lvgl.h>

#include "core/state.h"

namespace ui {

class SchedulesScreen {
 public:
  lv_obj_t* create(lv_obj_t* parent);
  void refresh(const core::SystemState& state);

 private:
  lv_obj_t* root_ = nullptr;
  lv_obj_t* list_ = nullptr;
};

}  // namespace ui
