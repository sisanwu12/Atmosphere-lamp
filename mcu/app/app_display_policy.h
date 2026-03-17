/**
 * @file app_display_policy.h
 * @brief Display policy derived from shared application state.
 */

#ifndef __APP_DISPLAY_POLICY_H
#define __APP_DISPLAY_POLICY_H

#include "app_state.h"

typedef enum
{
  DISPLAY_NONE = 0,
  DISPLAY_LEFT,
  DISPLAY_RIGHT,
  DISPLAY_UP,
  DISPLAY_DOWN,
  DISPLAY_STOP,
  DISPLAY_START,
} display_pattern_t;

display_pattern_t app_display_policy_resolve(
    const app_state_snapshot_t *snapshot);

#endif
