/**
 * @file app_state.c
 * @brief Shared application state implementation.
 */

#include "app_state.h"
#include "FreeRTOS.h"
#include "task.h"

static app_state_snapshot_t APP_STATE;

void app_state_init(void)
{
  taskENTER_CRITICAL();
  APP_STATE.steer = APP_STEER_CENTER;
  APP_STATE.motion = APP_MOTION_NORMAL;
  APP_STATE.user_hint = false;
  taskEXIT_CRITICAL();
}

void app_state_update_steer(app_steer_state_t state)
{
  taskENTER_CRITICAL();
  APP_STATE.steer = state;
  taskEXIT_CRITICAL();
}

void app_state_update_motion(app_motion_mode_t mode)
{
  taskENTER_CRITICAL();
  APP_STATE.motion = mode;
  taskEXIT_CRITICAL();
}

void app_state_set_user_hint(bool enabled)
{
  taskENTER_CRITICAL();
  APP_STATE.user_hint = enabled;
  taskEXIT_CRITICAL();
}

void app_state_get_snapshot(app_state_snapshot_t *out)
{
  if (out == NULL)
    return;

  taskENTER_CRITICAL();
  *out = APP_STATE;
  taskEXIT_CRITICAL();
}
