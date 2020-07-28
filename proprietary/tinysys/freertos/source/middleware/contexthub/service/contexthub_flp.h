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
#ifndef __FLP_SERVICE_H_INCLUDED__
#define __FLP_SERVICE_H_INCLUDED__

#define TOTAL_CNN_SCP_IPI_SIZE  140

/*Message header to/from CNN*/
typedef struct {
    unsigned int type;
    unsigned int length;
} MTK_FLP_MSG_T;

#ifdef CFG_GEOFENCE_SUPPORT
typedef struct {
    unsigned short MDM_TYPE;
    unsigned short MCC;
    unsigned short NET;
    unsigned short AREA;
    unsigned short OTHER;
    unsigned short SIGNAL_STRENGTH;
    unsigned int CELL_ID;
    unsigned int isCamping;
} modem_vec_t;
#endif


typedef enum {
    // From MPE
    CMD_START_MPE_REQ = 0x00, // no payload , request start of MPE
    CMD_STOP_MPE_REQ,         // no payload, request stop of MPE
    CMD_SET_MPE_MODE,         //  set MPE operational mode
    CMD_DEINIT_MPE_REQ,             //Request shutdown of MPE
    CMD_GET_SENSOR_RAW_REQ,    //  request for raw sensor data
    CMD_GET_SENSOR_CALIBRATION_REQ,  //  request for calibrated sensor data
    CMD_GET_SENSOR_FUSION_REQ, //  request for sensor fusion data (Euler angle)
    CMD_SET_GPS_AIDING_REQ,     // send gps aiding data to mpe
    CMD_GET_ADR_STATUS_REQ,     // Request MPE send AR & heading status (per request, by MNL)
    CMD_SEND_GPS_TIME_RES,       //Send GPS timeto MPE

    // To MPE
    CMD_START_MPE_RES = 0x20,  //no payload, response MPE start status
    CMD_STOP_MPE_RES,          //no payload, response MPE stop status
    CMD_SEND_SENSOR_RAW_RES,   // response  MPE sensor raw data
    CMD_SEND_SENSOR_CALIBRATION_RES, // response MPE  calibrated sensor data
    CMD_SEND_SENSOR_FUSION_RES, // response MPE  fused sensor  data
    CMD_SEND_GPS_AIDING_RES,     // response from mped
    CMD_SEND_ADR_STATUS_RES,     //ADR response AR & heading status upon request to MNL
    CMD_SEND_GPS_TIME_REQ,       //Request for GPS time for recording & replay purpose

    CMD_MPE_END = 0x30,

    // From FLP
    CMD_FLP_START_GNSS = 0x40,
    CMD_FLP_START_CELL,
    CMD_FLP_START_SENSOR,
    CMD_FLP_START_SMD_SENSOR,
    CMD_FLP_STOP_GNSS,
    CMD_FLP_STOP_CELL,
    CMD_FLP_STOP_SENSOR,
    CMD_FLP_STOP_SMD_SENSOR,
    CMD_FLP_SEND_GNSS_LOC,
    CMD_FLP_SEND_CELL_LOC,
    CMD_FLP_SCP_LOGGING_CTRL,
    CMD_FLP_SEND_ACK,

    // To FLP
    CMD_FLP_START_GNSS_RES = 0x50,
    CMD_FLP_START_CELL_RES,
    CMD_FLP_START_SENSOR_RES,
    CMD_FLP_START_SMD_SENSOR_RES,
    CMD_FLP_STOP_GNSS_RES,
    CMD_FLP_STOP_CELL_RES,
    CMD_FLP_STOP_SENSOR_RES,
    CMD_SCP_SEND_ADR,
    CMD_SCP_SEND_CELL_ID,
    CMD_SCP_SEND_SMD,
    CMD_SCP_SEND_SYN,
    CMD_FLP_MAX,
} MPE_CMD;


#endif //__FLP_SERVICE_H_INCLUDED__
