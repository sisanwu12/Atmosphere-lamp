/**
 * @file app_state.h
 * @brief Shared application state for lamp system.
 */

#ifndef __APP_STATE_H
#define __APP_STATE_H

#include <stdbool.h>

typedef enum
{
  APP_STEER_CENTER = 0,
  APP_STEER_LEFT,
  APP_STEER_RIGHT,
} app_steer_state_t;

typedef enum
{
  APP_MOTION_NORMAL = 0,
  APP_MOTION_UP,
  APP_MOTION_DOWN,
  APP_MOTION_STOP,
} app_motion_mode_t;

typedef struct
{
  app_steer_state_t steer;
  app_motion_mode_t motion;
  bool user_hint;
} app_state_snapshot_t;

void app_state_init(void);
void app_state_update_steer(app_steer_state_t state);
void app_state_update_motion(app_motion_mode_t mode);
void app_state_set_user_hint(bool enabled);
void app_state_get_snapshot(app_state_snapshot_t *out);

#endif
