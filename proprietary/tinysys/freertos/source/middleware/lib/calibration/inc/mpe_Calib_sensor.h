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
#ifndef __MPE_CALIB_SENSOR_H_INCLUDED__
#define __MPE_CALIB_SENSOR_H_INCLUDED__

#include <string.h>
#include "mpe_math.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MPECALIBRATION {
	/* acceleration values are in meter per second per second (m/s^2) */
	float acceleration_bias[3];
	int acceleration_accuracy;
	/* angular rate values are in radians per second */
	float angularVelocity_bias[3];
	int angularVelocity_accuracy;
	/* magnetic vector values are in micro-Tesla (uT) */
	float magnetic_bias[3];
	int magnetic_accuracy;
} MPECALIBRATION, *LPMPECALIBRATION;

int mpe_init_acc_calib(MPECALIBRATION flash);
int mpe_reinit_acc_calib(void);
int mpe_update_acc_calib(float *acc_raw, int deltaTime_us, float *acc_caled, int *accuracy);
int mpe_get_acc_bias(float *bias, int *accuracy);
int mpe_init_gyro_calib(MPECALIBRATION flash);
int mpe_reinit_gyro_calib(void);
int mpe_update_gyro_calib(float *gyro_raw, int deltaTime_us, float *gyro_caled, int *accuracy);
int mpe_get_gyro_bias(float *bias, int *accuracy);
int mpe_get_acc_mean_var(float * mean,float * var);
int mpe_get_gyro_mean_var(float * mean,float * var);

//--------------------------------------------------------------------------//
#if defined(__KERNEL__)
#include <linux/types.h>
#else
#include "stdint.h"
#endif
//-----------------Global function definition-------------------------------//
//int mpe_mag_calibration_init(struct mpe_mag_calibration *f);
int mpe_init_mag_calib();
int mpe_update_mag_calib(float *mag_raw, int deltaTime_us, float *mag_caled);
int mpe_get_mag_bias(float *bias, int *accuracy);
int mpe_get_mag_mean_var(float *mag_raw, float * mean,float * var);
//--------------------------------------------------------------------------//
int mpe_read_calib_parameter(LPMPECALIBRATION pCalib);
int mpe_save_calib_parameter(MPECALIBRATION calib);

#ifdef __cplusplus
}
#endif

#endif /* __MPE_CALIB_SENSOR_H_INCLUDED__ */
