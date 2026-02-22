#include "../ui_app.h"

namespace ui {

static UiContext *s_homeCtx;
static lv_obj_t *s_wifi;
static lv_obj_t *s_status;
static lv_obj_t *s_modules;
static lv_obj_t *s_rescanLbl;
static uint32_t s_topologyHash = 0;

static uint32_t modules_topology_hash(const app::SystemState *st) {
    uint32_t h = 2166136261u;
    for (const auto &m : st->modules) {
        h = (h ^ m.address) * 16777619u;
        h = (h ^ (uint8_t)m.channels.size()) * 16777619u;
    }
    return h;
}

static void rescan_cb(lv_event_t *e) {
    (void)e;
    AppEvent ev{};
    ev.type = AppEventType::ModuleRescan;
    s_homeCtx->bus->publish(ev, 0);
    ui_toast(tr(s_homeCtx->state->settings.language, "RESCAN"));
}

void build_dashboard_tab(lv_obj_t *parent, UiContext *ctx) {
    s_homeCtx = ctx;
    s_topologyHash = 0;

    lv_obj_set_layout(parent, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(parent, 8, 0);

    lv_obj_t *header = lv_obj_create(parent);
    lv_obj_set_size(header, LV_PCT(100), 58);
    lv_obj_set_layout(header, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);

    s_status = lv_label_create(header);
    lv_obj_set_width(s_status, LV_PCT(80));
    s_wifi = lv_label_create(header);

    lv_obj_t *rescan = lv_btn_create(parent);
    lv_obj_set_size(rescan, LV_PCT(100), 44);
    lv_obj_add_event_cb(rescan, rescan_cb, LV_EVENT_CLICKED, nullptr);
    s_rescanLbl = lv_label_create(rescan);
    lv_label_set_text(s_rescanLbl, tr(ctx->state->settings.language, "RESCAN"));
    lv_obj_center(s_rescanLbl);

    s_modules = lv_obj_create(parent);
    lv_obj_set_size(s_modules, LV_PCT(100), LV_PCT(100));
    lv_obj_set_layout(s_modules, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(s_modules, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(s_modules, LV_SCROLLBAR_MODE_AUTO);
}

void refresh_dashboard(UiContext *ctx) {
    if (xSemaphoreTake(ctx->stateMutex, pdMS_TO_TICKS(20)) != pdTRUE) return;

    lv_label_set_text_fmt(s_status, "%s %u | %s %lu/%lu",
                          LV_SYMBOL_LIST,
                          (unsigned)ctx->state->modules.size(),
                          LV_SYMBOL_SHUFFLE,
                          (unsigned long)ctx->state->comm.txFrames,
                          (unsigned long)ctx->state->comm.rxFrames);

    lv_label_set_text_fmt(s_wifi, "%s %s",
                          ctx->state->wifi.connected ? LV_SYMBOL_WIFI : LV_SYMBOL_WARNING,
                          ctx->state->wifi.ip.toString().c_str());

    lv_label_set_text_fmt(s_rescanLbl, "%s %s", LV_SYMBOL_REFRESH, tr(ctx->state->settings.language, "RESCAN"));

    uint32_t h = modules_topology_hash(ctx->state);
    if (h != s_topologyHash) {
        s_topologyHash = h;
        lv_obj_clean(s_modules);
        for (const auto &m : ctx->state->modules) {
            lv_obj_t *card = lv_obj_create(s_modules);
            lv_obj_set_size(card, LV_PCT(100), 54);
            lv_obj_t *lbl = lv_label_create(card);
            lv_label_set_text_fmt(lbl, "%s M%u | UID %lu | CH %u",
                                  LV_SYMBOL_DIRECTORY,
                                  m.address,
                                  (unsigned long)m.uid,
                                  (unsigned)m.channels.size());
            lv_obj_center(lbl);
        }
    }

    xSemaphoreGive(ctx->stateMutex);
}

} // namespace ui
