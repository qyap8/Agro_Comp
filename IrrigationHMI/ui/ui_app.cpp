// Корневой UI-модуль: 3 нижние вкладки Home/Zones/Settings и общий refresh.
#include "ui_app.h"

namespace ui {

static UiContext *g_ctx = nullptr;
static lv_obj_t *g_tabview = nullptr;
static lv_obj_t *g_toast = nullptr;
static lv_obj_t *g_keyboard = nullptr;
static lv_obj_t *g_keyboardHost = nullptr;
static uint32_t g_toastTs = 0;
static app::Lang g_lastLang = app::Lang::EN;

void build_dashboard_tab(lv_obj_t *parent, UiContext *ctx);
void refresh_dashboard(UiContext *ctx);
void build_manual_tab(lv_obj_t *parent, UiContext *ctx);
void refresh_manual(UiContext *ctx);
void build_settings_tab(lv_obj_t *parent, UiContext *ctx);
void refresh_settings(UiContext *ctx);

static const char *dict[][5] = {
    {"HOME", "Home", "Inicio", "Главная", "Գլխավոր"},
    {"ZONES", "Zones", "Zonas", "Зоны", "Գոտիներ"},
    {"SETTINGS", "Settings", "Ajustes", "Настройки", "Կարգավորումներ"},
    {"RESCAN", "Rescan modules", "Escanear módulos", "Перескан модулей", "Վերասքան մոդուլները"},
    {"WIFI_SAVE", "Save Wi-Fi", "Guardar Wi-Fi", "Сохранить Wi-Fi", "Պահել Wi‑Fi"},
    {"WIFI_SSID", "Wi-Fi SSID", "SSID Wi-Fi", "SSID Wi-Fi", "Wi‑Fi SSID"},
    {"WIFI_PASS", "Wi-Fi Password", "Contraseña Wi-Fi", "Пароль Wi-Fi", "Wi‑Fi գաղտնաբառ"},
    {"CONNECTED", "Connected", "Conectado", "Подключено", "Միացված"},
    {"AP_MODE", "AP mode", "Modo AP", "Режим AP", "AP ռեժիմ"},
    {"OFFLINE", "Offline", "Sin conexión", "Оффлайн", "Անցանց"},
    {"CH_UPDATED", "Channel updated", "Canal actualizado", "Канал обновлён", "Ալիքը թարմացված է"},
    {"SAVED", "Saved", "Guardado", "Сохранено", "Պահված"},
};

const char *tr(app::Lang lang, const char *key) {
    uint8_t li = static_cast<uint8_t>(lang) + 1;
    for (auto &row : dict) {
        if (strcmp(row[0], key) == 0) return row[li];
    }
    return key;
}

static void keyboard_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL || code == LV_EVENT_DEFOCUSED) {
        ui_keyboard_hide();
    }
}

void ui_keyboard_hide() {
    if (g_keyboard) {
        lv_obj_add_flag(g_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_keyboardHost) {
        lv_obj_set_style_pad_bottom(g_keyboardHost, 0, 0);
    }
}

void ui_attach_keyboard(lv_obj_t *textarea) {
    if (!g_keyboard || !textarea) return;
    lv_keyboard_set_textarea(g_keyboard, textarea);

    g_keyboardHost = lv_obj_get_parent(textarea);
    lv_obj_clear_flag(g_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(g_keyboard);

    // Резервируем место снизу, чтобы поле ввода не уезжало под клавиатуру.
    if (g_keyboardHost) {
        lv_obj_set_style_pad_bottom(g_keyboardHost, 190, 0);
        lv_obj_scroll_to_view_recursive(textarea, LV_ANIM_ON);
    }
}

void ui_toast(const char *msg) {
    if (!g_toast) return;
    lv_label_set_text(g_toast, msg);
    lv_obj_clear_flag(g_toast, LV_OBJ_FLAG_HIDDEN);
    g_toastTs = millis();
}

static void build_tabs() {
    g_tabview = lv_tabview_create(lv_scr_act(), LV_DIR_BOTTOM, 64);
    lv_obj_t *home = lv_tabview_add_tab(g_tabview, tr(g_ctx->state->settings.language, "HOME"));
    lv_obj_t *zones = lv_tabview_add_tab(g_tabview, tr(g_ctx->state->settings.language, "ZONES"));
    lv_obj_t *settings = lv_tabview_add_tab(g_tabview, tr(g_ctx->state->settings.language, "SETTINGS"));

    build_dashboard_tab(home, g_ctx);
    build_manual_tab(zones, g_ctx);
    build_settings_tab(settings, g_ctx);
}

static void rebuild_ui_for_language() {
    lv_obj_clean(lv_scr_act());
    build_tabs();

    g_toast = lv_label_create(lv_scr_act());
    lv_obj_set_style_bg_color(g_toast, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_text_color(g_toast, lv_color_white(), 0);
    lv_obj_set_style_pad_all(g_toast, 10, 0);
    lv_obj_align(g_toast, LV_ALIGN_BOTTOM_MID, 0, -72);
    lv_obj_add_flag(g_toast, LV_OBJ_FLAG_HIDDEN);

    g_keyboard = lv_keyboard_create(lv_scr_act());
    lv_obj_set_size(g_keyboard, LV_PCT(100), 180);
    lv_obj_align(g_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(g_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(g_keyboard, keyboard_event_cb, LV_EVENT_ALL, nullptr);

    g_lastLang = g_ctx->state->settings.language;
}

void ui_init(UiContext *ctx) {
    g_ctx = ctx;
    rebuild_ui_for_language();
    ui_refresh();
}

void ui_refresh() {
    if (!g_ctx) return;

    if (g_ctx->state->settings.language != g_lastLang) {
        rebuild_ui_for_language();
    }

    refresh_dashboard(g_ctx);
    refresh_manual(g_ctx);
    refresh_settings(g_ctx);

    if (g_toast && !lv_obj_has_flag(g_toast, LV_OBJ_FLAG_HIDDEN) && millis() - g_toastTs > 1500) {
        lv_obj_add_flag(g_toast, LV_OBJ_FLAG_HIDDEN);
    }
}

} // namespace ui
