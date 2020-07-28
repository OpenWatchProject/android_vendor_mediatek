/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef _MNL_COMMON_H_
#define _MNL_COMMON_H_
/*******************************************************************************
* Dependency
*******************************************************************************/
#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1


// #include <time.h>
// #include <log/log.h>
#include <hardware/gps.h>
#include "mtk_gps_type.h"

#define FLAG_HOT_START  0x0
#define FLAG_WARM_START  MTK_GPS_DELETE_EPHEMERIS
#define FLAG_COLD_START (MTK_GPS_DELETE_EPHEMERIS | MTK_GPS_DELETE_POSITION)
#define FLAG_FULL_START (MTK_GPS_DELETE_ALL)
#define FLAG_AGPS_START (MTK_GPS_DELETE_ALL&(~MTK_GPS_DELETE_CLK))
#define FLAG_DELETE_EPH_ALM_START (MTK_GPS_DELETE_EPHEMERIS | MTK_GPS_DELETE_ALMANAC)
#define FLAG_DELETE_TIME_START MTK_GPS_DELETE_TIME

#define FLAG_AGPS_HOT_START  MTK_GPS_DELETE_RTI
#define FLAG_AGPS_WARM_START  MTK_GPS_DELETE_EPHEMERIS
#define FLAG_AGPS_COLD_START (MTK_GPS_DELETE_EPHEMERIS | MTK_GPS_DELETE_POSITION | MTK_GPS_DELETE_TIME\
                                 | MTK_GPS_DELETE_IONO | MTK_GPS_DELETE_UTC | MTK_GPS_DELETE_HEALTH)
#define FLAG_AGPS_FULL_START MTK_GPS_DELETE_ALL
#define FLAG_AGPS_AGPS_START (MTK_GPS_DELETE_EPHEMERIS | MTK_GPS_DELETE_ALMANAC | MTK_GPS_DELETE_POSITION\
                                 | MTK_GPS_DELETE_TIME | MTK_GPS_DELETE_IONO | MTK_GPS_DELETE_UTC)

// extern UINT32 assist_data_bit_map;
/******************************************************************************
 * Definition
******************************************************************************/
#define C_INVALID_PID  -1    /*invalid process id*/
#define C_INVALID_TID  -1    /*invalid thread id*/
#define C_INVALID_FD   -1    /*invalid file handle*/
#define C_INVALID_SOCKET -1  /*invalid socket id*/
#define C_INVALID_TIMER -1   /*invalid timer */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#define PMTK_CONNECTION_SOCKET 1
#define PMTK_CONNECTION_SERIAL 2
#define PMTK_CONNECTION_SOCKET_PORT 7000
/******************************************************************************
 * Function Configuration
******************************************************************************/
#define READ_PROPERTY_FROM_FILE
/******************************************************************************/

/*enumeration for NEMA debug level*/
typedef enum {
    MNL_NMEA_DEBUG_NONE      = 0x0000,
    MNL_NEMA_DEBUG_SENTENCE  = 0x0001,  /*only output sentence*/
    MNL_NMEA_DEBUG_RX_PARTIAL    = 0x0002,
    MNL_NMEA_DEBUG_RX_FULL   = 0x0004,
    MNL_NMEA_DEBUG_TX_FULL   = 0x0008,
    MNL_NMEA_DEBUG_STORAGE   = 0x0010,
    MNL_NMEA_DEBUG_MESSAGE   = 0x0020,  /*mtk_sys_msg_send/mtk_sys_msg_recv*/

    MNL_NMEA_DISABLE_NOTIFY  = 0x1000,
    /*output sentence and brief RX data*/
    MNL_NMEA_DEBUG_NORMAL        = MNL_NEMA_DEBUG_SENTENCE | MNL_NMEA_DEBUG_RX_PARTIAL,
    /*output sentence and full RX/TX data*/
    MNL_NMEA_DEBUG_FULL      = 0x00FF,
} MNL_NMEA_DEBUG;

