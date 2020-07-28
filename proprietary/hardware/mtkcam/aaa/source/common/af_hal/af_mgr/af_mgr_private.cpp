/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "af_mgr_v3"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <utils/threads.h>  // For Mutex::Autolock.
#include <sys/stat.h>
#include <sys/time.h>
#include <cutils/properties.h>
#include <cutils/atomic.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <kd_camera_feature.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include <faces.h>
#include <private/aaa_hal_private.h>
#include <camera_custom_nvram.h>
#include <af_param.h>
#include <pd_param.h>
#include <af_tuning_custom.h>
#include <mcu_drv.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/sys/SensorProvider.h>
#include <cct_feature.h>
#include <af_feature.h>
#include <af_define.h>

#include "af_mgr.h"
#include <nvbuf_util.h>
//#include <isp_mgr.h>
#include "aaa_common_custom.h"
#include <pd_mgr_if.h>
#include "private/PDTblGen.h"
extern "C" {
#include <af_algo_if.h>
}
//
#include "mtkcam/utils/metadata/client/mtk_metadata_tag.h"

//configure HW
//#include <isp_mgr_af_stat.h>
#include <StatisticBuf.h>

#include <math.h>
#include <android/sensor.h>             // for g/gyro sensor listener
#include <mtkcam/utils/sys/SensorListener.h>    // for g/gyro sensor listener
#define SENSOR_ACCE_POLLING_MS  20
#define SENSOR_GYRO_POLLING_MS  20
#define SENSOR_ACCE_SCALE       100
#define SENSOR_GYRO_SCALE       100

#define LASER_TOUCH_REGION_W    0
#define LASER_TOUCH_REGION_H    0

#define AF_ENLOG_STATISTIC 2
#define AF_ENLOG_ROI 4

#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_AF(String) \
          do { \
              aee_system_warning( \
                  "af_mgr", \
                  NULL, \
                  DB_OPT_DEFAULT|DB_OPT_FTRACE, \
                  String); \
          } while(0)
#else
#define AEE_ASSERT_AF(String)
#endif

// LaunchCamTrigger
#define AESTABLE_TIMEOUT 0
#define VALIDPD_TIMEOUT  0

#define GYRO_THRESHOLD 15
#define MAX_PDMOVING_COUNT 6

using namespace NS3Av3;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;
using namespace NSCam::Utils;

MVOID AfMgr::checkStaticProperty(AF_DBG_S_PROPERTY_T& property)
{
    m_i4DgbDisableAF = property_get_int32("vendor.debug.af.force.disable", 0);

    m_i4DgbRtvEnable = property_get_int32("vendor.debug.af.rtv.enable", 0);
    if (m_i4DgbRtvEnable)
    {
        m_rtvId[0] = property_get_int32("vendor.debug.af.rtv.id_1", E_RTV_AF_CTRL_MODE);
        m_rtvId[1] = property_get_int32("vendor.debug.af.rtv.id_2", E_RTV_AF_CTRL_STATE);
        m_rtvId[2] = property_get_int32("vendor.debug.af.rtv.id_3", E_RTV_AF_ROI_TYPE);
        m_rtvId[3] = property_get_int32("vendor.debug.af.rtv.id_4", E_RTV_AF_SEARCH_STATE);
        m_rtvId[4] = property_get_int32("vendor.debug.af.rtv.id_5", E_RTV_AF_SEARCH_TYPE);
        m_rtvId[5] = property_get_int32("vendor.debug.af.rtv.id_6", E_RTV_AF_ALGO_MODE);
        m_rtvId[6] = property_get_int32("vendor.debug.af.rtv.id_7", E_RTV_AF_ALGO_STATE);
        m_rtvId[7] = property_get_int32("vendor.debug.af.rtv.id_8", E_RTV_AF_LENS_POS);
    }

    m_i4DbgAfegainQueue = property_get_int32("vendor.debug.afegainqueue", 0);
    m_i4DbgAdpAlarm     = property_get_int32("vendor.debug.af_mgr.adpalarm",   0);
    m_i4DbgAutoBitTrue   = property_get_int32("vendor.afo.autobittrue.enable", 0);

    // debug switch
    m_i4DgbLogLv        = property_get_int32("vendor.debug.af_mgr.enable", 0);             // af flow
    m_i4DbgPDVerifyEn   = property_get_int32("vendor.debug.pd_verify_flow.enable", 0);     // pd flow
    property.enable     = property_get_int32("vendor.debug.af.enable", AF_NONE_PROPERTY);  // algo

    if (property.enable)
    {
        property.log_enable         = property_get_int32("vendor.debug.af.log.enable",     AF_NONE_PROPERTY);
        // Tuning
        property.v3nvram            = property_get_int32("vendor.debug.af.v3nvram",        AF_NONE_PROPERTY);
        property.hbnvram            = property_get_int32("vendor.debug.af.hbnvram",        AF_NONE_PROPERTY);
        property.hbnvram2           = property_get_int32("vendor.debug.af.hbnvram2",       AF_NONE_PROPERTY);
        property.fdnvram            = property_get_int32("vendor.debug.af.fdnvram",        AF_NONE_PROPERTY);
        property.tuning_enable      = property_get_int32("vendor.debug.aftuning.enable",   AF_NONE_PROPERTY);
        property.tuning_TH0         = property_get_int32("vendor.debug.aftuning.TH0",      AF_NONE_PROPERTY);
        property.tuning_TH1         = property_get_int32("vendor.debug.aftuning.TH1",      AF_NONE_PROPERTY);
        property.tuning_TH2         = property_get_int32("vendor.debug.aftuning.TH2",      AF_NONE_PROPERTY);
        property.tuning_minth       = property_get_int32("vendor.debug.aftuning.minth",    AF_NONE_PROPERTY);
        property.tuning_gsthr       = property_get_int32("vendor.debug.aftuning.gsthr",    AF_NONE_PROPERTY);
        property.tuning_fvthr       = property_get_int32("vendor.debug.aftuning.fvthr",    AF_NONE_PROPERTY);
        property.set_infinite       = property_get_int32("vendor.debug.af.setinf",         AF_NONE_PROPERTY);
        property.lensmovetime       = property_get_int32("vendor.debug.af.lensmovetime",   AF_NONE_PROPERTY);
        // Mode
        property.fullscan_step      = property_get_int32("vendor.debug.af.fullscan.step",  AF_NONE_PROPERTY);
        property.localfullscan      = property_get_int32("vendor.debug.af.localfullscan",  AF_NONE_PROPERTY);
        property.thermalcali_ctrl   = property_get_int32("vendor.debug.af.thermalcali.ctrl", AF_NONE_PROPERTY);
        // ZoomEF
        property.zoomext            = property_get_int32("vendor.debug.af.zoomext",        AF_NONE_PROPERTY);
        property.zefp               = property_get_int32("vendor.debug.af.zefp",           AF_NONE_PROPERTY);
        property.zefsel             = property_get_int32("vendor.debug.af.zefsel",         AF_NONE_PROPERTY);
        property.zeccalib           = property_get_int32("vendor.debug.af.zeccalib",       AF_NONE_PROPERTY);
        // HybridAF
        property.hybrid_mode        = property_get_int32("vendor.debug.hybrid.mode",       AF_NONE_PROPERTY);
        property.pdafSingleWin      = property_get_int32("vendor.debug.af.pdafSingleWin",  AF_NONE_PROPERTY);
        property.pdsubidx           = property_get_int32("vendor.debug.af.pdsubidx",       AF_NONE_PROPERTY);
        property.exifpdsubwin       = property_get_int32("vendor.debug.af.exifpdsubwin",   AF_NONE_PROPERTY);
        // PLAF
        property.pldetect           = property_get_int32("vendor.debug.af.pldetect",       AF_NONE_PROPERTY);
        property.pltuning_Rtop      = property_get_int32("vendor.debug.pltuning.Rtop",     AF_NONE_PROPERTY);
        property.pltuning_Sa        = property_get_int32("vendor.debug.pltuning.Sa",       AF_NONE_PROPERTY);
        property.pltuning_Sr        = property_get_int32("vendor.debug.pltuning.Sr",       AF_NONE_PROPERTY);
        property.pltuning_Ba        = property_get_int32("vendor.debug.pltuning.Ba",       AF_NONE_PROPERTY);
        property.pltuning_Br        = property_get_int32("vendor.debug.pltuning.Br",       AF_NONE_PROPERTY);
        property.pltuning_Amin1     = property_get_int32("vendor.debug.pltuning.Amin1",    AF_NONE_PROPERTY);
        property.pltuning_Rmin1     = property_get_int32("vendor.debug.pltuning.Rmin1",    AF_NONE_PROPERTY);
        property.pltuning_Amax      = property_get_int32("vendor.debug.pltuning.Amax",     AF_NONE_PROPERTY);
        property.pltuning_Rmax      = property_get_int32("vendor.debug.pltuning.Rmax",     AF_NONE_PROPERTY);
        property.pltuning_CtrlBit   = property_get_int32("vendor.debug.pltuning.CtrlBit",  AF_NONE_PROPERTY);
        property.pl_SCNT            = property_get_int32("vendor.debug.pl.SCNT",           AF_NONE_PROPERTY);
        property.pl_convThrInf      = property_get_int32("vendor.debug.pl.convThrInf",     AF_NONE_PROPERTY);
        property.pl_convThrMac      = property_get_int32("vendor.debug.pl.convThrMac",     AF_NONE_PROPERTY);
        // thermal
        property.TempCaliTimeOut    = property_get_int32("vendor.debug.af.LTC.CaliTime",   AF_NONE_PROPERTY);
        property.TempDbgLog         = property_get_int32("vendor.debug.af.LTC.Log",        AF_NONE_PROPERTY);
        property.thermalclean       = property_get_int32("vendor.debug.af.LTC.Reset",      AF_NONE_PROPERTY);
        property.LTCOFF             = property_get_int32("vendor.debug.af.LTC.OFF",        AF_NONE_PROPERTY);
        property.LTCMode            = property_get_int32("vendor.debug.af.LTC.Mode",       AF_NONE_PROPERTY);
        property.LTCBias            = property_get_int32("vendor.debug.af.LTC.Bias",       AF_NONE_PROPERTY);

        // fsc
        property.fscFrmDelay        = property_get_int32("vendor.debug.xxxxx",                 AF_NONE_PROPERTY);
        // Dump Data
        property.dumpfdaf_enable    = property_get_int32("vendor.debug.dumpfdaf.enable",       AF_NONE_PROPERTY);
        property.PLWFile_enable     = property_get_int32("vendor.debug.PLWFile.enable",        AF_NONE_PROPERTY);
        property.prvdump_step       = property_get_int32("vendor.debug.af.prvdump.step",       AF_NONE_PROPERTY);
        property.prvdump_start      = property_get_int32("vendor.debug.af.prvdump.start",      AF_NONE_PROPERTY);
        property.prvdump_num        = property_get_int32("vendor.debug.af.prvdump.num",        AF_NONE_PROPERTY);
        property.prvdump_frameidle  = property_get_int32("vendor.debug.af.prvdump.frameidle",  AF_NONE_PROPERTY);
    }
}

MVOID AfMgr::checkDynamicProperty(AF_DBG_D_PROPERTY_T& property)
{
    // mgr
    if (m_i4DbgPDVerifyEn)
    {
        i4IsLockAERequest = property_get_int32("vendor.debug.pdmgr.lockae", 0) == 1 ? E_AF_LOCK_AE_REQ_FORCE_LOCK : E_AF_LOCK_AE_REQ_NO_LOCK;
    }
    // algo
    if (m_sStartInput.initAlgoInput.sProperty.enable)
    {
        // test mode
        property.trigger        = property_get_int32("vendor.debug.af.trigger",   AF_NONE_PROPERTY);
        property.frame          = property_get_int32("vendor.debug.af.frame",     AF_NONE_PROPERTY);
        property.scan           = property_get_int32("vendor.debug.af.scan",      AF_NONE_PROPERTY);
        property.inf            = property_get_int32("vendor.debug.af.inf",       AF_NONE_PROPERTY);
        // lens
        property.motor_disable  = property_get_int32("vendor.debug.af_motor.disable",   AF_NONE_PROPERTY);
        property.motor_position = property_get_int32("vendor.debug.af_motor.position",  AF_NONE_PROPERTY);
        property.motor_control  = property_get_int32("vendor.debug.af_motor.disable",   AF_NONE_PROPERTY);
        property.motor_dac      = property_get_int32("vendor.debug.af_motor.position",  AF_NONE_PROPERTY);
        property.motor_slavedac = property_get_int32("vendor.debug.af_motor.slavedac",  AF_NONE_PROPERTY);
        // Reserve
        property.db_rsv1        = property_get_int32("vendor.debug.af.db_rsv1", AF_NONE_PROPERTY);
        property.db_rsv2        = property_get_int32("vendor.debug.af.db_rsv2", AF_NONE_PROPERTY);
        property.db_rsv3        = property_get_int32("vendor.debug.af.db_rsv3", AF_NONE_PROPERTY);
        property.db_rsv4        = property_get_int32("vendor.debug.af.db_rsv4", AF_NONE_PROPERTY);
        property.db_rsv5        = property_get_int32("vendor.debug.af.db_rsv5", AF_NONE_PROPERTY);
    }
}

