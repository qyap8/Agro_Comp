#include "../ui_app.h"

namespace ui {

static UiContext *s_manualCtx;
static lv_obj_t *zoneBtns[APP_MAX_ZONES];

static void zone_cb(lv_event_t *e) {
    uint32_t zone = reinterpret_cast<uint32_t>(lv_event_get_user_data(e));
    AppEvent ev{};
    ev.type = AppEventType::ZoneToggle;
    ev.zone = static_cast<uint8_t>(zone);
    s_manualCtx->bus->publish(ev, 0);
}

static void stop_all_cb(lv_event_t *e) {
    (void)e;
    AppEvent ev{};
    ev.type = AppEventType::StopAll;
    s_manualCtx->bus->publish(ev, 0);
}

static void pump_test_cb(lv_event_t *e) {
    (void)e;
    AppEvent ev{};
    ev.type = AppEventType::PumpTest;
    s_manualCtx->bus->publish(ev, 0);
}

void build_manual_tab(lv_obj_t *parent, UiContext *ctx) {
    s_manualCtx = ctx;
    lv_obj_set_layout(parent, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *grid = lv_obj_create(parent);
    lv_obj_set_size(grid, LV_PCT(100), 300);
    lv_obj_set_layout(grid, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);

    for (uint8_t i = 0; i < APP_MAX_ZONES; ++i) {
        zoneBtns[i] = lv_btn_create(grid);
        lv_obj_set_size(zoneBtns[i], 120, 60);
        lv_obj_add_event_cb(zoneBtns[i], zone_cb, LV_EVENT_CLICKED, reinterpret_cast<void *>(i));
        lv_obj_t *lbl = lv_label_create(zoneBtns[i]);
        lv_label_set_text_fmt(lbl, "Zone %u", i + 1);
        lv_obj_center(lbl);
    }

    lv_obj_t *controls = lv_obj_create(parent);
    lv_obj_set_size(controls, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_layout(controls, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(controls, LV_FLEX_FLOW_ROW);

    lv_obj_t *stopBtn = lv_btn_create(controls);
    lv_obj_set_size(stopBtn, 180, 56);
    lv_obj_add_event_cb(stopBtn, stop_all_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *stopLbl = lv_label_create(stopBtn);
    lv_label_set_text(stopLbl, "Stop All");
    lv_obj_center(stopLbl);

    lv_obj_t *pumpBtn = lv_btn_create(controls);
    lv_obj_set_size(pumpBtn, 180, 56);
    lv_obj_add_event_cb(pumpBtn, pump_test_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *pumpLbl = lv_label_create(pumpBtn);
    lv_label_set_text(pumpLbl, "Pump Test");
    lv_obj_center(pumpLbl);
}

void refresh_manual(UiContext *ctx) {
    for (uint8_t i = 0; i < APP_MAX_ZONES; ++i) {
        if (ctx->state->zones[i]) {
            lv_obj_add_state(zoneBtns[i], LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(zoneBtns[i], LV_STATE_CHECKED);
        }
    }
}

} // namespace ui
