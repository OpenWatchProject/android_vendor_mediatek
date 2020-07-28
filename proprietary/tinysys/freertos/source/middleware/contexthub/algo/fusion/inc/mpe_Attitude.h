/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
/* Usage:
 * 1. Fill in struct IMU
 * 2. Call mpe_update_posture()
 * 3. Call mpe_get_euler_angle() or mpe_get_rotation_vector() to get
 *    <azimuth, pitch, roll> or the rotation vector.
 *
 * E.g.
 *      struct IMU imu;
 *
 *      // Fill in the acceleration value from the accelerometer
 *      imu.acceleration[0] = acc_x;
 *      imu.acceleration[1] = acc_y;
 *      imu.acceleration[2] = acc_z;
 *
 *      // Fill in the angular velocity from the gyroscope
 *      imu.angularVelocity[0] = gyro_x;
 *      imu.angularVelocity[1] = gyro_y;
 *      imu.angularVelocity[2] = gyro_z;
 *
 *      // Fill in magnetic field values from magnetic sensor
 *      imu.magnetic[0] = mag_x;
 *      imu.magnetic[1] = mag_y;
 *      imu.magnetic[2] = mag_z;
 *
 *      mpe_update_posture(&imu, (curr_time - prev_time));
 *
 *  #ifdef ANDROID_2_3
 *      float vector[3];
 *      mpe_get_rotation_vector(vector);
 *  #else
 *      float angle[3];
 *      mpe_get_euler_angle(angle);
 *  #endif
 */
#ifndef __MPE_ATTITUDE_H_INCLUDED__
#define __MPE_ATTITUDE_H_INCLUDED__

#include <sys/types.h>
#include <stdint.h>

#define use_speciall_name 0
/* Sensor Fusion Mode */
/*
 * 1) low latency (real time)
 * 2) 3-DOF rotation detection
 * 3) eliminate accumulated error
 * 4) Solve gyroscope temperature drift issue
 * 5) decouple linear acceleration and gravity of G-sensor
 * 6) provide absolute orientation (for applications like : navigation)
 * 7) anti-interference from sudden change of ambient magnetism
 */
#define SENSOR_FUSION_MODE_ACC_GYRO_MAG 0
/*
 * 1) Need AGM and AG output in the same time
 * 2) Provide both absolute yaw angle and reletive
 * 3) Support Android two sensor types
 */
#define SENSOR_FUSION_MODE_BOTH_AGM_AG 1

/**
 * 1) unbounded accumulate error
 * 2) temperature drift issue from gyro
 * 3) relative orientation (NOT absolute)
 */
#define SENSOR_FUSION_MODE_ACC_GYRO   2
/*
 * 1) delayed response
 * 2) can not resist the magnetic interference
 * 3) confuse by linear acceleration and gravity
 */
#define SENSOR_FUSION_MODE_ACC_MAG    3

/*
 * 1) use for pedestrian dead reckoning
 * 2) fused with radio localization system (like : GPS/WF)
 */
#define SENSOR_FUSION_MODE_PDR        4



#ifdef __cplusplus
extern "C" {
#endif

/*
 * Definitions of the axis
 *
 *          |   /
 *          |  /
 *          | /
 *     ---- |/ ------ x+
 *          /
 *         /|
 *        / |
 *       /  y-
 *     z+
 */
/*
 * The input data structure for the Sensor Fusion algorithm
 */
typedef struct IMU {
    /* acceleration values are in meter per second per second (m/s^2) */
    float acceleration[3];
    float acceleration_raw[3];
    /* angular rate values are in radians per second */
    float  angularVelocity[3];
    float  angularVelocity_raw[3];
    /* magnetic vector values are in micro-Tesla (uT) */
    float magnetic[3];
    float magnetic_raw[3];

    //float light[3];
    //float proximity[3];
    float thermometer;
    float pressure;
    int64_t input_time_acc;
    int64_t input_time_gyro;
    int64_t input_time_mag;
    int64_t input_time_baro;

} IMU, *LPIMU;

/*
 * Updates the posture of the device
 *
 * @param pImu A pointer to input data structure.
 * @param deltaTime The time difference between calls. The unit is millisecond.
 */
int mpe_update_posture(LPIMU pImu, int deltaTime);
/*
 * Filter the raw data of the gyroscope
 *
 * @param pImu A pointer to input data structure.
 * @param threshold Low-pass filter threshold. The unit is degree.
 */
void mpe_gyro_filter(LPIMU pImu, float threshold);
/*
 * Returns the rotation vector of the device
 *
 * The rotation vector represents the orientation of the device as a
 * combination of an angle and an axis, in which the device has rotated
 * through an angle theta around an axis [x, y, z]. The three elements
 * of the rotation vector are
 *
 * vector = [x*sin(theta/2), y*sin(theta/2), z*sin(theta/2]
 *
 * @param vector The result rotation vector
 * @return 0 on success, < 0 on failure
 */
int mpe_get_rotation_vector(float *vector);
/*
 * Returns a gravity vector
 *
 * Gravity vector provides faster tilt angle change detection.
 *
 * @param gravity a 3-element array contains the result gravity vector
 * @return 0 on success, < 0 on failure
 */
int mpe_get_gravity(float *gravity);
/*
 * Returns a acceleration vector without gravity
 *
 * Linear acceleration decouples gravity so that applications can get more
 * faster and precise motion.
 *
 * @param acceleration a 3-element array contains the result acceleration vector
 * @return 0 on success, < 0 on failure
 */
int mpe_get_linear_acceleration(float *acceleration);
/*
 * Returns euler angles of the device.
 *
 * Euler angles are used to describe the orientation of a rigid body. All
 * values are angles in degrees.
 * The three elements of euler angles are [azimuth, pitch, roll].
 *
 * @param angles a 3-element array contains the result euler angles
 * @return 0 on success, < 0 on failure
 */
int mpe_get_euler_angle(float *angles);

int mpe_get_game_rotation_vector(float *vector);
int mpe_get_rotation_matrix(float *matrix);
void mpe_set_fusion_mode(int mode);
int mpe_re_initialize(void);
int mpe_calculate_GEO_rotation_vector(float *gravity, float *geomagnetic, float *vector, float *euler);
int mpe_get_global_rotation(LPIMU pImu, float *rotation);

#ifdef __cplusplus
}
#endif

#endif /* __MPE_ATTITUDE_H_INCLUDED__ */

