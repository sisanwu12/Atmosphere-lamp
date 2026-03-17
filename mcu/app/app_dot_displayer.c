/**
 * @file		app_dot_displayer.c
 * @brief		点阵显示应用层。
 * @author	王广平
 */

#include "app_dot_displayer.h"
#include "FreeRTOS.h"
#include "app_display_policy.h"
#include "app_state.h"
#include "bsp_max7219.h"
#include "event_bus.h"
#include "task.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

static const uint8_t APP_DOTD_LEFT_ARROW[8] = {
    0b00011000, 0b00011100, 0b11111110, 0b11111111,
    0b11111110, 0b00011100, 0b00011000, 0b00000000};

static const uint8_t APP_DOTD_RIGHT_ARROW[8] = {
    0b00011000, 0b00111000, 0b01111111, 0b11111111,
    0b01111111, 0b00111000, 0b00011000, 0b00000000};

static const uint8_t APP_DOTD_START_PATTERN[8] = {
    0b00000000, 0b01000010, 0b11100111, 0b00000000,
    0b00000000, 0b10000001, 0b01111110, 0b00000000};

static const uint8_t APP_DOTD_UP_PATTERN[8] = {
    0b00011000, 0b00111100, 0b00111100, 0b00011000,
    0b00000000, 0b00011000, 0b00011000, 0b00000000};

static const uint8_t APP_DOTD_DOWN_PATTERN[8] = {
    0b00000000, 0b00011000, 0b00011000, 0b00000000,
    0b00011000, 0b00111100, 0b00111100, 0b00011000};

static const uint8_t APP_DOTD_STOP_PATTERN[8] = {
    0b11111100, 0b10000110, 0b10000110, 0b11111100,
    0b10000000, 0b10000000, 0b10000000, 0b00000000};

static uint8_t app_dotD_LEFT[8];
static uint8_t app_dotD_RIGHT[8];
static uint8_t app_dotD_START[8];
static uint8_t app_dotD_UP[8];
static uint8_t app_dotD_DOWN[8];
static uint8_t app_dotD_STOP[8];

#ifndef APP_DOTD_DEBUG_PRINT
#define APP_DOTD_DEBUG_PRINT 1
#endif

#ifndef APP_DOTD_POWERON_TEST
#define APP_DOTD_POWERON_TEST 1
#endif

#if APP_DOTD_TURN_COUNT > 0U
static RESULT_RUN app_dotD_turn_once(const uint8_t old[8], uint8_t out[8])
{
  uint8_t r = 0;

  if (old == NULL || out == NULL)
    return ERR_RUN_ERROR_UDIP;

  for (r = 0; r < 8; r++)
  {
    uint8_t c = 0;
    uint8_t new_row = 0;
    for (c = 0; c < 8; c++)
    {
      uint8_t bit = (old[7 - c] >> r) & 1U;
      new_row |= (uint8_t)(bit << (7 - c));
    }
    out[r] = new_row;
  }

  return ERR_RUN_Finished;
}
#endif

