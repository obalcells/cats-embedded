/*
 * CATS Flight Software
 * Copyright (C) 2021 Control and Telemetry Systems
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "util/types.h"

#include <stdbool.h>

#define BUZZER_COMMAND_MAX_LENGTH 9

#define BUZZER_SHORT_BEEP  100
#define BUZZER_LONG_BEEP   400
#define BUZZER_SHORT_PAUSE 200
#define BUZZER_LONG_PAUSE  1000

typedef enum {
  CATS_BUZZ_NONE = 0,
  CATS_BUZZ_BOOTUP,
  CATS_BUZZ_READY,
  CATS_BUZZ_CHANGED_MOVING,
  CATS_BUZZ_CHANGED_READY,
  } buzzer_status_e;

void buzzer_handler_update();
bool buzzer_queue_status(buzzer_status_e status);
