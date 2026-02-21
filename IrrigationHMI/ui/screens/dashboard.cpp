#include "../ui_app.h"

namespace ui {

static lv_obj_t *statusLbl;
static lv_obj_t *zonesLbl;
static lv_obj_t *pumpLbl;

static void nav_btn_cb(lv_event_t *e) {
    uint32_t tab = reinterpret_cast<uint32_t>(lv_event_get_user_data(e));
    lv_obj_t *tv = lv_obj_get_parent(lv_event_get_target(e));
    while (tv && !lv_obj_check_type(tv, &lv_tabview_class)) {
        tv = lv_obj_get_parent(tv);
    }
    if (tv) {
        lv_tabview_set_act(tv, tab, LV_ANIM_ON);
    }
}

void build_dashboard_tab(lv_obj_t *parent, UiContext *ctx) {
    (void)ctx;
    lv_obj_set_layout(parent, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);

    statusLbl = lv_label_create(parent);
    zonesLbl = lv_label_create(parent);
    pumpLbl = lv_label_create(parent);

    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_layout(row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW_WRAP);

    const char *names[] = {"Manual", "Schedules", "Modules", "Settings", "Diagnostics"};
    const uint32_t tabs[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; ++i) {
        lv_obj_t *btn = lv_btn_create(row);
        lv_obj_set_size(btn, 140, 56);
        lv_obj_add_event_cb(btn, nav_btn_cb, LV_EVENT_CLICKED, reinterpret_cast<void *>(tabs[i]));
        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, names[i]);
        lv_obj_center(lbl);
    }
}

void refresh_dashboard(UiContext *ctx) {
    const char *mode = "Idle";
    if (ctx->state->mode == app::SystemMode::Watering) mode = "Watering";
    if (ctx->state->mode == app::SystemMode::Error) mode = "Error";

    uint8_t active = 0;
    for (bool z : ctx->state->zones) active += z ? 1 : 0;

    lv_label_set_text_fmt(statusLbl, "Status: %s", mode);
    lv_label_set_text_fmt(zonesLbl, "Active zones: %u", active);
    lv_label_set_text_fmt(pumpLbl, "Pump Relay: %s | Pump DC: %s",
                          ctx->state->pumpRelay ? "ON" : "OFF",
                          ctx->state->pumpDc ? "ON" : "OFF");
}

} // namespace ui