#define MNL_NFW_CTRL_MAIN_BIT (0x01)
#define MNL_NFW_CTRL_GNSS_BIT (0x02)
#define MNL_NFW_CTRL_NLP_BIT (0x04)

typedef struct {
    GpsUtcTime time;
    int64_t timeReference;
    int uncertainty;
} ntp_context;

typedef struct {
    double latitude;
    double longitude;
    float accuracy;
    struct timespec ts;
    unsigned char  type;
    unsigned char  started;
} nlp_context;

/*******************************************************************************/
/* configruation property */
typedef struct {
    /*used by mnl_process.c*/
    int init_speed;
    int link_speed;
    int debug_nmea;
    int debug_mnl;
    int pmtk_conn;          /*PMTK_CONNECTION_SOCKET | PMTK_CONNECTION_SERIAL*/
    int socket_port;        /*PMTK_CONNECTION_SOCKET*/
    char dev_dbg[32];       /*PMTK_CONNECTION_SERAIL*/
    char dev_dsp[32];
    char dev_dsp2[32];  //For GPS L5
    char dev_gps[32];
    char bee_path[32];
    char epo_file[30];
    char epo_update_file[30];
    char qepo_file[30];
    char qepo_update_file[30];
    int delay_reset_dsp;    /*the delay time after issuing MTK_PARAM_CMD_RESET_DSP*/
    int nmea2file;
    int dbg2file;
    int nmea2socket;
    int dbg2socket;
    UINT32 dbglog_file_max_size;  // The max size limitation of one debug log file
    UINT32 dbglog_folder_max_size;  // The max size limitation of the debug log folder

    /*used by mnld.c*/
    int timeout_init;
    int timeout_monitor;
    int timeout_wakeup;
    int timeout_sleep;
    int timeout_pwroff;
    int timeout_ttff;

    /*used by agent*/
    int EPO_enabled;
    int BEE_enabled;
    int SUPL_enabled;
    int SUPLSI_enabled;
    int EPO_priority;
    int BEE_priority;
    int SUPL_priority;
    int fgGpsAosp_Ver;

    /*for FM TX*/
    int AVAILIABLE_AGE;
    int RTC_DRIFT;
    int TIME_INTERVAL;
    char u1AgpsMachine;
    int ACCURACY_SNR;
    int GNSSOPMode;
    int OFFLOAD_enabled;
    int OFFLOAD_testMode;
    int OFFLOAD_switchMode;    //0: always offload mode  1: Offload/Host-base auto switch mode
    int log_hide; //overwrite mtk_gps_log_hide if mtk_gps_log_hide == 0
    int adc_capture_enabled;
    #if defined(GPS_SUSPEND_SUPPORT)
    int SUSPEND_enabled;
    int SUSPEND_timeout;
    #endif
    int fast_HTTFF;
    //bit0: Main switch of NFW visibility and notify mechanism, 0: disable NFW visibility and notify mechanism, always sync location to NFW user
    //bit1: Switch of FW visibility and notify mechanism of GNSS location sycn to NFW user
    //bit2: Switch of FW visibility and notify mechanism of NLP location sync to NFW user
    int bitmap_nfw_ctrl;
} MNL_CONFIG_T;
/*---------------------------------------------------------------------------*/
typedef struct {
    time_t uSecond_start;
    time_t uSecond_expire;
}MNL_EPO_TIME_T;
/*---------------------------------------------------------------------------*/
typedef struct {
    int notify_fd;
    int port;
} MNL_DEBUG_SOCKET_T;
/*---------------------------------------------------------------------------*/
#ifdef _MNL_COMMON_C_
    #define C_EXT
#else
    #define C_EXT   extern
#endif
/*---------------------------------------------------------------------------*/
C_EXT int  mnl_utl_load_property(MNL_CONFIG_T* prConfig);
/*---------------------------------------------------------------------------*/
#undef C_EXT
/*---------------------------------------------------------------------------*/

int str2int(const char*  p, const char*  end);

#endif
