#include "../ui_app.h"

namespace ui {

static lv_obj_t *statsLbl;
static lv_obj_t *logList;

void build_diagnostics_tab(lv_obj_t *parent, UiContext *ctx) {
    (void)ctx;
    lv_obj_set_layout(parent, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);

    statsLbl = lv_label_create(parent);
    logList = lv_list_create(parent);
    lv_obj_set_size(logList, LV_PCT(100), 300);
}

void refresh_diagnostics(UiContext *ctx) {
    lv_label_set_text_fmt(statsLbl, "TX:%lu RX:%lu CRC:%lu TO:%lu",
                          (unsigned long)ctx->state->comm.txFrames,
                          (unsigned long)ctx->state->comm.rxFrames,
                          (unsigned long)ctx->state->comm.crcErrors,
                          (unsigned long)ctx->state->comm.timeouts);

    lv_obj_clean(logList);
    for (uint16_t i = 0; i < ctx->state->eventCount; ++i) {
        int idx = (ctx->state->eventHead + APP_EVENT_LOG_CAPACITY - ctx->state->eventCount + i) % APP_EVENT_LOG_CAPACITY;
        const auto &ev = ctx->state->eventLog[idx];
        lv_obj_t *txt = lv_list_add_text(logList, "");
        lv_label_set_text_fmt(txt, "[%lu] %s", (unsigned long)ev.ts, ev.text.c_str());
    }
}

} // namespace ui
