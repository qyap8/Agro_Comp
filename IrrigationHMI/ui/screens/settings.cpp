#include "../ui_app.h"

namespace ui {

static UiContext *s_settingsCtx;
static lv_obj_t *s_wifiStat;
static lv_obj_t *s_ssidTa;
static lv_obj_t *s_passTa;
static lv_obj_t *s_langDd;
static lv_obj_t *s_saveLbl;

static void ta_focus_cb(lv_event_t *e) {
    lv_obj_t *ta = lv_event_get_target(e);
    ui_attach_keyboard(ta);
}

static void ta_defocus_cb(lv_event_t *e) {
    (void)e;
    ui_keyboard_hide();
}

static void wifi_save_cb(lv_event_t *e) {
    (void)e;
    AppEvent ev{};
    ev.type = AppEventType::WifiSaveCreds;
    strlcpy(ev.ssid, lv_textarea_get_text(s_ssidTa), sizeof(ev.ssid));
    strlcpy(ev.pass, lv_textarea_get_text(s_passTa), sizeof(ev.pass));
    s_settingsCtx->bus->publish(ev, 0);
    ui_toast(tr(s_settingsCtx->state->settings.language, "SAVED"));
}

static void lang_cb(lv_event_t *e) {
    (void)e;
    AppEvent ev{};
    ev.type = AppEventType::SetLanguage;
    ev.language = static_cast<app::Lang>(lv_dropdown_get_selected(s_langDd));
    s_settingsCtx->bus->publish(ev, 0);
}

void build_settings_tab(lv_obj_t *parent, UiContext *ctx) {
    s_settingsCtx = ctx;
    lv_obj_set_layout(parent, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_ACTIVE);

    s_wifiStat = lv_label_create(parent);

    s_ssidTa = lv_textarea_create(parent);
    lv_textarea_set_placeholder_text(s_ssidTa, tr(ctx->state->settings.language, "WIFI_SSID"));
    lv_obj_set_width(s_ssidTa, LV_PCT(100));
    lv_obj_add_event_cb(s_ssidTa, ta_focus_cb, LV_EVENT_FOCUSED, nullptr);
    lv_obj_add_event_cb(s_ssidTa, ta_defocus_cb, LV_EVENT_DEFOCUSED, nullptr);

    s_passTa = lv_textarea_create(parent);
    lv_textarea_set_password_mode(s_passTa, true);
    lv_textarea_set_placeholder_text(s_passTa, tr(ctx->state->settings.language, "WIFI_PASS"));
    lv_obj_set_width(s_passTa, LV_PCT(100));
    lv_obj_add_event_cb(s_passTa, ta_focus_cb, LV_EVENT_FOCUSED, nullptr);
    lv_obj_add_event_cb(s_passTa, ta_defocus_cb, LV_EVENT_DEFOCUSED, nullptr);

    lv_obj_t *saveBtn = lv_btn_create(parent);
    lv_obj_set_size(saveBtn, LV_PCT(100), 56);
    lv_obj_add_event_cb(saveBtn, wifi_save_cb, LV_EVENT_CLICKED, nullptr);
    s_saveLbl = lv_label_create(saveBtn);
    lv_label_set_text(s_saveLbl, tr(ctx->state->settings.language, "WIFI_SAVE"));
    lv_obj_center(s_saveLbl);

    s_langDd = lv_dropdown_create(parent);
    lv_dropdown_set_options(s_langDd, "English\nEspañol\nРусский\nՀայերեն");
    lv_obj_set_width(s_langDd, LV_PCT(100));
    lv_obj_add_event_cb(s_langDd, lang_cb, LV_EVENT_VALUE_CHANGED, nullptr);
}

void refresh_settings(UiContext *ctx) {
    if (xSemaphoreTake(ctx->stateMutex, pdMS_TO_TICKS(20)) != pdTRUE) return;

    const char *ws = ctx->state->wifi.connected ? tr(ctx->state->settings.language, "CONNECTED")
                    : (ctx->state->wifi.apMode ? tr(ctx->state->settings.language, "AP_MODE")
                                               : tr(ctx->state->settings.language, "OFFLINE"));

    lv_label_set_text_fmt(s_wifiStat, "Wi-Fi: %s  %s", ws, ctx->state->wifi.ssid.c_str());
    if (strlen(lv_textarea_get_text(s_ssidTa)) == 0 && ctx->state->settings.wifiSsid.length()) {
        lv_textarea_set_text(s_ssidTa, ctx->state->settings.wifiSsid.c_str());
    }
    lv_label_set_text(s_saveLbl, tr(ctx->state->settings.language, "WIFI_SAVE"));
    lv_textarea_set_placeholder_text(s_ssidTa, tr(ctx->state->settings.language, "WIFI_SSID"));
    lv_textarea_set_placeholder_text(s_passTa, tr(ctx->state->settings.language, "WIFI_PASS"));
    lv_dropdown_set_selected(s_langDd, static_cast<uint16_t>(ctx->state->settings.language));
    xSemaphoreGive(ctx->stateMutex);
}

} // namespace ui
