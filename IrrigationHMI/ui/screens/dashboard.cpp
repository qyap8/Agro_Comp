#include "../ui_app.h"

namespace ui {

static UiContext *s_homeCtx;
static lv_obj_t *s_wifi;
static lv_obj_t *s_status;
static lv_obj_t *s_modules;

static void rescan_cb(lv_event_t *e) {
    (void)e;
    AppEvent ev{};
    ev.type = AppEventType::ModuleRescan;
    s_homeCtx->bus->publish(ev, 0);
    ui_toast("Rescan...");
}

void build_dashboard_tab(lv_obj_t *parent, UiContext *ctx) {
    s_homeCtx = ctx;
    lv_obj_set_layout(parent, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(parent, 12, 0);

    lv_obj_t *header = lv_obj_create(parent);
    lv_obj_set_size(header, LV_PCT(100), 72);
    lv_obj_set_layout(header, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);

    s_status = lv_label_create(header);
    lv_obj_set_width(s_status, 500);
    s_wifi = lv_label_create(header);

    lv_obj_t *rescan = lv_btn_create(parent);
    lv_obj_set_size(rescan, LV_PCT(100), 58);
    lv_obj_add_event_cb(rescan, rescan_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *rl = lv_label_create(rescan);
    lv_label_set_text(rl, "🔄 Rescan modules");
    lv_obj_center(rl);

    s_modules = lv_obj_create(parent);
    lv_obj_set_size(s_modules, LV_PCT(100), LV_PCT(100));
    lv_obj_set_layout(s_modules, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(s_modules, LV_FLEX_FLOW_COLUMN);
}

void refresh_dashboard(UiContext *ctx) {
    if (xSemaphoreTake(ctx->stateMutex, pdMS_TO_TICKS(20)) != pdTRUE) return;
    lv_label_set_text_fmt(s_status, "Modules: %u | RS485 TX:%lu RX:%lu",
                          (unsigned)ctx->state->modules.size(),
                          (unsigned long)ctx->state->comm.txFrames,
                          (unsigned long)ctx->state->comm.rxFrames);
    lv_label_set_text(s_wifi, ctx->state->wifi.connected ? "📶" : (ctx->state->wifi.apMode ? "📡 AP" : "⚠"));

    lv_obj_clean(s_modules);
    for (const auto &m : ctx->state->modules) {
        lv_obj_t *card = lv_obj_create(s_modules);
        lv_obj_set_size(card, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_t *lbl = lv_label_create(card);
        lv_label_set_text_fmt(lbl, "Module %u | UID %lu | FW %s | CH %u",
                              m.address, (unsigned long)m.uid, m.firmware.c_str(),
                              (unsigned)m.channels.size());
    }
    xSemaphoreGive(ctx->stateMutex);
}

} // namespace ui
