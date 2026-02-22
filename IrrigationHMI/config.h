#pragma once

#include <Arduino.h>

static constexpr uint32_t APP_RS485_BAUD_DEFAULT = 115200;
static constexpr int APP_RS485_TX_PIN = 15;
static constexpr int APP_RS485_RX_PIN = 16;

static constexpr uint8_t APP_MAX_MODULES = 16;
static constexpr uint16_t APP_MAX_CHANNELS = 128;
static constexpr uint16_t APP_EVENT_LOG_CAPACITY = 200;

static constexpr uint32_t APP_LOGIC_TASK_STACK = 8192;
static constexpr uint32_t APP_COMM_TASK_STACK = 8192;
static constexpr UBaseType_t APP_LOGIC_TASK_PRIO = 2;
static constexpr UBaseType_t APP_COMM_TASK_PRIO = 3;