static RESULT_Init app_dotD_pattern_init(void)
{
  memcpy(app_dotD_LEFT, APP_DOTD_LEFT_ARROW, sizeof(app_dotD_LEFT));
  memcpy(app_dotD_RIGHT, APP_DOTD_RIGHT_ARROW, sizeof(app_dotD_RIGHT));
  memcpy(app_dotD_START, APP_DOTD_START_PATTERN, sizeof(app_dotD_START));
  memcpy(app_dotD_UP, APP_DOTD_UP_PATTERN, sizeof(app_dotD_UP));
  memcpy(app_dotD_DOWN, APP_DOTD_DOWN_PATTERN, sizeof(app_dotD_DOWN));
  memcpy(app_dotD_STOP, APP_DOTD_STOP_PATTERN, sizeof(app_dotD_STOP));

#if APP_DOTD_TURN_COUNT > 0U
  uint8_t i = 0;

  for (i = 0; i < APP_DOTD_TURN_COUNT; i++)
  {
    uint8_t tmp[8];

    if (app_dotD_turn_once(app_dotD_LEFT, tmp) != ERR_RUN_Finished)
      return ERR_Init_ERROR_SPI;
    memcpy(app_dotD_LEFT, tmp, sizeof(tmp));

    if (app_dotD_turn_once(app_dotD_RIGHT, tmp) != ERR_RUN_Finished)
      return ERR_Init_ERROR_SPI;
    memcpy(app_dotD_RIGHT, tmp, sizeof(tmp));

    if (app_dotD_turn_once(app_dotD_START, tmp) != ERR_RUN_Finished)
      return ERR_Init_ERROR_SPI;
    memcpy(app_dotD_START, tmp, sizeof(tmp));

    if (app_dotD_turn_once(app_dotD_UP, tmp) != ERR_RUN_Finished)
      return ERR_Init_ERROR_SPI;
    memcpy(app_dotD_UP, tmp, sizeof(tmp));

    if (app_dotD_turn_once(app_dotD_DOWN, tmp) != ERR_RUN_Finished)
      return ERR_Init_ERROR_SPI;
    memcpy(app_dotD_DOWN, tmp, sizeof(tmp));

    if (app_dotD_turn_once(app_dotD_STOP, tmp) != ERR_RUN_Finished)
      return ERR_Init_ERROR_SPI;
    memcpy(app_dotD_STOP, tmp, sizeof(tmp));
  }
#endif

  return ERR_Init_Finished;
}

static RESULT_RUN app_dotD_render_pattern(display_pattern_t pattern)
{
  switch (pattern)
  {
  case DISPLAY_LEFT:
    return bsp_max7219_write_rows(app_dotD_LEFT);
  case DISPLAY_RIGHT:
    return bsp_max7219_write_rows(app_dotD_RIGHT);
  case DISPLAY_UP:
    return bsp_max7219_write_rows(app_dotD_UP);
  case DISPLAY_DOWN:
    return bsp_max7219_write_rows(app_dotD_DOWN);
  case DISPLAY_STOP:
    return bsp_max7219_write_rows(app_dotD_STOP);
  case DISPLAY_START:
    return bsp_max7219_write_rows(app_dotD_START);
  case DISPLAY_NONE:
  default:
    return bsp_max7219_clear();
  }
}

RESULT_Init app_dotD_Init(void)
{
  RESULT_Init ret = bsp_max7219_init();
  if (ret != ERR_Init_Finished)
    return ret;

  return app_dotD_pattern_init();
}

void app_dotD_dispose_Task(void)
{
  EventGroupHandle_t evt = event_bus_getHandle();

#if APP_DOTD_DEBUG_PRINT
  printf("[DOT] task start\r\n");
#endif

#if APP_DOTD_POWERON_TEST
  (void)bsp_max7219_set_test_mode(true);
  vTaskDelay(pdMS_TO_TICKS(200));
  (void)bsp_max7219_set_test_mode(false);
#endif

  (void)app_dotD_render_pattern(DISPLAY_START);
  vTaskDelay(pdMS_TO_TICKS(200));
  (void)app_dotD_render_pattern(DISPLAY_NONE);

  while (1)
  {
    app_state_snapshot_t snapshot;
    display_pattern_t pattern;
    RESULT_RUN show_ret;

    (void)xEventGroupWaitBits(evt, SIG_DISPLAY_UPDATE, pdTRUE, pdFALSE,
                              portMAX_DELAY);

    app_state_get_snapshot(&snapshot);
    pattern = app_display_policy_resolve(&snapshot);
    show_ret = app_dotD_render_pattern(pattern);

#if APP_DOTD_DEBUG_PRINT
    printf("[DOT] render=%d steer=%d motion=%d hint=%d\r\n", (int)pattern,
           (int)snapshot.steer, (int)snapshot.motion, snapshot.user_hint ? 1 : 0);
    if (show_ret != ERR_RUN_Finished)
      printf("[DOT] show error=%d\r\n", (int)show_ret);
#endif
  }
}
