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

#include "util/log.h"
#include "util/types.h"
#include "util/error_handler.h"

#define STD_NOISE_BARO   900000.0f    // From data analysis: 2.6f
#define STD_NOISE_BARO_INITIAL   9.0f    // From data analysis: 2.6f
#define STD_NOISE_IMU    0.004f  // From data analysis: 0.004f
#define STD_NOISE_OFFSET 0.000001f

void init_filter_struct(kalman_filter_t *filter);

void initialize_matrices(kalman_filter_t *filter);

void kalman_prediction(kalman_filter_t *filter);

void reset_kalman(kalman_filter_t *filter);

void kalman_update(kalman_filter_t *filter);

void kalman_step(kalman_filter_t *filter, flight_fsm_e flight_state);
