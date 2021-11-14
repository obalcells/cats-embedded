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

#include "util/actions.h"

#include "config/cats_config.h"
#include "config/globals.h"
#include "flash/fs.h"
#include "drivers/servo.h"

bool no_action_function(__attribute__((unused)) int16_t bummer);

bool os_delay(int16_t ticks);

bool high_current_channel_one(int16_t state);
bool high_current_channel_two(int16_t state);
bool high_current_channel_three(int16_t state);
bool high_current_channel_four(int16_t state);  // reserved for later use
bool high_current_channel_five(int16_t state);  // reserved for later use
bool high_current_channel_six(int16_t state);   // reserved for later use

bool low_level_channel_one(int16_t state);
bool low_level_channel_two(int16_t state);
bool low_level_channel_three(int16_t state);  // reserved for later use
bool low_level_channel_four(int16_t state);   // reserved for later use

bool servo_channel_one(int16_t angle);
bool servo_channel_two(int16_t angle);
bool servo_channel_three(int16_t angle);  // reserved for later use
bool servo_channel_four(int16_t angle);   // reserved for later use

const peripheral_act_fp action_table[NUM_ACTION_FUNCTIONS] = {no_action_function,         os_delay,
                                                              high_current_channel_one,   high_current_channel_two,
                                                              high_current_channel_three, high_current_channel_four,
                                                              high_current_channel_five,  high_current_channel_six,
                                                              low_level_channel_one,      low_level_channel_two,
                                                              low_level_channel_three,    low_level_channel_four,
                                                              servo_channel_one,          servo_channel_two,
                                                              servo_channel_three,        servo_channel_four,
                                                              set_recorder_state};

bool no_action_function(__attribute__((unused)) int16_t bummer) {
  // Sucks to be here...
  // it seems like someone didn't configure the actions right
  return false;
}

/* Be careful when setting these delays, right now the peripheral
 * task will sleep until this delay is finished, meaning that it
 * won't read from the queue until the delay is done. If you set too
 * long of a delay you might execute the next event too late, even
 * if the event came at the right time.
 *
 * Right now this is considered a feature and not a bug since we
 * assume the users know what they are doing when setting up these
 * delays. */
bool os_delay(int16_t ticks) {
  if (ticks > 0) {
    osDelay((uint32_t)ticks);
    return true;
  }
  return false;
}

// High current outputs for pyros, valves etc.
bool high_current_channel_one(int16_t state) {
  if (state == 0 || state == 1) {
    HAL_GPIO_WritePin(PYRO1_GPIO_Port, PYRO1_Pin, (GPIO_PinState)state);
    return true;
  }
  return false;
}

bool high_current_channel_two(int16_t state) {
  if (state == 0 || state == 1) {
    HAL_GPIO_WritePin(PYRO2_GPIO_Port, PYRO2_Pin, (GPIO_PinState)state);
    return true;
  }
  return false;
}

bool high_current_channel_three(int16_t state) {
  if (state == 0 || state == 1) {
    HAL_GPIO_WritePin(PYRO3_GPIO_Port, PYRO3_Pin, (GPIO_PinState)state);
    return true;
  }
  return false;
}

bool high_current_channel_four(int16_t state) {
  if (state == 0 || state == 1) {
    // HAL_GPIO_WritePin(PYRO_3_GPIO_Port, PYRO_3_Pin,  (GPIO_PinState)state);
    return true;
  }
  return false;
}

bool high_current_channel_five(int16_t state) {
  if (state == 0 || state == 1) {
    // HAL_GPIO_WritePin(PYRO_3_GPIO_Port, PYRO_3_Pin,  (GPIO_PinState)state);
    return true;
  }
  return false;
}

bool high_current_channel_six(int16_t state) {
  if (state == 0 || state == 1) {
    // HAL_GPIO_WritePin(PYRO_3_GPIO_Port, PYRO_3_Pin,  (GPIO_PinState)state);
    return true;
  }
  return false;
}

// Low level (3.3V) outputs

bool low_level_channel_one(int16_t state) {
  if (state == 0 || state == 1) {
    HAL_GPIO_WritePin(IO1_GPIO_Port, IO1_Pin, (GPIO_PinState)state);
    return true;
  }
  return false;
}

bool low_level_channel_two(int16_t state) {
  if (state == 0 || state == 1) {
    HAL_GPIO_WritePin(IO2_GPIO_Port, IO2_Pin, (GPIO_PinState)state);
    return true;
  }
  return false;
}

// Same as servo 1 but digital output
bool low_level_channel_three(int16_t state) {
  if (state == 0 || state == 1) {
    servo_set_onoff(&SERVO1, state);
    return true;
  }
  return false;
}

// Same as servo 2 but digital output
bool low_level_channel_four(int16_t state) {
  if (state == 0 || state == 1) {
    servo_set_onoff(&SERVO2, (bool)state);
    return true;
  }
  return false;
}

// Servo Outputs

bool servo_channel_one(int16_t angle) {
  if (angle >= 0 && angle <= 180) {
    servo_set_position(&SERVO1, angle);
    return true;
  }
  return false;
}

bool servo_channel_two(int16_t angle) {
  if (angle >= 0 && angle <= 180) {
    servo_set_position(&SERVO2, angle);
    return true;
  }
  return false;
}

bool servo_channel_three(int16_t angle) {
  if (angle >= 0 && angle <= 180) {
    // servo_set_position(&SERVO1, angle);
    return true;
  }
  return false;
}

bool servo_channel_four(int16_t angle) {
  if (angle >= 0 && angle <= 180) {
    // servo_set_position(&SERVO1, angle);
    return true;
  }
  return false;
}

/* TODO check if mutex should be used here */
bool set_recorder_state(int16_t state) {
  volatile recorder_status_e new_rec_state = (recorder_status_e)state;
  if (new_rec_state < REC_OFF || new_rec_state > REC_WRITE_TO_FLASH) {
    return false;
  }

  rec_cmd_type_e rec_cmd = REC_CMD_INVALID;

  switch (global_recorder_status) {
    case REC_OFF:
      if (new_rec_state == REC_FILL_QUEUE) {
        rec_cmd = REC_CMD_FILL_Q;
      } else if (new_rec_state == REC_WRITE_TO_FLASH) {
        rec_cmd = REC_CMD_WRITE;
      }
      break;
    case REC_FILL_QUEUE:
      if (new_rec_state == REC_OFF) {
        rec_cmd = REC_CMD_FILL_Q_STOP;
      } else if (new_rec_state == REC_WRITE_TO_FLASH) {
        rec_cmd = REC_CMD_WRITE;
      }
      break;
    case REC_WRITE_TO_FLASH:
      if (new_rec_state == REC_OFF || new_rec_state == REC_FILL_QUEUE) {
        rec_cmd = REC_CMD_WRITE_STOP;
        osStatus_t ret = osMessageQueuePut(rec_cmd_queue, &rec_cmd, 0U, 10U);
        if (ret != osOK) {
          log_error("Inserting an element to the recorder command queue failed! Error: %d", ret);
        }
        if (new_rec_state == REC_FILL_QUEUE) {
          rec_cmd = REC_CMD_FILL_Q;
        }
      }
      break;
    default:
      log_error("Invalid global recorder status!");
      return false;
  }

  global_recorder_status = new_rec_state;

  if (rec_cmd != REC_CMD_INVALID) {
    osStatus_t ret = osMessageQueuePut(rec_cmd_queue, &rec_cmd, 0U, 10U);
    if (ret != osOK) {
      log_error("Inserting an element to the recorder command queue failed! Error: %d", ret);
    }
  }

  return true;
}
