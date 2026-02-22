#include "../ui_app.h"
#include <vector>

namespace ui {

struct ChWidget {
    uint8_t module;
    uint8_t channel;
    bool lastState;
    lv_obj_t *btn;
    lv_obj_t *lbl;
};

static UiContext *s_zonesCtx;
static lv_obj_t *s_zoneRoot;
static std::vector<ChWidget> s_widgets;
static uint32_t s_topologyHash = 0;

static uint32_t topology_hash(const app::SystemState *st) {
    uint32_t h = 2166136261u;
    for (const auto &m : st->modules) {
        h = (h ^ m.address) * 16777619u;
        h = (h ^ (uint8_t)m.channels.size()) * 16777619u;
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
    s_topologyHash = 0;
    s_widgets.clear();

    lv_obj_set_layout(parent, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(parent, 8, 0);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_AUTO);

    s_zoneRoot = lv_obj_create(parent);
    lv_obj_set_size(s_zoneRoot, LV_PCT(100), LV_PCT(100));
    lv_obj_set_layout(s_zoneRoot, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(s_zoneRoot, LV_FLEX_FLOW_COLUMN);
}

void refresh_manual(UiContext *ctx) {
    if (xSemaphoreTake(ctx->stateMutex, pdMS_TO_TICKS(20)) != pdTRUE) return;

    uint32_t h = topology_hash(ctx->state);
    if (h != s_topologyHash) {
        s_topologyHash = h;
        s_widgets.clear();
        lv_obj_clean(s_zoneRoot);

        for (const auto &m : ctx->state->modules) {
            lv_obj_t *card = lv_obj_create(s_zoneRoot);
            lv_obj_set_size(card, LV_PCT(100), LV_SIZE_CONTENT);
            lv_obj_set_style_pad_all(card, 8, 0);
            lv_obj_set_layout(card, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW_WRAP);

            lv_obj_t *title = lv_label_create(card);
            lv_label_set_text_fmt(title, "%s Module %u", LV_SYMBOL_DIRECTORY, m.address);
            lv_obj_set_width(title, LV_PCT(100));

            for (const auto &ch : m.channels) {
                lv_obj_t *btn = lv_btn_create(card);
                lv_obj_set_size(btn, 112, 42);
                uint32_t packed = (static_cast<uint32_t>(m.address) << 8) | ch.id;
                lv_obj_add_event_cb(btn, channel_cb, LV_EVENT_CLICKED, reinterpret_cast<void *>(packed));
                lv_obj_t *lbl = lv_label_create(btn);
                lv_obj_center(lbl);

                ChWidget w{m.address, ch.id, !ch.state, btn, lbl};
                s_widgets.push_back(w);
            }
        }
    }

    for (auto &w : s_widgets) {
        for (const auto &m : ctx->state->modules) {
            if (m.address != w.module) continue;
            for (const auto &ch : m.channels) {
                if (ch.id != w.channel) continue;
                if (ch.state != w.lastState) {
                    w.lastState = ch.state;
                    if (ch.state) lv_obj_add_state(w.btn, LV_STATE_CHECKED);
                    else lv_obj_clear_state(w.btn, LV_STATE_CHECKED);
                    lv_label_set_text_fmt(w.lbl, "%s %u %s", LV_SYMBOL_POWER, ch.id + 1, ch.state ? "ON" : "OFF");
                }
            }
        }
    }

    xSemaphoreGive(ctx->stateMutex);
}

} // namespace ui
