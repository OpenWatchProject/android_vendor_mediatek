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
#ifndef SE_SENSOR_LISTENER_API_H
#define SE_SENSOR_LISTENER_API_H

#include "mpe_Attitude.h"
#include "mpe_DR.h"

#ifdef __cplusplus
  extern "C" {
#endif

typedef struct EVENT_DATA {
    float x;
    float y;
    float z;
    int64_t timestamp;
    uint64_t utc_timestamp;
} EVENT_DATA;

unsigned char mpe_sensor_get_listen_mode(void);
void mpe_sensor_set_listen_mode(unsigned char mode);
void mpe_sensor_get_accuracy(INT8 *accuracy);
unsigned char mpe_sensor_init(SENSOR_USER_ID id);
unsigned char mpe_sensor_deinit(SENSOR_USER_ID id);
unsigned char mpe_sensor_start(SENSOR_USER_ID id, SENSOR_TYPE mpe_type, UINT32 periodInMs);
unsigned char mpe_sensor_stop(SENSOR_USER_ID id, SENSOR_TYPE mpe_type);
void mpe_sensor_detect_changing_freq(int64_t curr_time, int64_t *last_time, int *count);
void mpe_sensor_get_calib_accuracy(INT8 *accuracy);
unsigned char mpe_sensor_get_calib_gyr_data(void);
unsigned char mpe_sensor_check_mnl_response(void);
UINT16 mpe_sensor_check_time();
UINT16 mpe_sensor_acquire_Data (IMU *data, uint64_t *data_utc);
void mpe_sensor_run (void);
void mpe_sensor_update_mnl_sec(UINT32 gps_sec, UINT32 leap_sec);
unsigned char mpe_sensor_sync_kernel_utc_time();
void mpe_publish_to_log_buf_pool(char *);
unsigned char mpe_check_sensor_name();

#ifdef __cplusplus
}
#endif

#endif //#ifndef SE_SENSOR_LISTENER_API_H
