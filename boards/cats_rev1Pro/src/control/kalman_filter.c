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

#include "control/kalman_filter.h"
#include "cmsis_os.h"
#include <string.h>
#include <arm_neon.h>
#include <math.h>
#include "util/error_handler.h"

void init_filter_struct(kalman_filter_t *const filter) {
  arm_mat_init_f32(&filter->Ad, 3, 3, filter->Ad_data);
  arm_mat_init_f32(&filter->Ad_T, 3, 3, filter->Ad_T_data);
  arm_mat_init_f32(&filter->Bd, 3, 1, filter->Bd_data);
  arm_mat_init_f32(&filter->GdQGd_T, 3, 3, filter->GdQGd_T_data);
  arm_mat_init_f32(&filter->H, 1, 3, filter->H_data);
  arm_mat_init_f32(&filter->H_T, 3, 1, filter->H_T_data);
  arm_mat_init_f32(&filter->K, 3, 1, filter->K_data);
  arm_mat_init_f32(&filter->x_hat, 3, 1, filter->x_hat_data);
  arm_mat_init_f32(&filter->x_bar, 3, 1, filter->x_bar_data);
  arm_mat_init_f32(&filter->P_hat, 3, 3, filter->P_hat_data);
  arm_mat_init_f32(&filter->P_bar, 3, 3, filter->P_bar_data);
}

void initialize_matrices(kalman_filter_t *const filter) {
  /* Matrix -> mat[9] = [0, 1, 2; 3, 4 , 5; 6, 7, 8];*/

  /* Initialize static values */
  float32_t Ad[9] = {1, filter->t_sampl, filter->t_sampl * filter->t_sampl / 2, 0, 1, filter->t_sampl, 0, 0, 1};
  arm_matrix_instance_f32 Ad_mat;
  arm_mat_init_f32(&Ad_mat, 3, 3, Ad);

  float32_t Ad_T[9] = {1, filter->t_sampl, filter->t_sampl * filter->t_sampl / 2, 0, 1, filter->t_sampl, 0, 0, 1};
  arm_matrix_instance_f32 Ad_T_mat;
  arm_mat_init_f32(&Ad_T_mat, 3, 3, Ad_T);
  arm_mat_trans_f32(&Ad_mat, &Ad_T_mat);

  float32_t Gd[6] = {filter->t_sampl, filter->t_sampl * filter->t_sampl / 2, 1, filter->t_sampl, 0, 1};
  // float32_t Gd[6] = {filter->t_sampl, 0, 1, 0, 0, 0};
  arm_matrix_instance_f32 Gd_mat;
  arm_mat_init_f32(&Gd_mat, 3, 2, Gd);

  float32_t Gd_T[6];
  arm_matrix_instance_f32 Gd_T_mat;
  arm_mat_init_f32(&Gd_T_mat, 2, 3, Gd_T);
  arm_mat_trans_f32(&Gd_mat, &Gd_T_mat);


  float32_t Bd[3] = {filter->t_sampl * filter->t_sampl / 2, filter->t_sampl, 0};
  arm_matrix_instance_f32 Bd_mat;
  arm_mat_init_f32(&Bd_mat, 3, 1, Bd);

  float32_t H[3] = {1, 0, 0};
  arm_matrix_instance_f32 H_mat;
  arm_mat_init_f32(&H_mat, 1, 3, H);

    float32_t H_T[3] = {1, 0, 0};
    arm_matrix_instance_f32 H_T_mat;
    arm_mat_init_f32(&H_T_mat, 3, 1, H_T);

  float32_t Q[4] = {STD_NOISE_IMU, 0, 0, STD_NOISE_OFFSET};
  arm_matrix_instance_f32 Q_mat;
  arm_mat_init_f32(&Q_mat, 2, 2, Q);

  float32_t GdQGd_T[9];
  arm_matrix_instance_f32 GdQGd_T_mat;
  arm_mat_init_f32(&GdQGd_T_mat, 3, 3, GdQGd_T);

  float32_t holder[6];
  arm_matrix_instance_f32 holder_mat;
  arm_mat_init_f32(&holder_mat, 3, 2, holder);

  arm_mat_mult_f32(&Gd_mat, &Q_mat, &holder_mat);
  arm_mat_mult_f32(&holder_mat, &Gd_T_mat, &GdQGd_T_mat);

  float32_t x_bar[3] = {0, 0, 0};
  arm_matrix_instance_f32 x_bar_mat;
  arm_mat_init_f32(&x_bar_mat, 3, 1, x_bar);

  float32_t x_hat[3] = {0, 0, 0};
  arm_matrix_instance_f32 x_hat_mat;
  arm_mat_init_f32(&x_hat_mat, 3, 1, x_hat);

    float32_t K[3] = {0, 0, 0};
    arm_matrix_instance_f32 K_mat;
    arm_mat_init_f32(&K_mat, 3, 1, K);


    float32_t P_hat[9] = {0.1f, 0, 0, 0, 0.1f, 0, 0, 0, 0.1f};
    arm_matrix_instance_f32 P_hat_mat;
    arm_mat_init_f32(&P_hat_mat, 3, 3, P_hat);

    float32_t P_bar[9] = {0.1f, 0, 0, 0, 0.1f, 0, 0, 0, 0.1f};
    arm_matrix_instance_f32 P_bar_mat;
    arm_mat_init_f32(&P_bar_mat, 3, 3, P_bar);

    filter->R = STD_NOISE_BARO;
  memcpy(filter->Ad_data, Ad, sizeof(Ad));
  memcpy(filter->Ad_T_data, Ad_T, sizeof(Ad_T));
  memcpy(filter->Bd_data, Bd, sizeof(Bd));
  memcpy(filter->GdQGd_T_data, GdQGd_T, sizeof(GdQGd_T));
  memcpy(filter->H_data, H, sizeof(H));
  memcpy(filter->H_T_data, H_T, sizeof(H_T));
  memcpy(filter->K_data, K, sizeof(K));
  memcpy(filter->x_bar_data, x_bar, sizeof(x_bar));
  memcpy(filter->x_hat_data, x_hat, sizeof(x_hat));
  memcpy(filter->P_bar_data, P_bar, sizeof(P_bar));
  memcpy(filter->P_hat_data, P_hat, sizeof(P_hat));
}

