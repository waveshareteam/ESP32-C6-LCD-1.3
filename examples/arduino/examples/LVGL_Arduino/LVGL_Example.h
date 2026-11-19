#pragma once

#include "LVGL_Driver.h"
#include "SD_Card.h"
#include "Wireless.h"
#include <FastLED.h>

#define EXAMPLE1_LVGL_TICK_PERIOD_MS  1000
#define WS2812B_PIN 8
#define WS2812B_NUM_LEDS 1

#define BUTTON_PIN 9
#define FLOW_LED_NUM 7
#define FLOW_LED_PINS {1, 2, 3, 16, 17, 20, 23}

void Lvgl_Example1(void);
void WS2812B_Init(void);
void WS2812B_SetColor(uint8_t r, uint8_t g, uint8_t b);
void WS2812B_StartupSequence(void);
void FlowLED_Init(void);
void FlowLED_Test(void);
void FlowLED_CheckButton(lv_timer_t * t);