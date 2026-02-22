#include "../ui_app.h"

namespace ui {

static UiContext *s_settingsCtx;
static lv_obj_t *s_wifiStat;
static lv_obj_t *s_ssidTa;
static lv_obj_t *s_passTa;
static lv_obj_t *s_kb;
static lv_obj_t *s_langDd;

static void kb_event(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        lv_obj_add_flag(s_kb, LV_OBJ_FLAG_HIDDEN);
    }
}

static void ta_focus_cb(lv_event_t *e) {
    lv_obj_t *ta = lv_event_get_target(e);
    lv_keyboard_set_textarea(s_kb, ta);
    lv_obj_clear_flag(s_kb, LV_OBJ_FLAG_HIDDEN);
}

static void wifi_save_cb(lv_event_t *e) {
    (void)e;
    AppEvent ev{};
    ev.type = AppEventType::WifiSaveCreds;
    strlcpy(ev.ssid, lv_textarea_get_text(s_ssidTa), sizeof(ev.ssid));
    strlcpy(ev.pass, lv_textarea_get_text(s_passTa), sizeof(ev.pass));
    s_settingsCtx->bus->publish(ev, 0);
    ui_toast("Wi-Fi saved");
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

    s_wifiStat = lv_label_create(parent);

    s_ssidTa = lv_textarea_create(parent);
    lv_textarea_set_placeholder_text(s_ssidTa, "Wi-Fi SSID");
    lv_obj_set_width(s_ssidTa, LV_PCT(100));
    lv_obj_add_event_cb(s_ssidTa, ta_focus_cb, LV_EVENT_FOCUSED, nullptr);

    s_passTa = lv_textarea_create(parent);
    lv_textarea_set_password_mode(s_passTa, true);
    lv_textarea_set_placeholder_text(s_passTa, "Wi-Fi Password");
    lv_obj_set_width(s_passTa, LV_PCT(100));
    lv_obj_add_event_cb(s_passTa, ta_focus_cb, LV_EVENT_FOCUSED, nullptr);

    lv_obj_t *saveBtn = lv_btn_create(parent);
    lv_obj_set_size(saveBtn, LV_PCT(100), 56);
    lv_obj_add_event_cb(saveBtn, wifi_save_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *sl = lv_label_create(saveBtn);
    lv_label_set_text(sl, "Save Wi-Fi");
    lv_obj_center(sl);

    s_langDd = lv_dropdown_create(parent);
    lv_dropdown_set_options(s_langDd, "English\nEspañol\nРусский\nՀայերեն");
    lv_obj_set_width(s_langDd, LV_PCT(100));
    lv_obj_add_event_cb(s_langDd, lang_cb, LV_EVENT_VALUE_CHANGED, nullptr);

    s_kb = lv_keyboard_create(parent);
    lv_obj_set_size(s_kb, LV_PCT(100), 180);
    lv_obj_add_flag(s_kb, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(s_kb, kb_event, LV_EVENT_ALL, nullptr);
}

void refresh_settings(UiContext *ctx) {
    if (xSemaphoreTake(ctx->stateMutex, pdMS_TO_TICKS(20)) != pdTRUE) return;
    lv_label_set_text_fmt(s_wifiStat, "Wi-Fi: %s  %s",
                          ctx->state->wifi.connected ? "Connected" : (ctx->state->wifi.apMode ? "AP mode" : "Offline"),
                          ctx->state->wifi.ssid.c_str());
    if (strlen(lv_textarea_get_text(s_ssidTa)) == 0 && ctx->state->settings.wifiSsid.length()) {
        lv_textarea_set_text(s_ssidTa, ctx->state->settings.wifiSsid.c_str());
    }
    lv_dropdown_set_selected(s_langDd, static_cast<uint16_t>(ctx->state->settings.language));
    xSemaphoreGive(ctx->stateMutex);
}

} // namespace ui