void reset_kalman(kalman_filter_t *filter) {
  log_debug("Resetting Kalman Filter...");
  float32_t x_dash[3] = {0, 0.0f, 0};
  float32_t P_dash[9] = {0.1f, 0, 0, 0, 0.1f, 0, 0, 0, 0.1f};

  memcpy(filter->x_bar_data, x_dash, sizeof(x_dash));
  memcpy(filter->x_bar_data, x_dash, sizeof(x_dash));
  memcpy(filter->P_bar_data, P_dash, sizeof(P_dash));
  memcpy(filter->P_bar_data, P_dash, sizeof(P_dash));
}

void thrust_reset_kalman(kalman_filter_t *filter) {
    log_debug("Resetting Kalman Filter...");
    float32_t x_dash[3] = {0, 0.0f, 0};
    float32_t P_dash[9] = {0};
    memcpy(P_dash, filter->P_bar_data, sizeof(filter->P_bar_data));
    P_dash[0] = 0.1f;
    P_dash[1] = 0.0f;
    P_dash[2] = 0.0f;
    P_dash[3] = 0.0f;
    P_dash[4] = 0.1f;
    P_dash[5] = 0.0f;
    P_dash[6] = 0.0f;
    P_dash[7] = 0.0f;

    memcpy(filter->x_hat_data, x_dash, sizeof(x_dash));
    memcpy(filter->x_bar_data, x_dash, sizeof(x_dash));
    memcpy(filter->P_hat_data, P_dash, sizeof(P_dash));
    memcpy(filter->P_bar_data, P_dash, sizeof(P_dash));
}

/* This Function Implements the kalman Prediction as long as more than 0 IMU
 * work */
void kalman_prediction(kalman_filter_t *filter) {


  float32_t holder_data[3];
  arm_matrix_instance_f32 holder_vec;
  arm_mat_init_f32(&holder_vec, 3, 1, holder_data);

  float32_t holder2_data[3];
  arm_matrix_instance_f32 holder2_vec;
  arm_mat_init_f32(&holder2_vec, 3, 1, holder2_data);

    float32_t holder[9];
    arm_matrix_instance_f32 holder_mat;
    arm_mat_init_f32(&holder_mat, 3, 3, holder);

    float32_t holder2[9];
    arm_matrix_instance_f32 holder2_mat;
    arm_mat_init_f32(&holder2_mat, 3, 3, holder2);

  /* Prediction Step */

  /* Calculate Prediction of the state: x_hat = A*x_bar + B*u */
  arm_mat_mult_f32(&filter->Ad, &filter->x_bar, &holder_vec);
  arm_mat_scale_f32(&filter->Bd, filter->measured_acceleration, &holder2_vec);
  arm_mat_add_f32(&holder_vec, &holder2_vec, &filter->x_hat);

    /* Update the Variance of the state P_hat = A*P_bar*A' + GQG' */
    arm_mat_mult_f32(&filter->Ad, &filter->P_bar, &holder_mat);
    arm_mat_mult_f32(&holder_mat, &filter->Ad_T, &holder2_mat);
    arm_mat_add_f32(&holder2_mat, &filter->GdQGd_T, &filter->P_hat);

  /* Prediction Step finished */
}

