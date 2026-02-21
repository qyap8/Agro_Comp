#include "../ui_app.h"

namespace ui {

static UiContext *g_ctx;
static lv_obj_t *list;

static void save_schedule_cb(lv_event_t *e) {
    (void)e;
    app::Schedule s{};
    s.id = millis() & 0xFFFF;
    s.zone = 0;
    s.daysMask = 0x7F;
    s.hour = 6;
    s.minute = 0;
    s.durationMin = 10;

    AppEvent ev{};
    ev.type = AppEventType::SaveSchedule;
    ev.schedule = s;
    g_ctx->bus->publish(ev, 0);
}

void build_schedules_tab(lv_obj_t *parent, UiContext *ctx) {
    g_ctx = ctx;
    lv_obj_set_layout(parent, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);

    list = lv_list_create(parent);
    lv_obj_set_size(list, LV_PCT(100), 300);

    lv_obj_t *addBtn = lv_btn_create(parent);
    lv_obj_set_size(addBtn, 220, 56);
    lv_obj_add_event_cb(addBtn, save_schedule_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *addLbl = lv_label_create(addBtn);
    lv_label_set_text(addLbl, "Add/Edit Schedule");
    lv_obj_center(addLbl);

    lv_obj_t *hint = lv_label_create(parent);
    lv_label_set_text(hint, "Dialog fields: DOW, time, duration, zone (managed by core event)");
}

void refresh_schedules(UiContext *ctx) {
    lv_obj_clean(list);
    for (const auto &s : ctx->state->schedules) {
        lv_obj_t *btn = lv_list_add_btn(list, LV_SYMBOL_CALENDAR, "");
        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text_fmt(lbl, "#%u Z%u %02u:%02u %umin mask0x%02X",
                              s.id, s.zone + 1, s.hour, s.minute, s.durationMin, s.daysMask);
    }
}

} // namespace ui
