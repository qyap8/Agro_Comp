#pragma once

#include <lvgl.h>

#include "core/state.h"

namespace ui {

class DashboardScreen {
 public:
  lv_obj_t* create(lv_obj_t* parent);
  void refresh(const core::SystemState& state);

 private:
  lv_obj_t* root_ = nullptr;
  lv_obj_t* statusLabel_ = nullptr;
  lv_obj_t* zonesLabel_ = nullptr;
  lv_obj_t* pumpLabel_ = nullptr;
  lv_obj_t* footerLabel_ = nullptr;
};

}  // namespace ui
