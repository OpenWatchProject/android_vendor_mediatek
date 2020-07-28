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

#include <mtkcam/utils/std/Log.h>
//#include <isp_tuning_mgr.h>
#include <mtkcam/def/common.h>
#include <af_param.h>
#include <nvbuf_util.h>
#include "af_mgr.h"

using namespace NS3Av3;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::SyncAFReadDatabase()
{
#if 0
    // AF sync data read for primary AF
    m_ptrNVRam->rDualCamNVRAM.i4BufferState |= 5;
#endif
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::SyncAFWriteDatabase()
{
#if 0
    // AF sync data restore for primary AF
    DUALCAM_NVRAM_T tempbuf = afc_getPosRecBuf();

    if ((tempbuf.i4PosData[0] != 0xFFFF) && (tempbuf.i4PosData[1] != 0xFFFF))
    {
        MINT32 i;
        NVRAM_LENS_DATA_PARA_STRUCT *ptrNVRAMBuf;
        NVRAM_LENS_PARA_STRUCT *ptrLensNVRAMBuf;
        if (m_pMcuDrv)
        {
            m_pMcuDrv->setLensNvramIdx();
        }
        MINT32 err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_LENS, m_i4CurrSensorDev, (void*&)ptrLensNVRAMBuf);
        // this part SHOULD be modified ASAP
        if (m_eCamScenarioMode >= AF_CAM_SCENARIO_NUM_2)
        {
            CAM_LOGE( "AF-%-15s: m_eCamScenarioMode > AF_CAM_SCENARIO_NUM!", __FUNCTION__);
            return;
        }
        ptrNVRAMBuf = &ptrLensNVRAMBuf->AF[m_eCamScenarioMode];
        if ( err != 0)
        {
            CAM_LOGE( "AF-%-15s: AfAlgo NvBufUtil get buf fail!", __FUNCTION__);
            return;
        }

        ptrNVRAMBuf->rDualCamNVRAM.i4BufferState = tempbuf.i4BufferState;
        ptrNVRAMBuf->rDualCamNVRAM.i4PosStatCnt  = tempbuf.i4PosStatCnt;
        CAM_LOGD("SmallAF Save %d %d %d 0x%x", tempbuf.i4PosData[0], tempbuf.i4PosData[1], tempbuf.i4PosStatCnt, tempbuf.i4BufferState);
        for (i = 0; i < 1024; i++)
        {
            ptrNVRAMBuf->rDualCamNVRAM.i4PosData[i] = tempbuf.i4PosData[i];
            if (tempbuf.i4PosData[i])
            {
                CAM_LOGD_IF( LEVEL_FLOW, "SmallAF SavePosRec %d[%d %d]", i, (tempbuf.i4PosData[i] >> 16) , (tempbuf.i4PosData[i] & 0xFFFF) );
            }
        }
        NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_LENS, m_i4CurrSensorDev);
    }
#endif
}
MVOID AfMgr::SyncAFGetSensorInfoForCCU(MINT32& slaveDevCCU, MINT32& slaveIdx)
{
    CAM_LOGD_IF(LEVEL_FLOW, "%s", __FUNCTION__);
    if (s_pIAfCxU )
    {
        if (s_pIAfCxU->isCCUAF())
        {
            s_pIAfCxU->SyncAF_getSensorInfoForCCU(slaveDevCCU, slaveIdx);
        }
        else
        {
            CAM_LOGW("%s SHOULD NOT execute by CPUAF", __FUNCTION__);
        }
    }
}
MVOID AfMgr::SyncAFProcess(MINT32 slaveDevCCU, MINT32 slaveIdxCCU, MINT32 sync2AMode, AF_SyncInfo_T syncInfo)
{
    CAM_LOGD_IF(LEVEL_FLOW, "%s", __FUNCTION__);
    if (s_pIAfCxU )
    {
        if (s_pIAfCxU->isCCUAF())
        {
            s_pIAfCxU->SyncAF_process(slaveDevCCU, slaveIdxCCU, sync2AMode, syncInfo);
        }
        else
        {
            CAM_LOGW("%s SHOULD NOT execute by CPUAF", __FUNCTION__);
        }
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::SyncAFSetMode( MINT32 a_i4SyncMode)
{
    CAM_LOGD_IF(LEVEL_FLOW, "%s", __FUNCTION__);
    if (s_pIAfCxU)
    {
        s_pIAfCxU->SyncAF_setSyncMode(a_i4SyncMode);
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::SyncAFGetMotorRange(AF_SyncInfo_T& sCamInfo)
{
    CAM_LOGD_IF(LEVEL_FLOW, "%s", __FUNCTION__);
    if (s_pIAfCxU)
    {
        s_pIAfCxU->SyncAF_motorRange(sCamInfo);
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::SyncAFGetInfo(AF_SyncInfo_T& sCamInfo)
{
    CAM_LOGD_IF(LEVEL_FLOW, "%s", __FUNCTION__);
    MINT32 ret = 0;
    if (s_pIAfCxU)
    {
        ret = s_pIAfCxU->SyncAF_getSyncInfo(sCamInfo);
    }
    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::SyncAFSetInfo( MINT32 a_i4Pos, AF_SyncInfo_T& sSlaveHisCamInfo)
{
    CAM_LOGD_IF(LEVEL_FLOW, "%s", __FUNCTION__);
    if (s_pIAfCxU)
    {
        s_pIAfCxU->SyncAF_setSyncInfo(a_i4Pos, sSlaveHisCamInfo);
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::SyncAFGetCalibPos(AF_SyncInfo_T& sCamInfo)
{
    CAM_LOGD_IF(LEVEL_FLOW, "%s", __FUNCTION__);
    if (s_pIAfCxU)
    {
        s_pIAfCxU->SyncAF_getMain2CalibPos(sCamInfo);
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::SyncAFCalibPos(AF_SyncInfo_T& sCamInfo)
{
    CAM_LOGD_IF(LEVEL_FLOW, "%s", __FUNCTION__);
    if (s_pIAfCxU)
    {
        s_pIAfCxU->SyncAF_calibPos(sCamInfo);
    }
}

// [SyncAF] Master/Slave : set mf mode
MVOID AfMgr::SyncSetOffMode()
{
    CAM_LOGD_IF(LEVEL_FLOW, "%s", __FUNCTION__);
    setAFMode(MTK_CONTROL_AF_MODE_OFF, AF_SYNC_CALLER);
}
// [SyncAF] Master : setMFPos to move lens via mf control
MVOID AfMgr::SyncSetMFPos(MINT32 targetPos)
{
    CAM_LOGD_IF(LEVEL_FLOW, "%s", __FUNCTION__);
    setMFPos(targetPos, AF_SYNC_CALLER);
}
// [SyncAF] Slave : move lens directly
MVOID AfMgr::SyncMoveLens(MINT32 targetPos)
{
    CAM_LOGD_IF(LEVEL_FLOW, "%s", __FUNCTION__);
    if (s_pIAfCxU)
    {
        s_pIAfCxU->SyncAF_moveLensSyncAF(targetPos);
    }
}


