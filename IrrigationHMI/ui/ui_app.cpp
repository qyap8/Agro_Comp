// Корневой UI-модуль: 3 нижние вкладки Home/Zones/Settings и общий refresh.
#include "ui_app.h"

namespace ui {

static UiContext *g_ctx = nullptr;
static lv_obj_t *g_tabview = nullptr;
static lv_obj_t *g_toast = nullptr;
static uint32_t g_toastTs = 0;

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
    {"RESCAN", "Rescan", "Escanear", "Перескан", "Վերասքան"},
    {"WIFI", "Wi-Fi", "Wi-Fi", "Wi-Fi", "Wi-Fi"},
    {"SAVED", "Saved", "Guardado", "Сохранено", "Պահված"},
};

const char *tr(app::Lang lang, const char *key) {
    uint8_t li = static_cast<uint8_t>(lang) + 1;
    for (auto &row : dict) {
        if (strcmp(row[0], key) == 0) return row[li];
    }
    return key;
}

void ui_toast(const char *msg) {
    if (!g_toast) return;
    lv_label_set_text(g_toast, msg);
    lv_obj_clear_flag(g_toast, LV_OBJ_FLAG_HIDDEN);
    g_toastTs = millis();
}

void ui_init(UiContext *ctx) {
    g_ctx = ctx;
    g_tabview = lv_tabview_create(lv_scr_act(), LV_DIR_BOTTOM, 64);

    lv_obj_t *home = lv_tabview_add_tab(g_tabview, tr(ctx->state->settings.language, "HOME"));
    lv_obj_t *zones = lv_tabview_add_tab(g_tabview, tr(ctx->state->settings.language, "ZONES"));
    lv_obj_t *settings = lv_tabview_add_tab(g_tabview, tr(ctx->state->settings.language, "SETTINGS"));

    build_dashboard_tab(home, ctx);
    build_manual_tab(zones, ctx);
    build_settings_tab(settings, ctx);

    g_toast = lv_label_create(lv_scr_act());
    lv_obj_set_style_bg_color(g_toast, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_text_color(g_toast, lv_color_white(), 0);
    lv_obj_set_style_pad_all(g_toast, 10, 0);
    lv_obj_align(g_toast, LV_ALIGN_BOTTOM_MID, 0, -72);
    lv_obj_add_flag(g_toast, LV_OBJ_FLAG_HIDDEN);

    ui_refresh();
}

void ui_refresh() {
    if (!g_ctx) return;
    refresh_dashboard(g_ctx);
    refresh_manual(g_ctx);
    refresh_settings(g_ctx);

    if (g_toast && !lv_obj_has_flag(g_toast, LV_OBJ_FLAG_HIDDEN) && millis() - g_toastTs > 1500) {
        lv_obj_add_flag(g_toast, LV_OBJ_FLAG_HIDDEN);
    }
}

} // namespace ui
