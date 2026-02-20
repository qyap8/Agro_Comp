/**
 * Minimal LVGL config for Arduino + ESP32-S3 + RGB565.
 * This file is intentionally lightweight to resolve
 * `fatal error: ../../lv_conf.h: No such file or directory`
 * in Arduino IDE builds.
 */

#ifndef LV_CONF_H
#define LV_CONF_H

/*====================
   COLOR SETTINGS
 *====================*/
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0

/*=========================
   MEMORY / PERFORMANCE
 *========================*/
#define LV_MEM_CUSTOM 0
#define LV_MEM_SIZE (128U * 1024U)

#define LV_USE_PERF_MONITOR 0
#define LV_USE_MEM_MONITOR 0

/*====================
   TICK / TIMERS
 *====================*/
#define LV_TICK_CUSTOM 0

/*====================
   DRAWING
 *====================*/
#define LV_DRAW_COMPLEX 1

/*====================
   LOG
 *====================*/
#define LV_USE_LOG 0

/*====================
   OS SUPPORT
 *====================*/
#define LV_USE_OS LV_OS_NONE

/*====================
   FONTS
 *====================*/
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1

/*====================
   THEMES
 *====================*/
#define LV_USE_THEME_DEFAULT 1

/*====================
   WIDGETS USED HERE
 *====================*/
#define LV_USE_LABEL 1
#define LV_USE_BTN 1
#define LV_USE_LIST 1
#define LV_USE_SLIDER 1
#define LV_USE_TABVIEW 1

#endif /*LV_CONF_H*/
