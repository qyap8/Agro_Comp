// Экран модулей: перезапрос модулей и отображение UID/addr/fw.
#include "../ui_app.h"

namespace ui {

static UiContext *g_ctx;
static lv_obj_t *list;

static void rescan_cb(lv_event_t *e) {
    (void)e;
    AppEvent ev{};
    ev.type = AppEventType::RescanModules;
    g_ctx->bus->publish(ev, 0);
}

void build_modules_tab(lv_obj_t *parent, UiContext *ctx) {
    g_ctx = ctx;
    lv_obj_set_layout(parent, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *rescanBtn = lv_btn_create(parent);
    lv_obj_set_size(rescanBtn, 180, 56);
    lv_obj_add_event_cb(rescanBtn, rescan_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *rescanLbl = lv_label_create(rescanBtn);
    lv_label_set_text(rescanLbl, "Rescan");
    lv_obj_center(rescanLbl);

    list = lv_list_create(parent);
    lv_obj_set_size(list, LV_PCT(100), 280);

    lv_obj_t *hint = lv_label_create(parent);
    lv_label_set_text(hint, "Assign address dialog available via core event");
}

void refresh_modules(UiContext *ctx) {
    lv_obj_clean(list);
    for (const auto &m : ctx->state->modules) {
        lv_obj_t *item = lv_list_add_text(list, "");
        lv_label_set_text_fmt(item, "UID:%lu Addr:%u FW:%s %s",
                              static_cast<unsigned long>(m.uid), m.address,
                              m.firmware.c_str(), m.online ? "ONLINE" : "OFFLINE");
    }
}

} // namespace ui
