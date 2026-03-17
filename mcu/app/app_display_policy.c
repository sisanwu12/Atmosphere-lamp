/**
 * @file app_display_policy.c
 * @brief Display policy implementation.
 */

#include "app_display_policy.h"
#include <stddef.h>

display_pattern_t app_display_policy_resolve(
    const app_state_snapshot_t *snapshot)
{
  if (snapshot == NULL)
    return DISPLAY_NONE;

  if (snapshot->steer == APP_STEER_LEFT)
    return DISPLAY_LEFT;
  if (snapshot->steer == APP_STEER_RIGHT)
    return DISPLAY_RIGHT;

  switch (snapshot->motion)
  {
  case APP_MOTION_STOP:
    return DISPLAY_STOP;
  case APP_MOTION_DOWN:
    return DISPLAY_DOWN;
  case APP_MOTION_UP:
    return DISPLAY_UP;
  case APP_MOTION_NORMAL:
  default:
    break;
  }

  if (snapshot->user_hint)
    return DISPLAY_START;

  return DISPLAY_NONE;
}
