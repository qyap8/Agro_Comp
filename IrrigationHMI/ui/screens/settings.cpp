#include "../ui_app.h"

namespace ui {

static UiContext *s_settingsCtx;
static lv_obj_t *s_wifiStat;
static lv_obj_t *s_ipStat;
static lv_obj_t *s_langDd;
static lv_obj_t *s_timeoutDd;
static lv_obj_t *s_timeoutLbl;

static lv_obj_t *s_wifiModal;
static lv_obj_t *s_wifiSsidTa;
static lv_obj_t *s_wifiPassTa;
static lv_obj_t *s_wifiModalStat;
static lv_obj_t *s_wifiOpenLbl;

static void ta_focus_cb(lv_event_t *e) { ui_attach_keyboard(lv_event_get_target(e)); }
static void ta_defocus_cb(lv_event_t *e) { (void)e; ui_keyboard_hide(); }

static void wifi_save_cb(lv_event_t *e) {
    (void)e;
    AppEvent ev{};
    ev.type = AppEventType::WifiSaveCreds;
    strlcpy(ev.ssid, lv_textarea_get_text(s_wifiSsidTa), sizeof(ev.ssid));
    strlcpy(ev.pass, lv_textarea_get_text(s_wifiPassTa), sizeof(ev.pass));
    s_settingsCtx->bus->publish(ev, 0);
    ui_toast(tr(s_settingsCtx->state->settings.language, "SAVED"));
}

static void wifi_modal_close_cb(lv_event_t *e) {
    (void)e;
    ui_keyboard_hide();
    lv_obj_add_flag(s_wifiModal, LV_OBJ_FLAG_HIDDEN);
}

static void wifi_open_cb(lv_event_t *e) {
    (void)e;
    lv_obj_clear_flag(s_wifiModal, LV_OBJ_FLAG_HIDDEN);
}


static uint16_t timeout_from_idx(uint16_t idx){ const uint16_t v[]={0,30,60,120,300}; return v[idx<5?idx:2]; }
static uint16_t idx_from_timeout(uint16_t sec){ if(sec==0) return 0; if(sec<=30) return 1; if(sec<=60) return 2; if(sec<=120) return 3; return 4; }

static void timeout_cb(lv_event_t *e) {
    (void)e;
    AppEvent ev{};
    ev.type = AppEventType::SetScreenTimeout;
    ev.value16 = timeout_from_idx(lv_dropdown_get_selected(s_timeoutDd));
    s_settingsCtx->bus->publish(ev, 0);
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
    lv_obj_set_style_pad_all(parent, 8, 0);

    s_wifiStat = lv_label_create(parent);
    s_ipStat = lv_label_create(parent);

    lv_obj_t *wifiBtn = lv_btn_create(parent);
    lv_obj_set_size(wifiBtn, LV_PCT(100), 44);
    lv_obj_add_event_cb(wifiBtn, wifi_open_cb, LV_EVENT_CLICKED, nullptr);
    s_wifiOpenLbl = lv_label_create(wifiBtn);
    lv_label_set_text_fmt(s_wifiOpenLbl, "%s Wi-Fi manager", LV_SYMBOL_WIFI);
    lv_obj_center(s_wifiOpenLbl);

    s_langDd = lv_dropdown_create(parent);
    lv_dropdown_set_options(s_langDd, "English\nEspañol\nРусский\nՀայերեն");
    lv_obj_set_width(s_langDd, LV_PCT(100));
    lv_obj_add_event_cb(s_langDd, lang_cb, LV_EVENT_VALUE_CHANGED, nullptr);

    s_timeoutLbl = lv_label_create(parent);
    lv_label_set_text(s_timeoutLbl, "Screen timeout");

    s_timeoutDd = lv_dropdown_create(parent);
    lv_dropdown_set_options(s_timeoutDd, "Off\n30 sec\n60 sec\n120 sec\n300 sec");
    lv_obj_set_width(s_timeoutDd, LV_PCT(100));
    lv_obj_add_event_cb(s_timeoutDd, timeout_cb, LV_EVENT_VALUE_CHANGED, nullptr);

    s_wifiModal = lv_obj_create(lv_scr_act());
    lv_obj_set_size(s_wifiModal, LV_PCT(90), LV_PCT(68));
    lv_obj_align(s_wifiModal, LV_ALIGN_CENTER, 0, -10);
    lv_obj_set_style_pad_all(s_wifiModal, 10, 0);
    lv_obj_set_layout(s_wifiModal, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(s_wifiModal, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_flag(s_wifiModal, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *title = lv_label_create(s_wifiModal);
    lv_label_set_text_fmt(title, "%s Wi-Fi", LV_SYMBOL_WIFI);

    s_wifiModalStat = lv_label_create(s_wifiModal);

    s_wifiSsidTa = lv_textarea_create(s_wifiModal);
    lv_obj_set_width(s_wifiSsidTa, LV_PCT(100));
    lv_textarea_set_placeholder_text(s_wifiSsidTa, tr(ctx->state->settings.language, "WIFI_SSID"));
    lv_obj_add_event_cb(s_wifiSsidTa, ta_focus_cb, LV_EVENT_FOCUSED, nullptr);
    lv_obj_add_event_cb(s_wifiSsidTa, ta_defocus_cb, LV_EVENT_DEFOCUSED, nullptr);

    s_wifiPassTa = lv_textarea_create(s_wifiModal);
    lv_obj_set_width(s_wifiPassTa, LV_PCT(100));
    lv_textarea_set_password_mode(s_wifiPassTa, true);
    lv_textarea_set_placeholder_text(s_wifiPassTa, tr(ctx->state->settings.language, "WIFI_PASS"));
    lv_obj_add_event_cb(s_wifiPassTa, ta_focus_cb, LV_EVENT_FOCUSED, nullptr);
    lv_obj_add_event_cb(s_wifiPassTa, ta_defocus_cb, LV_EVENT_DEFOCUSED, nullptr);

    lv_obj_t *row = lv_obj_create(s_wifiModal);
    lv_obj_set_size(row, LV_PCT(100), 52);
    lv_obj_set_layout(row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);

    lv_obj_t *saveBtn = lv_btn_create(row);
    lv_obj_set_size(saveBtn, LV_PCT(58), 40);
    lv_obj_add_event_cb(saveBtn, wifi_save_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *saveLbl = lv_label_create(saveBtn);
    lv_label_set_text(saveLbl, tr(ctx->state->settings.language, "WIFI_SAVE"));
    lv_obj_center(saveLbl);

    lv_obj_t *closeBtn = lv_btn_create(row);
    lv_obj_set_size(closeBtn, LV_PCT(38), 40);
    lv_obj_add_event_cb(closeBtn, wifi_modal_close_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *closeLbl = lv_label_create(closeBtn);
    lv_label_set_text_fmt(closeLbl, "%s", LV_SYMBOL_CLOSE);
    lv_obj_center(closeLbl);
}

void refresh_settings(UiContext *ctx) {
    if (xSemaphoreTake(ctx->stateMutex, pdMS_TO_TICKS(20)) != pdTRUE) return;

    const char *ws = ctx->state->wifi.connected ? tr(ctx->state->settings.language, "CONNECTED")
                    : (ctx->state->wifi.apMode ? tr(ctx->state->settings.language, "AP_MODE")
                                               : tr(ctx->state->settings.language, "OFFLINE"));

    lv_label_set_text_fmt(s_wifiStat, "%s %s", LV_SYMBOL_WIFI, ws);
    lv_label_set_text_fmt(s_ipStat, "IP: %s", ctx->state->wifi.ip.toString().c_str());
    lv_label_set_text_fmt(s_wifiOpenLbl, "%s Wi-Fi manager", LV_SYMBOL_WIFI);

    lv_label_set_text_fmt(s_wifiModalStat, "SSID: %s | IP: %s", ctx->state->wifi.ssid.c_str(), ctx->state->wifi.ip.toString().c_str());
    if (strlen(lv_textarea_get_text(s_wifiSsidTa)) == 0 && ctx->state->settings.wifiSsid.length()) {
        lv_textarea_set_text(s_wifiSsidTa, ctx->state->settings.wifiSsid.c_str());
    }

    lv_textarea_set_placeholder_text(s_wifiSsidTa, tr(ctx->state->settings.language, "WIFI_SSID"));
    lv_textarea_set_placeholder_text(s_wifiPassTa, tr(ctx->state->settings.language, "WIFI_PASS"));
    lv_dropdown_set_selected(s_langDd, static_cast<uint16_t>(ctx->state->settings.language));
    lv_dropdown_set_selected(s_timeoutDd, idx_from_timeout(ctx->state->settings.screenTimeoutSec));
    lv_label_set_text_fmt(s_timeoutLbl, "%s: %us", "Screen timeout", ctx->state->settings.screenTimeoutSec);

    xSemaphoreGive(ctx->stateMutex);
}

} // namespace ui
