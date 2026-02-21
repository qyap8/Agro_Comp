#include "../ui_app.h"
#include "../../config.h"

namespace ui {

static UiContext *s_settingsCtx;
static lv_obj_t *pulseSlider;
static lv_obj_t *closeSwitch;
static lv_obj_t *pulseLabel;

static void pulse_cb(lv_event_t *e) {
    (void)e;
    uint16_t v = lv_slider_get_value(pulseSlider);
    AppEvent ev{};
    ev.type = AppEventType::SetPulseWidth;
    ev.value16 = v;
    s_settingsCtx->bus->publish(ev, 0);
}

static void close_boot_cb(lv_event_t *e) {
    (void)e;
    AppEvent ev{};
    ev.type = AppEventType::SetCloseAllOnBoot;
    ev.valueBool = lv_obj_has_state(closeSwitch, LV_STATE_CHECKED);
    s_settingsCtx->bus->publish(ev, 0);
}

void build_settings_tab(lv_obj_t *parent, UiContext *ctx) {
    s_settingsCtx = ctx;
    lv_obj_set_layout(parent, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);

    pulseLabel = lv_label_create(parent);
    pulseSlider = lv_slider_create(parent);
    lv_obj_set_width(pulseSlider, LV_PCT(100));
    lv_slider_set_range(pulseSlider, APP_MIN_PULSE_WIDTH_MS, APP_MAX_PULSE_WIDTH_MS);
    lv_obj_add_event_cb(pulseSlider, pulse_cb, LV_EVENT_VALUE_CHANGED, nullptr);

    lv_obj_t *closeRow = lv_obj_create(parent);
    lv_obj_set_size(closeRow, LV_PCT(100), 64);
    lv_obj_set_layout(closeRow, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(closeRow, LV_FLEX_FLOW_ROW);
    lv_label_create(closeRow);
    lv_label_set_text(lv_obj_get_child(closeRow, 0), "Close all on boot");
    closeSwitch = lv_switch_create(closeRow);
    lv_obj_add_event_cb(closeSwitch, close_boot_cb, LV_EVENT_VALUE_CHANGED, nullptr);

    lv_label_create(parent);
    lv_label_set_text(lv_obj_get_child(parent, 3), "Brightness slider (stub)");
    lv_obj_t *b = lv_slider_create(parent);
    lv_obj_set_width(b, LV_PCT(100));

    lv_label_create(parent);
    lv_label_set_text(lv_obj_get_child(parent, 5), "Language selector (stub)");
    lv_dropdown_create(parent);
    lv_dropdown_set_options(lv_obj_get_child(parent, 6), "English\nSpanish\nPortuguese");
}

void refresh_settings(UiContext *ctx) {
    lv_slider_set_value(pulseSlider, ctx->state->settings.pulseWidthMs, LV_ANIM_OFF);
    lv_label_set_text_fmt(pulseLabel, "Pulse width: %ums", ctx->state->settings.pulseWidthMs);
    if (ctx->state->settings.closeAllOnBoot) {
        lv_obj_add_state(closeSwitch, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(closeSwitch, LV_STATE_CHECKED);
    }
}

} // namespace ui