/* This function implements the Kalman update when no Barometer is faulty */
void kalman_update(kalman_filter_t *filter) {

  float32_t holder_single[1];
  arm_matrix_instance_f32 holder_single_mat;
  arm_mat_init_f32(&holder_single_mat, 1, 1, holder_single);

  float32_t holder2_data[3];
  arm_matrix_instance_f32 holder2_vec;
  arm_mat_init_f32(&holder2_vec, 3, 1, holder2_data);

    float32_t holder_0_1x3[3];
    arm_matrix_instance_f32 holder_0_1x3_mat;
    arm_mat_init_f32(&holder_0_1x3_mat, 1, 3, holder_0_1x3);

  /* Update Step */

    /* Calculate K = P_hat*H_T*(H*P_Hat*H_T+R)^-1 */
    arm_mat_mult_f32(&filter->H, &filter->P_hat, &holder_0_1x3_mat);
    arm_mat_mult_f32(&holder_0_1x3_mat, &filter->H_T, &holder_single_mat);
    holder_single[0] += filter->R;

    arm_mat_mult_f32(&filter->P_hat, &filter->H_T, &holder2_vec);
    arm_mat_scale_f32(&holder2_vec, 1.0f/holder_single[0], &filter->K);

  /* Calculate x_bar = x_hat+K*(y-Hx_hat); */

  arm_mat_mult_f32(&filter->H, &filter->x_hat, &holder_single_mat);
  arm_mat_scale_f32(&filter->K, filter->measured_AGL-holder_single[0], &holder2_vec);
  arm_mat_add_f32(&holder2_vec, &filter->x_hat, &filter->x_bar);

  /* Finished Calculating x_bar */

    /* Calculate P_bar = (eye-K*H)*P_hat */
    float32_t eye[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
    arm_matrix_instance_f32 eye_mat;
    arm_mat_init_f32(&eye_mat, 3, 3, eye);
    float32_t holder_3x3[9];
    arm_matrix_instance_f32 holder_3x3_mat;
    arm_mat_init_f32(&holder_3x3_mat, 3, 3, holder_3x3);
    float32_t holder2_3x3[9];
    arm_matrix_instance_f32 holder2_3x3_mat;
    arm_mat_init_f32(&holder2_3x3_mat, 3, 3, holder2_3x3);

    arm_mat_mult_f32(&filter->K, &filter->H, &holder_3x3_mat);
    arm_mat_sub_f32(&eye_mat, &holder_3x3_mat, &holder2_3x3_mat);
    arm_mat_mult_f32(&holder2_3x3_mat, &filter->P_hat, &filter->P_bar);
}

float32_t R_interpolation(float32_t velocity) {
    float32_t lower_bound = 20;
    float32_t upper_bound = 100;
    float32_t f_lower_bound = 0.1f;
    float32_t f_upper_bound = 1;


    float32_t m = (f_lower_bound - f_upper_bound) / (lower_bound - upper_bound);
    float32_t b = f_upper_bound - m * upper_bound;
    if (velocity < lower_bound){
        return powf(f_lower_bound, 5.0f);
    }
    else if(velocity < upper_bound){
        return powf(m * velocity + b, 5.0f);
    }
    else{
        return f_upper_bound;
    }

}



void kalman_step(kalman_filter_t *filter, flight_fsm_e flight_state) {

    /* Update R Value */
    switch (flight_state) {
        case READY:
            filter->R = STD_NOISE_BARO_INITIAL;
            break;
        case MOVING:
            filter->R = STD_NOISE_BARO_INITIAL;
            break;
        case THRUSTING_1:
            filter->R = STD_NOISE_BARO;
            thrust_reset_kalman(filter);
            break;
        case COASTING:
            filter->R = STD_NOISE_BARO*R_interpolation(filter->x_bar_data[1]);
            break;
        default:
            break;
    }

    /* if all ACCELS have been eliminated, only work ith the BARO */
    if(get_error_by_tag(CATS_ERR_FILTER_ACC) == CATS_ERR_OK){
        kalman_prediction(filter);
    }
    else{
      memcpy(filter->x_hat_data, filter->x_bar_data, sizeof(filter->x_bar_data));
      memcpy(filter->P_hat_data, filter->P_bar_data, sizeof(filter->P_bar_data));
    }

/* if all BAROS have been eliminated, only work ith the ACCEL */
    if(get_error_by_tag(CATS_ERR_FILTER_HEIGHT) == CATS_ERR_OK){
        kalman_update(filter);
    }
    else{
        memcpy(filter->x_bar_data, filter->x_hat_data, sizeof(filter->x_hat_data));
        memcpy(filter->P_bar_data, filter->P_hat_data, sizeof(filter->P_hat_data));
    }

}
