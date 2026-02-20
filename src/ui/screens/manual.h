#pragma once

#include <array>

#include <lvgl.h>

#include "core/event_bus.h"
#include "core/state.h"

namespace ui {

class ManualScreen {
 public:
  lv_obj_t* create(lv_obj_t* parent, core::EventBus* bus);
  void refresh(const core::SystemState& state);

 private:
  static void zoneBtnCb(lv_event_t* e);
  static void stopAllCb(lv_event_t* e);

  lv_obj_t* root_ = nullptr;
  std::array<lv_obj_t*, core::kMaxZones> zoneBtns_{};
  core::EventBus* bus_ = nullptr;
};

}  // namespace ui
