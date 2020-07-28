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

#ifndef MPE_API_H
#define MPE_API_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* Include
******************************************************************************/
#include "mpe_Attitude.h"
#include "mpe_DR.h"
#include "mpe_math.h"
/******************************************************************************
* Define
******************************************************************************/
#define ACC_IN_20MS       2
#define GYRO_IN_20MS      4
#define MAG_IN_20MS       1
#define DR_OUTPUT_RATE    1

#define GYRO_SF    (float)(1.745329251994329576922e-5) //gyro conversion from degree/s * 1000 to rad/s
#define MAG_SF     (0.01f) // magnetometer conversion from  nT to microT
#define ACC_SF     (0.001f) //accelerometer conversion from mm/s^2 to m/s^2
#define BARO_SF    (0.01f)
#define DATA_OUTPUT_SF_100  100
#define DATA_OUTPUT_SF_1000  1000
#define DATA_OUTPUT_SF_1000000 1000000

//Classification of algo type based on implemented sensor raw data type
#define REG_ORIENTATION_ALGO    (1U<<0)
#define REG_GRAVITY_ALGO        (1U<<1)
#define REG_ROT_VEC_ALGO        (1U<<2)
#define REG_LINEAR_ACC_ALGO     (1U<<3)
#define REG_PDR_ALGO            (1U<<4)
#define REG_ADR_ALGO            (1U<<5)
#define REG_GAME_ROT_VEC        (1U<<6)
#define REG_GEOMAG_ROT_VEC_ALGO (1U<<7)

#define REG_AGM_ALGO            (1U<<0)
#define REG_AG_ALGO             (1U<<1)
#define REG_AM_ALGO             (1U<<2)

// Define ASCII character type
typedef  char           CH;      // ASCII character
typedef unsigned char   U1;
typedef  U1  BL;      // boolean logical (TRUE or FALSE only)
typedef unsigned char           mtk_uint8;
typedef signed char             mtk_int8;
typedef unsigned short int      mtk_uint16;
typedef signed short int        mtk_int16;
typedef unsigned int            mtk_uint32;
typedef signed int              mtk_int32;
typedef float                   mtk_r4;
typedef double                  mtk_r8;
typedef unsigned long long      mtk_uint64;
typedef void *PVOID;
typedef PVOID HANDLE;

/******************************************************************************
* enum
******************************************************************************/
//MPE task ID
typedef enum {
    MPE_MODE_INVALID = -1,
    MPE_MODE_RUN = 0,
    MPE_MODE_PAUSE,
    MPE_MODE_CONTINUE,
    MPE_MODE_READ_RAW_DATA,
    MPE_MODE_READ_CALIBRATE_DATA,
    MPE_MODE_READ_FUSION_DATA,
    MPE_MODE_DR_SET_MODE,
    MPE_MODE_MNL_INPUT_LOC,
    MPE_MODE_FLP_INPUT_LOC,
    MPE_MODE_DEINIT,
    MPE_MODE_END
} MPE_MODE;

typedef enum {
    // To MPE
    CMD_START_MPE_REQ = 0x00, // no payload , request start of MPE
    CMD_STOP_MPE_REQ,         // no payload, request stop of MPE
    CMD_SET_MPE_MODE,         //  set MPE operational mode
    CMD_DEINIT_MPE_REQ,             //Request shutdown of MPE
    CMD_GET_SENSOR_RAW_REQ,    //  request for raw sensor data
    CMD_GET_SENSOR_CALIBRATION_REQ,  //  request for calibrated sensor data
    CMD_GET_SENSOR_FUSION_REQ, //  request for sensor fusion data (Euler angle)
    CMD_GET_PDR_STATUS_REQ,     //Request MPE send fused location (per request, by FLP)
    CMD_GET_ADR_STATUS_REQ,     // Request MPE send AR & heading status (per request, by MNL)
    CMD_SEND_GPS_TIME_RES,       //Send GPS timeto MPE

    // From MPE
    CMD_START_MPE_RES = 0x20,  //no payload, response MPE start status
    CMD_STOP_MPE_RES,          //no payload, response MPE stop status
    CMD_SEND_SENSOR_RAW_RES,   // response  MPE sensor raw data
    CMD_SEND_SENSOR_CALIBRATION_RES, // response MPE  calibrated sensor data
    CMD_SEND_SENSOR_FUSION_RES, // response MPE  fused sensor  data
    CMD_SEND_PDR_STATUS_RES,     //PDR response fused loc status upon request to FLP
    CMD_SEND_ADR_STATUS_RES,     //ADR response AR & heading status upon request to MNL
    CMD_SEND_GPS_TIME_REQ,       //Request for GPS time for recording & replay purpose
    CMD_MPE_END = 0x30
} MPE_CMD;

