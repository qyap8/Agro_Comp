// Корневой UI-модуль: создаёт вкладки и централизованно обновляет экраны.
#include "ui_app.h"

namespace ui {

static UiContext *g_ctx = nullptr;
static lv_obj_t *tabview = nullptr;

void build_dashboard_tab(lv_obj_t *parent, UiContext *ctx);
void refresh_dashboard(UiContext *ctx);
void build_manual_tab(lv_obj_t *parent, UiContext *ctx);
void refresh_manual(UiContext *ctx);
void build_schedules_tab(lv_obj_t *parent, UiContext *ctx);
void refresh_schedules(UiContext *ctx);
void build_modules_tab(lv_obj_t *parent, UiContext *ctx);
void refresh_modules(UiContext *ctx);
void build_settings_tab(lv_obj_t *parent, UiContext *ctx);
void refresh_settings(UiContext *ctx);
void build_diagnostics_tab(lv_obj_t *parent, UiContext *ctx);
void refresh_diagnostics(UiContext *ctx);

void ui_init(UiContext *ctx) {
    g_ctx = ctx;
    tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 56);

    build_dashboard_tab(lv_tabview_add_tab(tabview, "Dashboard"), ctx);
    build_manual_tab(lv_tabview_add_tab(tabview, "Manual"), ctx);
    build_schedules_tab(lv_tabview_add_tab(tabview, "Schedules"), ctx);
    build_modules_tab(lv_tabview_add_tab(tabview, "Modules"), ctx);
    build_settings_tab(lv_tabview_add_tab(tabview, "Settings"), ctx);
    build_diagnostics_tab(lv_tabview_add_tab(tabview, "Diag"), ctx);

    ui_refresh();
}

void ui_refresh() {
    if (!g_ctx) return;
    refresh_dashboard(g_ctx);
    refresh_manual(g_ctx);
    refresh_schedules(g_ctx);
    refresh_modules(g_ctx);
    refresh_settings(g_ctx);
    refresh_diagnostics(g_ctx);
}

} // namespace ui
