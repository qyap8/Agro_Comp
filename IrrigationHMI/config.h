#pragma once

#include <Arduino.h>

static constexpr uint32_t APP_RS485_BAUD_DEFAULT = 115200;
static constexpr int APP_RS485_TX_PIN = 15;
static constexpr int APP_RS485_RX_PIN = 16;

static constexpr uint8_t APP_MAX_ZONES = 16;
static constexpr uint8_t APP_MAX_MODULES = 32;
static constexpr uint8_t APP_MAX_SCHEDULES = 16;
static constexpr uint16_t APP_EVENT_LOG_CAPACITY = 200;

static constexpr uint16_t APP_DEFAULT_PULSE_WIDTH_MS = 200;
static constexpr uint16_t APP_MIN_PULSE_WIDTH_MS = 150;
static constexpr uint16_t APP_MAX_PULSE_WIDTH_MS = 300;

static constexpr uint32_t APP_LOGIC_TASK_STACK = 6144;
static constexpr uint32_t APP_COMM_TASK_STACK = 6144;
static constexpr UBaseType_t APP_LOGIC_TASK_PRIO = 2;
static constexpr UBaseType_t APP_COMM_TASK_PRIO = 3;

