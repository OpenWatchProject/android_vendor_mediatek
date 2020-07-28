/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
  /*******************************************************************************
 * Filename:
 * ---------
 *  API_sensor_calibration.h
 *
 * Project:
 * --------
 * Everest
 *
 * Description:
 * ------------
 * MPE(virtual sensor) sensor calibration header file
 *
 *******************************************************************************/
#ifndef __API_SENSOR_K_H
#define __API_SENSOR_K_H

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
* FUNCTION
*  Acc_init_calibration
* DESCRIPTION
*   Init calibration of Accelerometer
* PARAMETERS:
*   1. MPE_SENSOR_DATA structure array of pre-recorded bias (unit in 1000*mm/s^2), input [0 0 0 ] if N.A
* RETURNS
*   None
*****************************************************************************/
void Acc_init_calibration(float *bias);

/*****************************************************************************
* FUNCTION
*  Gyro_init_calibration
* DESCRIPTION
*   Init calibration of Gyro
* PARAMETERS:
*   1. MPE_SENSOR_DATA structure array of pre-recorded bias (unit in 1000*deg/s), input [0 0 0]if N.A
* RETURNS
*   None
*****************************************************************************/
void Gyro_init_calibration(float *slope, float *intercept);

/*****************************************************************************
* FUNCTION
*  Acc_run_calibration
* DESCRIPTION
*   Run calibration of Accelerometer
* PARAMETERS:
*   1. [IN]  delta time between samples, unit in micro sec
*   2. [IN]  input data count
*   3. [IN]  MPE_SENSOR_DATA structure array of raw data
*   4. [OUT]  MPE_SENSOR_DATA structure array of calibrated data
*   5. [OUT]  calibrate_acc, accuracy of calibrated data:
*                0: calibration not yet available
*                1: accuracy low
*                2: accuracy medium
*                3: accuracy high
* RETURNS
*   success(0) or fail(-1)
*****************************************************************************/
int Acc_run_calibration(int dt, float *raw_data_input, float *calibrated_data_output, int *calibrate_acc );
int Acc_run_factory_calibration(int dt, float *raw_data_input, float *calibrated_data_output, int *calibrate_acc );
int Acc_get_calibration_parameter(float *bias);

/*****************************************************************************
* FUNCTION
*  Gyro_run_calibration
* DESCRIPTION
*   Run calibration of Gyro
* PARAMETERS:
*   1. [IN]  delta time between samples, unit in micro sec
*   2. [IN]  input data count
*   3. [IN]  MPE_SENSOR_DATA structure array of raw data
*   4. [OUT]  MPE_SENSOR_DATA structure array of calibrated data
*   5. [OUT]  calibrate_acc, accuracy of calibrated data:
*                0: calibration not yet available
*                1: accuracy low
*                2: accuracy medium
*                3: accuracy high
* RETURNS
*   success(0) or fail(-1)
*****************************************************************************/
int Gyro_run_calibration(int dt, float *raw_data_input, float *calibrated_data_output, int *calibrate_acc, float temperature );
int Gyro_run_factory_calibration(int dt, float *raw_data_input, float *calibrated_data_output, int *calibrate_acc, float temperature );
int Gyro_get_calibration_parameter(float *bias, float *slope, float *intercept );

#ifdef __cplusplus
}
#endif

#endif /* __API_SENSOR_K_H */
