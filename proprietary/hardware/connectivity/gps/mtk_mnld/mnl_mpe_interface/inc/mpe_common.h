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
#ifndef MPE_COMMON_H
#define MPE_COMMON_H

#include <string.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/time.h>
#include "mtk_gps_type.h"

#ifdef __cplusplus
#include <climits>   // C++ header
#else
#include <limits.h>  // C header
#endif

/******************************************************************************
* Define
******************************************************************************/
/* MPE Conf flag */
#define MPE_CONF_MPE_ENABLE     (1U<<0)
#define MPE_CONF_PRT_RAWDATA    (1U<<1)
#define MPE_CONF_AUTO_CALIB     (1U<<2)
#define MPE_CONF_INDOOR_ENABLE  (1U<<3)

// define TRUE and FALSE
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// Ensure NULL is defined
#ifdef NULL
#undef NULL
#endif
#define NULL 0

#define SECS_IN_DAY  (86400)            // Seconds in a Day
#define DIFF_GPS_C_TIME ((365*10+2+5)*SECS_IN_DAY) // GPS epoch = 06Jan1980,  C epoch   = 01Jan1970,10years+leap years+,5 days Jan * sec in day

typedef pthread_mutex_t MPE_MUTEX;
typedef pthread_cond_t MPE_EVENT;

/*****************************************************************************
 * FLP specific types
 *****************************************************************************/
//Define debug buffer size
#define DEBUG_LOG
#define MNL_MPE_MAX_BUFF_SIZE 256
#define MAX_NUM_SAMPLES 15
#define MPE_LOG_NAME_MAX_LEN 128

#define CMD_SEND_FROM_MNLD 0x40
#define CMD_MPED_REBOOT_DONE 0x41
/******************************************************************************
* enum
******************************************************************************/
typedef enum {
    MPE_IDLE_MODE = 0,
    MPE_START_MODE  = 1,
} MPE_OPERATION_MODE;

typedef enum {
    SENSOR_USER_ID_CALIB = 0,
    SENSOR_USER_ID_MPE = 1,
    SENSOR_USER_ID_MAX
} SENSOR_USER_ID;

typedef enum {
    SENSOR_TYPE_ACC = 0,
    SENSOR_TYPE_GYR,
    SENSOR_TYPE_UNCAL_GYR,
    SENSOR_TYPE_MAG,
    SENSOR_TYPE_BAR,
    SENSOR_TYPE_GYR_TMP,
    SENSOR_TYPE_MAX
} SENSOR_TYPE;

#ifdef __cplusplus
  extern "C" {
#endif

/******************************************************************************
* functions Prototype
******************************************************************************/
unsigned char mpe_sys_sensor_threads_create(void);
UINT32 mpe_sys_get_mpe_conf_flag( void );
void mpe_sys_read_mpe_conf_flag(void);
INT32 mpe_sys_gps_to_sys_time ( UINT32 gps_sec);
void mpe_sys_get_time_stamp(double* timetag, UINT32 leap_sec);

void mpe_log_init();
void mpe_log_deinit();
#ifdef MNLD_WRITE_STORAGE
UINT16 mpe_log_check_file();
#endif
void mpe_log_mtklogger_check(INT16 record_mode, char *logpath, INT8 log_location);

void mpe_run_algo( void );
void mpe_kernel_initialize( void );
void *mpe_calib_thread(void * arg);
void *mpe_sensor_thread(void * arg);
int mnl2mpe_hdlr(int fd);
void mnl2mpe_hdlr_init(void);
int mpe2mnl_hdlr(char *buff);

#ifdef __cplusplus
  }
#endif

#endif //#ifndef MPE_COMMON_H