MINT32 AfMgr::process_doCommand(AFCommand_T& command)
{
    MINT32 err = 0;

    AAA_TRACE_D("%s #(%d,%d)", __FUNCTION__, m_u4ReqMagicNum, m_u4StaMagicNum);

    char  dbgMsgBuf[DBG_MSG_BUF_SZ];
    char* logBuf = dbgMsgBuf;
    if (LEVEL_FLOW)
    {
        logBuf += sprintf( logBuf,
                           "#(%5d,%5d) %s Dev(%d) ",
                           m_u4ReqMagicNum,
                           m_u4StaMagicNum,
                           __FUNCTION__,
                           m_i4CurrSensorDev);
        logBuf += sprintf( logBuf,
                           "metacommand: reqNum = %4d, afMode = %1d, focusDistance = %f, mfPos = %4d, ",
                           command.requestNum,
                           command.afMode,
                           command.focusDistance,
                           command.mfPos);
        if (command.afRegions.u4Count > 0)
        {
            logBuf += sprintf( logBuf,
                               "afRegions = Count(%d), [L,T,R,B] = [%4d, %4d, %4d, %4d], ",
                               command.afRegions.u4Count,
                               command.afRegions.rAreas[0].i4Left,
                               command.afRegions.rAreas[0].i4Top,
                               command.afRegions.rAreas[0].i4Right,
                               command.afRegions.rAreas[0].i4Bottom);
        }
        logBuf += sprintf( logBuf,
                           "eAutofocus = %1d, ePrecapture = %1d, triggerAF = %1d, pauseAF = %1d, ",
                           command.eAutofocus,
                           command.ePrecapture,
                           command.triggerAF,
                           command.pauseAF);
        logBuf += sprintf( logBuf,
                           "cropRegion = %4d, %4d, %4d, %4d, ",
                           command.cropRegion_X,
                           command.cropRegion_Y,
                           command.cropRegion_W,
                           command.cropRegion_H);
        logBuf += sprintf( logBuf,
                           "captureIntent = %1d, ",
                           command.isCaptureIntent);
    }
    // Seperatly show command log if LEVEL_VERBOSE
    if (LEVEL_VERBOSE)
    {
        CAM_LOGD_IF(LEVEL_VERBOSE, "%s", dbgMsgBuf);
        memset(dbgMsgBuf, 0, sizeof(DBG_MSG_BUF_SZ));
    }
    // 0. check dynamic property
    if (!err)
    {
        checkDynamicProperty(m_sDoAFInput.afInput.dProperty);
    }
    // 1. metaCommand -> algoCommand
    if (!err)
    {
        //===== Regular commands =====//
        // requestNum
        m_sDoAFInput.algoCommand.requestNum = command.requestNum;
        m_u4ReqMagicNum = command.requestNum;
        m_eEvent = EVENT_NONE;

        if (m_i4IsCctOper)
        {
            command.afMode = m_i4CctAfMode;
            if (m_i4CctTriggerAf == 1) // trigger ONCE if necessary
            {
                command.triggerAF = 1;
                m_i4CctTriggerAf = 2;
            }
            if (m_i4CctAfArea.u4Count > 0)
            {
                memcpy(&command.afRegions, &m_i4CctAfArea, sizeof(CameraFocusArea_T));
            }
        }


        // afMode->LibAfMode
        setAFMode( command.afMode, AF_CMD_CALLER);
        m_sDoAFInput.algoCommand.afLibMode = m_eLIB3A_AFMode;

        // focusDistance -> mfPos
        setFocusDistance( command.focusDistance); // focusDistance -> mfPos
        m_sDoAFInput.algoCommand.mfPos = (!m_i4IsCctOper) ? m_i4MFPos : m_i4CctMfPos;

        // cropRegion
        setCropRegionInfo( command.cropRegion_X, command.cropRegion_Y, command.cropRegion_W, command.cropRegion_H, AF_CMD_CALLER);
        // afRegions
        setAFArea( command.afRegions);
        // pauseAF->eLockAlgo
        setPauseAF( command.pauseAF);
        // ePrecapture
        checkPrecapture(command.ePrecapture);
        // eAutofocus
        checkAutoFocus(command.eAutofocus);
        if (m_bForceUnlockAE)
        {
            // cancel the current search
            m_sDoAFInput.algoCommand.bCancel = MTRUE;
            m_bForceUnlockAE = MFALSE;
        }
        // triggerAF
        if (command.triggerAF)
        {
            triggerAF(AF_CMD_CALLER);
        }

        // skipAf
        m_sDoAFInput.algoCommand.isSkipAf = isSkipHandleAF(); // skip handleAF

        // captureIntent
        m_sDoAFInput.afInput.i4IsCaptureIntent = command.isCaptureIntent;

        if (LEVEL_VERBOSE)
        {
            logBuf = dbgMsgBuf;
            logBuf += sprintf( logBuf,
                               "#(%5d,%5d) %s Dev(%d) ",
                               m_u4ReqMagicNum,
                               m_u4StaMagicNum,
                               __FUNCTION__,
                               m_i4CurrSensorDev);
        }

        if (LEVEL_FLOW)
        {
            logBuf += sprintf( logBuf,
                               "algoCommand: requestNum = %4d, afLibMode = %2d, mfPos = %4d, eLockAlgo = %1d, bTrigger = %1d, bCancel = %1d, bTargetAssistMove = %1d, isSkipAf = %1d / ",
                               m_sDoAFInput.algoCommand.requestNum,
                               m_sDoAFInput.algoCommand.afLibMode,
                               m_sDoAFInput.algoCommand.mfPos,
                               m_sDoAFInput.algoCommand.eLockAlgo,
                               m_sDoAFInput.algoCommand.bTrigger,
                               m_sDoAFInput.algoCommand.bCancel,
                               m_sDoAFInput.algoCommand.bTargetAssistMove,
                               m_sDoAFInput.algoCommand.isSkipAf);
        }
    }
    AAA_TRACE_END_D;

    CAM_LOGD_IF(LEVEL_FLOW, "%s", dbgMsgBuf);

    return err;
}
MINT32 AfMgr::process_doStt(MUINT32 reqNumber, MVOID* prtAfStt)
{
    MINT32 err = 0;
    // To update request number from command
    m_u4ReqMagicNum = reqNumber;

    // To set m_sDoSttInput
    m_sDoSttInput.u4ReqMagicNum = reqNumber;
    m_sDoSttInput.i4AfPos = m_sAFOutput->i4AFPos;
    m_sDoSttInput.i4HWBlkNumX = m_i4HWBlkNumX;
    m_sDoSttInput.i4HWBlkNumY = m_i4HWBlkNumY;
    m_sDoSttInput.i4HWBlkSizeX = m_i4HWBlkSizeX;
    m_sDoSttInput.i4HWBlkSizeY = m_i4HWBlkSizeY;
    m_sDoSttInput.ptrAfStt = prtAfStt;

    // To set m_sDoSttOutput.pAfSttProfile
    m_sDoSttOutput.pAfSttProfile = &(m_sAFInput->sStatProfile);

    err = s_pIAfCxU->doStt(m_sDoSttInput, m_sDoSttOutput);

    if (!err)
    {
        // To update parameters from m_sDoSttOutput
        m_u4StaMagicNum = m_sDoSttOutput.u4StaMagicNum;
        m_u4ConfigHWNum = m_sDoSttOutput.u4ConfigHWNum;
        m_sAFInput->i4IsSupportN2Frame = (m_sDoSttOutput.u4ConfigLatency == 3) ? 0 : 1;
        m_u8SofTimeStamp = m_sDoSttOutput.u8SofTimeStamp;
        m_sAFInput->sStatProfile.u4AfoStatMode = m_u4AfoStatMode;  // New, afo foot-print type ; (bit-depth) x (high/low)
        m_sAFInput->sStatProfile.u4AfoBlkSzByte = m_u4AfoBlkSzByte; // New, afo foot-print block size
        m_pAfoBuf = m_sDoSttOutput.pAfoBuf;
    }
    return err;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::process_doData(AFInputData_T& data)
{
    MINT32 err = 0;

    AAA_TRACE_D("%s #(%d,%d)", __FUNCTION__, m_u4ReqMagicNum, m_u4StaMagicNum);
    char  dbgMsgBuf[DBG_MSG_BUF_SZ];
    char* logBuf = dbgMsgBuf;
    if (LEVEL_FLOW)
    {
        logBuf += sprintf( logBuf,
                           "#(%5d,%5d) %s Dev(%d) ",
                           m_u4ReqMagicNum,
                           m_u4StaMagicNum,
                           __FUNCTION__,
                           m_i4CurrSensorDev);

        logBuf += sprintf( logBuf,
                           "IN: isFlashFrm = %d, ",
                           m_sAFInput->i4IsFlashFrm);
        logBuf += sprintf( logBuf,
                           "GMV : %d, %d, %d, %d, %d; ",
                           data.gmvInfo.GMV_X,
                           data.gmvInfo.GMV_Y,
                           data.gmvInfo.GMV_Conf_X,
                           data.gmvInfo.GMV_Conf_Y,
                           data.gmvInfo.GMV_Max);
    }

    //===== Irregular commands : To update nvramIndex & FDInfo =====//
    if (m_sAFIrgCommand.preNvramIndex != m_sAFIrgCommand.curNvramIndex)
    {
        // To update NVRAM just before doAF.
        m_sAFIrgCommand.preNvramIndex = m_sAFIrgCommand.curNvramIndex;
        updateNVRAM(m_sAFIrgCommand.preNvramIndex);
    }
    if (m_sAFIrgCommand.FDROI.i4IsFdValid)
    {
        // To update the latest FDInfo to AFInput
        memcpy(&m_sAFInput->sFDInfo, &m_sAFIrgCommand.FDROI, sizeof(FD_INFO_T));
        // To update the latest existing FDInfo to m_sPDRois[eIDX_ROI_ARRAY_FD]
        if (m_sAFInput->sFDInfo.i4Count > 0)
        {
            m_sPDRois[eIDX_ROI_ARRAY_FD].valid = MTRUE; // This would be reset while no fdinfo for a while (100ms)
            m_sPDRois[eIDX_ROI_ARRAY_FD].info.sType  = eAF_ROI_TYPE_FD;
            memcpy(&m_sPDRois[eIDX_ROI_ARRAY_FD].info.sPDROI, &m_sAFInput->sFDInfo.sRect[0], sizeof(AREA_T));
        }
        m_sAFIrgCommand.FDROI.i4IsFdValid = 0;
    }

    // 1. flow information
    m_sAFInput->i4FrameNum = m_u4StaMagicNum;
    m_sAFInput->i4HybridAFMode = GetHybridAFMode();
    m_sAFInput->i4IsEnterCam = m_i4IsEnterCam;
    m_sAFInput->i4IsFlashFrm = data.isFlashFrm;
    // 2. Adaptive Compensation
    setAdptCompInfo( m_u8SofTimeStamp);
    // 3. AF information
    m_sAFInput->sTGSz.i4W = m_i4TGSzW;
    m_sAFInput->sTGSz.i4H = m_i4TGSzH;
    m_sAFInput->sHWSz.i4W = m_i4BINSzW;
    m_sAFInput->sHWSz.i4H = m_i4BINSzH;
    m_sAFInput->sAFArea.i4Count  = 1;
    m_sAFInput->sAFArea.sRect[0] = SelROIToFocusing( *m_sAFOutput);

    // 4. AE information
    // 4.1 push ispAeInfo(reqNum) into the Queue
    setIspAEInfo(data.ispAeInfo);
    // 4.2 get ispAeInfo(sttNum) from the Queue
    ISPAEInfo_T* ptrSensorSetting;
    ptrSensorSetting = getMatchedISPAEInfoFromFrameId(m_u4StaMagicNum);
    if (ptrSensorSetting->FrameNum != -1)
    {
        // settings => N+2
        m_sAFInput->i4ISO              = ptrSensorSetting->realISOValue;
        m_sAFInput->i4ShutterValue     = ptrSensorSetting->exposuretime;
        m_sAFInput->u4AEidxCurrentF    = ptrSensorSetting->aeIdxCurrentF;
        // statistic ==> N
        for (int i = 0; i < 25; i++)
        {
            m_sAFInput->i4AEBlockY[i] = data.ispAeInfo.aeBlockV[i];
        }
        m_sAFInput->u4AECmv            = data.ispAeInfo.aeCWValue;
        m_sAFInput->i4SceneLV          = data.ispAeInfo.lightValue_x10;
        m_sAFInput->i4IsAEStable       = data.ispAeInfo.isAEStable;
        m_sAFInput->i4IsAELocked       = data.ispAeInfo.isAELock;
        m_sAFInput->i4DeltaBV          = data.ispAeInfo.deltaIndex;
        m_sAFInput->i4IsAEidxReset     = data.ispAeInfo.isAEScenarioChange;
        m_sAFInput->u4AEFinerEVIdxBase = data.ispAeInfo.aeFinerEVIdxBase;
        if (m_eCurAFMode == MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO || m_eCurAFMode == MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE)
        {
            m_sAFInput->i4IsLVDiffLarge = data.ispAeInfo.isLVChangeTooMuch;
        }
        else
        {
            m_sAFInput->i4IsLVDiffLarge = 0;
        }

        m_i4isAEStable = ((m_sAFInput->i4IsAEStable == 1) || (m_sAFInput->i4IsAELocked == 1)) ? 1 : 0;
        if (m_i4isAEStable)
        {
            UpdateState( EVENT_AE_IS_STABLE);
        }
    }
    // 5. update g/gyro sensor data
    prepareSPData();
    memcpy(m_sAFInput->i4Acce, m_i4AcceInfo, sizeof(MINT32) * 3);
    m_sAFInput->u4AcceScale = m_u4ACCEScale;
    memcpy(m_sAFInput->i4Gyro, m_i4GyroInfo, sizeof(MINT32) * 3);
    m_sAFInput->u4GyroScale = m_u4GyroScale;
    // 6. Global motion vector information
    m_sAFInput->i4GmvX      = data.gmvInfo.GMV_X;
    m_sAFInput->i4GmvY      = data.gmvInfo.GMV_Y;
    m_sAFInput->i4GmvConfX  = data.gmvInfo.GMV_Conf_X;
    m_sAFInput->i4GmvConfY  = data.gmvInfo.GMV_Conf_Y;
    m_sAFInput->i4GmvMax    = data.gmvInfo.GMV_Max;
    // 7. update SDAF/LDAF/PDAF data
    prepareSDData();
    prepareLDData();
    preparePDData();
    // 8. Pre-processing input data for hybrid AF.
    HybridAFPreprocessing();
    AAA_TRACE_END_D;

    if (LEVEL_FLOW)
    {
        // 1. Flow information
        logBuf += sprintf( logBuf,
                           "OUT: hybrid mode(0x%x), temp(%d), ",
                           m_sAFInput->i4HybridAFMode,
                           m_sAFInput->u4CurTemperature);
        // 2. Adaptive Compensation
        logBuf += sprintf( logBuf,
                           "tsSOF = %" PRId64 ", tsMLStart = %" PRId64 ", ",
                           m_sAFInput->TS_SOF,
                           m_sAFInput->TS_MLStart);
        // 4. AE information
        logBuf += sprintf( logBuf,
                           "ispAeInfo : AECmv = %d, ISO = %d, SceneLV = %d, Shutter = %d, IsAEStable = %d, IsAELocked = %d, DeltaBV = %d, IsAEidxReset = %d, AEFinerEVIdxBase = %d, AEidxCurrentF = %d, i4IsLVDiffLarge = %d",
                           m_sAFInput->u4AECmv,
                           m_sAFInput->i4ISO,
                           m_sAFInput->i4SceneLV,
                           m_sAFInput->i4ShutterValue,
                           m_sAFInput->i4IsAEStable,
                           m_sAFInput->i4IsAELocked,
                           m_sAFInput->i4DeltaBV,
                           m_sAFInput->i4IsAEidxReset,
                           m_sAFInput->u4AEFinerEVIdxBase,
                           m_sAFInput->u4AEidxCurrentF,
                           m_sAFInput->i4IsLVDiffLarge);
        // 7. update PD data
        logBuf += sprintf( logBuf,
                           "PDExpt(%d %d), PDVd(%d), LaunchCam(%d), FirsetCalPDFrameCount(%d)",
                           m_bPdInputExpected,
                           m_ptrNVRam->rAFNVRAM.i4EasyTuning[2],
                           m_sAFInput->sPDInfo.i4Valid,
                           m_i4LaunchCamTriggered,
                           m_i4FirsetCalPDFrameCount);
    }
    CAM_LOGD_IF(LEVEL_FLOW, "%s", dbgMsgBuf);

    return err;
}
MINT32 AfMgr::process_doAF()
{
    MINT32 err = 0;

    AAA_TRACE_D("%s #(%d,%d)", __FUNCTION__, m_u4ReqMagicNum, m_u4StaMagicNum);
    if (s_pIAfCxU)
    {
        s_pIAfCxU->doAF(m_sDoAFInput, m_sDoAFOutput, m_pAfoBuf);  // handleAF + MoveLensTo + ispConfig
    }
    AAA_TRACE_END_D;

    // clear algoCommand after doAF
    COMMAND_CLEAR(m_sDoAFInput.algoCommand);

    return err;
}

MINT32 AfMgr::process_doOutput()
{
    MINT32 err = 0;
    MINT32 AfSearchChange = 0;
    AAA_TRACE_D("%s #(%d,%d)", __FUNCTION__, m_u4ReqMagicNum, m_u4StaMagicNum);
    // 0. To get algo output
    if (!err)
    {
        if (s_pIAfCxU)
        {
            s_pIAfCxU->updateOutput(m_sDoAFOutput);
        }
        // Keep skipping algo if HwConstraint ever happened
        if (m_i4SkipAlgoDueToHwConstaint == 0)
        {
            m_i4SkipAlgoDueToHwConstaint = (m_sDoAFOutput.ispOutput.hwConfigInfo.hwConstrainErr > 0);
        }
    }

    // 1. To update flow control parameters according to algo output
    if (!err && s_pIAfCxU)
    {
        m_lensState = getLensState();
        m_i4LensPosExit = m_sAFOutput->i4AFPos;
        // Update parameter for flow controlling
        m_i4IsSelHWROI_PreState = m_i4IsSelHWROI_CurState;
        m_i4IsSelHWROI_CurState = m_sAFOutput->i4IsSelHWROI;
        m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState;
        m_i4IsAFSearch_CurState = m_sAFOutput->i4IsAfSearch;

        // To update MCUInfo after updateOutput
        mcuMotorInfo rMotorInfo;
        m_pMcuDrv->getMCUInfo( &rMotorInfo);
        m_sCurLensInfo.i4CurrentPos   = (MINT32)rMotorInfo.u4CurrentPosition;
        m_sCurLensInfo.bIsMotorOpen   =         rMotorInfo.bIsMotorOpen;
        m_sCurLensInfo.bIsMotorMoving =         rMotorInfo.bIsMotorMoving;
        m_sCurLensInfo.i4InfPos       = (MINT32)rMotorInfo.u4InfPosition;
        m_sCurLensInfo.i4MacroPos     = (MINT32)rMotorInfo.u4MacroPosition;
        m_sCurLensInfo.bIsSupportSR   =         rMotorInfo.bIsSupportSR;

        // update params for flow control
        memcpy(&m_sArea_HW, &m_sDoAFOutput.ispOutput.hwConfigInfo.hwArea, sizeof(AREA_T)); // output info
        // data below SHOULD be move the afobufmgr (read from register)
        m_i4HWBlkNumX = m_sDoAFOutput.ispOutput.hwConfigInfo.hwBlkNumX;     // for parsing AFO
        m_i4HWBlkNumY = m_sDoAFOutput.ispOutput.hwConfigInfo.hwBlkNumY;     // for parsing AFO
        m_i4HWBlkSizeX = m_sDoAFOutput.ispOutput.hwConfigInfo.hwBlkSizeX;   // for parsing AFO
        m_i4HWBlkSizeY = m_sDoAFOutput.ispOutput.hwConfigInfo.hwBlkSizeY;   // for parsing AFO

        // To update the extended FDArea to Area_OTFD
        memcpy(&m_sArea_OTFD, &m_sAFOutput->sExtFdArea.sRect[0], sizeof(AREA_T));
        m_sAFInput->sFDInfo.i4IsFdValid = 0; // reset the isFdValid
        // To update LockAERequest: lockAE when Contrast AF
        if ( !m_i4DbgPDVerifyEn)
        {
            i4IsLockAERequest = (m_sAFOutput->i4IsLockAEReq == 1) ? E_AF_LOCK_AE_REQ_LOCK : E_AF_LOCK_AE_REQ_NO_LOCK;
        }
        // To update timestamp of moveLens
        m_u8MvLensTS = m_sDoAFOutput.mvLensTS;

        // Warning for AdpAlarm
        if (m_sAFOutput->i4AdpAlarm)
        {
            // error log
            CAM_LOGE("#(%5d,%5d) %s Dev(%d) i4AdpAlarm(%d)",
                     m_u4ReqMagicNum,
                     m_u4StaMagicNum,
                     __FUNCTION__,
                     m_i4CurrSensorDev,
                     m_sAFOutput->i4AdpAlarm);
            if (property_get_int32("vendor.debug.af_mgr.adpalarm", 0) > 0)
            {
                // yellow screen (only enable after SQC1)
                AEE_ASSERT_AF("AF AdptAlarm \nCRDISPATCH_KEY: AF_MGR_ASSERT");
            }
        }

        //------------------------------------Update parameter for flow controlling----------------------------------
        m_i4IsFocused = m_sAFOutput->i4IsFocused;
        UpdatePDParam( m_sAFOutput->i4ROISel);

        //Event
        if (m_i4IsAFSearch_PreState != m_i4IsAFSearch_CurState)
        {
            AfSearchChange = 1;
            if (m_i4IsAFSearch_CurState == AF_SEARCH_DONE)
            {
                // Negative Edge: Searching End
                if (m_pMcuDrv)
                {
                    m_pMcuDrv->setMCUParam(MCU_CMD_OIS_DISABLE, m_i4OISDisable);
                }
                SetMgrDbgInfo();

                // It have to send callback once got autofocus command from host
                m_bNeedSendCallback |= m_bNeedLock;

                if (m_bNeedSendCallback)
                {
                    UpdateState( EVENT_SEARCHING_END);
                    m_bNeedSendCallback = 0;
                    m_i4IsCAFWithoutFace = 0;
                    m_bForceDoAlgo = MFALSE;
                }
                else
                {
                    UpdateState( EVENT_SEARCHING_DONE_RESET_PARA); /* Hybrid AF : PDAF, LDAF, ... */
                }
                // The af search for LaunchCamTrigger is done
                if (m_i4LaunchCamTriggered == E_LAUNCH_AF_TRIGGERED)
                {
                    m_i4LaunchCamTriggered = E_LAUNCH_AF_DONE;
                }
            }
            else if ( m_i4IsAFSearch_PreState == AF_SEARCH_CONTRAST && (m_bNeedSendCallback & 2)) // get out of AF_SEARCH_CONTRAST
            {
                CAM_LOGD("#(%5d,%5d) %s Dev(%d) SEARCHING_END because of getting out of AF_SEARCH_CONTRAST",
                         m_u4ReqMagicNum,
                         m_u4StaMagicNum,
                         __FUNCTION__,
                         m_i4CurrSensorDev);
                UpdateState( EVENT_SEARCHING_END);
                m_bNeedSendCallback = 0;
                m_i4IsCAFWithoutFace = 0;
                m_bForceDoAlgo = MFALSE;
            }
            else if ( m_i4IsAFSearch_CurState != AF_SEARCH_DONE)
            {
                // Positive Edge: Searching Start
                if (m_pMcuDrv)
                {
                    m_pMcuDrv->setMCUParam(MCU_CMD_OIS_DISABLE, MTRUE);
                }
                CleanMgrDbgInfo();

                // CONTINUOUS_VIDEO or CONTINUOUS_PICTURE ==> need to check if send callback
                if (m_bNeedCheckSendCallback)
                {
                    if (m_bNeedLock)
                    {
                        /* It have to send callback once got autofocus command from host */
                        m_bNeedSendCallback = 1;
                    }
                    else
                    {
                        m_sCallbackInfo.isSearching        = m_i4IsAFSearch_CurState;
                        m_sCallbackInfo.CompSet_PDCL.Value = m_sAFOutput->i4PdTrigUiConf;
                        m_sCallbackInfo.CompSet_ISO.Value  = m_sAFInput->i4ISO;
                        m_sCallbackInfo.CompSet_FPS.Value  = (m_sAFInput->TS_SOF - TS_SOF_Pre > 0) ? 1000000 / (m_sAFInput->TS_SOF - TS_SOF_Pre) : 0;
                        m_bNeedSendCallback |= checkSendCallback(m_sCallbackInfo); // isSearch switch between 1~3 still need SendCallback
                        CAM_LOGD("#(%5d,%5d) %s Dev(%d) checkSendCallback: m_bNeedSendCallback(%d), AfterAutoMode(%d), Searching(%d), PDCL(%d, %d), ISO(%d, %d), FPS(%d, %d)",
                                 m_u4ReqMagicNum,
                                 m_u4StaMagicNum,
                                 __FUNCTION__,
                                 m_i4CurrSensorDev,
                                 m_bNeedSendCallback,
                                 m_sCallbackInfo.isAfterAutoMode,
                                 m_sCallbackInfo.isSearching,
                                 m_sCallbackInfo.CompSet_PDCL.Value, m_sCallbackInfo.CompSet_PDCL.Target,
                                 m_sCallbackInfo.CompSet_ISO.Value, m_sCallbackInfo.CompSet_ISO.Target,
                                 m_sCallbackInfo.CompSet_FPS.Value, m_sCallbackInfo.CompSet_FPS.Target);
                    }
                    if (m_bNeedSendCallback == 1 && m_sAFOutput->i4ROISel != AF_ROI_SEL_FD)
                    {
                        m_i4IsCAFWithoutFace = 1;   // CAFWithoutFace && NeedSendCallback
                    }
                }
                else // else ==> always send callback
                {
                    m_bNeedSendCallback = 1;
                }

                if (m_bNeedSendCallback == 1)
                {
                    UpdateState( EVENT_SEARCHING_START);
                    m_bNeedSendCallback |= 2;
                }
            }

        }
        // update SD param
        if (!err)
        {
            updateSDParam();
        }

        // prepare FSC data
        if (!err)
        {
            if (m_pMcuDrv)
            {
                mcuMotorInfo rMotorInfo;
                m_pMcuDrv->getMCUInfo(&rMotorInfo);
                DACList.push_front(rMotorInfo.u4CurrentPosition);
                if (DACList.size() > 3)
                {
                    DACList.pop();
                }
            }
            // To update fsc info
            updateFscFrmInfo();
        }
        //---------------------------------------------------------------------------------------------------
        //get current sensor's temperature
        m_sAFInput->u4CurTemperature = getSensorTemperature();
    }

    if (!err)
    {
        // log
        char  dbgMsgBuf[DBG_MSG_BUF_SZ];
        char* logBuf = dbgMsgBuf;
        if (LEVEL_FLOW || AfSearchChange)
        {
            logBuf += sprintf( logBuf,
                               "#(%5d,%5d) %s Dev(%d) cfgNum(%d) ",
                               m_u4ReqMagicNum,
                               m_u4StaMagicNum,
                               __FUNCTION__,
                               m_i4CurrSensorDev,
                               m_sAFOutput->sAFStatConfig.u4ConfigNum);

            logBuf += sprintf( logBuf,
                               "FV(%" PRId64 "), ",
                               m_sAFOutput->i8AFValue);
            if (AfSearchChange)
            {
                logBuf += sprintf( logBuf, "isafsearch changed ");
            }
            logBuf += sprintf( logBuf,
                               "IsAFSearch(%d->%d), IsSelHWROI(%d->%d), ROISel(%d), isLockAEReq(%d) / ",
                               m_i4IsAFSearch_PreState,
                               m_i4IsAFSearch_CurState,
                               m_i4IsSelHWROI_PreState,
                               m_i4IsSelHWROI_CurState,
                               m_sAFOutput->i4ROISel,
                               m_sAFOutput->i4IsLockAEReq);
        }
        CAM_LOGD_IF((LEVEL_FLOW || AfSearchChange), "%s", dbgMsgBuf);

        // real time viewer
        if (m_i4DgbRtvEnable)
        {
            // prepare the af_rtv property
            char value[500] = {'\0'};
            MINT32 count = updateRTVString(value);
            if (count > 0)
            {
                CAM_LOGD("AFRTV : %s", value);
                property_set("vendor.debug.af_rtv.data", value );
            }
        }
    }

    AAA_TRACE_END_D;
    return err;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::updateNVRAM(MUINT32 a_eNVRAMIndex)
{
    if (m_i4DgbDisableAF)
    {
        return S_AF_OK;
    }
    /**********************************************************
     * This function should be called before af mgr is started!!
     **********************************************************/
    // [change mode] or [unvalid mode]
    if ( (a_eNVRAMIndex != m_eNVRAMIndex) || (m_eNVRAMIndex >= AF_CAM_SCENARIO_NUM_2))
    {
        // if original CamScenarioMode is unvalid, reset it to CAM_SCENARIO_PREVIEW
        if (m_eNVRAMIndex >= AF_CAM_SCENARIO_NUM_2)
        {
            m_eNVRAMIndex = 0;
        }
        CAM_LOGD( "#(%5d,%5d) Dev(0x%04x), %s %d -> %d",
                  m_u4ReqMagicNum,
                  m_u4StaMagicNum,
                  m_i4CurrSensorDev,
                  __FUNCTION__,
                  m_eNVRAMIndex,
                  a_eNVRAMIndex);

        if ( a_eNVRAMIndex < AF_CAM_SCENARIO_NUM_2)
        {
            if ( m_ptrLensNVRam != NULL)
            {
                /*************************************************************************************************
                 *
                 * m_ptrLensNVRam is initialized in af mgr start function.
                 * m_ptrLensNVRam!=NULL means AF NVRAM data is valid, so NVRAM data should be updated in algroithm.
                 *
                 *************************************************************************************************/
                m_ptrNVRam = &(m_ptrLensNVRam->AF[a_eNVRAMIndex]);
                CAM_LOGD_IF( LEVEL_FLOW,
                             "#(%5d,%5d) %s [Path]%s [nvram][Offset]%d [Normal Num]%d",
                             m_u4ReqMagicNum,
                             m_u4StaMagicNum,
                             __FUNCTION__,
                             m_ptrNVRam->rFilePath,
                             m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4Offset,
                             m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum);

                // LaunchCamTrigger
                m_i4AEStableTriggerTimeout = m_ptrNVRam->rAFNVRAM.i4EasyTuning[10] > 0 ? m_ptrNVRam->rAFNVRAM.i4EasyTuning[10] : m_i4AEStableTriggerTimeout;
                m_i4ValidPDTriggerTimeout  = m_ptrNVRam->rAFNVRAM.i4EasyTuning[11] > 0 ? m_ptrNVRam->rAFNVRAM.i4EasyTuning[11] : m_i4ValidPDTriggerTimeout;

                // checkSendCallback info : update the target form NVRAM
                m_sCallbackInfo.CompSet_PDCL.Target = m_ptrNVRam->rAFNVRAM.i4EasyTuning[20];
                m_sCallbackInfo.CompSet_ISO.Target = m_ptrNVRam->rAFNVRAM.i4EasyTuning[22];
                m_sCallbackInfo.CompSet_FPS.Target = m_ptrNVRam->rAFNVRAM.i4EasyTuning[21];

                if (m_i4isAFStarted) // start() has been called
                {
                    // To get g/gyro data to get the initPos with posture compensation
                    prepareSPData();
                    m_sStartInput.initAlgoInput.i4AcceScale     = m_u4ACCEScale;
                    memcpy(&(m_sStartInput.initAlgoInput.i4Acce), &(m_i4AcceInfo), sizeof(MINT32) * 3);
                    m_sStartInput.initAlgoInput.i4IsEnterCam    = m_i4IsEnterCam;
                    m_sStartInput.initAlgoInput.i4ISO           = m_sAFInput->i4ISO;
                    m_sStartInput.initAlgoInput.eAFMode         = m_eLIB3A_AFMode;
                    // AFArea
                    memcpy(&m_sStartInput.initAlgoInput.sAFArea.sRect[0], &m_sAFInput->sAFArea.sRect[0], sizeof(AREA_T));
                    memcpy(&(m_sStartInput.initAlgoInput.sEZoom),    &(m_sAFInput->sEZoom),  sizeof(AREA_T));
                    // LensInfo
                    memcpy(&(m_sStartInput.initAlgoInput.sLensInfo), &(m_sCurLensInfo), sizeof(LENS_INFO_T));
                    // config/nvram
                    memcpy(&m_sStartInput.initAlgoInput.sAFNvram, &(m_ptrNVRam->rAFNVRAM), sizeof(AF_NVRAM_T));
                    // check if cancel algo is required
                    if ( m_bLock != MTRUE)
                    {
                        COMMAND_CLEAR(m_sStartInput.initAlgoCommand);
                        m_sStartInput.initAlgoCommand.bCancel = MTRUE;
                    }
                    if (s_pIAfCxU)
                        s_pIAfCxU->updateNVRAM(m_sStartInput, m_sStartOutput);
                    // get AF table start & end from updateAFtableBoundary
                    m_i4AFTabStr = m_sStartOutput.initAlgoOutput.otpAfTableStr;
                    m_i4AFTabEnd = m_sStartOutput.initAlgoOutput.otpAfTableEnd;
                    if ( (m_i4AFTabStr > 0) && ( m_i4AFTabEnd > m_i4AFTabStr))
                    {
                        m_sDAF_TBL.af_dac_min = m_i4AFTabStr;
                        m_sDAF_TBL.af_dac_max = m_i4AFTabEnd;
                    }
                }
            }
            else
            {
                CAM_LOGD( "#(%5d,%5d) %s NVRAM is not initialized, update m_eCamScenarioMode only, no need update to algo", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__);
            }

            //
            m_eNVRAMIndex = a_eNVRAMIndex;
        }
        else
        {
            CAM_LOGD( "#(%5d,%5d) %s Camera NVRAM index invalid (%d), no need update to algo", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, a_eNVRAMIndex);
        }

    }

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT64 AfMgr::MoveLensTo( MINT32 &i4TargetPos, MUINT32 u4Caller)
{
    MUINT64 TS_BeforeMoveMCU = 0; // timestamp of before moveMCU
    MUINT64 TS_AfterMoveMCU = 0;  // timestamp of after moveMCU

    if (u4Caller != AF_SYNC_CALLER)
    {
        CAM_LOGE("%s SHOULD NOT BE USED besides SYNCAF", __FUNCTION__);
        return m_u8MvLensTS;
    }

    if ( m_pMcuDrv)
    {
        if ( m_i4MvLensTo != i4TargetPos)
        {
            if (u4Caller == AF_SYNC_CALLER)
            {
                CAM_LOGD("#(%5d,%5d) SYNC-%s Dev(%d) DAC(%d)", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev, i4TargetPos);
            }

            CAM_LOGD_IF( LEVEL_STT, "%s+", __FUNCTION__);
            TS_BeforeMoveMCU = getTimeStamp_us();
            if (m_pMcuDrv->moveMCU( i4TargetPos))
            {
                m_i4MvLensTo = i4TargetPos;
            }
            TS_AfterMoveMCU = getTimeStamp_us();
            CAM_LOGD_IF( LEVEL_STT, "%s-", __FUNCTION__);
        }

        if ( m_i4DbgOISDisable == 1)
        {
            MUINT32 DbgOISPos = property_get_int32("vendor.debug.af_ois.position", 0);
            // DbgOISPos(XXXXYYYY) = X * 10000 + Y; Ex: 10000200
            if (m_i4DbgOISPos != DbgOISPos)
            {
                CAM_LOGD("Set OIS Position %d", DbgOISPos);
                m_pMcuDrv->setMCUParam(0x02, DbgOISPos);
                m_i4DbgOISPos = DbgOISPos;
            }
        }
    }
    else
    {
        CAM_LOGD_IF( LEVEL_FLOW, "%s Fail, Dev %d", __FUNCTION__, m_i4CurrSensorDev);
    }

    if (TS_BeforeMoveMCU != 0)
    {
        // update time stamp of moveMCU
        return (MUINT64)((TS_BeforeMoveMCU + TS_AfterMoveMCU) / 2);
    }
    else
    {
        // inherit the original one
        return m_u8MvLensTS;
    }
}

MVOID AfMgr::resetAFParams()
{
    //reset member.
    memset( m_sAFInput,           0, sizeof(AF_INPUT_T ));
    memset( m_sAFOutput,          0, sizeof(AF_OUTPUT_T));
    memset( &m_sDoSttInput,        0, sizeof(DoSttIn_T));
    memset( &m_sDoSttOutput,       0, sizeof(DoSttOut_T));
    memset( &m_sCropRegionInfo,    0, sizeof(AREA_T     ));
    memset( &m_sArea_Focusing,     0, sizeof(AREA_T     ));
    memset( &m_sArea_Center,       0, sizeof(AREA_T     ));
    memset( &m_sArea_APCmd,        0, sizeof(AREA_T     ));
    memset( &m_sArea_APCheck,      0, sizeof(AREA_T     ));
    memset( &m_sArea_OTFD,         0, sizeof(AREA_T     ));
    memset( &m_sArea_HW,           0, sizeof(AREA_T     ));
    memset( &m_sArea_Bokeh,        0, sizeof(AREA_T     ));
    memset( &m_sPDRois[0],         0, sizeof(PD_CALCULATION_ROI_T)*eIDX_ROI_ARRAY_NUM);
    memset( &m_sPDCalculateWin[0], 0, sizeof(AFPD_BLOCK_ROI_T    )*AF_PSUBWIN_NUM    );
    memset( &m_aAEBlkVal,          0, sizeof(MUINT8              ) * 25                );
    memset( &m_sIspAeInfo,         0, sizeof(ISPAEInfo_T));
    memset( &m_sFRMInfo,           0, sizeof(AF_FRAME_INFO_T));
    memset( &m_sAFResultConfig,    0, sizeof(AFRESULT_ISPREG_T));
    memset( &m_i4CctAfArea,        0, sizeof(CameraFocusArea_T));

    DACList.clear();
    m_vFrmInfo.clear();
    m_vISPAEQueue.clear();
    m_i4EnableAF    = -1;
    m_i4PDCalculateWinNum = 0;
    m_u8RecvFDTS = 0;
    m_sFocusDis.i4LensPos = 0; /* default value, ref: af_tuning_customer.cpp*/
    m_sFocusDis.fDist    = 0.33; /* default value, ref: af_tuning_customer.cpp*/
    m_i4MvLensTo    = -1;
    m_i4MvLensToPre = 0;
    m_i4HWBlkNumX   = 0;
    m_i4HWBlkNumY   = 0;
    m_i4HWBlkSizeX  = 0;
    m_i4HWBlkSizeY  = 0;
    m_i4IsFocused   = 0;
    m_u4ReqMagicNum = 0;
    m_u4StaMagicNum = 0;
    m_u4ConfigHWNum = 0;
    m_u4ConfigLatency = 3;
    m_bPdInputExpected = MFALSE;
    m_sArea_TypeSel = AF_ROI_SEL_NONE;

    m_i4OTFDLogLv   = 0;
    m_eEvent        = EVENT_CMD_START;
    m_i4IsAFSearch_PreState = AF_SEARCH_DONE; /*Force to select ROI to focusing as first in, ref:SelROIToFocusing */
    m_i4IsAFSearch_CurState = AF_SEARCH_DONE;
    m_bRunPDEn          = MFALSE;
    m_bSDAFEn           = MFALSE;
    m_i4TAFStatus       = TAF_STATUS_RESET;
    m_lensState         = 0;
    m_i4DbgOISPos       = 0;
    i4IsLockAERequest   = E_AF_LOCK_AE_REQ_NO_LOCK;
    m_i4IsCAFWithoutFace = 0;
    m_bForceDoAlgo = MFALSE;
    CAM_LOGD("AF-%-15s: Dev(0x%04x), set CurAFMode to EDOF to enable LaunchCamTrigger",
             __FUNCTION__,
             m_i4CurrSensorDev);
    m_eCurAFMode = MTK_CONTROL_AF_MODE_EDOF;
    // LaunchCamTrigger : disable at the first
    m_i4LaunchCamTriggered = E_LAUNCH_AF_IDLE;
    m_i4AEStableFrameCount = -1;
    m_i4ValidPDFrameCount = -1;
    m_i4AEStableTriggerTimeout = AESTABLE_TIMEOUT;
    m_i4ValidPDTriggerTimeout = VALIDPD_TIMEOUT;

    m_i4FirsetCalPDFrameCount = -1;
    m_i4isAEStable = MFALSE;
    m_i4ContinuePDMovingCount = 0;

    m_eAFStatePre = m_eAFState;
    m_i4EnThermalComp = 0;
    m_i4IsCctOper = 0;
    m_i4CctAfMode = 0;
    m_i4CctTriggerAf = 0;
    m_i4CctMfPos = 1024;
    m_fMfFocusDistance = -1;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setCropRegionInfo( MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height, MUINT32 u4Caller)
{
    if ( u4Width == 0 || u4Height == 0 )
    {
        return S_AF_OK;
    }

    /* Digital Zoom : skip crop reggion info durning AF seraching */
    if ( m_i4IsAFSearch_CurState != AF_SEARCH_DONE)
    {
        return S_AF_OK;
    }

    if ( (u4XOffset < m_i4TGSzW) &&
            (u4YOffset < m_i4TGSzH) &&
            (u4XOffset + u4Width <= m_i4TGSzW) &&
            (u4YOffset + u4Height <= m_i4TGSzH) &&
            (u4Width > 0) &&
            (u4Height > 0) )
    {
        // set crop region information and update center ROI automatically.
        if ( m_sCropRegionInfo.i4X != (MINT32)u4XOffset ||
                m_sCropRegionInfo.i4Y != (MINT32)u4YOffset ||
                m_sCropRegionInfo.i4W != (MINT32)u4Width   ||
                m_sCropRegionInfo.i4H != (MINT32)u4Height )
        {
            m_sCropRegionInfo.i4X = (MINT32)u4XOffset;
            m_sCropRegionInfo.i4Y = (MINT32)u4YOffset;
            m_sCropRegionInfo.i4W = (MINT32)u4Width;
            m_sCropRegionInfo.i4H = (MINT32)u4Height;

            //calculate zoom information : 1X-> 100, 2X->200, ...
            MUINT32 dzfX = 100 * m_i4TGSzW / m_sCropRegionInfo.i4W;
            MUINT32 dzfY = 100 * m_i4TGSzH / m_sCropRegionInfo.i4H;
            if ( u4Caller == AF_MGR_CALLER)
            {
                CAM_LOGD( "#(%5d,%5d) %s (x,y,w,h)=(%d, %d, %d, %d), dzX(%d), dzY(%d)",
                          m_u4ReqMagicNum,
                          m_u4StaMagicNum,
                          __FUNCTION__,
                          u4XOffset,
                          u4YOffset,
                          u4Width,
                          u4Height,
                          dzfX,
                          dzfY);
            }
            else
            {
                CAM_LOGD( "#(%5d,%5d) cmd-%s (x,y,w,h)=(%d, %d, %d, %d), dzX(%d), dzY(%d)",
                          m_u4ReqMagicNum,
                          m_u4StaMagicNum,
                          __FUNCTION__,
                          u4XOffset,
                          u4YOffset,
                          u4Width,
                          u4Height,
                          dzfX,
                          dzfY);
            }

            // Accroding to crop region(digital zoom), resizing ROI-C coordinate.
            UpdateCenterROI( m_sArea_Center);
            //Reset all focusing window.
            m_sArea_APCheck = m_sArea_APCmd = m_sArea_OTFD = m_sArea_HW = m_sArea_Center;
            m_sArea_TypeSel = eAF_ROI_TYPE_CENTER;

            // it is always valid for calculation center roi once crop region is set.
            m_sPDRois[eIDX_ROI_ARRAY_CENTER].valid       = MTRUE;
            m_sPDRois[eIDX_ROI_ARRAY_CENTER].info.sType  = eAF_ROI_TYPE_CENTER;
            m_sPDRois[eIDX_ROI_ARRAY_CENTER].info.sPDROI = m_sArea_Center;
        }
        else
        {
            CAM_LOGD_IF( LEVEL_VERBOSE, "%s same cmd", __FUNCTION__);
        }
    }
    else
    {
        CAM_LOGD_IF( LEVEL_FLOW, "%s not valid", __FUNCTION__);
    }
    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setAFArea( CameraFocusArea_T &sInAFArea)
{
    /* sInAFArea is TG base coordinate */

    MRESULT ret = E_3A_ERR;

    char  dbgMsgBuf[DBG_MSG_BUF_SZ];
    char* ptrMsgBuf = dbgMsgBuf;

    if ( LEVEL_FLOW)
    {
        ptrMsgBuf += sprintf( ptrMsgBuf, "#(%5d,%5d) cmd-%s : [Cnt]%d [L]%d [R]%d [U]%d [B]%d, ",
                              m_u4ReqMagicNum,
                              m_u4StaMagicNum,
                              __FUNCTION__,
                              sInAFArea.u4Count,
                              sInAFArea.rAreas[0].i4Left,
                              sInAFArea.rAreas[0].i4Right,
                              sInAFArea.rAreas[0].i4Top,
                              sInAFArea.rAreas[0].i4Bottom);
    }

    if ( 1 <= sInAFArea.u4Count)
    {
        if ( // boundary check.
            (sInAFArea.rAreas[0].i4Left < sInAFArea.rAreas[0].i4Right ) &&
            (sInAFArea.rAreas[0].i4Top  < sInAFArea.rAreas[0].i4Bottom) &&
            (0 <= sInAFArea.rAreas[0].i4Left && sInAFArea.rAreas[0].i4Right <= (MINT32)m_i4TGSzW) &&
            (0 <= sInAFArea.rAreas[0].i4Top  && sInAFArea.rAreas[0].i4Bottom <= (MINT32)m_i4TGSzH))
        {
            // To remain the center
            MUINT32 InWidth  = sInAFArea.rAreas[0].i4Right - sInAFArea.rAreas[0].i4Left;
            MUINT32 InHeight = sInAFArea.rAreas[0].i4Bottom - sInAFArea.rAreas[0].i4Top;
            MUINT32 Center_X = sInAFArea.rAreas[0].i4Left + InWidth / 2;
            MUINT32 Center_Y = sInAFArea.rAreas[0].i4Top + InHeight / 2;

            MUINT32 Percent_TouchAFROI = 100;
            // To resize the Touch AF ROI if it is required from users.
            if (m_ptrNVRam->rAFNVRAM.i4EasyTuning[3] > 0 && m_ptrNVRam->rAFNVRAM.i4EasyTuning[3] <= 200)
            {
                Percent_TouchAFROI = m_ptrNVRam->rAFNVRAM.i4EasyTuning[3];
            }
            // To override the Percent_TouchAFROI based on ISO target
            if (m_ptrNVRam->rAFNVRAM.i4EasyTuning[5] > 0 && m_ptrNVRam->rAFNVRAM.i4EasyTuning[5] <= 200 && m_ptrNVRam->rAFNVRAM.i4EasyTuning[4] > 0)
            {
                if (m_sAFInput->i4ISO > m_ptrNVRam->rAFNVRAM.i4EasyTuning[4])
                {
                    Percent_TouchAFROI = m_ptrNVRam->rAFNVRAM.i4EasyTuning[5];
                }
            }
            // To resize the Touch AF ROI if it is required from users.
            if (Percent_TouchAFROI != 100)
            {
                InWidth =  InWidth * Percent_TouchAFROI / 100.0;
                InHeight = InHeight * Percent_TouchAFROI / 100.0;
                // To resize the ROI from tuning parameter
                MUINT32 TempLeft   = Center_X - (InWidth / 2);
                MUINT32 TempRight  = TempLeft + InWidth;
                MUINT32 TempTop    = Center_Y - (InHeight / 2);
                MUINT32 TempBottom = TempTop  + InHeight;
                // Left Boundary Check
                if (TempLeft < 0)
                {
                    TempLeft  = 0;
                    TempRight = InWidth;
                }
                // Right Boundary Check
                if (TempRight >= m_i4TGSzW)
                {
                    TempRight = (m_i4TGSzW - 1);
                    TempLeft  = TempRight - InWidth;
                }
                // Top Boundary Check
                if (TempTop < 0)
                {
                    TempTop    = 0;
                    TempBottom = InHeight;
                }
                // Bottom Boundary Check
                if (TempBottom >= m_i4TGSzH)
                {
                    TempBottom = (m_i4TGSzH - 1);
                    TempTop    = TempBottom - InHeight;
                }
                CAM_LOGD_IF(LEVEL_FLOW,
                            "#(%5d,%5d) cmd-%s InAFArea is resized from users' requirement, Resize Percent(%d), ROI(L,R,U,B): Ori(%d,%d,%d,%d)->Rescale(%d,%d,%d,%d)",
                            m_u4ReqMagicNum,
                            m_u4StaMagicNum,
                            __FUNCTION__,
                            Percent_TouchAFROI,
                            sInAFArea.rAreas[0].i4Left,
                            sInAFArea.rAreas[0].i4Right,
                            sInAFArea.rAreas[0].i4Top,
                            sInAFArea.rAreas[0].i4Bottom,
                            TempLeft, TempRight, TempTop, TempBottom );
                // To update the resized touch AF ROI
                sInAFArea.rAreas[0].i4Left   = TempLeft;
                sInAFArea.rAreas[0].i4Right  = TempRight;
                sInAFArea.rAreas[0].i4Top    = TempTop;
                sInAFArea.rAreas[0].i4Bottom = TempBottom;
            }
            /*
            // If InAFArea.W < ROIWidth || InAFArea.H < ROIWidth, resize the AreaSize
            // Resize rule : the rectangular centered by the touch position
            //  - Center = InAFArea.Center
            //  - Width = ShortLine * PERCENT_AFROI / 100.0;
            */
            MINT32 Percent_AFROI = 10; // default : 20%
            if (m_ptrNVRam->rAFNVRAM.i4EasyTuning[1] >= 1 && m_ptrNVRam->rAFNVRAM.i4EasyTuning[1] <= 100)
            {
                Percent_AFROI = m_ptrNVRam->rAFNVRAM.i4EasyTuning[1];
            }
            MDOUBLE ShortLine = min(m_i4TGSzW, m_i4TGSzH);
            MUINT32 ROIWidth  = (MINT32)(ShortLine * Percent_AFROI / 100.0);
            if (InWidth < ROIWidth || InHeight < ROIWidth)
            {
                // To resize the rectangular
                MUINT32 TempLeft   = Center_X - ROIWidth / 2;
                MUINT32 TempRight  = TempLeft + ROIWidth;
                MUINT32 TempTop    = Center_Y - ROIWidth / 2;
                MUINT32 TempBottom = TempTop  + ROIWidth;
                // Left Boundary Check
                if (TempLeft < 0)
                {
                    TempLeft  = 0;
                    TempRight = ROIWidth;
                }
                // Right Boundary Check
                if (TempRight >= m_i4TGSzW)
                {
                    TempRight = m_i4TGSzW - 1;
                    TempLeft  = TempRight - ROIWidth;
                }
                // Top Boundary Check
                if (TempTop < 0)
                {
                    TempTop    = 0;
                    TempBottom = ROIWidth;
                }
                // Bottom Boundary Check
                if (TempBottom >= m_i4TGSzH)
                {
                    TempBottom = (m_i4TGSzH - 1);
                    TempTop    = TempBottom - ROIWidth;
                }
                CAM_LOGD("#(%5d,%5d) cmd-%s InAFArea is too small, MinROIWidth(%d), Ori(%d,%d,%d,%d)->rescale(%d,%d,%d,%d)",
                         m_u4ReqMagicNum,
                         m_u4StaMagicNum,
                         __FUNCTION__,
                         ROIWidth,
                         sInAFArea.rAreas[0].i4Left,
                         sInAFArea.rAreas[0].i4Right,
                         sInAFArea.rAreas[0].i4Top,
                         sInAFArea.rAreas[0].i4Bottom,
                         TempLeft, TempRight, TempTop, TempBottom );
                // update the InAFArea
                sInAFArea.rAreas[0].i4Left   = TempLeft;
                sInAFArea.rAreas[0].i4Right  = TempRight;
                sInAFArea.rAreas[0].i4Top    = TempTop;
                sInAFArea.rAreas[0].i4Bottom = TempBottom;
            }

            AREA_T roi;

            roi.i4X =  sInAFArea.rAreas[0].i4Left;
            roi.i4Y =  sInAFArea.rAreas[0].i4Top;
            roi.i4W =  sInAFArea.rAreas[0].i4Right  - sInAFArea.rAreas[0].i4Left;
            roi.i4H =  sInAFArea.rAreas[0].i4Bottom - sInAFArea.rAreas[0].i4Top;
            roi.i4Info = AF_MARK_NONE;

            if ( memcmp( &roi, &m_sArea_APCheck, sizeof(AREA_T)) != 0)
            {
                //store command.
                m_sArea_APCheck = m_sArea_APCmd = roi;

                m_sPDRois[eIDX_ROI_ARRAY_AP].info.sType  = eAF_ROI_TYPE_AP;
                m_sPDRois[eIDX_ROI_ARRAY_AP].info.sPDROI = m_sArea_APCmd;

                UpdateState( EVENT_CMD_SET_AF_REGION);

                CAM_LOGD( "#(%5d,%5d) cmd-%s Got ROI changed cmd. [Cnt]%d (L,R,U,B)=(%d,%d,%d,%d) => (X,Y,W,H)=(%d,%d,%d,%d)",
                          m_u4ReqMagicNum,
                          m_u4StaMagicNum,
                          __FUNCTION__,
                          sInAFArea.u4Count,
                          sInAFArea.rAreas[0].i4Left,
                          sInAFArea.rAreas[0].i4Right,
                          sInAFArea.rAreas[0].i4Top,
                          sInAFArea.rAreas[0].i4Bottom,
                          m_sArea_APCmd.i4X,
                          m_sArea_APCmd.i4Y,
                          m_sArea_APCmd.i4W,
                          m_sArea_APCmd.i4H);

                // control laser AF touch behavior.
                if (m_bLDAFEn == MTRUE)
                {
                    MINT32 Centr_X = m_sCropRegionInfo.i4X + (m_sCropRegionInfo.i4W / 2);
                    MINT32 Centr_Y = m_sCropRegionInfo.i4Y + (m_sCropRegionInfo.i4H / 2);
                    MINT32 Touch_X = (sInAFArea.rAreas[0].i4Left + sInAFArea.rAreas[0].i4Right) / 2;
                    MINT32 Touch_Y = (sInAFArea.rAreas[0].i4Top  + sInAFArea.rAreas[0].i4Bottom) / 2;

                    m_sAFInput->sLaserInfo.i4AfWinPosX = abs(Centr_X - Touch_X);
                    m_sAFInput->sLaserInfo.i4AfWinPosY = abs(Centr_Y - Touch_Y);

                    if ((m_sAFInput->sLaserInfo.i4AfWinPosX < LASER_TOUCH_REGION_W) && (m_sAFInput->sLaserInfo.i4AfWinPosY < LASER_TOUCH_REGION_H))
                    {
                        if ((m_eLIB3A_AFMode == LIB3A_AF_MODE_AFS) || (m_eLIB3A_AFMode == LIB3A_AF_MODE_MACRO))
                        {
                            m_sAFInput->sLaserInfo.i4AfWinPosCnt++;
                        }
                    }
                    else
                    {
                        m_sAFInput->sLaserInfo.i4AfWinPosCnt = 0;
                    }
                }

                ret = S_AF_OK;
            }
            else
            {
                if ( LEVEL_FLOW)
                {
                    ptrMsgBuf += sprintf( ptrMsgBuf, "ROI cmd is same");
                }
            }
        }
        else
        {
            //command is not valid, using center window.
            if ( LEVEL_FLOW)
            {
                ptrMsgBuf += sprintf( ptrMsgBuf, "ROI cmd is not correct");
            }
        }
    }
    else
    {
        //command is not valid, using center window.
        if ( LEVEL_FLOW)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "ROI cnt=0!!");
        }
    }

    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setIspAEInfo(ISPAEInfo_T ispAeInfo)
{
    CAM_LOGD_IF( LEVEL_FLOW,
                 "cmd-%s FrameNum(%5d), isAEStable(%5d), isAELock(%4d), isAEScenarioChange(%d), deltaIndex(%d), afeGain(%d), ispGain(%d), exposuretime(%4d), realISOValue(%d), aeFinerEVIdxBase(%d), aeIdxCurrentF(%d), aeCWValue(%d), lightValue_x10(%d)",
                 __FUNCTION__,
                 ispAeInfo.FrameNum,
                 ispAeInfo.isAEStable,
                 ispAeInfo.isAELock,
                 ispAeInfo.isAEScenarioChange,
                 ispAeInfo.deltaIndex,
                 ispAeInfo.afeGain,
                 ispAeInfo.ispGain,
                 ispAeInfo.exposuretime,
                 ispAeInfo.realISOValue,
                 ispAeInfo.aeFinerEVIdxBase,
                 ispAeInfo.aeIdxCurrentF,
                 ispAeInfo.aeCWValue,
                 ispAeInfo.lightValue_x10);

    m_vISPAEQueue.pushHeadAnyway(ispAeInfo);

    if (m_i4DbgAfegainQueue & 0x1)
    {
        MUINT32 front = m_vISPAEQueue.head;
        MUINT32 end = m_vISPAEQueue.tail;

        CAM_LOGD("--> Head:%d, Tail:%d, FrameId:%d, AfeGain:%d, IspGain:%d, exposuretime:%d",
                 front,
                 end,
                 ispAeInfo.FrameNum,
                 ispAeInfo.afeGain,
                 ispAeInfo.ispGain,
                 ispAeInfo.exposuretime);

        if (end > front)
        {
            front += m_vISPAEQueue.queueSize;
        }

        for (MUINT32 i = front; i > end; i--)
        {
            MUINT32 idx = i % m_vISPAEQueue.queueSize;
            CAM_LOGD("AfeGain qIdx(%d): frameId(%d), afeGain(%d), ispGain(%d), exposuretime:%d",
                     idx,
                     m_vISPAEQueue.content[idx].FrameNum,
                     m_vISPAEQueue.content[idx].afeGain,
                     m_vISPAEQueue.content[idx].ispGain,
                     m_vISPAEQueue.content[idx].exposuretime);
        }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setFocusDistance( MFLOAT lens_focusDistance)
{
    if ( lens_focusDistance < 0)
    {
        return;
    }
    MINT32 i4ResultDac = 0;
    // data from OTP
    MFLOAT fDAC_Inf        = (MFLOAT)m_sDAF_TBL.af_dac_inf; // unit:DAC
    MFLOAT fDAC_Macro      = (MFLOAT)m_sDAF_TBL.af_dac_marco; // unit:DAC
    MFLOAT fDistance_Inf   = (MFLOAT)m_sDAF_TBL.af_distance_inf; // unit:mm
    MFLOAT fDistance_Macro = (MFLOAT)m_sDAF_TBL.af_distance_marco; // unit:mm
    MFLOAT fDist_Inf_rev   = 0.0;
    MFLOAT fDist_Macro_rev = 0.0;
    if (m_eLIB3A_AFMode == LIB3A_AF_MODE_MF)
    {
        m_fMfFocusDistance = lens_focusDistance;

        // set fd_inf and fd_macro to default if there's no data in OTP
        if (fDistance_Inf <= fDistance_Macro)
        {
            fDistance_Inf = 5000;
            fDistance_Macro = 100;
        }
        fDist_Inf_rev = 1000.0 / fDistance_Inf;
        fDist_Macro_rev = 1000.0 / fDistance_Macro;

        // focusDistance -> DAC
        i4ResultDac = (MINT32)(fDAC_Macro + ((lens_focusDistance - fDist_Macro_rev) / (fDist_Inf_rev - fDist_Macro_rev) * (fDAC_Inf - fDAC_Macro)));
    }
    else
    {
        m_fMfFocusDistance = -1;
    }

    // API2: At MTK_CONTROL_AF_MODE_OFF mode, configure algorithm as MF mode.
    //          The auto-focus routine does not control the lens. Lens is controlled by the application.
    if ( (m_eLIB3A_AFMode == LIB3A_AF_MODE_MF) &&
            (m_sAFOutput->i4AFPos != i4ResultDac) &&
            (0 <= i4ResultDac) &&
            (i4ResultDac <= 1023))
    {
        CAM_LOGD( "#(%5d,%5d) cmd-%s Dev(%d) DAC(%d->%d) Dist(%f); fDist_Macro_rev %f, fDist_Inf_rev %f, fDAC_Macro %f, fDAC_Inf %f",
                  m_u4ReqMagicNum,
                  m_u4StaMagicNum,
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  m_sAFOutput->i4AFPos,
                  i4ResultDac,
                  lens_focusDistance,
                  fDist_Macro_rev,
                  fDist_Inf_rev,
                  fDAC_Macro,
                  fDAC_Inf);

        m_i4MFPos = i4ResultDac;
    }
    else
    {
        if ((LEVEL_FLOW) || (m_eLIB3A_AFMode == LIB3A_AF_MODE_MF))
        {
            CAM_LOGD("#(%5d,%5d) cmd-%s Dev(%d) !!SKIP!! DAC(%d->%d) Dist(%f) lib_afmode(%d)",
                     m_u4ReqMagicNum,
                     m_u4StaMagicNum,
                     __FUNCTION__,
                     m_i4CurrSensorDev,
                     m_sAFOutput->i4AFPos,
                     i4ResultDac,
                     lens_focusDistance,
                     m_eLIB3A_AFMode);
        }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::checkPrecapture( eAFControl ePrecap)
{
    if ( m_i4EnableAF == 0)
    {
        return S_AF_OK;
    }

    if ( ePrecap == AfCommand_Start)
    {
        UpdateState( EVENT_SET_WAIT_FORCE_TRIGGER);
    }
    else if (ePrecap == AfCommand_Cancel)
    {
        UpdateState( EVENT_CANCEL_WAIT_FORCE_TRIGGER);
    }
    return S_AF_OK;
}
MRESULT AfMgr::checkAutoFocus(eAFControl eAF)
{
    if ( m_i4EnableAF == 0)
    {
        return S_AF_OK;
    }

    if (eAF == AfCommand_Start)
    {
        autoFocus();
    }
    else if (eAF == AfCommand_Cancel)
    {
        cancelAutoFocus();
    }

    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::triggerAF( MUINT32 u4Caller)
{
    if ( u4Caller == AF_MGR_CALLER)
        CAM_LOGD( "%s  Dev(%d) lib_afmode(%d)", __FUNCTION__, m_i4CurrSensorDev, m_eLIB3A_AFMode);
    else
        CAM_LOGD( "cmd-%s  Dev(%d) lib_afmode(%d)", __FUNCTION__, m_i4CurrSensorDev, m_eLIB3A_AFMode);

    UpdateState( EVENT_CMD_TRIGGERAF_WITH_AE_STBL);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::autoFocus()
{
    CAM_LOGD( "cmd-%s Dev %d : lib_afmode %d", __FUNCTION__, m_i4CurrSensorDev, m_eLIB3A_AFMode);

    if ( m_i4EnableAF == 0)
    {
        CAM_LOGD( "autofocus : dummy lens");
        return;
    }

    UpdateState( EVENT_CMD_AUTOFOCUS);

    m_sDoAFInput.algoCommand.bTargetAssistMove = MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::cancelAutoFocus()
{
    CAM_LOGD( "#(%5d,%5d) cmd-%s Dev(%d) lib_afmode(%d)",
              m_u4ReqMagicNum,
              m_u4StaMagicNum,
              __FUNCTION__,
              m_i4CurrSensorDev,
              m_eLIB3A_AFMode);

    //update parameters and status.
    UpdateState(EVENT_CMD_AUTOFOCUS_CANCEL);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setAdptCompInfo( MUINT64 TS_SOF)
{
    // info from AfMgr
    m_sAFInput->TS_MLStart  = m_u8MvLensTS; // unit: us

    // info from Hal3A
    TS_SOF_Pre = m_sAFInput->TS_SOF;  // previous TS_AFDone unit: us
    m_sAFInput->TS_SOF = TS_SOF;      // current TS_AFDone, unit: us

    // debug
    if (LEVEL_FLOW)
    {
        char  dbgMsgBuf[DBG_MSG_BUF_SZ];
        char* ptrMsgBuf = dbgMsgBuf;
        ptrMsgBuf += sprintf( ptrMsgBuf, "#(%5d,%5d) cmd-%s Dev(%d): ", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        ptrMsgBuf += sprintf( ptrMsgBuf, "TS_SOF = %" PRId64 " \\ ", m_sAFInput->TS_SOF);
        ptrMsgBuf += sprintf( ptrMsgBuf, "TS_MLStart= %" PRId64 " \\ ", m_sAFInput->TS_MLStart);
        ptrMsgBuf += sprintf( ptrMsgBuf, "PixelClk = %d \\ ", m_sAFInput->PixelClk);
        ptrMsgBuf += sprintf( ptrMsgBuf, "PixelInLine = %d \\ ", m_sAFInput->PixelInLine);
        CAM_LOGD("%s", dbgMsgBuf);
    }
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISPAEInfo_T* AfMgr::getMatchedISPAEInfoFromFrameId(MINT32 frameId)
{
    m_sIspAeInfo.FrameNum = -1;

    MUINT32 front = m_vISPAEQueue.head;
    MUINT32 end = m_vISPAEQueue.tail;

    if (end > front)
        front += m_vISPAEQueue.queueSize;

    // assume frameid is continuous, calculate idx by frameid difference
    MUINT32 offset = m_vISPAEQueue.content[(front % m_vISPAEQueue.queueSize)].FrameNum - frameId;
    MUINT32 idx = (front - offset) % m_vISPAEQueue.queueSize;

    int enableDebug = m_i4DbgAfegainQueue & 0x2;
    if (enableDebug)
    {
        CAM_LOGD("search afegain queue... magic: %d, head frameid: %d, idx: %d, indexed frameid: %d",
                 frameId, m_vISPAEQueue.content[(front % m_vISPAEQueue.queueSize)].FrameNum, idx, m_vISPAEQueue.content[idx].FrameNum);
    }

    if (idx < m_vISPAEQueue.queueSize && m_vISPAEQueue.content[idx].FrameNum == frameId) //found
    {
        memcpy(&m_sIspAeInfo, &(m_vISPAEQueue.content[idx]), sizeof(ISPAEInfo_T));
    }
    else // fail by indexing, search one by one
    {
        if (enableDebug)
        {
            CAM_LOGD("search afegain queue... Failed by indexing, search one by one, found idx = %d", idx);
        }
        for (MUINT32 i = front; i > end; i--)
        {
            idx = i % m_vISPAEQueue.queueSize;
            if (m_vISPAEQueue.content[idx].FrameNum == frameId) //found
                memcpy(&m_sIspAeInfo, &(m_vISPAEQueue.content[idx]), sizeof(ISPAEInfo_T));
        }
    }
    // cannot get matched afegain from queue
    if (m_sIspAeInfo.FrameNum == -1)
    {
        CAM_LOGW("Cannot get matched afegain from m_vISQueue!");

        if (enableDebug)
        {
            front = m_vISPAEQueue.head;
            end = m_vISPAEQueue.tail;

            if (end > front)
                front += m_vISPAEQueue.queueSize;
            for (MUINT32 i = front; i > end; i--)
            {
                MUINT32 idx = i % m_vISPAEQueue.queueSize;
                CAM_LOGD("AfeGain qIdx(%d): frameId(%d), afeGain(%d), ispGain(%d), AEIdxCurrentF(%d)",
                         idx, m_vISPAEQueue.content[idx].FrameNum, m_vISPAEQueue.content[idx].afeGain, m_vISPAEQueue.content[idx].ispGain, m_vISPAEQueue.content[idx].aeIdxCurrentF);
            }
        }
    }
    return &m_sIspAeInfo;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAfSupport()
{
    MINT32 isAfSupport = 0;
    // with VCM ==> TRUE
    // else     ==> FALSE
    if (m_i4DgbDisableAF)
    {
        m_i4LensSupport = 0;
        CAM_LOGW( "[%s] disabled by ADB", __FUNCTION__);
    }
    else if (m_i4LensSupport == -1)
    {
        IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();

        SensorStaticInfo rSensorStaticInfo;
        switch ( m_i4CurrSensorDev)
        {
        case ESensorDev_Main:
            pIHalSensorList->querySensorStaticInfo( NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
            break;
        case ESensorDev_Sub:
            pIHalSensorList->querySensorStaticInfo( NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
            break;
        case ESensorDev_MainSecond:
            pIHalSensorList->querySensorStaticInfo( NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
            break;
        case ESensorDev_SubSecond:
            pIHalSensorList->querySensorStaticInfo( NSCam::SENSOR_DEV_SUB_2, &rSensorStaticInfo);
            break;
        case ESensorDev_MainThird:
            pIHalSensorList->querySensorStaticInfo( NSCam::SENSOR_DEV_MAIN_3, &rSensorStaticInfo);
            break;
        default:
            CAM_LOGE( "Invalid sensor device: %d", m_i4CurrSensorDev);
            break;
        }
        MCUDrv *pMcuDrv = MCUDrv::getInstance(m_i4CurrSensorDev);
        m_i4LensSupport = pMcuDrv->isLensSupport(rSensorStaticInfo.sensorDevID) == 0 ? 0 : AF_WIN_NUM_SPOT;
    }
    isAfSupport = (m_i4LensSupport > 0) ? AF_WIN_NUM_SPOT : 0;

    return isAfSupport;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAfThreadEnable()
{
    MINT32 isAfThreadEnable = 0;
    if (m_i4DgbDisableAF)
    {
        CAM_LOGW( "[%s] disabled by ADB", __FUNCTION__);
    }
    else
    {
        // AFSupport      ==> TRUE
        // FVOutputInFF   ==> TRUE
        // else           ==> FALSE
        if (getAfSupport())
        {
            isAfThreadEnable = 1;
        }
        else
        {
            if (m_i4IsEnableFVInFixedFocus == -1) // check once only
            {
                m_i4IsEnableFVInFixedFocus = ForceEnableFVInFixedFocus(m_i4CurrSensorDev);
                MINT32 type = property_get_int32("vendor.debug.af_fv.switch", 0);
                if (m_i4IsEnableFVInFixedFocus == 0 && type == 1)
                {
                    m_i4IsEnableFVInFixedFocus = 1;
                }
                if (m_i4IsEnableFVInFixedFocus == 1 && type == 2)
                {
                    m_i4IsEnableFVInFixedFocus = 0;
                }
            }
            isAfThreadEnable = m_i4IsEnableFVInFixedFocus;
        }
    }

    return isAfThreadEnable;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAfHwEnableByP1()
{
    MINT32 isAfHwEnableByP1 = 0;

    if (m_i4DgbDisableAF)
    {
        CAM_LOGW( "[%s] disabled by ADB", __FUNCTION__);
    }
    else
    {
        if (!s_pIAfCxU)
        {
            CAM_LOGD("%s IAfCxU::getInstance", __FUNCTION__);
            s_pIAfCxU = IAfCxU::getInstance(m_i4CurrSensorDev, m_i4SensorIdx, m_i4ForceCPU);
        }
        else if (s_pIAfCxU->isCCUAF() == 0)
        {
            // AfCxU::getInstance again if current instance is CPUAF
            s_pIAfCxU = IAfCxU::getInstance(m_i4CurrSensorDev, m_i4SensorIdx, m_i4ForceCPU);
        }
        if (s_pIAfCxU)
        {
            m_i4IsCCUAF = s_pIAfCxU->isCCUAF();
        }

        // w/  CCU ==> FALSE
        // w/o CCU
        //---- isAFSupport    ==> TRUE
        //---- FVoutputInFF   ==> TRUE
        //---- else           ==> FALSE
        if (!m_i4IsCCUAF) // CPUAF : afo may need to be enable by P1
        {
            if (getAfSupport()) // AF support ==> afo SHOULD be enabled
            {
                isAfHwEnableByP1 = 1;
                m_i4IsEnableFVInFixedFocus = 0;
            }
            else // AF not support ==> check if m_i4IsEnableFVInFixedFocus
            {
                if (m_i4IsEnableFVInFixedFocus == -1) // check once only
                {
                    m_i4IsEnableFVInFixedFocus = ForceEnableFVInFixedFocus(m_i4CurrSensorDev);
                    MINT32 type = property_get_int32("vendor.debug.af_fv.switch", 0);
                    if (m_i4IsEnableFVInFixedFocus == 0 && type == 1)
                    {
                        m_i4IsEnableFVInFixedFocus = 1;
                    }
                    if (m_i4IsEnableFVInFixedFocus == 1 && type == 2)
                    {
                        m_i4IsEnableFVInFixedFocus = 0;
                    }
                }
                isAfHwEnableByP1 = m_i4IsEnableFVInFixedFocus;
            }
        }
    }
    return isAfHwEnableByP1;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getMaxLensPos()
{
    MINT32 maxLensPos = 0;
    if ( m_i4EnableAF && m_pMcuDrv)
    {
        mcuMotorInfo MotorInfo;
        m_pMcuDrv->getMCUInfo( &MotorInfo);
        maxLensPos = (MINT32)MotorInfo.u4MacroPosition;
    }
    CAM_LOGD_IF( LEVEL_FLOW, "[%s] EnableAF(%d), MCU (%p), MaxLensPos(%d)", __FUNCTION__, m_i4EnableAF, (void*)m_pMcuDrv, maxLensPos);

    return maxLensPos;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getMinLensPos()
{
    MINT32 minLensPos = 0;
    if ( m_i4EnableAF && m_pMcuDrv)
    {
        mcuMotorInfo MotorInfo;
        m_pMcuDrv->getMCUInfo( &MotorInfo);
        minLensPos = (MINT32)MotorInfo.u4InfPosition;
    }
    CAM_LOGD_IF( LEVEL_FLOW, "[%s] EnableAF(%d), MCU (%p), MinLensPos(%d)", __FUNCTION__, m_i4EnableAF, (void*)m_pMcuDrv, minLensPos);

    return minLensPos;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 AfMgr::getSensorTemperature()
{
    MUINT32 u4temperature = 0;

    if ( m_ptrIHalSensor)
    {
        MUINT64 timestamp0 = 0, timestamp1 = 0;
        MUINT32 timediff = 0;

        timestamp0 = getTimeStamp_us();
        m_ptrIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_TEMPERATURE_VALUE, (MINTPTR)& u4temperature, 0, 0);
        timestamp1 = getTimeStamp_us();

        timediff = (MUINT32)((timestamp1 - timestamp0) / 1000);

        if (timediff > 20)
        {
            CAM_LOGW( "[%s] Time duration of SENSOR_CMD_GET_TEMPERATURE_VALUE is over %d ms", __FUNCTION__, timediff);
        }
    }
    else
    {
        CAM_LOGE( "%s m_ptrIHalSensor is NULL", __FUNCTION__);
    }

    return u4temperature;
}
// obsolete function for HAL1 only
MRESULT AfMgr::setMFPos( MINT32 a_i4Pos, MUINT32 u4Caller)
{
    if ( (m_eLIB3A_AFMode == LIB3A_AF_MODE_MF) &&
            (m_i4MFPos != a_i4Pos) &&
            (0 <= a_i4Pos) &&
            (a_i4Pos <= 1023))
    {
        if ( u4Caller == AF_MGR_CALLER)
        {
            CAM_LOGD( "%s set MF pos (%d)->(%d)",
                      __FUNCTION__,
                      m_sAFOutput->i4AFPos,
                      a_i4Pos);
        }
        else if (u4Caller == AF_CMD_CALLER)
        {
            CAM_LOGD( "cmd-%s set MF pos (%d)->(%d)",
                      __FUNCTION__,
                      m_sAFOutput->i4AFPos,
                      a_i4Pos);
        }
        else if (u4Caller == AF_SYNC_CALLER)
        {
            CAM_LOGD( "sync-%s set MF pos (%d)->(%d)",
                      __FUNCTION__,
                      m_sAFOutput->i4AFPos,
                      a_i4Pos);
        }

        m_i4MFPos = a_i4Pos;

        m_sDoAFInput.algoCommand.mfPos = m_i4MFPos;
        m_sDoAFInput.algoCommand.bTrigger = MTRUE;
    }
    else
    {
        if ( u4Caller == AF_MGR_CALLER)
        {
            CAM_LOGD_IF( LEVEL_FLOW, "%s skip set MF pos (%d)->(%d), lib_afmode(%d)",
                         __FUNCTION__,
                         m_sAFOutput->i4AFPos,
                         a_i4Pos,
                         m_eLIB3A_AFMode);
        }
        else if (u4Caller == AF_CMD_CALLER)
        {
            CAM_LOGD_IF( LEVEL_FLOW, "cmd-%s skip set MF pos (%d)->(%d), lib_afmode(%d)",
                         __FUNCTION__,
                         m_sAFOutput->i4AFPos,
                         a_i4Pos,
                         m_eLIB3A_AFMode);
        }
        else if (u4Caller == AF_SYNC_CALLER)
        {
            CAM_LOGD_IF( LEVEL_FLOW, "sync-%s skip set MF pos (%d)->(%d), lib_afmode(%d)",
                         __FUNCTION__,
                         m_sAFOutput->i4AFPos,
                         a_i4Pos,
                         m_eLIB3A_AFMode);
        }
    }
    return S_AF_OK;
}
MINT32 AfMgr::isFocusFinish()
{
    MINT32 ret = 1;
    CAM_LOGD_IF(LEVEL_FLOW, "%s, m_eAFState=%d", __FUNCTION__, m_eAFState);
    if ( (m_eAFState == E_AF_PASSIVE_SCAN) ||
            (m_eAFState == E_AF_ACTIVE_SCAN) )
    {
        ret = 0;
    }
    return ret;
}
MINT32 AfMgr::isFocused()
{
    MINT32 ret = 0;

    if ( (m_eAFState == E_AF_PASSIVE_FOCUSED) ||
            (m_eAFState == E_AF_FOCUSED_LOCKED))
    {
        ret = 1;
    }

    CAM_LOGD_IF( LEVEL_FLOW,
                 "%s %d %d",
                 __FUNCTION__,
                 m_eAFState,
                 ret);
    return ret;
}
E_AF_STATE_T AfMgr::getAFState()
{
    E_AF_STATE_T ret;

    if (m_u4ReqMagicNum <= AF_START_MAGIC_NUMBER && m_eAFState == E_AF_PASSIVE_SCAN)
    {
        /**
         * If AF starts searching at first request.
         * Force to return inactive state as initial state.
         */
        ret = E_AF_INACTIVE;
    }
    else
    {
        ret = m_eAFState;
    }

    if (AP_NOT_SHOW_FDROI_AUTO && m_i4IsCAFWithoutFace == 1 && ret == E_AF_PASSIVE_SCAN && m_sAFOutput->i4ROISel == AF_ROI_SEL_FD)
    {
        // For AP which WOULD NOT show Face ROI automatically while CAFing
        // faces show up while CAFing
        // overwrite AfState for MW to send callback
        if (m_bNeedLock)
        {
            ret = E_AF_FOCUSED_LOCKED;
        }
        else
        {
            ret = E_AF_PASSIVE_FOCUSED;
        }
        if ( m_eAFStatePre != ret)
        {
            CAM_LOGD("cmd-%s Dev %d : ret = %d, Face shows up while CAFing => Send callback to clear CAF ROI",
                     __FUNCTION__,
                     m_i4CurrSensorDev,
                     ret);
        }
    }
    else
    {
        if ( m_eAFStatePre != ret)
        {
            CAM_LOGD( "cmd-%s Dev %d : %d->%d",
                      __FUNCTION__,
                      m_i4CurrSensorDev,
                      m_eAFStatePre,
                      ret);
        }
    }


    m_eAFStatePre = ret;


    return ret;
}
MVOID AfMgr::getAFRefWin( CameraArea_T &rWinSize)
{
    //The coordinate is base on TG coordinate which is applied binning information.
    if (m_i4EnableAF == 0)
    {
        rWinSize.i4Left = 0;
        rWinSize.i4Right = 0;
        rWinSize.i4Top = 0;
        rWinSize.i4Bottom = 0;
        return;
    }
    else
    {
        rWinSize.i4Left = m_sArea_HW.i4X;
        rWinSize.i4Right = m_sArea_HW.i4X + m_sArea_HW.i4W;
        rWinSize.i4Top = m_sArea_HW.i4Y;
        rWinSize.i4Bottom = m_sArea_HW.i4Y + m_sArea_HW.i4H;

        rWinSize.i4Left = rWinSize.i4Left * m_i4BINSzW / m_i4TGSzW;
        rWinSize.i4Right = rWinSize.i4Right * m_i4BINSzW / m_i4TGSzW;
        rWinSize.i4Top = rWinSize.i4Top * m_i4BINSzH / m_i4TGSzH;
        rWinSize.i4Bottom = rWinSize.i4Bottom * m_i4BINSzH / m_i4TGSzH;
    }
}
MVOID AfMgr::getAF2AEInfo(AF2AEInfo_T &rAFInfo)
{
    rAFInfo.i4IsAFDone      = (m_sAFOutput->i4IsAfSearch == AF_SEARCH_DONE) ? 1 : 0;
    rAFInfo.i4AfDac         = m_sCurLensInfo.i4CurrentPos;
    rAFInfo.i4IsSceneStable = m_sAFOutput->i4IsSceneStable;
}
MINT32 AfMgr::getLensState()
{
    MINT32 ret = 0;

    if (m_i4GyroValue > GYRO_THRESHOLD && m_sAFOutput->i4IsAfSearch == AF_SEARCH_TARGET_MOVE)
    {
        // FAKE STATIONARY : Phone Moving while TRACKING
        ret = 0;
        if (m_bForceCapture == 0)
        {
            CAM_LOGW("%s Phone Moving while TRACKING, ForceCapture: LensState = %d", __FUNCTION__, ret);
        }
        m_bForceCapture = 1;
        m_i4ContinuePDMovingCount = 0;
    }
    else if (m_sAFOutput->i4IsAfSearch == AF_SEARCH_TARGET_MOVE)
    {
        MINT32 maxPdMovingCount = 20;
        if (m_ptrNVRam)
        {
            maxPdMovingCount = (m_ptrNVRam->rAFNVRAM.i4EasyTuning[6] > 6) ? (m_ptrNVRam->rAFNVRAM.i4EasyTuning[6]) : maxPdMovingCount;
        }
        m_i4ContinuePDMovingCount += (m_i4ContinuePDMovingCount < maxPdMovingCount) ? 1 : 0;
        if (m_i4ContinuePDMovingCount >= maxPdMovingCount)
        {
            // FAKE STATIONARY : Keep PDMoving with the stable phone for more than 6 frames
            ret = 0;
            if (m_bForceCapture == 0)
            {
                CAM_LOGW("%s Keep PDMoving for more than %d frames, ForceCapture: LensState = %d", __FUNCTION__, maxPdMovingCount, ret);
            }
            m_bForceCapture = 1;
        }
        else
        {
            // MOVING : PDMoving
            ret = 1;
            m_bForceCapture = 0;
        }
    }
    else if (m_sAFOutput->i4IsAfSearch != AF_SEARCH_DONE)
    {
        // MOVING : FineSearch or ContrastAF
        ret = 1;
        m_i4ContinuePDMovingCount = 0;
        m_bForceCapture = 0;
    }
    else
    {
        // STATIONARY
        ret = 0;
        m_i4ContinuePDMovingCount = 0;
        m_bForceCapture = 0;
    }


    m_i4MvLensToPre = m_i4MvLensTo;

    return ret;
}
MFLOAT AfMgr::getFocusDistance()
{
    if ( m_i4EnableAF == 0)
    {
        CAM_LOGD_IF(LEVEL_FLOW, "cmd-%s no AF", __FUNCTION__);
        return 0.0;
    }

    MINT32 LensPos;
    if ( m_eLIB3A_AFMode == LIB3A_AF_MODE_MF)
    {
        /* In MF mode, algorithm will take some step to target position.
                  So directly using MFpos instead of using m_sAFOutput.i4AFPos.*/
        LensPos = m_i4MFPos;

        // setFocusDistance followed by getFocusDistance in MFMode
        if (m_fMfFocusDistance != -1)
        {
            return m_fMfFocusDistance;
        }
    }
    else
    {
        LensPos = m_sAFOutput->i4AFPos;
    }

    // Lens position is changed, calculating new focus distance :
    if ( m_sFocusDis.i4LensPos != LensPos)
    {
        // data from OTP
        MFLOAT fDAC_Inf        = (MFLOAT)m_sDAF_TBL.af_dac_inf; // unit:DAC
        MFLOAT fDAC_Macro      = (MFLOAT)m_sDAF_TBL.af_dac_marco; // unit:DAC
        MFLOAT fDistance_Inf   = (MFLOAT)m_sDAF_TBL.af_distance_inf; // unit:mm
        MFLOAT fDistance_Macro = (MFLOAT)m_sDAF_TBL.af_distance_marco; // unit:mm
        // current lensPos in DAC
        MFLOAT fCurLensPos     = (MFLOAT)LensPos;

        // set fd_inf and fd_macro to default if there's no data in OTP
        if (fDistance_Inf <= fDistance_Macro)
        {
            fDistance_Inf = 5000;
            fDistance_Macro = 100;
        }
        // (Linear Interpolation) : distance_xxx_rev is linear to DAC
        MFLOAT fDist_Inf_rev = 1000.0 / fDistance_Inf;
        MFLOAT fDist_Macro_rev = 1000.0 / fDistance_Macro;
        // DAC -> FocusDistance
        MFLOAT fDist_rev = fDist_Macro_rev + (fCurLensPos - fDAC_Macro) / (fDAC_Inf - fDAC_Macro) * (fDist_Inf_rev - fDist_Macro_rev);

        CAM_LOGD("cmd-%s Pos %d->%d, Dis %f->%f; fDist_Macro_rev %f, fDist_Inf_rev %f, fDAC_Macro %f, fDAC_Inf %f",
                 __FUNCTION__,
                 m_sFocusDis.i4LensPos,
                 LensPos,
                 m_sFocusDis.fDist,
                 fDist_rev,
                 fDist_Macro_rev,
                 fDist_Inf_rev,
                 fDAC_Macro,
                 fDAC_Inf);

        //record.
        m_sFocusDis.i4LensPos = LensPos;
        m_sFocusDis.fDist = fDist_rev;
    }

    return m_sFocusDis.fDist;
}
MVOID AfMgr::getFocusRange( MFLOAT *vnear, MFLOAT *vfar)
{
    MINT32 i4tblLL;
    MINT32 fdacidx       = 0;
    MINT32 i4ResultDist  = 100;
    MINT32 i4ResultRange = 100;
    MINT32 i4ResultNear  = 100;
    MINT32 i4ResultFar   = 100;

    if ( m_i4EnableAF == 0)
    {
        CAM_LOGD_IF(LEVEL_FLOW, "no focus lens");
        *vnear = 1 / 0.6;
        *vfar  = 1 / 3.0;
        return;
    }

    if ( m_pAFParam == NULL)
        return;

    i4tblLL = m_pAFParam->i4TBLL;
    for ( fdacidx = 0; fdacidx < i4tblLL; fdacidx++)
    {
        if ( m_sAFOutput->i4AFPos > m_pAFParam->i4Dacv[fdacidx])
            break;
    }

    if (fdacidx == 0)
    {
        i4ResultDist = m_pAFParam->i4Dist[0];
    }
    else
    {
        i4ResultDist =
            ( m_pAFParam->i4Dist[fdacidx  ] * (m_pAFParam->i4Dacv[fdacidx - 1] - m_sAFOutput->i4AFPos       )  +
              m_pAFParam->i4Dist[fdacidx - 1] * (m_sAFOutput->i4AFPos          - m_pAFParam->i4Dacv[fdacidx])) /
            ( m_pAFParam->i4Dacv[fdacidx - 1] - m_pAFParam->i4Dacv[fdacidx]);

        i4ResultRange =
            ( m_pAFParam->i4FocusRange[fdacidx  ] * (m_pAFParam->i4Dacv[fdacidx - 1] - m_sAFOutput->i4AFPos       )  +
              m_pAFParam->i4FocusRange[fdacidx - 1] * (m_sAFOutput->i4AFPos          - m_pAFParam->i4Dacv[fdacidx])) /
            ( m_pAFParam->i4Dacv[fdacidx - 1] - m_pAFParam->i4Dacv[fdacidx]);
    }
    if ( i4ResultDist <= 0)  i4ResultDist = m_pAFParam->i4Dist[i4tblLL - 1];
    if ( i4ResultRange <= 0) i4ResultRange = m_pAFParam->i4Dist[i4tblLL - 1];

    i4ResultNear = i4ResultDist - (i4ResultRange / 2);
    i4ResultFar  = i4ResultDist + (i4ResultRange / 2);

    *vnear = 1000.0 / ((MFLOAT)i4ResultNear);
    *vfar  = 1000.0 / ((MFLOAT)i4ResultFar);

    //CAM_LOGD("[getFocusRange] [%f, %f]", *vnear,*vfar);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::sendAFNormalPipe( MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    MBOOL ret = MFALSE;

    INormalPipe* pPipe = (INormalPipe*)INormalPipeUtils::get()->createDefaultNormalPipe( m_i4SensorIdx, LOG_TAG);

    if ( pPipe == NULL)
    {
        CAM_LOGE( "Fail to create NormalPipe");
    }
    else
    {
        ret = pPipe->sendCommand( cmd, arg1, arg2, arg3);
        pPipe->destroyInstance( LOG_TAG);
    }

    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::readOTP(CAMERA_CAM_CAL_TYPE_ENUM enCamCalEnum)
{
    MUINT32 result = 0;
    CamCalDrvBase *pCamCalDrvObj = CamCalDrvBase::createInstance();
    MINT32 i4SensorDevID;

    CAM_LOGD("Read (%d) calibration data from EEPROM by camcal", enCamCalEnum);
    switch ( m_i4CurrSensorDev)
    {
    case ESensorDev_Main:
        i4SensorDevID = SENSOR_DEV_MAIN;
        break;
    case ESensorDev_Sub:
        i4SensorDevID = SENSOR_DEV_SUB;
        break;
    case ESensorDev_MainSecond:
        i4SensorDevID = SENSOR_DEV_MAIN_2;
        break;
    case ESensorDev_SubSecond:
        i4SensorDevID = SENSOR_DEV_SUB_2;
        break;
    case ESensorDev_MainThird:
        i4SensorDevID = SENSOR_DEV_MAIN_3;
        break;
    default:
        i4SensorDevID = SENSOR_DEV_NONE;
        return S_AF_OK;
    }
    if (enCamCalEnum == CAMERA_CAM_CAL_DATA_3A_GAIN)
    {
        CAM_CAL_2A_DATA_STRUCT GetCamCalData;
        result = pCamCalDrvObj->GetCamCalCalDataV2(i4SensorDevID, enCamCalEnum, &GetCamCalData, sizeof(CAM_CAL_2A_DATA_STRUCT));

        if (result & CamCalReturnErr[enCamCalEnum])
        {
            CAM_LOGD( "%s : err (%s)", __FUNCTION__, CamCalErrString[enCamCalEnum]);
            //return E_AF_NOSUPPORT;
        }

        CAM_LOGD_IF( LEVEL_FLOW,
                     "OTP data [S2aBitEn]%d [S2aAfBitflagEn]%d [S2aAf0]%d [S2aAf1]%d",
                     GetCamCalData.Single2A.S2aBitEn,
                     GetCamCalData.Single2A.S2aAfBitflagEn,
                     GetCamCalData.Single2A.S2aAf[0],
                     GetCamCalData.Single2A.S2aAf[1]);

        m_i4InfPos    = GetCamCalData.Single2A.S2aAf[0];
        m_i4MacroPos  = GetCamCalData.Single2A.S2aAf[1];
        m_i450cmPos   = GetCamCalData.Single2A.S2aAf[3];
        m_i4MiddlePos = GetCamCalData.Single2A.S2aAF_t.AF_Middle_calibration;

        if ( 0 > m_i4InfPos || m_i4MacroPos > 1023 || m_i4MacroPos <= m_i4InfPos)
        {
            // data from otp is abnormal
            m_i4InfPos   = 0;
            m_i4MacroPos = 1023;
            // warning issue
            char  dbgMsgBuf[DBG_MSG_BUF_SZ];
            char* ptrMsgBuf = dbgMsgBuf;
            sprintf( ptrMsgBuf, "%s Dev(%d) cam_cal is abnormal \nCRDISPATCH_KEY: AF_MGR_ASSERT",
                     __FUNCTION__,
                     m_i4CurrSensorDev);
            AEE_ASSERT_AF(dbgMsgBuf);
        }

        if ( 0 <= m_i4InfPos && m_i4MacroPos <= 1023 && m_i4MacroPos > m_i4InfPos )
        {
            if ( m_i4MiddlePos > m_i4InfPos && m_i4MiddlePos < m_i4MacroPos)
            {
                CAM_LOGD( "Middle OTP cal:%d\n", m_i4MiddlePos);
            }
            else
            {
                m_i4MiddlePos = m_i4InfPos + (m_i4MacroPos - m_i4InfPos) * m_ptrNVRam->rAFNVRAM.i4DualAFCoefs[1] / 100;
                CAM_LOGD( "Middle OTP adjust:%d NVRAM:%d INF:%d Mac:%d\n", m_i4MiddlePos, m_ptrNVRam->rAFNVRAM.i4DualAFCoefs[1], m_i4InfPos, m_i4MacroPos);
            }


            if (m_ptrNVRam->rAFNVRAM.i4EasyTuning[0] == 10000 && m_i4LensPosExit != 0)
            {
                m_i4InitPos = m_i4LensPosExit;
            }
            else
            {
                m_i4InitPos = m_i4InfPos + (m_i4MacroPos - m_i4InfPos) * m_ptrNVRam->rAFNVRAM.i4EasyTuning[0] / 100;
                m_i4InitPos = (m_i4InitPos < 0) ? 0 : ((m_i4InitPos > 1023) ? 1023 : m_i4InitPos);
            }

            // adjust depth dac_min, dac_max
            m_sDAF_TBL.af_dac_inf    = m_i4InfPos;
            m_sDAF_TBL.af_dac_marco  = m_i4MacroPos;
            m_sDAF_TBL.af_dac_min    = m_i4InfPos;
            m_sDAF_TBL.af_dac_max    = m_i4MacroPos;

            m_sDAF_TBL.af_distance_inf   = GetCamCalData.Single2A.S2aAF_t.AF_infinite_pattern_distance;
            m_sDAF_TBL.af_distance_marco = GetCamCalData.Single2A.S2aAF_t.AF_Macro_pattern_distance;
            CAM_LOGD( "AF-%-15s: calibrated data [af_dac_inf]%d [af_dac_marco]%d [af_dac_min]%d [af_dac_max]%d [af_distance_inf]%d [af_distance_marco]%d\n",
                      __FUNCTION__,
                      (MINT32)m_sDAF_TBL.af_dac_inf,
                      (MINT32)m_sDAF_TBL.af_dac_marco,
                      (MINT32)m_sDAF_TBL.af_dac_min,
                      (MINT32)m_sDAF_TBL.af_dac_max,
                      (MINT32)m_sDAF_TBL.af_distance_inf,
                      (MINT32)m_sDAF_TBL.af_distance_marco);

            CAM_LOGD( "%s : [Inf]%d [Macro]%d [50cm]%d [InitPos]%d", __FUNCTION__, m_i4InfPos, m_i4MacroPos, m_i450cmPos, m_i4InitPos);

            if (m_bLDAFEn == MTRUE && m_pLaserMgr != MNULL)
            {
                m_pLaserMgr->setAFCaliData(m_sDAF_TBL.af_dac_inf, m_sDAF_TBL.af_distance_inf,
                                           m_sDAF_TBL.af_dac_marco, m_sDAF_TBL.af_distance_marco,
                                           0, 0);
            }
        }
    }

    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::HybridAFPreprocessing()
{
    char  dbgMsgBuf[DBG_MSG_BUF_SZ];
    char* ptrMsgBuf = dbgMsgBuf;

    if ( LEVEL_FLOW)
    {
        ptrMsgBuf += sprintf( ptrMsgBuf, "#(%5d,%5d) %s Dev(%d) PDEn(%d) LaserEn(%d) FDDetect(%d): ",
                              m_u4ReqMagicNum,
                              m_u4StaMagicNum,
                              __FUNCTION__,
                              m_i4CurrSensorDev,
                              m_bEnablePD,
                              m_bLDAFEn,
                              m_sAFOutput->i4FDDetect);
    }


    /* do PD data preprocessing */
    if ( m_bEnablePD)
    {
        MINT32   PDOut_numRes = m_sAFInput->sPDInfo.i4PDBlockInfoNum;
        MUINT16 pPDAF_DAC = 0;
        MUINT16 pPDAF_Conf = 0;

        if ( LEVEL_FLOW)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "[PD (DAC, CL)] org{ ");
            for ( MINT32 i = 0; i < PDOut_numRes; i++)
            {
                pPDAF_DAC = m_sAFInput->sPDInfo.sPDBlockInfo[i].i4PDafDacIndex;
                pPDAF_Conf = m_sAFInput->sPDInfo.sPDBlockInfo[i].i4PDafConfidence;
                ptrMsgBuf += sprintf( ptrMsgBuf, "#%d(%d, %d) ", i, pPDAF_DAC, pPDAF_Conf);
            }
            ptrMsgBuf += sprintf( ptrMsgBuf, "}, ");
        }

        /*****************************  PD result preprocessing (start) *****************************/
#if 0
        {
            // To Do :
            // customer can process PD result here for customization.
        }
#endif
        /*****************************  PD result preprocessing (end)  *****************************/

        if ( LEVEL_FLOW)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "new{ ");
            for ( MINT32 i = 0; i < PDOut_numRes; i++)
            {
                pPDAF_DAC = m_sAFInput->sPDInfo.sPDBlockInfo[i].i4PDafDacIndex;
                pPDAF_Conf = m_sAFInput->sPDInfo.sPDBlockInfo[i].i4PDafConfidence;
                ptrMsgBuf += sprintf( ptrMsgBuf, "#%d(%d, %d) ", i, pPDAF_DAC, pPDAF_Conf);
            }
            ptrMsgBuf += sprintf( ptrMsgBuf, "}, ");
        }

        // LaunchCamTrigger
        if (m_i4LaunchCamTriggered == E_LAUNCH_AF_WAITING)
        {
            if (m_i4ValidPDFrameCount == -1 && PDOut_numRes > 0)
            {
                for (MINT32 i = 0; i < PDOut_numRes; i++)
                {
                    pPDAF_Conf = m_sAFInput->sPDInfo.sPDBlockInfo[i].i4PDafConfidence;
                    if (pPDAF_Conf > 0)
                    {
                        m_i4ValidPDFrameCount = m_u4ReqMagicNum;
                        CAM_LOGD("#(%5d,%5d) %s Dev(%d) LaunchCamTrigger VALID PD(%d) - BUT NOT TRIGGER YET",
                                 m_u4ReqMagicNum, m_u4StaMagicNum,
                                 __FUNCTION__,
                                 m_i4CurrSensorDev,
                                 m_i4ValidPDFrameCount);
                        break;
                    }
                }
            }

            if (m_i4ValidPDFrameCount != -1 && m_u4ReqMagicNum >= (m_i4ValidPDFrameCount + m_i4ValidPDTriggerTimeout))
            {
                // Valid PD and m_i4ValidPDTriggerTimeout(for waiting Face)
                m_i4LaunchCamTriggered = E_LAUNCH_AF_TRIGGERED;
                CAM_LOGD( "#(%5d,%5d) %s Dev(%d) LaunchCamTrigger VALID PD(%d + %d) - UnlockAlgo + TRIGGERAF lib_afmode(%d)",
                          m_u4ReqMagicNum, m_u4StaMagicNum,
                          __FUNCTION__,
                          m_i4CurrSensorDev,
                          m_i4ValidPDFrameCount, m_i4ValidPDTriggerTimeout,
                          m_eLIB3A_AFMode);
                UnlockAlgo();
                triggerAF(AF_MGR_CALLER);
                m_i4IsLockForLaunchCamTrigger = 0;
            }

        }
    }
    else
    {
        // To set the target to make sure the ROI always be drawn with Contrast AF.
        m_sCallbackInfo.CompSet_PDCL.Target = 101;  // value > target ==> no draw
    }

    /* do laser data preprocessing */
    if (m_bLDAFEn == MTRUE && m_pLaserMgr != MNULL)
    {
        MINT32 i4LaserStatus = m_pLaserMgr->getLaserCurStatus();

        if (LEVEL_FLOW)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "[Laser %d (DAC, CONF, DIST)] org:(%d, %d, %d) ",
                                  i4LaserStatus,
                                  m_sAFInput->sLaserInfo.i4CurPosDAC,
                                  m_sAFInput->sLaserInfo.i4Confidence,
                                  m_sAFInput->sLaserInfo.i4CurPosDist);
        }

        /*****************************  Laser result preprocessing (start) *****************************/
        {
            switch (i4LaserStatus)
            {
            case STATUS_RANGING_VALID:
                m_sAFInput->sLaserInfo.i4Confidence = 80;
                break;

            case STATUS_MOVE_DMAX:
            case STATUS_MOVE_MAX_RANGING_DIST:
                m_sAFInput->sLaserInfo.i4Confidence = 49;
                break;

            default:
                m_sAFInput->sLaserInfo.i4Confidence = 20;
                break;
            }

            if (m_sAFInput.sLaserInfo.i4CurPosDist < 70)
            {
                m_sAFInput.sLaserInfo.i4Confidence = 20;
            }

            //Touch AF : if the ROI isn't in the center, the laser data need to set low confidence.
            if ( (m_eLIB3A_AFMode == LIB3A_AF_MODE_AFS) || (m_eLIB3A_AFMode == LIB3A_AF_MODE_MACRO))
            {
                if ( m_sAFInput->sLaserInfo.i4AfWinPosX >= LASER_TOUCH_REGION_W ||
                        m_sAFInput->sLaserInfo.i4AfWinPosY >= LASER_TOUCH_REGION_H ||
                        m_sAFInput->sLaserInfo.i4AfWinPosCnt > 1 )
                {
                    m_sAFInput->sLaserInfo.i4Confidence = 20;
                    m_sAFInput->sLaserInfo.i4CurPosDAC  = 0;
                }
            }

            if ( 1 == m_sAFOutput->i4FDDetect)
            {
                m_sAFInput->sLaserInfo.i4Confidence = 0;
                m_sAFInput->sLaserInfo.i4CurPosDAC  = 0;
            }
        }
        /*****************************  Laser result preprocessing (end)  *****************************/


        if ( LEVEL_FLOW)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "[Laser %d (DAC, CONF, DIST)] new:(%d, %d, %d) ",
                                  i4LaserStatus,
                                  m_sAFInput->sLaserInfo.i4CurPosDAC,
                                  m_sAFInput->sLaserInfo.i4Confidence,
                                  m_sAFInput->sLaserInfo.i4CurPosDist);
        }
    }

    CAM_LOGD_IF( LEVEL_FLOW, "%s", dbgMsgBuf);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 AfMgr::GetHybridAFMode()
{
    MUINT32 HybridAFMode = 0;

    //Depth AF
    if ((m_sDAF_TBL.is_daf_run & E_DAF_RUN_DEPTH_ENGINE) && (m_bSDAFEn == MTRUE))
    {
        HybridAFMode |= 1;
    }

    //PDAF
    if (m_bEnablePD)
    {
        HybridAFMode |= 2;  //2'b 0010
    }

    //Laser AF
    if (m_bLDAFEn == MTRUE)
    {
        HybridAFMode |= 4;
    }

    CAM_LOGD_IF( LEVEL_VERBOSE, "%s:(0x%x)", __FUNCTION__, HybridAFMode);

    return HybridAFMode;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::PrintHWRegSetting( AF_CONFIG_T & sAFHWCfg __unused)
{
#if 0
    CAM_LOGD( "HW-TGSZ %d, %d, BINSZ %d, %d",
              sAFHWCfg.sTG_SZ.i4W,
              sAFHWCfg.sTG_SZ.i4H,
              sAFHWCfg.sBIN_SZ.i4W,
              sAFHWCfg.sBIN_SZ.i4H);

    CAM_LOGD( "HW-sROI %d, %d %d, %d %d",
              sAFHWCfg.sRoi.i4X,
              sAFHWCfg.sRoi.i4Y,
              sAFHWCfg.sRoi.i4W,
              sAFHWCfg.sRoi.i4H,
              sAFHWCfg.sRoi.i4Info);

    CAM_LOGD( "HW-nBLK %d, %d",
              sAFHWCfg.AF_BLK_XNUM,
              sAFHWCfg.AF_BLK_YNUM);

    CAM_LOGD( "HW-SGG %d, %d, %d, %d, %d, %d, %d, %d",
              sAFHWCfg.i4SGG_GAIN,
              sAFHWCfg.i4SGG_GMR1,
              sAFHWCfg.i4SGG_GMR2,
              sAFHWCfg.i4SGG_GMR3,
              sAFHWCfg.i4SGG_GMR4,
              sAFHWCfg.i4SGG_GMR5,
              sAFHWCfg.i4SGG_GMR6,
              sAFHWCfg.i4SGG_GMR7);


    CAM_LOGD( "HW-HVGL %d, %d, %d",
              sAFHWCfg.AF_H_GONLY,
              sAFHWCfg.AF_V_GONLY,
              sAFHWCfg.AF_V_AVG_LVL);

    CAM_LOGD( "HW-BLF %d, %d, %d, %d",
              sAFHWCfg.AF_BLF[0],
              sAFHWCfg.AF_BLF[1],
              sAFHWCfg.AF_BLF[2],
              sAFHWCfg.AF_BLF[3]);

    CAM_LOGD( "HW-TH %d, %d, %d, %d",
              sAFHWCfg.AF_TH_H[0],
              sAFHWCfg.AF_TH_H[1],
              sAFHWCfg.AF_TH_V,
              sAFHWCfg.AF_TH_G_SAT);

    CAM_LOGD( "HW-FIL0 %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
              sAFHWCfg.AF_FIL_H0[0],
              sAFHWCfg.AF_FIL_H0[1],
              sAFHWCfg.AF_FIL_H0[2],
              sAFHWCfg.AF_FIL_H0[3],
              sAFHWCfg.AF_FIL_H0[4],
              sAFHWCfg.AF_FIL_H0[5],
              sAFHWCfg.AF_FIL_H0[6],
              sAFHWCfg.AF_FIL_H0[7],
              sAFHWCfg.AF_FIL_H0[8],
              sAFHWCfg.AF_FIL_H0[9],
              sAFHWCfg.AF_FIL_H0[10],
              sAFHWCfg.AF_FIL_H0[11]);

    CAM_LOGD( "HW-FIL1 %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
              sAFHWCfg.AF_FIL_H1[0],
              sAFHWCfg.AF_FIL_H1[1],
              sAFHWCfg.AF_FIL_H1[2],
              sAFHWCfg.AF_FIL_H1[3],
              sAFHWCfg.AF_FIL_H1[4],
              sAFHWCfg.AF_FIL_H1[5],
              sAFHWCfg.AF_FIL_H1[6],
              sAFHWCfg.AF_FIL_H1[7],
              sAFHWCfg.AF_FIL_H1[8],
              sAFHWCfg.AF_FIL_H1[9],
              sAFHWCfg.AF_FIL_H1[10],
              sAFHWCfg.AF_FIL_H1[11]);

    CAM_LOGD( "HW-FILV %d, %d, %d, %d",
              sAFHWCfg.AF_FIL_V[0],
              sAFHWCfg.AF_FIL_V[1],
              sAFHWCfg.AF_FIL_V[2],
              sAFHWCfg.AF_FIL_V[3]);
#endif
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AREA_T& AfMgr::SelROIToFocusing( AF_OUTPUT_T & sInAFInfo)
{
    if ( (m_i4IsSelHWROI_PreState != m_i4IsSelHWROI_CurState && m_i4IsSelHWROI_CurState == MTRUE) ||
            (m_bLatchROI == MTRUE) ||
            (LEVEL_FLOW))
    {
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) latch(%d) type(%d) sel(%d) issel(%d) ROI(X,Y,W,H) : Center(%d, %d, %d, %d), AP(%d, %d, %d, %d), OT(%d, %d, %d, %d), HW(%d, %d, %d, %d)",
                  m_u4ReqMagicNum,
                  m_u4StaMagicNum,
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  m_bLatchROI,
                  m_sArea_TypeSel,
                  sInAFInfo.i4ROISel,
                  m_i4IsSelHWROI_CurState,
                  m_sArea_Center.i4X,
                  m_sArea_Center.i4Y,
                  m_sArea_Center.i4W,
                  m_sArea_Center.i4H,
                  m_sArea_APCmd.i4X,
                  m_sArea_APCmd.i4Y,
                  m_sArea_APCmd.i4W,
                  m_sArea_APCmd.i4H,
                  m_sArea_OTFD.i4X,
                  m_sArea_OTFD.i4Y,
                  m_sArea_OTFD.i4W,
                  m_sArea_OTFD.i4H,
                  m_sArea_HW.i4X,
                  m_sArea_HW.i4Y,
                  m_sArea_HW.i4W,
                  m_sArea_HW.i4H);
    }


    if ( m_bLatchROI == MTRUE) /* Got changing ROI command from host. Should be trigger searching.*/
    {
        /**
         *   force using the new ROI which is sent from host, and do one time searching :
         *   @LIB3A_AF_MODE_AFS -> wiait autofocus command.
         *   @LIB3A_AF_MODE_AFC_VIDEO, LIB3A_AF_MODE_AFC ->  focuse to trigger searching by switching af mode to auto mode in AF HAL.
         */
        m_sArea_Focusing = m_sArea_APCmd;
        m_sArea_TypeSel  = AF_ROI_SEL_AP;

        CAM_LOGD("#(%5d,%5d) %s [CMD] %d (X,Y,W,H)=(%d, %d, %d, %d)",
                 m_u4ReqMagicNum,
                 m_u4StaMagicNum,
                 __FUNCTION__,
                 m_sArea_TypeSel,
                 m_sArea_Focusing.i4X,
                 m_sArea_Focusing.i4Y,
                 m_sArea_Focusing.i4W,
                 m_sArea_Focusing.i4H);

        //used to control select AF ROI at AFS mode.
    }
    else if ( sInAFInfo.i4ROISel == AF_ROI_SEL_NONE)
    {
        /**
         *  Do nothing
         *  This case is just happened after af is inited.
         *  Wait algo to check using FD or center ROI to do focusing.
         *  Should get i4IsMonitorFV==TRUE. i4IsMonitorFV will be FALSE when ROI is selected.
         */
    }
    else if ( m_i4IsSelHWROI_CurState == MTRUE) /* Without got changing ROI command from host, and need to do searching*/
    {
        switch ( sInAFInfo.i4ROISel)
        {
        case AF_ROI_SEL_NONE :
            //This case cannot be happened.
            break;
        case AF_ROI_SEL_AP :
            //This case cannot be happened.
            m_sArea_Focusing = m_sArea_APCmd;
            break;
        case AF_ROI_SEL_OT :
        case AF_ROI_SEL_FD :
            m_sArea_Focusing = m_sArea_APCmd = m_sArea_OTFD; //rest AP ROI
            break;
        case AF_ROI_SEL_CENTER :
        case AF_ROI_SEL_DEFAULT :
        default :
            m_sArea_Focusing = m_sArea_APCmd = m_sArea_Center; //rest AP ROI
            break;
        }
        m_sArea_TypeSel = sInAFInfo.i4ROISel;

        CAM_LOGD( "#(%5d,%5d) %s [SEL] %d (X,Y,W,H)=(%d, %d, %d, %d)",
                  m_u4ReqMagicNum,
                  m_u4StaMagicNum,
                  __FUNCTION__,
                  m_sArea_TypeSel,
                  m_sArea_Focusing.i4X,
                  m_sArea_Focusing.i4Y,
                  m_sArea_Focusing.i4W,
                  m_sArea_Focusing.i4H);
    }

    return m_sArea_Focusing;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT64 AfMgr::getTimeStamp_us()
{
    struct timespec t;

    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);

    MINT64 timestamp = ((t.tv_sec) * 1000000000LL + t.tv_nsec) / 1000;
    return timestamp; // from nano to us
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::WDBGInfo( MUINT32 i4InTag, MUINT32 i4InVal, MUINT32 i4InLineKeep)
{
    MRESULT ret = E_3A_ERR;

    if ( m_i4MgrExifSz < MGR_EXIF_SIZE)
    {
        m_sMgrExif[ m_i4MgrExifSz].u4FieldID    = AAATAG( AAA_DEBUG_AF_MODULE_ID, i4InTag, i4InLineKeep);
        m_sMgrExif[ m_i4MgrExifSz].u4FieldValue = i4InVal;
        m_i4MgrExifSz++;
        ret = S_3A_OK;
    }
    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::WDBGCapInfo( MUINT32 i4InTag, MUINT32 i4InVal, MUINT32 i4InLineKeep)
{
    MRESULT ret = E_3A_ERR;

    if ( m_i4MgrCapExifSz < MGR_CAPTURE_EXIF_SIZE)
    {
        m_sMgrCapExif[ m_i4MgrCapExifSz].u4FieldID    = AAATAG( AAA_DEBUG_AF_MODULE_ID, i4InTag, i4InLineKeep);
        m_sMgrCapExif[ m_i4MgrCapExifSz].u4FieldValue = i4InVal;
        m_i4MgrCapExifSz++;
        ret = S_3A_OK;
    }
    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::SetMgrDbgInfo()
{
    MRESULT ret = E_3A_ERR;

    CAM_LOGD_IF( LEVEL_FLOW, "%s", __FUNCTION__);

    ret = CleanMgrDbgInfo();
    if ( ret == S_3A_OK)
    {
        ret = WDBGInfo( MGR_TG_W, m_i4TGSzW, 0);
        ret = WDBGInfo( MGR_TG_H, m_i4TGSzH, 1);
        ret = WDBGInfo( MGR_BIN_W, m_i4BINSzW, 0);
        ret = WDBGInfo( MGR_BIN_H, m_i4BINSzH, 1);
        ret = WDBGInfo( MGR_CROP_WIN_X, m_sCropRegionInfo.i4X, 0);
        ret = WDBGInfo( MGR_CROP_WIN_Y, m_sCropRegionInfo.i4Y, 1);
        ret = WDBGInfo( MGR_CROP_WIN_W, m_sCropRegionInfo.i4W, 1);
        ret = WDBGInfo( MGR_CROP_WIN_H, m_sCropRegionInfo.i4H, 1);
        ret = WDBGInfo( MGR_FOCUSING_WIN_X, m_sArea_Focusing.i4X, 0);
        ret = WDBGInfo( MGR_FOCUSING_WIN_Y, m_sArea_Focusing.i4Y, 1);
        ret = WDBGInfo( MGR_FOCUSING_WIN_W, m_sArea_Focusing.i4W, 1);
        ret = WDBGInfo( MGR_FOCUSING_WIN_H, m_sArea_Focusing.i4H, 1);
        ret = WDBGInfo( MGR_OTFD_WIN_X, m_sArea_OTFD.i4X, 0);
        ret = WDBGInfo( MGR_OTFD_WIN_Y, m_sArea_OTFD.i4Y, 1);
        ret = WDBGInfo( MGR_OTFD_WIN_W, m_sArea_OTFD.i4W, 1);
        ret = WDBGInfo( MGR_OTFD_WIN_H, m_sArea_OTFD.i4H, 1);
        ret = WDBGInfo( MGR_CENTER_WIN_X, m_sArea_Center.i4X, 0);
        ret = WDBGInfo( MGR_CENTER_WIN_Y, m_sArea_Center.i4Y, 1);
        ret = WDBGInfo( MGR_CENTER_WIN_W, m_sArea_Center.i4W, 1);
        ret = WDBGInfo( MGR_CENTER_WIN_H, m_sArea_Center.i4H, 1);
        ret = WDBGInfo( MGR_CMD_WIN_X, m_sArea_APCmd.i4X, 0);
        ret = WDBGInfo( MGR_CMD_WIN_Y, m_sArea_APCmd.i4Y, 1);
        ret = WDBGInfo( MGR_CMD_WIN_W, m_sArea_APCmd.i4W, 1);
        ret = WDBGInfo( MGR_CMD_WIN_H, m_sArea_APCmd.i4H, 1);

        //ret = WDBGInfo( MGR_LASER_VAL, m_sAFInput->sLaserInfo.i4CurPosDist, 0);
        //ret = WDBGInfo( MGR_FOCUSING_POS, m_sFocusDis.i4LensPos, 0);
        //ret = WDBGInfo( MGR_FOCUSING_DST, (MINT32)m_sFocusDis.fDist, 0);
    }

    return ret;

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::SetMgrCapDbgInfo()
{
    MRESULT ret = E_3A_ERR;

    CAM_LOGD_IF( LEVEL_VERBOSE, "%s", __FUNCTION__);

    ret = CleanMgrCapDbgInfo();

    if ( ret == S_3A_OK)
    {
        ret = WDBGCapInfo( MGR_TS, m_u4StaMagicNum, 0); // current frameNumber for capturing
        ret = WDBGCapInfo( MGR_CURRENT_POS, m_sCurLensInfo.i4CurrentPos, 1);
        for (auto it = m_gyroValueQueue.begin(); it != m_gyroValueQueue.end(); ++it)
        {
            if (it == m_gyroValueQueue.begin())
            {
                ret = WDBGCapInfo( MGR_GYRO_SENSOR_X, *it, 0);
            }
            else
            {
                ret = WDBGCapInfo( MGR_GYRO_SENSOR_X, *it, 1);
            }
        }
    }

    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::CleanMgrDbgInfo()
{
    CAM_LOGD_IF( LEVEL_VERBOSE, "%s", __FUNCTION__);
    memset( &m_sMgrExif[0], 0, sizeof( AAA_DEBUG_TAG_T)*MGR_EXIF_SIZE);
    m_i4MgrExifSz = 0;
    return S_3A_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::CleanMgrCapDbgInfo()
{
    CAM_LOGD_IF( LEVEL_VERBOSE, "%s", __FUNCTION__);
    memset( &m_sMgrCapExif[0], 0, sizeof( AAA_DEBUG_TAG_T)*MGR_CAPTURE_EXIF_SIZE);
    m_i4MgrCapExifSz = 0;
    return S_3A_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::GetMgrDbgInfo( AF_DEBUG_INFO_T & sOutMgrDebugInfo)
{
    MRESULT ret = E_3A_ERR;

    /* Store current status to EXIF*/
    SetMgrCapDbgInfo();

    /* Output */
    MUINT32 idx = AF_DEBUG_TAG_SIZE;
    for ( MUINT32 i = 0; i < AF_DEBUG_TAG_SIZE; i++)
    {
        if ( sOutMgrDebugInfo.Tag[i].u4FieldID == 0)
        {
            idx = i;
            break;
        }
    }

    CAM_LOGD_IF( LEVEL_VERBOSE,
                 "%s %d %d %d %d %d %d",
                 __FUNCTION__,
                 AF_DEBUG_TAG_SIZE,
                 idx,
                 MGR_EXIF_SIZE,
                 m_i4MgrExifSz,
                 MGR_CAPTURE_EXIF_SIZE,
                 m_i4MgrCapExifSz);

    // MgrDbgExif
    if ( (idx < AF_DEBUG_TAG_SIZE) && ((AF_DEBUG_TAG_SIZE - idx) >= m_i4MgrExifSz) && (0 < m_i4MgrExifSz) && (m_i4MgrExifSz <= MGR_EXIF_SIZE))
    {
        memcpy( &sOutMgrDebugInfo.Tag[idx], &m_sMgrExif[0], sizeof( AAA_DEBUG_TAG_T)*m_i4MgrExifSz);
        ret = S_3A_OK;
        idx += m_i4MgrExifSz;
    }
    // MgrCapExif : Gyro & lensPos @ Cap
    if ( (idx < AF_DEBUG_TAG_SIZE) && (AF_DEBUG_TAG_SIZE - idx) >= m_i4MgrCapExifSz && (0 < m_i4MgrCapExifSz) && (m_i4MgrCapExifSz <= MGR_CAPTURE_EXIF_SIZE))
    {
        memcpy( &sOutMgrDebugInfo.Tag[idx], &m_sMgrCapExif[0], sizeof( AAA_DEBUG_TAG_T)*m_i4MgrCapExifSz);
        ret = S_3A_OK;
        idx += m_i4MgrCapExifSz;
    }

    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
E_AF_STATE_T AfMgr::UpdateStateMFMode( E_AF_STATE_T & eInCurSate, AF_EVENT_T & sInEvent)
{
    E_AF_STATE_T NewState = E_AF_INACTIVE;
    switch ( sInEvent)
    {
    case EVENT_CMD_CHANGE_MODE :
        m_bNeedCheckSendCallback = MFALSE;
        NewState = E_AF_INACTIVE;
        m_i4TAFStatus = TAF_STATUS_RESET;
        // LaunchCamTrigger get disable in afmodes except ContinuousMode
        if (m_i4LaunchCamTriggered == E_LAUNCH_AF_WAITING)
        {
            UnlockAlgo();
        }
        m_i4LaunchCamTriggered = E_LAUNCH_AF_DONE;
        m_i4AEStableFrameCount = -1;
        m_i4ValidPDFrameCount = -1;
        // Reseting the para to preventing undesired isAFSearch change.
        m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState = AF_SEARCH_DONE;
        break;
    case EVENT_CMD_AUTOFOCUS :
    case EVENT_CMD_TRIGGERAF_WITH_AE_STBL :
        NewState = eInCurSate;
        m_sDoAFInput.algoCommand.bTrigger = MTRUE;
        break;
    case EVENT_SET_WAIT_FORCE_TRIGGER :
    case EVENT_CANCEL_WAIT_FORCE_TRIGGER :
        NewState = eInCurSate;
        break;
    case EVENT_CMD_AUTOFOCUS_CANCEL :
        // Reseting the para to preventing undesired isAFSearch change.
        m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState = AF_SEARCH_DONE;
    // intentionally fall through
    case EVENT_CMD_SET_AF_REGION :
    case EVENT_CMD_STOP :
    case EVENT_SEARCHING_START :
    case EVENT_SEARCHING_END :
    case EVENT_AE_IS_STABLE :
    default :
        m_sCallbackInfo.isAfterAutoMode = 0;
        NewState = E_AF_INACTIVE;
        break;
    }

    return NewState;

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
E_AF_STATE_T AfMgr::UpdateStateOFFMode( E_AF_STATE_T & eInCurSate, AF_EVENT_T & sInEvent)
{
    E_AF_STATE_T NewState = E_AF_INACTIVE;
    switch ( sInEvent)
    {
    case EVENT_CMD_CHANGE_MODE :
        NewState = E_AF_INACTIVE;
        m_i4TAFStatus = TAF_STATUS_RESET;
        m_bNeedCheckSendCallback = MFALSE;
        // LaunchCamTrigger get disable in afmodes except ContinuousMode
        if (m_i4LaunchCamTriggered == E_LAUNCH_AF_WAITING)
        {
            UnlockAlgo();
        }
        m_i4LaunchCamTriggered = E_LAUNCH_AF_DONE;
        m_i4AEStableFrameCount = -1;
        m_i4ValidPDFrameCount = -1;
        // Reseting the para to preventing undesired isAFSearch change.
        m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState = AF_SEARCH_DONE;
        break;
    case EVENT_CMD_AUTOFOCUS :
    case EVENT_CMD_TRIGGERAF_WITH_AE_STBL :
        NewState = eInCurSate;
        m_sDoAFInput.algoCommand.bTrigger = MTRUE;
        break;
    case EVENT_SET_WAIT_FORCE_TRIGGER :
    case EVENT_CANCEL_WAIT_FORCE_TRIGGER :
        NewState = eInCurSate;
        break;
    case EVENT_CMD_AUTOFOCUS_CANCEL :
        // Reseting the para to preventing undesired isAFSearch change.
        m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState = AF_SEARCH_DONE;
    // intentionally fall through
    case EVENT_CMD_SET_AF_REGION :
    case EVENT_CMD_STOP :
    case EVENT_SEARCHING_START :
    case EVENT_SEARCHING_END :
    case EVENT_AE_IS_STABLE :
    default :
        m_sCallbackInfo.isAfterAutoMode = 0;
        NewState = E_AF_INACTIVE;
        break;
    }

    return NewState;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
E_AF_STATE_T AfMgr::UpdateStateContinuousMode( E_AF_STATE_T & eInCurSate, AF_EVENT_T & sInEvent)
{
    E_AF_STATE_T NewState = E_AF_INACTIVE;
    switch ( sInEvent)
    {
    case EVENT_CMD_CHANGE_MODE :
        /**
         *  For normal continuous AF flow, AF will do lock once got autofocus command from host.
         *  Reset flags.
         */
        m_bLock = m_bNeedLock = MFALSE;

        /**
         *  For normal TAF flow, AF HAL got both auto mode and AF region information.
         *  Setting m_bForceTrigger and m_bLatchROI as MFALSE here to wait trigger searching(change ROI only).
         */
        m_bLatchROI = m_bForceTrigger = m_bTriggerCmdVlid = m_bWaitForceTrigger = MFALSE;

        m_bNeedCheckSendCallback = MTRUE;

        // Reseting the para to preventing undesired isAFSearch change.
        m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState = AF_SEARCH_DONE;

        if ( eInCurSate == E_AF_INACTIVE || m_i4LaunchCamTriggered != E_LAUNCH_AF_DONE)
        {
            // To prevent fail because of the wrong af state while switching mode w/o trigger (original: E_AF_INACTIVE)
            NewState = E_AF_PASSIVE_UNFOCUSED;
            if ( m_i4TAFStatus == TAF_STATUS_RECEIVE_CANCELAUTOFOCUS)
            {
                CAM_LOGD( "Don't do AF searching after TAF");
            }
            else
            {
                CAM_LOGD( "[%s] LaunchCamTrigger, LockAlgo for waiting (AEStable or PDValid) + FDinfo", __FUNCTION__);
                m_bForceTrigger = m_bTriggerCmdVlid = MTRUE;
                // LaunchCamTrigger get enabled while force trigger searching
                m_i4LaunchCamTriggered = E_LAUNCH_AF_WAITING;
                m_i4AEStableFrameCount = -1;
                m_i4ValidPDFrameCount = -1;
                LockAlgo(); // to stop continuous af triggered by algo while launching camera
                m_i4IsLockForLaunchCamTrigger = 1;
            }
        }
        else if ( eInCurSate == E_AF_ACTIVE_SCAN       ) NewState = E_AF_PASSIVE_UNFOCUSED;
        else if ( eInCurSate == E_AF_FOCUSED_LOCKED    ) NewState = E_AF_PASSIVE_FOCUSED;
        else if ( eInCurSate == E_AF_NOT_FOCUSED_LOCKED) NewState = E_AF_PASSIVE_UNFOCUSED;
        else                                                NewState = eInCurSate;

        m_i4TAFStatus = TAF_STATUS_RESET;
        break;

    case EVENT_CMD_AUTOFOCUS :
        if ( m_bForceTrigger == MTRUE)
        {
            // force to trigger searching when changing ROI at continuous mode.
            NewState = E_AF_PASSIVE_SCAN;
            CAM_LOGD( "Wait force trigger and lock until searching done");
            m_bNeedLock = MTRUE;
        }
        else
        {
            if (      eInCurSate == E_AF_INACTIVE          ) NewState = E_AF_NOT_FOCUSED_LOCKED;
            else if ( eInCurSate == E_AF_PASSIVE_SCAN      )
            {
                if ( m_eCurAFMode == MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE)
                {
                    /**
                     * During continuous-picture mode :
                     * Eventual transition once the focus is good.
                     * If autofocus(AF_TRIGGER) command is sent during searching, AF will be locked once searching done.
                     */
                    NewState = eInCurSate;
                }
                else
                {
                    /**
                     * During continuous-video mode :
                     * Immediate transition to lock state
                     */
                    NewState = E_AF_NOT_FOCUSED_LOCKED;
                }
            }
            else if ( eInCurSate == E_AF_PASSIVE_FOCUSED   ) NewState = E_AF_FOCUSED_LOCKED;
            else if ( eInCurSate == E_AF_PASSIVE_UNFOCUSED ) NewState = E_AF_NOT_FOCUSED_LOCKED;
            else if ( eInCurSate == E_AF_FOCUSED_LOCKED    ) NewState = E_AF_FOCUSED_LOCKED;
            else if ( eInCurSate == E_AF_NOT_FOCUSED_LOCKED) NewState = E_AF_NOT_FOCUSED_LOCKED;
            else                                                NewState = eInCurSate; /*Should not be happened*/

            if ( m_eEvent & EVENT_CMD_CHANGE_MODE)
            {
                NewState = E_AF_PASSIVE_SCAN;
                CAM_LOGD( "Got changing mode and AF_TRIGGER at same time, force trigger searching");
                m_sDoAFInput.algoCommand.bCancel = MTRUE;
                m_sDoAFInput.algoCommand.bTrigger = MTRUE;
            }

            if ( NewState != E_AF_PASSIVE_SCAN)
            {
                LockAlgo();
            }
            else
            {
                // It will change AF state
                CAM_LOGD( "LockAF until searching done");
                m_bNeedLock = MTRUE;
            }
        }
        break;

    case EVENT_AE_IS_STABLE :
        /*
            if (NO_TRIGGER_CMD) keep previous AF state.  -> NO_TRIGGER_CMD = !(TRIGGER_CMD)
            else                trigger AF
        */
        if (!((m_bForceTrigger == MTRUE && m_bTriggerCmdVlid == MTRUE) && m_bWaitForceTrigger == MFALSE))
        {
            NewState = eInCurSate;
            break;
        }
        else
        {
            // LaunchCamTrigger
            if (m_i4LaunchCamTriggered == E_LAUNCH_AF_WAITING)
            {
                if (m_i4AEStableFrameCount == -1)
                {
                    m_i4AEStableFrameCount = m_u4ReqMagicNum;
                    CAM_LOGD("#(%5d,%5d) %s Dev(%d) LaunchCamTrigger EVENT_AE_IS_STABLE(%d) - BUT NOT TRIGGER YET",
                             m_u4ReqMagicNum, m_u4StaMagicNum,
                             __FUNCTION__,
                             m_i4CurrSensorDev,
                             m_i4AEStableFrameCount);
                }

                if (m_i4AEStableFrameCount != -1 && m_u4ReqMagicNum >= (MUINT32)(m_i4AEStableFrameCount + m_i4AEStableTriggerTimeout))
                {
                    // AE stable and AEStableTimeout(for waiting Face)
                    m_i4LaunchCamTriggered = E_LAUNCH_AF_TRIGGERED;
                    CAM_LOGD( "#(%5d,%5d) %s Dev(%d) LaunchCamTrigger EVENT_AE_IS_STABLE(%d + %d) - UnlockAlgo + TRIGGERAF lib_afmode(%d)",
                              m_u4ReqMagicNum, m_u4StaMagicNum,
                              __FUNCTION__,
                              m_i4CurrSensorDev,
                              m_i4AEStableFrameCount, m_i4AEStableTriggerTimeout,
                              m_eLIB3A_AFMode);
                    UnlockAlgo();
                    m_i4IsLockForLaunchCamTrigger = 0;
                    // no break : Intentionally fall through to triggerAF
                }
                else
                {
                    NewState = eInCurSate;
                    break; // AE stable but not timeout yet ==> skip triggerAF
                }
            }
            else
            {
                NewState = eInCurSate;
                break; // no break : Intentionally fall through to triggerAF
            }
        }
    case EVENT_CMD_TRIGGERAF_WITH_AE_STBL :
        if ( m_bForceTrigger == MTRUE && m_bTriggerCmdVlid == MTRUE)
        {
            NewState = eInCurSate;

            m_bTriggerCmdVlid = MFALSE;
            // force to trigger searching when changing ROI at continuous mode.
            CAM_LOGD( "Force trigger searching [NeedLock(%d) LatchROI(%d) isAFSearch(%d)]", m_bNeedLock, m_bLatchROI, m_i4IsAFSearch_CurState);
            // Cancel + Trigger at the same frame ==> Cancel followed by Trigger for re-triggering AF for precapture
            m_sDoAFInput.algoCommand.bCancel = MTRUE;
            m_sDoAFInput.algoCommand.bTrigger = MTRUE;
            /* HAL trigger AF during seraching without callback */
            m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState = AF_SEARCH_DONE;
        }
        else
        {
            NewState = eInCurSate;
        }
        break;

    case EVENT_CMD_AUTOFOCUS_CANCEL :
        if ( eInCurSate == E_AF_FOCUSED_LOCKED)
        {
            NewState = E_AF_PASSIVE_FOCUSED;
            UnlockAlgo();
        }
        else if ( eInCurSate == E_AF_NOT_FOCUSED_LOCKED)
        {
            NewState = E_AF_PASSIVE_UNFOCUSED;
            UnlockAlgo();
        }
        else if ( eInCurSate == E_AF_PASSIVE_SCAN)
        {
            NewState = E_AF_INACTIVE;
            CAM_LOGD( "Abort search");
            m_sDoAFInput.algoCommand.bCancel = MTRUE;
        }
        else
        {
            NewState = eInCurSate;
        }
        m_bNeedLock = MFALSE;
        // Reseting the para to preventing undesired isAFSearch change.
        m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState = AF_SEARCH_DONE;
        break;

    case EVENT_CMD_SET_AF_REGION :
        if ( eInCurSate == E_AF_PASSIVE_SCAN)
        {
            NewState = E_AF_PASSIVE_UNFOCUSED;
            m_sDoAFInput.algoCommand.bCancel = MTRUE;
        }
        else
        {
            NewState = eInCurSate;
        }

        if ( eInCurSate != E_AF_FOCUSED_LOCKED && eInCurSate != E_AF_NOT_FOCUSED_LOCKED)
        {
            /**
             *  For normal TAF flow, AF HAL got both auto mode and AF region information.
             *  So both m_bForceTrigger and m_bLatchROI will be set MTURE under normal TAF flow.
             *  If TAF is processed under continuous mode, setting m_bForceTrigger and m_bLatchROI as MTRUE here to force trigger searching.
             */
            m_bLatchROI = m_bForceTrigger = m_bTriggerCmdVlid = MTRUE;

            CAM_LOGD( "Force trigger with setting ROI");
        }
        break;

    case EVENT_SEARCHING_START :
        if (m_sCallbackInfo.isAfterAutoMode > 0)
        {
            m_sCallbackInfo.isAfterAutoMode--;
        }
        if (      eInCurSate == E_AF_INACTIVE         ) NewState = E_AF_PASSIVE_SCAN;
        else if ( eInCurSate == E_AF_PASSIVE_FOCUSED  ) NewState = E_AF_PASSIVE_SCAN;
        else if ( eInCurSate == E_AF_PASSIVE_UNFOCUSED) NewState = E_AF_PASSIVE_SCAN;
        else                                                 NewState = eInCurSate;
        break;

    case EVENT_CANCEL_WAIT_FORCE_TRIGGER :
        m_bLatchROI = m_bForceTrigger = m_bTriggerCmdVlid = m_bWaitForceTrigger = MFALSE;
        CAM_LOGD( "isAFSearch[%d]", m_i4IsAFSearch_CurState);
        if ( m_i4IsAFSearch_CurState)
        {
            NewState = eInCurSate;
            break;
        }
    case EVENT_CMD_STOP : // intentionally fall through
        m_bLatchROI = m_bForceTrigger = m_bTriggerCmdVlid = m_bWaitForceTrigger = MFALSE;
    // intentionally fall through
    case EVENT_SEARCHING_END :
        CAM_LOGD( "TriggerCmdValid(%d)/WaitForceTrigger(%d)/bNeedLock(%d)/IsFocused(%d)/eInCurSate(%d)",
                  m_bTriggerCmdVlid,
                  m_bWaitForceTrigger,
                  m_bNeedLock,
                  m_i4IsFocused,
                  eInCurSate);
        if ( m_bTriggerCmdVlid == MTRUE && m_bWaitForceTrigger == MTRUE)
        {
            NewState = E_AF_PASSIVE_SCAN;
        }
        else
        {
            //reset parameters
            m_bLatchROI = m_bForceTrigger = m_bTriggerCmdVlid = m_bWaitForceTrigger = MFALSE;

            if ( m_bNeedLock == MTRUE)
            {
                if ( m_i4IsFocused == 1) NewState = E_AF_FOCUSED_LOCKED;
                else                  NewState = E_AF_NOT_FOCUSED_LOCKED;

                //lock
                LockAlgo();
                m_bNeedLock = MFALSE;

            }
            else if ( eInCurSate == E_AF_PASSIVE_SCAN)
            {
                if ( m_i4IsFocused == 1) NewState = E_AF_PASSIVE_FOCUSED;
                else                  NewState = E_AF_PASSIVE_UNFOCUSED;
            }
            else
            {
                NewState = eInCurSate;

                if ( sInEvent == EVENT_CMD_STOP && eInCurSate == E_AF_INACTIVE)
                {
                    // force doing one time searching when next time start preview.
                    m_bLatchROI = m_bForceTrigger = m_bTriggerCmdVlid = MTRUE;
                    CAM_LOGD( "Do one time searching when next time start preview.");
                }
            }
        }
        break;

    case EVENT_SEARCHING_DONE_RESET_PARA:
        NewState = eInCurSate;
        if (eInCurSate == E_AF_INACTIVE || eInCurSate == E_AF_PASSIVE_UNFOCUSED)
        {
            if (m_i4IsFocused == 1)
                NewState = E_AF_PASSIVE_FOCUSED;
        }
        if ( m_bTriggerCmdVlid == MTRUE && m_bWaitForceTrigger == MTRUE)
        {
            /* For capture with flash */
            CAM_LOGD( "hybrid AF searching durig capture state");
            /*    _________            _____________
                           |__________|                     First time searching with PD doesn't change state
                  ___________________            ________
                                     |__________|           capture flow during first time searching with PD
             */
        }
        else
        {
            //reset parameters
            m_bLatchROI = m_bForceTrigger = m_bTriggerCmdVlid = MFALSE;
        }
        break;

    case EVENT_SET_WAIT_FORCE_TRIGGER :
        if ( eInCurSate != E_AF_FOCUSED_LOCKED && eInCurSate != E_AF_NOT_FOCUSED_LOCKED)
        {
            if (m_eCurAFMode == MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE)
            {
                m_bTriggerCmdVlid = m_bForceTrigger = m_bWaitForceTrigger = MTRUE;
                CAM_LOGD( "Set wait force trigger for preCap");
            }
            if ( m_i4IsAFSearch_CurState == AF_SEARCH_DONE)
            {
                LockAlgo();
            }
            // send callback for the next two times
            if (m_sCallbackInfo.isAfterAutoMode == 1)
            {
                m_sCallbackInfo.isAfterAutoMode = 2;
            }
        }

        NewState = eInCurSate;
        break;

    default : /*Should not be happened*/
        NewState = eInCurSate;
        break;
    }

    return NewState;

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
E_AF_STATE_T AfMgr::UpdateStateAutoMode( E_AF_STATE_T & eInCurSate, AF_EVENT_T & sInEvent)
{
    E_AF_STATE_T NewState = E_AF_INACTIVE;
    switch ( sInEvent)
    {
    case EVENT_CMD_CHANGE_MODE :
        NewState = E_AF_INACTIVE;
        /**
         *  For normal TAF flow, AF HAL got both auto mode and AF region information.
         *  Setting m_bForceTrigger and m_bLatchROI as MTRUE here to wait trigger searching..
         */
        m_bNeedCheckSendCallback = MFALSE;
        m_i4TAFStatus   = TAF_STATUS_RESET;
        // LaunchCamTrigger get disable in afmodes except ContinuousMode
        if (m_i4LaunchCamTriggered == E_LAUNCH_AF_WAITING)
        {
            UnlockAlgo();
        }
        m_i4LaunchCamTriggered = E_LAUNCH_AF_DONE;
        m_i4AEStableFrameCount = -1;
        m_i4ValidPDFrameCount = -1;
        // Reseting the para to preventing undesired isAFSearch change.
        m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState = AF_SEARCH_DONE;
        break;

    case EVENT_CMD_AUTOFOCUS :
        if (      eInCurSate == E_AF_INACTIVE          ) NewState = E_AF_ACTIVE_SCAN;
        else if ( eInCurSate == E_AF_FOCUSED_LOCKED    ) NewState = E_AF_ACTIVE_SCAN;
        else if ( eInCurSate == E_AF_NOT_FOCUSED_LOCKED) NewState = E_AF_ACTIVE_SCAN;
        else                                                  NewState = eInCurSate; /*Should be at E_AF_ACTIVE_SCAN*/

        // get Autofocus when [(CenterROI exist) or (AE locked)] ==> ForceDoAlgo to speed up SEARCH_END
        if (m_i4IsCAFWithoutFace == 1 || i4IsLockAERequest == 1)
        {
            m_bForceDoAlgo = MTRUE;
        }
        m_sCallbackInfo.isAfterAutoMode = 1;
        m_i4TAFStatus = TAF_STATUS_RECEIVE_AUTOFOCUS;
        break;

    case EVENT_CMD_TRIGGERAF_WITH_AE_STBL :
        NewState = eInCurSate;
        m_sDoAFInput.algoCommand.bTrigger = MTRUE;
        break;

    case EVENT_CMD_AUTOFOCUS_CANCEL :
        if (      eInCurSate == E_AF_ACTIVE_SCAN       ) NewState = E_AF_INACTIVE;
        else if ( eInCurSate == E_AF_FOCUSED_LOCKED    ) NewState = E_AF_INACTIVE;
        else if ( eInCurSate == E_AF_NOT_FOCUSED_LOCKED) NewState = E_AF_INACTIVE;
        else                                            NewState = eInCurSate; /*Should be at E_AF_INACTIVE*/

        if ( m_i4TAFStatus == TAF_STATUS_RECEIVE_AUTOFOCUS)
        {
            m_i4TAFStatus = TAF_STATUS_RECEIVE_CANCELAUTOFOCUS;
            CAM_LOGD( "receive cancelautofocus after seraching end");
        }

        m_sDoAFInput.algoCommand.bCancel = MTRUE;
        // Reseting the para to preventing undesired isAFSearch change.
        m_i4IsAFSearch_PreState = m_i4IsAFSearch_CurState = AF_SEARCH_DONE;
        break;

    case EVENT_CMD_SET_AF_REGION :
        /**
         *  For normal TAF flow, AF HAL got both auto mode and AF region information.
         *  Setting m_bForceTrigger and m_bLatchROI as MTRUE here to wait trigger searching..
         */
        m_bLatchROI = MTRUE;
        NewState = eInCurSate;
        break;

    case EVENT_SEARCHING_START :
        NewState = E_AF_ACTIVE_SCAN;
        if (m_eEvent & EVENT_CMD_AUTOFOCUS_CANCEL)
        {
            CAM_LOGD("It has happened cancelautofocus & searching start at the same time");
            NewState = E_AF_INACTIVE;
        }
        break;

    case EVENT_CANCEL_WAIT_FORCE_TRIGGER :
    case EVENT_CMD_STOP :
    case EVENT_SEARCHING_END :
        m_bLatchROI = MFALSE;
        if ( eInCurSate == E_AF_ACTIVE_SCAN)
        {
            if ( m_i4IsFocused == 1) NewState = E_AF_FOCUSED_LOCKED;
            else                  NewState = E_AF_NOT_FOCUSED_LOCKED;
        }
        else
        {
            NewState = eInCurSate;
        }
        break;

    case EVENT_SET_WAIT_FORCE_TRIGGER :
        NewState = eInCurSate;
        break;

    case EVENT_AE_IS_STABLE :
    default : /*Should not be happened*/
        NewState = eInCurSate;
        break;
    }

    return NewState;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
E_AF_STATE_T AfMgr::UpdateState( AF_EVENT_T sInEvent)
{
    Mutex::Autolock lock( m_AFStateLock);

    E_AF_STATE_T NewState;

    switch ( sInEvent)
    {
    case EVENT_CMD_CHANGE_MODE:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_CMD_CHANGE_MODE", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_CMD_CHANGE_MODE;
        break;
    case EVENT_CMD_AUTOFOCUS:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_CMD_AUTOFOCUS", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_CMD_AUTOFOCUS;
        break;
    case EVENT_CMD_TRIGGERAF_WITH_AE_STBL:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_CMD_TRIGGERAF_WITH_AE_STBL", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_CMD_TRIGGERAF_WITH_AE_STBL;
        break;
    case EVENT_CMD_AUTOFOCUS_CANCEL:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_CMD_AUTOFOCUS_CANCEL", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_CMD_AUTOFOCUS_CANCEL;
        break;
    case EVENT_CMD_SET_AF_REGION:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_CMD_SET_AF_REGION", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_CMD_SET_AF_REGION;
        break;
    case EVENT_CMD_STOP:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_CMD_STOP", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_CMD_STOP;
        break;
    case EVENT_SEARCHING_START:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_SEARCHING_START", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_SEARCHING_START;
        break;
    case EVENT_SEARCHING_END:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_SEARCHING_END", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_SEARCHING_END;
        break;
    case EVENT_SEARCHING_DONE_RESET_PARA:
        m_eEvent |= EVENT_SEARCHING_DONE_RESET_PARA;
        break;
    case EVENT_AE_IS_STABLE:
        //CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_AE_IS_STABLE", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_AE_IS_STABLE;
        break;
    case EVENT_SET_WAIT_FORCE_TRIGGER :
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_SET_WAIT_FORCE_TRIGGER", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_SET_WAIT_FORCE_TRIGGER;
        break;
    case EVENT_CANCEL_WAIT_FORCE_TRIGGER :
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_CANCEL_WAIT_FORCE_TRIGGER", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        m_eEvent |= EVENT_CANCEL_WAIT_FORCE_TRIGGER;
        break;
    default:
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d) EVENT_WRONG", m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__, m_i4CurrSensorDev);
        break;
    }

    switch ( m_eCurAFMode)
    {
    case MTK_CONTROL_AF_MODE_OFF :
        NewState = UpdateStateMFMode( m_eAFState, sInEvent);
        break;
    case MTK_CONTROL_AF_MODE_AUTO :
    case MTK_CONTROL_AF_MODE_MACRO :
        NewState = UpdateStateAutoMode( m_eAFState, sInEvent);
        break;
    case MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO :
    case MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE :
        NewState = UpdateStateContinuousMode( m_eAFState, sInEvent);
        break;
    case MTK_CONTROL_AF_MODE_EDOF :
    default :
        NewState = UpdateStateOFFMode( m_eAFState, sInEvent);
        break;
    }

    if ( m_eAFState != NewState)
    {
        CAM_LOGD( "#(%5d,%5d) %s  Dev(%d) : %d->%d",
                  m_u4ReqMagicNum,
                  m_u4StaMagicNum,
                  __FUNCTION__,
                  m_i4CurrSensorDev,
                  m_eAFState,
                  NewState);
    }

    m_eAFState = NewState;

    return NewState;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::UpdateCenterROI( AREA_T & sOutAreaCenter) /*ApplyZoomEffect*/
{
    MUINT32 scalex = 100;
    MUINT32 scaley = 100;

    if ( m_bEnablePD)
    {
        scalex = m_ptrNVRam->rAFNVRAM.i4HybridAFCoefs[1];
        scaley = m_ptrNVRam->rAFNVRAM.i4HybridAFCoefs[2];
    }
    else
    {
        scalex = m_ptrNVRam->rAFNVRAM.i4Common[12];
        scaley = m_ptrNVRam->rAFNVRAM.i4Common[13];
    }

    if (     100 < scalex) scalex = 100;
    else if ( scalex <= 0) scalex = 15;

    if (     100 < scaley) scaley = 100;
    else if ( scaley <= 0) scaley = 15;

    CAM_LOGD( "%s %d %d %d %d - scale %d %d",
              __FUNCTION__,
              m_ptrNVRam->rAFNVRAM.i4HybridAFCoefs[1],
              m_ptrNVRam->rAFNVRAM.i4HybridAFCoefs[2],
              m_ptrNVRam->rAFNVRAM.i4Common[12],
              m_ptrNVRam->rAFNVRAM.i4Common[13],
              scalex,
              scaley);

    MUINT32 cropRegionCenter_X = m_sCropRegionInfo.i4X + (m_sCropRegionInfo.i4W / 2);
    MUINT32 cropRegionCenter_Y = m_sCropRegionInfo.i4Y + (m_sCropRegionInfo.i4H / 2);
    MUINT32 croiw = m_i4TGSzW * scalex / 100;
    MUINT32 croih = m_i4TGSzH * scaley / 100;
    MUINT32 croix = (cropRegionCenter_X > (croiw / 2)) ? cropRegionCenter_X - (croiw / 2) : 0;
    MUINT32 croiy = (cropRegionCenter_Y > (croih / 2)) ? cropRegionCenter_Y - (croih / 2) : 0;

    //updateing.
    sOutAreaCenter.i4X =  croix;
    sOutAreaCenter.i4Y =  croiy;
    sOutAreaCenter.i4W =  croiw;
    sOutAreaCenter.i4H =  croih;
    sOutAreaCenter.i4Info = AF_MARK_NONE;

    CAM_LOGD( "%s ROI-C [X]%d [Y]%d [W]%d [H]%d",
              __FUNCTION__,
              sOutAreaCenter.i4X,
              sOutAreaCenter.i4Y,
              sOutAreaCenter.i4W,
              sOutAreaCenter.i4H);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::LockAlgo(MUINT32 u4Caller)
{
    if ( u4Caller == AF_MGR_CALLER)
    {
        m_bLock = MTRUE;
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d): Lock(%d), Pause(%d)",
                  m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__,
                  m_i4CurrSensorDev, m_bLock, m_bPauseAF);
    }
    else
    {
        m_bPauseAF = MTRUE;
        CAM_LOGD( "#(%5d,%5d) cmd-%s Dev(%d): Lock(%d), Pause(%d)",
                  m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__,
                  m_i4CurrSensorDev, m_bLock, m_bPauseAF);
    }

    if (m_bLock || m_bPauseAF)
    {
        m_sDoAFInput.algoCommand.eLockAlgo = AfCommand_Start;
    }

    return m_bLock || m_bPauseAF;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::UnlockAlgo(MUINT32 u4Caller)
{
    if ( u4Caller == AF_MGR_CALLER)
    {
        m_bLock = MFALSE;
        CAM_LOGD( "#(%5d,%5d) %s Dev(%d): Lock(%d), Pause(%d)",
                  m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__,
                  m_i4CurrSensorDev, m_bLock, m_bPauseAF);
    }
    else
    {
        m_bPauseAF = MFALSE;
        CAM_LOGD( "#(%5d,%5d) cmd-%s Dev(%d): Lock(%d), Pause(%d)",
                  m_u4ReqMagicNum, m_u4StaMagicNum, __FUNCTION__,
                  m_i4CurrSensorDev, m_bLock, m_bPauseAF);
    }

    if (m_bLock == MFALSE && m_bPauseAF == MFALSE)
    {
        m_sDoAFInput.algoCommand.eLockAlgo = AfCommand_Cancel;
    }

    return m_bLock || m_bPauseAF;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::isSkipHandleAF()
{
    MINT32 isSkip = -1;
    MBOOL isHWRdy = MTRUE;

    if (m_i4DbgAutoBitTrue)
    {
        isSkip = 1;
        CAM_LOGD("%s skip af for auto bit true");
    }
    else if (m_i4SkipAlgoDueToHwConstaint == 1)
    {
        isSkip = 1;
        CAM_LOGW(" %s HwConstraint Happened ==> KEEP SKIPPING AF", __FUNCTION__);
    }
    else if ( m_eCurAFMode == MTK_CONTROL_AF_MODE_OFF)
    {
        isSkip = 0; // AF off mode don't need to check statistic data, but need run handleAF.
    }
    else if (m_bForceDoAlgo == MTRUE)
    {
        isSkip = 0;
    }
    else if (m_sAFOutput->i4ZECChg == 1 || m_sAFOutput->i4IsTargetAssitMove == 1)
    {
        isSkip = 0;
    }
    else if (m_sAFOutput->i4ROISel == AF_ROI_SEL_NONE)
    {
        isSkip = 0; // The condition AF_ROI_SEL_NONE is for the first time lunch camera only. [Ref] SelROIToFocusing
    }

    if ( LEVEL_FLOW || isSkip == 1)
    {
        CAM_LOGD("#(%5d,%5d) %s Dev(%d) Config(%d) bSkip(%d) PauseAF(%d) Afmode(%d) ForceDoAlgo(%d) IsTargetAssitMove(%d) isHWRdy(%d) ROISel(%d) ZECChg(%d)",
                 m_u4ReqMagicNum,
                 m_u4StaMagicNum,
                 __FUNCTION__,
                 m_i4CurrSensorDev,
                 m_u4ConfigHWNum,
                 isSkip,
                 m_bPauseAF,
                 m_eCurAFMode,
                 m_bForceDoAlgo,
                 m_sAFOutput->i4IsTargetAssitMove,
                 isHWRdy,
                 m_sAFOutput->i4ROISel,
                 m_sAFOutput->i4ZECChg);
    }
    return isSkip;
}
// TODO : need to chech
MVOID AfMgr::IspMgrAFGetROIFromHw(AREA_T & Area, MUINT32 & isTwin __unused)
{
    // ISP_MGR_AF_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).getROIFromHw(Area, isTwin);
    // transfer AFROI(BINSize) to AFROI(TGSize)
    if (m_i4BINSzW != 0)
    {
        Area.i4X *= m_i4TGSzW / m_i4BINSzW;
        Area.i4Y *= m_i4TGSzW / m_i4BINSzW;
        Area.i4W *= m_i4TGSzW / m_i4BINSzW;
        Area.i4H *= m_i4TGSzW / m_i4BINSzW;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::preparePDData()
{

    if ( m_bRunPDEn == MTRUE)
    {
#define PERFRAME_GET_PD_RESULT 1
        // LaunchCamTrigger Search Done ==> get PD every 2 frames
        if ( m_i4LaunchCamTriggered != E_LAUNCH_AF_DONE)
        {
            m_bPdInputExpected = (m_i4FirsetCalPDFrameCount > 0) && (m_u4ReqMagicNum > m_i4FirsetCalPDFrameCount);
        }
        else if ( m_ptrNVRam->rAFNVRAM.i4EasyTuning[2] == PERFRAME_GET_PD_RESULT || m_sPDRois[eIDX_ROI_ARRAY_FD].valid)
        {
            m_bPdInputExpected = MTRUE;
        }
        else
        {
            m_bPdInputExpected = 1 - m_bPdInputExpected;
        }
        CAM_LOGD("%s, pdInputExpected: %d", __FUNCTION__, m_bPdInputExpected);

        m_sAFInput->sPDInfo.i4Valid = 0;
        if ( m_bPdInputExpected)
        {
            PD_CALCULATION_OUTPUT *ptrPDRes = nullptr;
            if ( IPDMgr::getInstance().getPDTaskResult( m_i4CurrSensorDev, &ptrPDRes) == S_3A_OK)
            {
                if ( ptrPDRes)
                {
                    // set pd result to pd algorithm
                    memset( &m_sAFInput->sPDInfo, 0, sizeof(AFPD_INFO_T));
                    m_sAFInput->sPDInfo.i4Valid  = MTRUE;
                    m_sAFInput->sPDInfo.i4FrmNum = ptrPDRes->frmNum;

                    MINT32 _idx = 0;
                    for (MINT32 i = 0; i < ptrPDRes->numRes; i++)
                    {
                        if (ptrPDRes->Res[i].sROIInfo.sType == eAF_ROI_TYPE_AP)
                        {
                            if ( /* for multi touch case*/
                                (ptrPDRes->Res[i].sROIInfo.sPDROI.i4X != m_sArea_APCmd.i4X) ||
                                (ptrPDRes->Res[i].sROIInfo.sPDROI.i4Y != m_sArea_APCmd.i4Y) ||
                                (ptrPDRes->Res[i].sROIInfo.sPDROI.i4W != m_sArea_APCmd.i4W) ||
                                (ptrPDRes->Res[i].sROIInfo.sPDROI.i4H != m_sArea_APCmd.i4H))
                            {
                                continue;
                            }
                        }

                        m_sAFInput->sPDInfo.sPDBlockInfo[_idx].sBlockROI.sType   = ptrPDRes->Res[i].sROIInfo.sType;
                        m_sAFInput->sPDInfo.sPDBlockInfo[_idx].sBlockROI.sPDROI  = ptrPDRes->Res[i].sROIInfo.sPDROI;
                        m_sAFInput->sPDInfo.sPDBlockInfo[_idx].i4PDafDacIndex    = ptrPDRes->Res[i].DesLensPos;
                        m_sAFInput->sPDInfo.sPDBlockInfo[_idx].i4PDafConverge    = ptrPDRes->Res[i].PhaseDifference;
                        m_sAFInput->sPDInfo.sPDBlockInfo[_idx].i4PDafConfidence  = (MUINT32)ptrPDRes->Res[i].ConfidenceLevel;
                        m_sAFInput->sPDInfo.sPDBlockInfo[_idx].i4SatPercent      = ptrPDRes->Res[i].SaturationPercent;

                        // print log if necessary
                        if ( LEVEL_STT)
                        {
                            CAM_LOGD("[%s] #%d (%d,%4d,%4d,%4d,%4d,%3d->%3d,%6d,%3d,%d)",
                                     __FUNCTION__,
                                     _idx,
                                     m_sAFInput->sPDInfo.sPDBlockInfo[_idx].sBlockROI.sType,
                                     m_sAFInput->sPDInfo.sPDBlockInfo[_idx].sBlockROI.sPDROI.i4X,
                                     m_sAFInput->sPDInfo.sPDBlockInfo[_idx].sBlockROI.sPDROI.i4Y,
                                     m_sAFInput->sPDInfo.sPDBlockInfo[_idx].sBlockROI.sPDROI.i4W,
                                     m_sAFInput->sPDInfo.sPDBlockInfo[_idx].sBlockROI.sPDROI.i4H,
                                     ptrPDRes->Res[i].CurLensPos,
                                     m_sAFInput->sPDInfo.sPDBlockInfo[_idx].i4PDafDacIndex,
                                     m_sAFInput->sPDInfo.sPDBlockInfo[_idx].i4PDafConverge,
                                     m_sAFInput->sPDInfo.sPDBlockInfo[_idx].i4PDafConfidence,
                                     m_sAFInput->sPDInfo.sPDBlockInfo[_idx].i4SatPercent);
                        }

                        //
                        _idx++;
                    }
                    m_sAFInput->sPDInfo.i4PDBlockInfoNum = _idx;

                    //release resource
                    delete ptrPDRes;
                    ptrPDRes = nullptr;
                }
                else
                {
                    m_sAFInput->sPDInfo.i4Valid = 0;
                    CAM_LOGE( "get null pointer result from pd manager, should not be happened !!");
                }
            }
            else
            {
                m_sAFInput->sPDInfo.i4Valid = 0;
                CAM_LOGW( "pd result is not ready! statMagicNum(%d)", m_u4StaMagicNum);
            }
        }
    }

    //
    if ( LEVEL_STT)
    {
        CAM_LOGD("[%s] vd(%d), Run PD flow(%d), PD input is expected(%d),  Hybrid AF input:frmNum(%d), PD win number(%d)",
                 __FUNCTION__,
                 m_sAFInput->sPDInfo.i4Valid,
                 m_bRunPDEn,
                 m_bPdInputExpected,
                 m_sAFInput->sPDInfo.i4FrmNum,
                 m_sAFInput->sPDInfo.i4PDBlockInfoNum);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::UpdatePDParam( MINT32 & i4InROISel)
{
    if ( m_bEnablePD)
    {
        char  dbgMsgBuf[DBG_MSG_BUF_SZ];
        char* ptrMsgBuf = dbgMsgBuf;

        if ( LEVEL_FLOW)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf,
                                  "#(%5d,%5d) %s total roi Fmt(%d) Info(vd,Fmt,X,Y,W,H,Info): ",
                                  m_u4ReqMagicNum,
                                  m_u4StaMagicNum,
                                  __FUNCTION__,
                                  eIDX_ROI_ARRAY_NUM);
        }


        MINT32 PDRoisNum = 0;
        AFPD_BLOCK_ROI_T PDRois[eIDX_ROI_ARRAY_NUM];

        for ( MUINT16 i = 0; i < eIDX_ROI_ARRAY_NUM; i++)
        {
            MBOOL select = MFALSE;

            if ( LEVEL_FLOW)
            {

                ptrMsgBuf += sprintf( ptrMsgBuf,
                                      "#%d(%d,%d,%d,%d,%d,%d,%d) ",
                                      i,
                                      m_sPDRois[i].valid,
                                      m_sPDRois[i].info.sType,
                                      m_sPDRois[i].info.sPDROI.i4X,
                                      m_sPDRois[i].info.sPDROI.i4Y,
                                      m_sPDRois[i].info.sPDROI.i4W,
                                      m_sPDRois[i].info.sPDROI.i4H,
                                      m_sPDRois[i].info.sPDROI.i4Info);
            }

            switch (i)
            {
            case eIDX_ROI_ARRAY_AP:
                select = m_sPDRois[i].info.sType == eAF_ROI_TYPE_AP ? MTRUE : MFALSE;
                m_sPDRois[i].valid = i4InROISel == AF_ROI_SEL_AP ? MTRUE : m_sPDRois[i].valid; // force calculating FD window when hybrid AF need PD result of AP roi.
                break;

            case eIDX_ROI_ARRAY_FD:
                select = m_sPDRois[i].info.sType == eAF_ROI_TYPE_FD ? MTRUE : MFALSE;
                m_sPDRois[i].valid = ((i4InROISel == AF_ROI_SEL_FD) || (i4InROISel == AF_ROI_SEL_OT)) ? MTRUE : m_sPDRois[i].valid; // force calculating FD window when hybrid AF is at FDAF stage.
                break;

            case eIDX_ROI_ARRAY_CENTER:
                select = m_sPDRois[i].info.sType == eAF_ROI_TYPE_CENTER ? MTRUE : MFALSE;
                break;

            default :
                select = MFALSE;
                break;

            }

            // Only center ROI is calculated for verification flow.
            if ( m_i4DbgPDVerifyEn)
            {
                m_sPDRois[i].valid = (i == eIDX_ROI_ARRAY_CENTER) ? MTRUE : MFALSE;
            }

            if ( select && m_sPDRois[i].valid)
            {
                //
                PDRois[PDRoisNum] = m_sPDRois[i].info;

                //debug log
                if ( LEVEL_FLOW)
                {
                    ptrMsgBuf += sprintf( ptrMsgBuf,
                                          "->(%d,%d,%d,%d,%d,%d,%d) ",
                                          m_sPDRois[i].valid,
                                          PDRois[PDRoisNum].sType,
                                          PDRois[PDRoisNum].sPDROI.i4X,
                                          PDRois[PDRoisNum].sPDROI.i4Y,
                                          PDRois[PDRoisNum].sPDROI.i4W,
                                          PDRois[PDRoisNum].sPDROI.i4H,
                                          PDRois[PDRoisNum].sPDROI.i4Info);
                }

                //
                PDRoisNum++;
            }
        }

        GetPdBlockInput_T pdBlockInput;
        memcpy(pdBlockInput.inAfRoi, PDRois, sizeof(AFPD_BLOCK_ROI_T) * 3);
        memcpy(&(pdBlockInput.inFdInfo), &(m_sAFInput->sFDInfo), sizeof(FD_INFO_T));
        pdBlockInput.inAfRoiNum = PDRoisNum;
        pdBlockInput.inWinBufSz = AF_PSUBWIN_NUM;
        GetPdBlockOutput_T pdBlockOutput;
        memcpy(pdBlockOutput.outWins,  m_sPDCalculateWin, sizeof(AFPD_BLOCK_ROI_T) * 36);
        pdBlockOutput.outWinNum = m_i4PDCalculateWinNum;
        MRESULT res = E_3A_ERR;
        if (s_pIAfCxU)
        {
            res = s_pIAfCxU->getPdBlocks(pdBlockInput, pdBlockOutput);
        }
        m_i4PDCalculateWinNum = pdBlockOutput.outWinNum;
        memcpy(m_sPDCalculateWin, pdBlockOutput.outWins, sizeof(AFPD_BLOCK_ROI_T) * 36);

        // error check
        if ( LEVEL_FLOW)
        {
            ptrMsgBuf += sprintf( ptrMsgBuf, "PDCalculateWin(total%d) Info(Fmt,X,Y,W,H,Info): ", m_i4PDCalculateWinNum);
        }

        for ( MINT32 i = 0; i < m_i4PDCalculateWinNum; i++)
        {
            if ( LEVEL_FLOW)
            {
                ptrMsgBuf += sprintf( ptrMsgBuf,
                                      "#%d(%d,%d,%d,%d,%d,%d) ",
                                      i,
                                      m_sPDCalculateWin[i].sType,
                                      m_sPDCalculateWin[i].sPDROI.i4X,
                                      m_sPDCalculateWin[i].sPDROI.i4Y,
                                      m_sPDCalculateWin[i].sPDROI.i4W,
                                      m_sPDCalculateWin[i].sPDROI.i4H,
                                      m_sPDCalculateWin[i].sPDROI.i4Info);
            }

            if ( /* Boundary checking */
                (m_sPDCalculateWin[i].sPDROI.i4W <= 0) ||
                (m_sPDCalculateWin[i].sPDROI.i4H <= 0) ||
                (m_sPDCalculateWin[i].sPDROI.i4X <  0) ||
                (m_sPDCalculateWin[i].sPDROI.i4Y <  0) )
            {
                CAM_LOGE( "PD Calculation window is wrong, please check function(getPdBlocks) behavior!!");
                res = E_3A_ERR;
                break;
            }

        }

        if ( res != S_3A_OK)
        {
            m_i4PDCalculateWinNum = 1;
            m_sPDCalculateWin[0]  = m_sPDRois[eIDX_ROI_ARRAY_CENTER].info;
        }
        else if ( m_i4DbgPDVerifyEn && (m_i4PDCalculateWinNum < AF_PSUBWIN_NUM))
        {
            m_i4PDCalculateWinNum++;
            m_sPDCalculateWin[m_i4PDCalculateWinNum - 1] = m_sPDRois[eIDX_ROI_ARRAY_CENTER].info;
        }

        m_bRunPDEn = MTRUE;

        CAM_LOGD_IF( LEVEL_FLOW, "%s", dbgMsgBuf);

        /* exectuing from first request */
        MBOOL isForceCalPD = (MBOOL)((m_i4LaunchCamTriggered != E_LAUNCH_AF_DONE) || (m_sPDRois[eIDX_ROI_ARRAY_FD].valid));
        AAA_TRACE_D("UpdatePDParam (%d)", m_u4StaMagicNum);
        IPDMgr::getInstance().UpdatePDParam( m_i4CurrSensorDev,
                                             m_u4StaMagicNum,
                                             m_i4PDCalculateWinNum,
                                             m_sPDCalculateWin,
                                             m_sAFOutput->i4SearchRangeInf,
                                             m_sAFOutput->i4SearchRangeMac,
                                             isForceCalPD);
        AAA_TRACE_END_D;

        /*****************************************************************************************************************
         * FD roi is calculated once a FD region is set
         * So reset status once region is read out
         *****************************************************************************************************************/
        if ( m_sPDRois[eIDX_ROI_ARRAY_FD].valid)
        {
            MUINT64 u8CalFDTS = getTimeStamp_us();

            if (u8CalFDTS > m_u8RecvFDTS)
            {
                if (u8CalFDTS - m_u8RecvFDTS > 100000) /* unit : us*/
                {
                    m_sPDRois[eIDX_ROI_ARRAY_FD].valid = MFALSE;
                }
            }
        }

        //
        if ((m_i4FirsetCalPDFrameCount == -1) && (m_u4ReqMagicNum > 0))
        {
            m_i4FirsetCalPDFrameCount = m_u4ReqMagicNum;
        }

    }
    else
    {
        m_bRunPDEn = MFALSE;
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::initLD()
{
    CAM_LOGD_IF( LEVEL_FLOW, "[%s]", __FUNCTION__);
    // --- init Laser ---
    if (m_bLDAFEn == MFALSE)
    {
        m_pLaserMgr = LaserMgr::getInstance(m_i4CurrSensorDev);

        if (m_pLaserMgr != MNULL)
        {
            if (m_pLaserMgr->init() != MFALSE)
            {
                m_bLDAFEn = MTRUE;
                CAM_LOGD("[%s] LaserMgr init() done", __FUNCTION__);
            }
            else
            {
                CAM_LOGD("[%s] LaserMgr init() fail", __FUNCTION__);
            }
        }
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::startLD()
{
    CAM_LOGD_IF( LEVEL_FLOW, "[%s]", __FUNCTION__);

    if (m_bLDAFEn == MTRUE && m_pLaserMgr != MNULL)
    {
        if ( property_get_int32("vendor.laser.calib.disable", 0) == 0 )
        {
            MINT32 i4OffsetData = m_ptrNVRam->rAFNVRAM.i4Revs[201];
            MINT32 i4XTalkData  = m_ptrNVRam->rAFNVRAM.i4Revs[202];
            m_pLaserMgr->setLaserCaliData(i4OffsetData, i4XTalkData);
        }

        MINT32 i4LaserMaxDisRatio = m_ptrNVRam->rAFNVRAM.i4Revs[200];
        MINT32 i4LaserMaxDistance = m_ptrNVRam->rAFNVRAM.i4Revs[203];
        MINT32 i4LaserTableNum    = m_ptrNVRam->rAFNVRAM.i4Revs[204];
        m_pLaserMgr->setLaserGoldenTable(&m_ptrNVRam->rAFNVRAM.i4Revs[205], i4LaserTableNum, i4LaserMaxDistance, i4LaserMaxDisRatio);

        m_sAFInput.sLaserInfo.i4AfWinPosCnt = 0;

        if (m_pLaserMgr->checkHwSetting() == 0 || property_get_int32("vendor.laser.disable", 0) == 1)
        {
            m_pLaserMgr->uninit();
            m_pLaserMgr = NULL;
            m_bLDAFEn = MFALSE;
            CAM_LOGE( "AF-%-15s: LaserMgr checkHwSetting() fail", __FUNCTION__);
        }
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::camPwrOffLD()
{
    CAM_LOGD_IF( LEVEL_FLOW, "[%s]", __FUNCTION__);

    if (m_bLDAFEn == MTRUE && m_pLaserMgr != MNULL)
    {
        CAM_LOGD( "[%s] LaserMgr uninit() done", __FUNCTION__);
        m_pLaserMgr->uninit();
        m_pLaserMgr = MNULL;
        m_bLDAFEn = MFALSE;
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::prepareLDData()
{
    if (m_bLDAFEn == MTRUE && m_pLaserMgr != NULL)
    {
        MINT32 i4LaserStatus = m_pLaserMgr->getLaserCurStatus();

        if (i4LaserStatus == STATUS_RANGING_VALID)
        {
            m_sAFInput.sLaserInfo.i4CurPosDAC   = m_pLaserMgr->getLaserCurDac();
            m_sAFInput.sLaserInfo.i4CurPosDist  = m_pLaserMgr->getLaserCurDist();
        }
        else
        {
            m_sAFInput.sLaserInfo.i4CurPosDAC   = m_pLaserMgr->predictAFStartPosDac();
            m_sAFInput.sLaserInfo.i4CurPosDist  = m_pLaserMgr->predictAFStartPosDist();
            m_sAFInput.sLaserInfo.i4AfWinPosCnt = 0;
        }
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::initSD()
{
    CAM_LOGD_IF( LEVEL_FLOW, "[%s]", __FUNCTION__);
    m_sDAF_TBL.is_daf_run = E_DAF_OFF;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::startSD()
{
    CAM_LOGD_IF( LEVEL_FLOW, "[%s]", __FUNCTION__);

    if ( m_i4CurrLensId != 0xffff)
    {
        MINT32 AFtbl_Num;
        MINT32 AFtbl_Marco;
        MINT32 AFtbl_Inf;

        m_bSDAFEn = MFALSE;
        AFtbl_Num =  m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum;
        if (AFtbl_Num > 1)
        {
            AFtbl_Inf   = m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4Offset + m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4Pos[0];
            AFtbl_Marco = m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4Offset + m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4Pos[AFtbl_Num - 1];
        }
        else //AF table method2
        {
            AFtbl_Inf   = m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4Pos[25] - m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4Pos[21];
            if (AFtbl_Inf   < 0        ) AFtbl_Inf = 0;
            AFtbl_Marco = AFtbl_Inf + m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4Pos[24] + m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4Pos[21];
            if (AFtbl_Marco > 1023     ) AFtbl_Marco = 1023;
            if (AFtbl_Marco < AFtbl_Inf) AFtbl_Marco = AFtbl_Inf + 1;
        }

        m_sDAF_TBL.af_dac_min        = AFtbl_Inf;
        m_sDAF_TBL.af_dac_max        = AFtbl_Marco;
        m_sDAF_TBL.af_dac_inf        = AFtbl_Inf;
        m_sDAF_TBL.af_dac_marco      = AFtbl_Marco;
        m_sDAF_TBL.af_distance_inf   = 0;
        m_sDAF_TBL.af_distance_marco = 0;

        CAM_LOGD( "AF-%-15s: default data [af_dac_inf]%d [af_dac_marco]%d [af_dac_min]%d [af_dac_max]%d [af_distance_inf]%d [af_distance_marco]%d\n",
                  __FUNCTION__,
                  (MINT32)m_sDAF_TBL.af_dac_inf,
                  (MINT32)m_sDAF_TBL.af_dac_marco,
                  (MINT32)m_sDAF_TBL.af_dac_min,
                  (MINT32)m_sDAF_TBL.af_dac_max,
                  (MINT32)m_sDAF_TBL.af_distance_inf,
                  (MINT32)m_sDAF_TBL.af_distance_marco);
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::prepareSDData()
{

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::updateSDParam()
{
    //==========
    // Prepare AF info to Vsdof
    //==========
    if ( m_sDAF_TBL.is_daf_run & E_DAF_RUN_STEREO)
    {
        MUINT32 MagicNum, CurDafTblIdx;

        MagicNum = m_u4StaMagicNum + 1; // m_sAFOutput can match next image status

        CurDafTblIdx = MagicNum % DAF_TBL_QLEN;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].frm_mun          = MagicNum;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].af_mode          = m_eCurAFMode;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].af_roi_sel       = m_sAFOutput->i4ROISel;
        // Contrast AF output info
        m_sDAF_TBL.daf_vec[CurDafTblIdx].af_dac_pos       = m_sAFOutput->i4AFPos;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].thermal_lens_pos = m_sAFOutput->i4ThermalLensPos;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].posture_dac      = m_sAFOutput->i4PostureDac;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].is_af_stable     = ((m_i4IsAFSearch_CurState == AF_SEARCH_DONE) && m_sCurLensInfo.bIsMotorOpen) ? 1 : 0;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].is_scene_stable  = !m_sAFOutput->i4IsSceneChange;

        if (m_i4IsAFSearch_CurState != AF_SEARCH_DONE)
        {
            if (m_sArea_TypeSel == AF_ROI_SEL_AP)
            {
                m_sArea_Bokeh = m_sArea_Focusing;
            }
            else if (m_sArea_TypeSel == AF_ROI_SEL_FD)
            {
                // use the current face roi to avoid the finally inconsistency between depth point and focusing roi
                m_sArea_Bokeh = m_sArea_OTFD;
            }
            else
            {
                if (m_i4IsAFSearch_CurState == AF_SEARCH_CONTRAST)
                {
                    m_sArea_Bokeh = m_sArea_Focusing;
                }
                else
                {
                    m_sArea_Bokeh = m_sAFOutput->sPDWin;
                }
            }
        }

        if ((m_sArea_Bokeh.i4W == 0) || (m_sArea_Bokeh.i4H == 0))
        {
            m_sArea_Bokeh = m_sArea_Center;
        }

        m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_start_x = (MUINT16)m_sArea_Bokeh.i4X;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_start_y = (MUINT16)m_sArea_Bokeh.i4Y;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_end_x   = (MUINT16)m_sArea_Bokeh.i4X + m_sArea_Bokeh.i4W;
        m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_end_y   = (MUINT16)m_sArea_Bokeh.i4Y + m_sArea_Bokeh.i4H;

        CAM_LOGD_IF(LEVEL_FLOW, "[%s] #%d X1(%d) X2(%d) Y1(%d) Y2(%d), isAFStable(%d), DAC(%d)", __FUNCTION__, CurDafTblIdx,
                    m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_start_x, m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_end_x,
                    m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_start_y, m_sDAF_TBL.daf_vec[CurDafTblIdx].af_win_end_y,
                    m_sDAF_TBL.daf_vec[CurDafTblIdx].is_af_stable, m_sDAF_TBL.daf_vec[CurDafTblIdx].af_dac_pos);
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::initSP()
{
    // --- Create SensorProvider Object ---
    AAA_TRACE_D("SensorProvider");
    if ( mpSensorProvider == NULL)
    {
        mpSensorProvider = SensorProvider::createInstance(LOG_TAG);

        if ( mpSensorProvider != nullptr)
        {
            MUINT32 interval = 30;

            interval = property_get_int32("vendor.debug.af_mgr.gyrointerval", SENSOR_GYRO_POLLING_MS);
            if ( mpSensorProvider->enableSensor(SENSOR_TYPE_GYRO, interval))
            {
                m_bGryoVd = MTRUE;
                CAM_LOGD("Dev(%d):enable SensorProvider success for Gyro ", m_i4CurrSensorDev);
            }
            else
            {
                m_bGryoVd = MFALSE;
                CAM_LOGE("Enable SensorProvider fail for Gyro");
            }

            interval = property_get_int32("vendor.debug.af_mgr.gyrointerval", SENSOR_ACCE_POLLING_MS);
            if ( mpSensorProvider->enableSensor(SENSOR_TYPE_ACCELERATION, interval))
            {
                m_bAcceVd = MTRUE;
                CAM_LOGD("Dev(%d):enable SensorProvider success for Acce ", m_i4CurrSensorDev);
            }
            else
            {
                m_bAcceVd = MFALSE;
                CAM_LOGE("Enable SensorProvider fail for Acce");
            }

        }
    }
    AAA_TRACE_END_D;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::unintSP()
{
    if ( mpSensorProvider != NULL)
    {
        mpSensorProvider->disableSensor(SENSOR_TYPE_GYRO);
        mpSensorProvider->disableSensor(SENSOR_TYPE_ACCELERATION);
        mpSensorProvider = NULL;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::prepareSPData()
{
    char  dbgMsgBuf[DBG_MSG_BUF_SZ];
    char* ptrMsgBuf = dbgMsgBuf;

    //-------------------------------------------------------------------------------------------------------------
    if (m_ptrNVRam->rAFNVRAM.i4Common[18] > 0)
    {
        // get gyro/acceleration data
        SensorData gyroDa;
        MBOOL gyroDaVd = m_bGryoVd ? mpSensorProvider->getLatestSensorData( SENSOR_TYPE_GYRO, gyroDa) : MFALSE;
        if ( gyroDaVd && gyroDa.timestamp)
        {
            m_u8PreGyroTS = m_u8GyroTS;
            m_i4GyroInfo[0] = gyroDa.gyro[0] * SENSOR_GYRO_SCALE;
            m_i4GyroInfo[1] = gyroDa.gyro[1] * SENSOR_GYRO_SCALE;
            m_i4GyroInfo[2] = gyroDa.gyro[2] * SENSOR_GYRO_SCALE;
            m_u8GyroTS = gyroDa.timestamp;

            m_u4GyroScale = m_u8GyroTS != m_u8PreGyroTS ? SENSOR_GYRO_SCALE : 0;
        }
        else
        {
            CAM_LOGD_IF( LEVEL_VERBOSE, "Gyro InValid!");
            m_u4GyroScale = 0; // set scale 0 means invalid to algo
        }

        SensorData acceDa;
        MBOOL acceDaVd = m_bAcceVd ? mpSensorProvider->getLatestSensorData( SENSOR_TYPE_ACCELERATION, acceDa) : MFALSE;
        if ( acceDaVd && acceDa.timestamp)
        {
            m_u8PreAcceTS = m_u8AcceTS;
            m_i4AcceInfo[0] = acceDa.acceleration[0] * SENSOR_ACCE_SCALE;
            m_i4AcceInfo[1] = acceDa.acceleration[1] * SENSOR_ACCE_SCALE;
            m_i4AcceInfo[2] = acceDa.acceleration[2] * SENSOR_ACCE_SCALE;
            m_u8AcceTS = acceDa.timestamp;

            m_u4ACCEScale = m_u8AcceTS != m_u8PreAcceTS ? SENSOR_ACCE_SCALE : 0;
        }
        else
        {
            CAM_LOGD_IF( LEVEL_VERBOSE, "Acce InValid!");
            m_u4ACCEScale = 0; // set scale 0 means invalid to algo
        }
    }
    else
    {
        m_u4GyroScale = 0;
        m_u4ACCEScale = 0;
    }

    if ( LEVEL_FLOW)
    {
        ptrMsgBuf += sprintf( ptrMsgBuf,
                              "Acce preTS(%11lu) TS(%11lu) Info(%4d %4d %4d)/Gyro preTS(%11lu) TS(%11lu) Info(%4d %4d %4d)/",
                              m_u8PreAcceTS,
                              m_u8AcceTS,
                              m_i4AcceInfo[0],
                              m_i4AcceInfo[1],
                              m_i4AcceInfo[2],
                              m_u8PreGyroTS,
                              m_u8GyroTS,
                              m_i4GyroInfo[0],
                              m_i4GyroInfo[1],
                              m_i4GyroInfo[2]);
    }

    MINT32 SqrGyroX = m_i4GyroInfo[0] * m_i4GyroInfo[0];
    MINT32 SqrGyroY = m_i4GyroInfo[1] * m_i4GyroInfo[1];
    MINT32 SqrGyroZ = m_i4GyroInfo[2] * m_i4GyroInfo[2];
    m_i4GyroValue = (MINT32)(sqrt((double)(SqrGyroX + SqrGyroY + SqrGyroZ)));
    if (m_gyroValueQueue.size() >= 10)
    {
        m_gyroValueQueue.pop();
    }
    m_gyroValueQueue.push(m_i4GyroValue);

    if ( LEVEL_FLOW)
    {
        ptrMsgBuf += sprintf( ptrMsgBuf,
                              " Gyro VectorValue(%4d)",
                              m_i4GyroValue);
    }

    CAM_LOGD_IF( LEVEL_FLOW, "%s %d", dbgMsgBuf, (MINT32)(ptrMsgBuf - dbgMsgBuf));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID* AfMgr::getAFTable()
{
    MVOID* ptr = NULL;
    if (m_ptrNVRam)
    {
        if (m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum > 0)
        {
            ptr = (MVOID*)m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4Pos;
        }
        else //AF table method 2
        {
            MINT32 i4TBLStar = m_sDAF_TBL.af_dac_min;
            MINT32 i4TBLEnd = m_sDAF_TBL.af_dac_max;
            MINT32 i4TBLStep = m_ptrNVRam->rAFNVRAM.sAF_Coef.sTABLE.i4Pos[21];
            for (MINT32 i = 0; i < AF_TABLE_NUM; i++) m_i4AFTable[i] = 0;

            if ((i4TBLStar < 0 || i4TBLStar > 960) || (i4TBLEnd < i4TBLStar || i4TBLEnd > 1023) || (i4TBLStep < 1 || i4TBLStep > 256))
                ptr = (MVOID*)m_i4AFTable;

            MINT32 i4TBLNum = (i4TBLEnd - i4TBLStar) / i4TBLStep + 1;
            if (i4TBLNum < 3 || i4TBLNum > AF_TABLE_NUM)
                ptr = (MVOID*)m_i4AFTable;

            for (MINT32 i = 0; i < i4TBLNum; i++)
            {
                m_i4AFTable[i] = i4TBLStar + i4TBLStep * i;
                if ( m_i4AFTable[i] > 1023)   m_i4AFTable[i] = 1023;
            }
            ptr = (MVOID*)m_i4AFTable;
        }
    }
    return ptr;
}

MVOID AfMgr::updateFscFrmInfo()
{
    fscInfo.SttNum = m_i4IsCCUAF ? m_u4ReqMagicNum - 2 : m_u4StaMagicNum;
    fscInfo.SetCount = 0;
    MINT32 needUpdateFSCInfo = 0;
    needUpdateFSCInfo |= ((DACList.size() > 1) && (DACList[0] != DACList[1]));
    needUpdateFSCInfo |= ((DACList.size() > 2) && (DACList[1] != DACList[2]));
    if (needUpdateFSCInfo)
    {
        MINT64 TimeROIBottomReadOut = ((MINT64)m_i4ReadOutTimePerLine * (MINT64)(m_sArea_HW.i4Y + m_sArea_HW.i4H)) / 1000; // us
        MINT64 TS_AFROIBottom = m_sAFInput->TS_SOF + TimeROIBottomReadOut;
        MINT32 A = m_sAFInput->i4ShutterValue;   // unit : us
        MINT32 B = (m_sArea_HW.i4Y + m_sArea_HW.i4H) * m_i4ReadOutTimePerLine / 1000; // unit : us
        MINT32 C = m_i4P1TimeReadOut - B;       // unit : us
        MINT32 D = TS_AFROIBottom - m_sAFInput->TS_MLStart;
        fscInfo.SetCount = ((D > 0) && (D <= A + B) && (DACList.size() > 2)) ? 2 : 1;

        // To prevent from updating only 1 set with the same DAC.
        if ((fscInfo.SetCount == 1) && (DACList[0] == DACList[1]))
        {
            fscInfo.SetCount = 0;
        }

        for (int i = 0; i < fscInfo.SetCount; i++)
        {
            fscInfo.DACInfo[i].DAC_To    = DACList[i + 0];
            fscInfo.DACInfo[i].DAC_From  = DACList[i + 1];
            fscInfo.DACInfo[i].Percent   = ((A + B - (D * i)) * 100) / (A + B + C);
        }
        if (LEVEL_FLOW)
        {
            CAM_LOGD("[%s] EAFMgrCtrl_GetFSCInfo FSC (A,B,C,D) = (%d, %d, %d, %d)",
                     __FUNCTION__,
                     A, B, C, D);
            CAM_LOGD("[%s] EAFMgrCtrl_GetFSCInfo SttNum(%d) setCount(%d)",
                     __FUNCTION__,
                     fscInfo.SttNum,
                     fscInfo.SetCount);
            for (int i = 0; i < fscInfo.SetCount; i++)
            {
                CAM_LOGD("[%s] EAFMgrCtrl_GetFSCInfo Set %d : DAC (%d)->(%d), Percent (%d)",
                         __FUNCTION__,
                         i,
                         fscInfo.DACInfo[i].DAC_From,
                         fscInfo.DACInfo[i].DAC_To,
                         fscInfo.DACInfo[i].Percent);
            }
        }
    }
    else if ( DACList.size() == 1 )
    {
        fscInfo.SetCount = 1;
        fscInfo.DACInfo[0].DAC_To    = DACList[0];
        fscInfo.DACInfo[0].DAC_From  = 0;
        fscInfo.DACInfo[0].Percent   = 0;
    }
}

MINT32 AfMgr::updateRTVString(char* stringBuffer)
{
    MINT32 count = 0;
    MINT32 totalCount = 0;
    if (!stringBuffer)
    {
        return totalCount;
    }
    else
    {
        MINT32 elementTable[E_AF_INDEX_MAX] = {0}; //0~8, 9items
        for (int i = 0; i < RTVMAXCOUNT; i++)
        {
            if (elementTable[m_rtvId[i]] == 0)
            {
                elementTable[m_rtvId[i]] = 1;
                count++;    // number of different rtv info to be show.
            }
        }
        if (count > 0)
        {
            char tmpString[40] = {"\0"};
            MINT32 length = sprintf(tmpString, "AF[%d]ReqNum[%d]:", m_i4CurrSensorDev, m_u4ReqMagicNum);
            strncpy(stringBuffer, tmpString, length);
            for (int i = 1; i < E_AF_INDEX_MAX; i++)
            {
                if (elementTable[i] != 0)
                {
                    MINT32 tmpEnum = i;
                    MINT32 tmpValue;

                    memset(tmpString, 0, sizeof(char) * 40);
                    switch (i)
                    {
                    case E_RTV_AF_CTRL_MODE:
                        tmpValue = m_eCurAFMode;
                        break;
                    case E_RTV_AF_CTRL_STATE:
                        tmpValue = m_eAFState;
                        break;
                    case E_RTV_AF_ROI_TYPE:
                        tmpValue = m_sAFOutput->i4ROISel;
                        break;
                    case E_RTV_AF_SEARCH_STATE:
                        tmpValue = m_sAFOutput->i4IsAfSearch;
                        break;
                    case E_RTV_AF_SEARCH_TYPE:
                        tmpValue = 0; // TODO
                        break;
                    case E_RTV_AF_ALGO_MODE:
                        tmpValue = m_eLIB3A_AFMode;
                        break;
                    case E_RTV_AF_ALGO_STATE:
                        tmpValue = 0; // TODO
                        break;
                    case E_RTV_AF_LENS_POS:
                        tmpValue = m_sAFOutput->i4AFPos;
                        break;
                    case E_RTV_AF_RSV_1:
                    case E_RTV_AF_RSV_2:
                    case E_RTV_AF_RSV_3:
                    case E_RTV_AF_RSV_4:
                    case E_RTV_AF_RSV_5:
                    case E_RTV_AF_RSV_6:
                    case E_RTV_AF_RSV_7:
                    case E_RTV_AF_RSV_8:
                        tmpValue = m_sAFOutput->i4DbgRtfData[E_RTV_AF_RSV_8 - E_RTV_AF_RSV_1];
                        break;
                    default:
                        tmpEnum = 0;
                        tmpValue = 0;
                        break;
                    }
                    if (tmpEnum != 0)
                    {
                        MINT32 n = sprintf(tmpString, "%d,%d;", tmpEnum, tmpValue);
                        strncat(stringBuffer, tmpString, n);
                        if (count-- == 0)
                        {
                            break;
                        }
                    }
                }
            }
            totalCount = strlen(stringBuffer);
            CAM_LOGD_IF(LEVEL_FLOW, "%s total(%d): %s", __FUNCTION__, totalCount, stringBuffer);
        }
        else
        {
            CAM_LOGD_IF(LEVEL_FLOW, "%s : No RTV info need to show", __FUNCTION__);
        }
    }
    return totalCount;
}

