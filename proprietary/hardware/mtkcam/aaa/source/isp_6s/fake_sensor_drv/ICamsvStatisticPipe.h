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

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_AAA_FAKEDRV_ICAMSVSTATPIPE_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_AAA_FAKEDRV_ICAMSVSTATPIPE_H_

// #include "IHalCamIO.h"
#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSCamIOPipe {

#define NORMALPIPE_MAX_INST_CNT 3
#define MAX_SENSOR_CNT 4
/*****************************************************************************
*
* @class INormalPipe
* @brief CamIO Pipe Interface for Normal pipe in frame based mode.
* @details
* The data path will be Sensor --> ISP-- Mem.
*
******************************************************************************/
class ICamsvStatisticPipe
{
public:     ////                    Instantiation.
    static  ICamsvStatisticPipe  *createInstance(MUINT32 sensorIndex, char const* szCallerName, MUINT32 FeatureIdx = 0);
    virtual                 ~ICamsvStatisticPipe(){};

public:     ////                    Attributes.

    virtual MVOID   destroyInstance(char const* szCallerName) = 0;

    virtual MBOOL   init() = 0;
    virtual MBOOL   uninit() = 0;

    virtual MBOOL   start() = 0;
    virtual MBOOL   stop() = 0;

    virtual MBOOL   enque(QBufInfo const& rQBuf) = 0;

    virtual MBOOL   deque(PortID port, QBufInfo& rQBuf, ESTT_CACHE_SYNC_POLICY cacheSyncPolicy = ESTT_CacheInvalidByRange, MUINT32 u4TimeoutMs = 0xFFFFFFFF) = 0;

    virtual MBOOL   reset() = 0;

    virtual MBOOL   configPipe(QInitStatParam const& vInPorts, MINT32 burstQnum = 1) = 0;

    virtual MBOOL   sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3) = 0;

public:
    virtual MBOOL   wait(EPipeSignal eSignal, EPipeSignalClearType eClear, const MINT32 mUserKey,
                            MUINT32 TimeoutMs = 0xFFFFFFFF, SignalTimeInfo *pTime = NULL) = 0;

    virtual MBOOL   abortDma(PortID port, char const* szCallerName) = 0;
};

enum ECAMSVSPipeCmd {
    ECAMSVSPipeCmd_GET_TG_INDEX                = 0x0800,
    ECAMSVSPipeCmd_GET_BURSTQNUM               = 0x0801,
    ECAMSVSPipeCmd_GET_CAMSV_IMGO_SIZE         = 0x0802,
    ECAMSVSPipeCmd_SET_CAMSV_SENARIO_ID        = 0x0803,
    ECAMSVSPipeCmd_GET_CAMSV_ENQUE_BUFFER_INFO = 0x0804,
    ECAMSVSPipeCmd_GET_CAMSV_DEQUE_BUFFER_INFO = 0x0805,
    ECAMSVSPipeCmd_GET_CAMSV_GET_ERROR_CODE    = 0x0806,
    ECAMSVSPipeCmd_GET_TG_OUT_SIZE             = 0x110F,
    ECAMSVSIPECmd_MAX
};

enum ECAMSVSPipeErrorCode {
    ECAMSVSPipeErrorCode_CREATE_HALSENSOR_FAIL,
    ECAMSVSPipeErrorCode_MULTILE_CONFIGPIPE,
    ECAMSVSPipeErrorCode_INVALID_SCENARIO_ID,
    ECAMSVSPipeErrorCode_QUERY_SENSOR_DYINFO_FAIL,
    ECAMSVSPipeErrorCode_UNKNOWN_TG_ID,
    ECAMSVSPipeErrorCode_SENSOR_NOT_SUPPORT_HDR,
    ECAMSVSPipeErrorCode_SENSOR_NOT_SUPPORT_PDAF,
    ECAMSVSPipeErrorCode_UNKNOWN_TG_CAMSV_ID,
    ECAMSVSPipeErrorCode_CREATE_CAMSAV_IOPIPE_FAILE,
    ECAMSVSPipeErrorCode_INIT_CAMSAV_IOPIPE_FAILE,
    ECAMSVSPipeErrorCode_INIT_QUERY_SIZE_FAIL,
    ECAMSVSPipeErrorCode_CAMSAV_IOPIPE_CONFIGPIPE_FAIL,
    ECAMSVSPipeErrorCode_CAMSAV_ENQUE_FAIL,
    ECAMSVSPipeErrorCode_CAMSAV_START_FAIL,
    ECAMSVSPipeErrorCode_CAMSAV_DEQUE_FAIL,
    ECAMSVSPipeErrorCode_UNKNOWN = 0xFFFF,
};

typedef enum {
    EPIPE_CAMSV_FEATURE_PDAF = 0,
    EPIPE_CAMSV_FEATURE_MVHDR,
    EPIPE_CAMSV_FEATURE_NUM
} EPIPE_CAMSV_FEATURE_E;



/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCamIO
};  //namespace NSIoPipe
};  //namespace NSCam

#endif // _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_CAMIO_ICAMSVSTATPIPE_H_

