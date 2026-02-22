#include "../ui_app.h"

namespace ui {

static UiContext *s_zonesCtx;
static lv_obj_t *s_zoneRoot;
static uint32_t s_lastZonesHash = 0;

static uint32_t zones_hash(const app::SystemState *st) {
    uint32_t h = 2166136261u;
    for (const auto &m : st->modules) {
        h = (h ^ m.address) * 16777619u;
        for (const auto &ch : m.channels) {
            h = (h ^ ch.id) * 16777619u;
            h = (h ^ (ch.state ? 1 : 0)) * 16777619u;
        }
    }
    return h;
}

static void channel_cb(lv_event_t *e) {
    uint32_t packed = reinterpret_cast<uint32_t>(lv_event_get_user_data(e));
    AppEvent ev{};
    ev.type = AppEventType::SetChannelState;
    ev.moduleAddr = (packed >> 8) & 0xFF;
    ev.channelId = packed & 0xFF;
    ev.valueBool = !lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED);
    s_zonesCtx->bus->publish(ev, 0);
    ui_toast(tr(s_zonesCtx->state->settings.language, "CH_UPDATED"));
}

void build_manual_tab(lv_obj_t *parent, UiContext *ctx) {
    s_zonesCtx = ctx;
    s_lastZonesHash = 0;

    lv_obj_set_layout(parent, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(parent, 10, 0);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_ACTIVE);

    s_zoneRoot = lv_obj_create(parent);
    lv_obj_set_size(s_zoneRoot, LV_PCT(100), LV_PCT(100));
    lv_obj_set_layout(s_zoneRoot, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(s_zoneRoot, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(s_zoneRoot, LV_SCROLLBAR_MODE_OFF);
}

void refresh_manual(UiContext *ctx) {
    if (xSemaphoreTake(ctx->stateMutex, pdMS_TO_TICKS(20)) != pdTRUE) return;

    uint32_t h = zones_hash(ctx->state);
    if (h != s_lastZonesHash) {
        s_lastZonesHash = h;
        lv_obj_clean(s_zoneRoot);

        for (const auto &m : ctx->state->modules) {
            lv_obj_t *card = lv_obj_create(s_zoneRoot);
            lv_obj_set_size(card, LV_PCT(100), LV_SIZE_CONTENT);
            lv_obj_set_layout(card, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW_WRAP);

            lv_obj_t *title = lv_label_create(card);
            lv_label_set_text_fmt(title, "Module %u", m.address);
            lv_obj_set_width(title, LV_PCT(100));

            for (const auto &ch : m.channels) {
                lv_obj_t *btn = lv_btn_create(card);
                lv_obj_set_size(btn, 150, 56);
                if (ch.state) lv_obj_add_state(btn, LV_STATE_CHECKED);
                uint32_t packed = (static_cast<uint32_t>(m.address) << 8) | ch.id;
                lv_obj_add_event_cb(btn, channel_cb, LV_EVENT_CLICKED, reinterpret_cast<void *>(packed));
                lv_obj_t *lbl = lv_label_create(btn);
                lv_label_set_text_fmt(lbl, "%s %u %s", LV_SYMBOL_POWER, ch.id + 1, ch.state ? "ON" : "OFF");
                lv_obj_center(lbl);
            }
        }
    }
    xSemaphoreGive(ctx->stateMutex);
}

} // namespace ui