//location source
typedef enum {
    MPE_LOC_SOURCE_FLP = 1,
    MPE_LOC_SOURCE_GNSS = 2,
    MPE_LOC_SOURCE_OTHER = 4,
    MPE_LOC_SOURCE_END
} MPE_LOC_SOURCE;

//return type
typedef enum {
    MPE_FALSE = 0,
    MPE_TRUE = 1,
    MPE_INVALID = -1
} MPE_RET_TYPE;

//Raw data type
typedef enum {
    SENSOR_ACC = 0,
    SENSOR_MAG,
    SENSOR_GYRO,
    SENSOR_BARO,
    SENSOR_ALL
} MPE_RAW_SENSOR_TYPE;

//Algorithm type
typedef enum {
    ORIENTATION_ALGO = 0,
    GRAVITY_ALGO,
    ROT_VEC_ALGO,
    LINEAR_ACC_ALGO,
    PDR_ALGO,
    ADR_ALGO,
    GAME_ROT_VEC,
    GEOMAG_ROT_VEC_ALGO,
    TOTAL_ALGO
} MPE_ALGO_TYPE;

/******************************************************************************
* structure
******************************************************************************/

typedef struct {
    float accelerometer_raw[3];  /* raw data of accelerometer*/
    float gyro_raw[3];           /* raw data of gyro*/
    float magnetometer_raw[3];   /* raw data of magnetometer */
} MPE_SENSOR_RAW;

typedef struct {
    float accelerometer_k[3];  /* calibrated data of accelerometer*/
    float gyro_k[3];           /* calibrated data of gyro*/
    float magnetometer_k[3];   /* calibrated data of magnetometer */
} MPE_SENSOR_CALIBRATED;

typedef struct {
    float yaw;   /* calculated yaw angle*/
    float pitch; /* calculated pitch angle*/
    float roll;  /* calculated roll angle*/
} MPE_SENSOR_FUSION;

typedef struct {
    float x;   /* calculated x*/
    float y; /* calculated y*/
    float z;  /* calculated z*/
} MPE_SENSOR_RESULT;

typedef struct {
    float x;
    float y;
    float z;
    mtk_uint32 accuracy;
    mtk_uint64 timestamp;
} MPE_SENSOR_COMMON;

typedef struct {
    mtk_int32 adr_status;
    float baro_height;
} MPE_ADR_RESULT;

/******************************************************************************
* functions Prototype
******************************************************************************/
void mpe_init(void);
void mpe_set_sensor_input(MPE_SENSOR_COMMON *data_in, mtk_int32 data_size, U1 data_type);
mtk_int8 mpe_run_algo(mtk_uint32 algo_type, mtk_uint32 algo_group);
void mpe_get_fusion_output(MPE_SENSOR_RESULT *result, MPE_ALGO_TYPE algo_type);
void GyroSensor_set_timebase_correction_parameter(mtk_uint32 event_delay_ns, mtk_int8 TB_PERCENT);

#ifdef __cplusplus
}
#endif

#endif //#ifndef MPE_API_H
