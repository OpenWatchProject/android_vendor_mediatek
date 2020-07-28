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

#define LOG_TAG "sync3a"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <mtkcam/utils/std/Log.h>
#include <aaa_types.h>
#include <ISync3A.h>
#include <aaa_scheduling_custom.h>

#include <libsync3a/MTKSyncAe.h>
#include <libsync3a/MTKSyncAwb.h>

#include "Local.h"
#include <ae_mgr/ae_mgr_if.h>
#include <awb_mgr/awb_mgr_if.h>
#include <isp_mgr/isp_mgr.h>
#include <sensor_mgr/aaa_sensor_mgr.h>

#include <nvbuf_util.h>
#include <cutils/properties.h>

#include "n3d_sync2a_tuning_param.h"
#include <camera_custom_imgsensor_cfg.h>
#include <mtkcam/drv/IHalSensor.h>
#include <isp_tuning.h>

#include <debug_exif/ver1/dbg_id_param.h>        // for DEBUG_XXX_KEY_ID
#include <debug_exif/cam/dbg_cam_param.h>
#include <debug_exif/cam/dbg_cam_n3d_param.h>

#include <string>
#include <math.h>

#include <debug_exif/aaa/dbg_aaa_param.h>
#include <debug_exif/aaa/dbg_af_param.h>
#include <camera_custom_nvram.h>
#include <aaa/af_param.h>
#include <af_feature.h>
#include <af_algo_if.h>
#include <af_mgr_if.h>
#include <StatisticBuf.h>

using namespace NS3Av3;
using namespace android;
using namespace NSIspTuningv3;

#define GET_PROP(prop, init, val)\
{\
    char value[PROPERTY_VALUE_MAX] = {'\0'};\
    property_get(prop, value, (init));\
    (val) = atoi(value);\
}

#define MIN(a,b)  ((a) < (b) ? (a) : (b))

static NSCam::MRect
get2AStatROI(
    float main1_FOV_vertical,
    float main1_FOV_horizontal,
    float main2_FOV_vertical,
    float main2_FOV_horizontal,
    int tg_size_w, int tg_size_h)
{
    NSCam::MRect CropResult;

    int width, height;
    int offset_x, offset_y;

    width  = (int)( ::tan(main1_FOV_horizontal/180.0f*(float)::acos(0.0f)) / ::tan(main2_FOV_horizontal/180.0f*(float)::acos(0.0f)) * tg_size_w/2 )*2 ;
    height = (int)( ::tan(main1_FOV_vertical/180.0f*(float)::acos(0.0f)) / ::tan(main2_FOV_vertical/180.0f*(float)::acos(0.0f)) * tg_size_h/2 )*2 ;
    offset_x = ((( tg_size_w - width )>>2 )<<1) ;
    offset_y = ((( tg_size_h - height)>>2 )<<1) ;

    CAM_LOGD("[%s] FOV1_V(%3.3f), FOV1_H(%3.3f), FOV2_V(%3.3f), FOV2_H(%3.3f), tg(%d, %d)", __FUNCTION__,
        main1_FOV_vertical, main1_FOV_horizontal, main2_FOV_vertical, main2_FOV_horizontal, tg_size_w, tg_size_h);

    CropResult.p.x = offset_x;
    CropResult.p.y = offset_y;

    CropResult.s.w = width;
    CropResult.s.h = height;

    return CropResult;
}

/******************************************************************************
 *  Sync3AWrapper
 ******************************************************************************/
class Sync3AWrapper
{
public:
    struct ManualAe
    {
        MUINT32 u4ExpTime;  // in micro second
        MUINT32 u4AfeGain;  // 1024 = 1x
        MUINT32 u4IspGain;  // 1024 = 1x
        MBOOL   fgOnOff;
        ManualAe()
            : u4ExpTime(10000000)
            , u4AfeGain(1024)
            , u4IspGain(1024)
            , fgOnOff(MFALSE)
        {}
    };

    static Sync3AWrapper*       getInstance();

    virtual MBOOL               init(MINT32 i4Master, MINT32 i4Slave);

    virtual MBOOL               uninit();

    virtual MBOOL               syncCalc(MINT32 i4Opt,const StatisticBufInfo* pMasterSttBuf, const StatisticBufInfo* pSlaveSttBuf);

    virtual MBOOL               setDebugInfo(void* prDbgInfo) const;

    virtual MBOOL               isAeStable() const {return (m_fgMasterAeStable && m_fgSlaveAeStable);}

protected:
    Sync3AWrapper();
    virtual ~Sync3AWrapper(){}

    virtual MBOOL               update(MINT32 i4Opt);
    SYNCAE_CAMERA_TYPE_ENUM     querySensorType(MINT32 i4SensorDev);
    MBOOL                       syncAeInit();
    MBOOL                       syncAwbInit();

    MBOOL                       syncAeMain(const StatisticBufInfo* pMasterSttBuf, const StatisticBufInfo* pSlaveSttBuf);
    MBOOL                       syncAwbMain();

    mutable Mutex               m_Lock;
    MINT32                      m_i4User;

    MINT32                      m_i4Master;
    MINT32                      m_i4Slave;

    MBOOL                       m_fgMasterAeStable;
    MBOOL                       m_fgSlaveAeStable;

    ManualAe                    m_rManualAeMaster;
    ManualAe                    m_rManualAeSlave;

    MTKSyncAe*                  m_pMTKSyncAe;
    MTKSyncAwb*                 m_pMTKSyncAwb;

    SYNC_AWB_INPUT_INFO_STRUCT  m_rAwbSyncInput;
    SYNC_AWB_OUTPUT_INFO_STRUCT m_rAwbSyncOutput;
    SYNC_AE_OUTPUT_STRUCT       m_rAeSyncOutput;
};

/******************************************************************************
 *  Sync3AWrapper Implementation
 ******************************************************************************/
Sync3AWrapper::
Sync3AWrapper()
    : m_Lock()
    , m_i4User(0)
{}

Sync3AWrapper*
Sync3AWrapper::getInstance()
{
    static Sync3AWrapper _rWrap;
    return &_rWrap;
}

MBOOL
Sync3AWrapper::
init(MINT32 i4Master, MINT32 i4Slave)
{
    Mutex::Autolock lock(m_Lock);

    if (m_i4User > 0)
    {

    }
    else
    {
        CAM_LOGD("[%s] User(%d), eSensor(%d,%d)", __FUNCTION__, m_i4User, i4Master, i4Slave);
        m_i4Master = i4Master;
        m_i4Slave = i4Slave;
        m_fgMasterAeStable = MFALSE;
        m_fgSlaveAeStable = MFALSE;
        syncAeInit();
        syncAwbInit();
    }

    m_i4User++;

    return MTRUE;
}

MBOOL
Sync3AWrapper::
uninit()
{
    Mutex::Autolock lock(m_Lock);

    if (m_i4User > 0)
    {
        // More than one user, so decrease one User.
        m_i4User--;

        if (m_i4User == 0) // There is no more User after decrease one User
        {
            CAM_LOGD("[%s]", __FUNCTION__);
            if (m_pMTKSyncAe)
            {
                m_pMTKSyncAe->destroyInstance();
                m_pMTKSyncAe = NULL;
            }
            if (m_pMTKSyncAwb)
            {
                m_pMTKSyncAwb->destroyInstance();
                m_pMTKSyncAwb = NULL;
            }
        }
        else  // There are still some users.
        {
            //CAM_LOGD(m_bDebugEnable,"Still %d users \n", mi4User);
        }
    }

    return MTRUE;
}

MBOOL
Sync3AWrapper::
syncCalc(MINT32 i4Opt, const StatisticBufInfo* pMasterSttBuf, const StatisticBufInfo* pSlaveSttBuf)
{
    if(pMasterSttBuf == NULL || pSlaveSttBuf == NULL)
    {
        CAM_LOGE("pMasterSttBuf = NULL || pSlaveSttBuf = NULL");
        return MFALSE;
    }
    if (i4Opt & (ISync3A::E_SYNC3A_DO_AE|ISync3A::E_SYNC3A_DO_AE_PRECAP))
    {
        if (!syncAeMain(pMasterSttBuf, pSlaveSttBuf))
            CAM_LOGE("syncAeMain fail");
    }

    if (i4Opt & ISync3A::E_SYNC3A_DO_AWB)
    {
        if (!syncAwbMain())
            CAM_LOGE("syncAwbMain fail");
    }

    return update(i4Opt);
}


MBOOL
Sync3AWrapper::
syncAeInit()
{
    CAM_LOGD("[%s] +", __FUNCTION__);

    m_pMTKSyncAe = MTKSyncAe::createInstance();
    if (!m_pMTKSyncAe)
    {
        CAM_LOGE("MTKSyncAe::createInstance() fail");
        return MFALSE;
    }

    SYNC_AE_INIT_STRUCT rAEInitData;
    ::memset(&rAEInitData, 0x0, sizeof(SYNC_AE_INIT_STRUCT));  //  initialize, Set all to 0
    rAEInitData.SyncScenario = SYNC_AE_SCENARIO_N3D;
    rAEInitData.SyncNum = 2; //N3D 2 cam

    AWB_Calibration_Data AWBCalData = {512, 512, 512};   // todo remove ??

    const strSyncAEInitInfo* pSyncAEInitInfo = getSyncAEInitInfo();

    rAEInitData.SyncAeTuningParam.syncPolicy = (SYNC_AE_POLICY)pSyncAEInitInfo->SyncWhichEye;
    rAEInitData.SyncAeTuningParam.enPPGainComp = pSyncAEInitInfo->isDoGainRegression;
    rAEInitData.SyncAeTuningParam.u4FixSyncGain = pSyncAEInitInfo->FixSyncGain;

    rAEInitData.main_param.syncAECalData.Golden = AWBCalData;  // todo
    rAEInitData.main_param.syncAECalData.Unit = AWBCalData;    // todo
    rAEInitData.sub_param.syncAECalData.Golden = AWBCalData;   // todo
    rAEInitData.sub_param.syncAECalData.Unit = AWBCalData;     // todo
    rAEInitData.main_param.syncAECamType = querySensorType(m_i4Master);
    rAEInitData.sub_param.syncAECamType = querySensorType(m_i4Slave);

    ::memcpy(rAEInitData.main_param.BVOffset, pSyncAEInitInfo->EVOffset_main, sizeof(MUINT32)*2);
    ::memcpy(rAEInitData.sub_param.BVOffset, pSyncAEInitInfo->EVOffset_main2, sizeof(MUINT32)*2);
    ::memcpy(rAEInitData.main_param.RGB2YCoef, pSyncAEInitInfo->RGB2YCoef_main, sizeof(MUINT32)*3);
    ::memcpy(rAEInitData.sub_param.RGB2YCoef, pSyncAEInitInfo->RGB2YCoef_main2, sizeof(MUINT32)*3);

    m_rAeSyncOutput.main_ch.u4SyncGain = 1024;
    m_rAeSyncOutput.sub_ch.u4SyncGain = 1024;
    NVRAM_CAMERA_3A_STRUCT* p3aNvram;
    NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_i4Master, (void*&)p3aNvram, 0);
    rAEInitData.main_param.pAeNvram = &p3aNvram->rAENVRAM[0];
    NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_i4Slave, (void*&)p3aNvram, 0);
    rAEInitData.sub_param.pAeNvram = &p3aNvram->rAENVRAM[0];

    m_pMTKSyncAe->SyncAeInit(reinterpret_cast<void*>(&rAEInitData));

    CAM_LOGD("[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
Sync3AWrapper::
syncAwbInit()
{
    CAM_LOGD("[%s] +", __FUNCTION__);

    m_pMTKSyncAwb = MTKSyncAwb::createInstance();
    if (!m_pMTKSyncAwb)
    {
        CAM_LOGE("MTKSyncAwb::createInstance() fail");
        return MFALSE;
    }

    SYNC_AWB_INIT_INFO_STRUCT rAWBInitData;
    ::memset(&rAWBInitData, 0x0, sizeof(SYNC_AWB_INIT_INFO_STRUCT));  //  initialize, Set all to 0

    AWB_GAIN_T defGain = {512, 512, 512};   // todo remove ??
    const strSyncAWBInitInfo* pAwbInitInfo = getSyncAWBInitInfo();

    rAWBInitData.SyncAwbTuningParam.SyncMode = pAwbInitInfo->syncAWBMethod;
    ::memcpy(rAWBInitData.SyncAwbTuningParam.GainRatioTh, pAwbInitInfo->SyncAWB_GainINT_method_Limit, sizeof(MUINT32)*4);
    ::memcpy(rAWBInitData.SyncAwbTuningParam.CCTDiffTh, pAwbInitInfo->SyncAWB_CCT_method_th, sizeof(MUINT32)*5);
    rAWBInitData.SyncAwbTuningParam.PP_method_valid_block_num_ratio = pAwbInitInfo->SyncAWB_PP_method_valid_block_num_ratio;
    rAWBInitData.SyncAwbTuningParam.PP_method_Y_threshold = pAwbInitInfo->SyncAWB_PP_method_Y_Threshold;
    ::memcpy(rAWBInitData.SyncAwbTuningParam.BlendingTh, pAwbInitInfo->SyncAWB_Blending_Th, sizeof(MUINT32)*4);

    rAWBInitData.SyncScenario = SYNC_AWB_SCENARIO_N3D;

    IAwbMgr::getInstance().CCTOPAWBGetNVRAMParam(m_i4Master, AWB_NVRAM_IDX_NORMAL, &rAWBInitData.main_param);
    IAwbMgr::getInstance().CCTOPAWBGetNVRAMParam(m_i4Slave, AWB_NVRAM_IDX_NORMAL, &rAWBInitData.sub_param);

    m_pMTKSyncAwb->SyncAwbInit(reinterpret_cast<void*>(&rAWBInitData));

    CAM_LOGD("[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
Sync3AWrapper::
syncAeMain(const StatisticBufInfo* pMasterSttBuf, const StatisticBufInfo* pSlaveSttBuf)
{
    CAM_LOGD("[%s] +", __FUNCTION__);

    AWB_GAIN_T defGain = {512, 512, 512};   // todo remove ??

    SYNC_AE_INPUT_STRUCT rAeSyncInput;
    // rAeSyncInput.eSyncAEMode = SYNC_AE_PLINES_CONST_W;
    rAeSyncInput.eSyncAEMode = SYNC_AE_EXPTIME_CONST_W;
    // CAM_LOGD("[%s] rAeSyncInput.eSyncAEMode = SYNC_AE_EXPTIME_CONST_W", __FUNCTION__);

    rAeSyncInput.SyncCamScenario = SYNC_AE_CAM_SCENARIO_PREVIEW;
    AE_MODE_CFG_T _a_rAEOutput_main, _a_rAEOutput_main2;
    strAETable strPreviewAEPlineTableMain, strCaptureAEPlineTabMain;
    strAETable strPreviewAEPlineTableMain2, strCaptureAEPlineTabMain2;
    strAETable strStrobeAEPlineTableMain, strStrobeAEPlineTableMain2;
    strAFPlineInfo strStrobeAFPlineTabMain, strStrobeAFPlineTabMain2;
    LCEInfo_T rLCEInfoMain, rLCEInfoMain2;
    MUINT32 u4AEConditionMain, u4AEConditionMain2;

    AWB_OUTPUT_T rAwbMaster, rAwbSlave;

    IAwbMgr::getInstance().getAWBOutput(m_i4Master, rAwbMaster);
    IAwbMgr::getInstance().getAWBOutput(m_i4Slave, rAwbSlave);


    IAeMgr::getInstance().getPreviewParams(m_i4Master, _a_rAEOutput_main);
    IAeMgr::getInstance().getPreviewParams(m_i4Slave, _a_rAEOutput_main2);
    IAeMgr::getInstance().getCurrentPlineTable(m_i4Master, strPreviewAEPlineTableMain, strCaptureAEPlineTabMain, strStrobeAEPlineTableMain, strStrobeAFPlineTabMain);
    IAeMgr::getInstance().getCurrentPlineTable(m_i4Slave, strPreviewAEPlineTableMain2, strCaptureAEPlineTabMain2, strStrobeAEPlineTableMain2, strStrobeAFPlineTabMain2);
    IAeMgr::getInstance().getLCEPlineInfo(m_i4Master, rLCEInfoMain);
    IAeMgr::getInstance().getLCEPlineInfo(m_i4Slave, rLCEInfoMain2);
    m_fgMasterAeStable = IAeMgr::getInstance().IsAEStable(m_i4Master);
    m_fgSlaveAeStable  = IAeMgr::getInstance().IsAEStable(m_i4Slave);
    u4AEConditionMain = ((IAeMgr::getInstance().getAECondition(m_i4Master, AE_CONDITION_BACKLIGHT)) == MTRUE ? AE_CONDITION_BACKLIGHT : 0 ) +
                        ((IAeMgr::getInstance().getAECondition(m_i4Master, AE_CONDITION_OVEREXPOSURE)) == MTRUE ? AE_CONDITION_OVEREXPOSURE : 0) +
                        ((IAeMgr::getInstance().getAECondition(m_i4Master, AE_CONDITION_HIST_STRETCH)) == MTRUE ? AE_CONDITION_HIST_STRETCH : 0) +
                        ((IAeMgr::getInstance().getAECondition(m_i4Master, AE_CONDITION_SATURATION_CHECK)) == MTRUE ? AE_CONDITION_SATURATION_CHECK : 0) +
                        ((IAeMgr::getInstance().getAECondition(m_i4Master, AE_CONDITION_FACEAE)) == MTRUE ? AE_CONDITION_FACEAE : 0);
    u4AEConditionMain2 = ((IAeMgr::getInstance().getAECondition(m_i4Slave, AE_CONDITION_BACKLIGHT)) == MTRUE ? AE_CONDITION_BACKLIGHT : 0 ) +
                        ((IAeMgr::getInstance().getAECondition(m_i4Slave, AE_CONDITION_OVEREXPOSURE)) == MTRUE ? AE_CONDITION_OVEREXPOSURE : 0) +
                        ((IAeMgr::getInstance().getAECondition(m_i4Slave, AE_CONDITION_HIST_STRETCH)) == MTRUE ? AE_CONDITION_HIST_STRETCH : 0) +
                        ((IAeMgr::getInstance().getAECondition(m_i4Slave, AE_CONDITION_SATURATION_CHECK)) == MTRUE ? AE_CONDITION_SATURATION_CHECK : 0) +
                        ((IAeMgr::getInstance().getAECondition(m_i4Slave, AE_CONDITION_FACEAE)) == MTRUE ? AE_CONDITION_FACEAE : 0);


    rAeSyncInput.main_ch.Input.bAEStable = IAeMgr::getInstance().IsAEStable(m_i4Master);
    rAeSyncInput.main_ch.Input.EvSetting.u4Eposuretime = _a_rAEOutput_main.u4Eposuretime;
    rAeSyncInput.main_ch.Input.EvSetting.u4AfeGain = _a_rAEOutput_main.u4AfeGain;
    rAeSyncInput.main_ch.Input.EvSetting.u4IspGain = _a_rAEOutput_main.u4IspGain;
    rAeSyncInput.main_ch.Input.EvSetting.uIris = 0;  // Iris fix
    rAeSyncInput.main_ch.Input.EvSetting.uSensorMode = 0;  // sensor mode don't change
    rAeSyncInput.main_ch.Input.EvSetting.uFlag = 0; // No hypersis
    rAeSyncInput.main_ch.Input.Bv = IAeMgr::getInstance().getBVvalue(m_i4Master);
    rAeSyncInput.main_ch.Input.u4AECondition = u4AEConditionMain;
    rAeSyncInput.main_ch.Input.i4DeltaBV = rLCEInfoMain.i4AEidxNext - rLCEInfoMain.i4AEidxCur;
    rAeSyncInput.main_ch.Input.u4ISO = _a_rAEOutput_main.u4RealISO;
    rAeSyncInput.main_ch.Input.u2FrameRate = _a_rAEOutput_main.u2FrameRate;
    rAeSyncInput.main_ch.Input.i2FlareOffset = _a_rAEOutput_main.i2FlareOffset;
    rAeSyncInput.main_ch.Input.i2FlareGain = _a_rAEOutput_main.i2FlareGain;
    rAeSyncInput.main_ch.Input.i2FaceDiffIndex = IAeMgr::getInstance().getAEFaceDiffIndex(m_i4Master);
    rAeSyncInput.main_ch.Input.i4AEidxCurrent = rLCEInfoMain.i4AEidxCur;
    rAeSyncInput.main_ch.Input.i4AEidxNext = rLCEInfoMain.i4AEidxNext;
    rAeSyncInput.main_ch.Input.u4CWValue = _a_rAEOutput_main.u4CWValue;
    rAeSyncInput.main_ch.u4SyncGain = m_rAeSyncOutput.main_ch.u4SyncGain;
    rAeSyncInput.main_ch.u4ExposureMode = _a_rAEOutput_main.u4ExposureMode;   // exposure time
    rAeSyncInput.main_ch.pAETable = &strPreviewAEPlineTableMain;
    rAeSyncInput.main_ch.aao_buffer_info.pAEStatisticBuf = (MVOID*) pMasterSttBuf->mVa; // reinterpret_cast<MVOID *>(IAAOBufMgr::getInstance().getCurrHwBuf(m_i4Master));
    rAeSyncInput.main_ch.aao_buffer_info.buf_line_size = IAeMgr::getInstance().getAAOLineByteSize(m_i4Master);
    rAeSyncInput.main_ch.wb_gain = rAwbMaster.rAWBSyncInput_N3D.rAlgGain;

    rAeSyncInput.sub_ch.Input.bAEStable = IAeMgr::getInstance().IsAEStable(m_i4Slave);
    rAeSyncInput.sub_ch.Input.EvSetting.u4Eposuretime = _a_rAEOutput_main2.u4Eposuretime;
    rAeSyncInput.sub_ch.Input.EvSetting.u4AfeGain = _a_rAEOutput_main2.u4AfeGain;
    rAeSyncInput.sub_ch.Input.EvSetting.u4IspGain = _a_rAEOutput_main2.u4IspGain;
    rAeSyncInput.sub_ch.Input.EvSetting.uIris = 0;  // Iris fix
    rAeSyncInput.sub_ch.Input.EvSetting.uSensorMode = 0;  // sensor mode don't change
    rAeSyncInput.sub_ch.Input.EvSetting.uFlag = 0; // No hypersis
    rAeSyncInput.sub_ch.Input.Bv = IAeMgr::getInstance().getBVvalue(m_i4Slave);
    rAeSyncInput.sub_ch.Input.u4AECondition = u4AEConditionMain2;
    rAeSyncInput.sub_ch.Input.i4DeltaBV = rLCEInfoMain2.i4AEidxNext - rLCEInfoMain2.i4AEidxCur;
    rAeSyncInput.sub_ch.Input.u4ISO = _a_rAEOutput_main.u4RealISO;
    rAeSyncInput.sub_ch.Input.u2FrameRate = _a_rAEOutput_main2.u2FrameRate;
    rAeSyncInput.sub_ch.Input.i2FlareOffset = _a_rAEOutput_main2.i2FlareOffset;
    rAeSyncInput.sub_ch.Input.i2FlareGain = _a_rAEOutput_main2.i2FlareGain;
    rAeSyncInput.sub_ch.Input.i2FaceDiffIndex = IAeMgr::getInstance().getAEFaceDiffIndex(m_i4Slave);
    rAeSyncInput.sub_ch.Input.i4AEidxCurrent = rLCEInfoMain2.i4AEidxCur;
    rAeSyncInput.sub_ch.Input.i4AEidxNext = rLCEInfoMain2.i4AEidxNext;
    rAeSyncInput.sub_ch.Input.u4CWValue = _a_rAEOutput_main2.u4CWValue;
    rAeSyncInput.sub_ch.u4SyncGain = m_rAeSyncOutput.sub_ch.u4SyncGain;
    rAeSyncInput.sub_ch.u4ExposureMode = _a_rAEOutput_main2.u4ExposureMode;   // exposure time
    rAeSyncInput.sub_ch.pAETable = &strPreviewAEPlineTableMain2;
    rAeSyncInput.sub_ch.aao_buffer_info.pAEStatisticBuf = (MVOID*) pSlaveSttBuf->mVa; //reinterpret_cast<MVOID *>(IAAOBufMgr::getInstance().getCurrHwBuf(m_i4Slave));
    rAeSyncInput.sub_ch.aao_buffer_info.buf_line_size = IAeMgr::getInstance().getAAOLineByteSize(m_i4Slave);
    rAeSyncInput.sub_ch.wb_gain = rAwbSlave.rAWBSyncInput_N3D.rAlgGain;

    m_pMTKSyncAe->SyncAeMain(&rAeSyncInput, &m_rAeSyncOutput);

    CAM_LOGD("[%s] -", __FUNCTION__);

    return MTRUE;
}

MBOOL
Sync3AWrapper::
syncAwbMain()
{
    CAM_LOGD("[%s] +", __FUNCTION__);

    AWB_OUTPUT_T rAwbMaster, rAwbSlave;

    AWB_PARENT_BLK_STAT_T& rAwbParentStatMaster = rAwbMaster.rAWBSyncInput_N3D.rAwbParentStatBlk;
    AWB_PARENT_BLK_STAT_T& rAwbParentStatSlave = rAwbSlave.rAWBSyncInput_N3D.rAwbParentStatBlk;
    MINT32& iParentBlkNumX_Main = rAwbMaster.rAWBSyncInput_N3D.ParentBlkNumX;
    MINT32& iParentBlkNumY_Main = rAwbMaster.rAWBSyncInput_N3D.ParentBlkNumY;
    MINT32& iParentBlkNumX_Sub = rAwbSlave.rAWBSyncInput_N3D.ParentBlkNumX;
    MINT32& iParentBlkNumY_Sub = rAwbSlave.rAWBSyncInput_N3D.ParentBlkNumY;

    IAwbMgr::getInstance().getAWBOutput(m_i4Master, rAwbMaster);
    IAwbMgr::getInstance().getAWBOutput(m_i4Slave, rAwbSlave);

    IAwbMgr::getInstance().getAWBParentStat(m_i4Master, rAwbParentStatMaster, &iParentBlkNumX_Main, &iParentBlkNumY_Main);
    IAwbMgr::getInstance().getAWBParentStat(m_i4Slave, rAwbParentStatSlave, &iParentBlkNumX_Sub, &iParentBlkNumY_Sub);

    ::memcpy(&m_rAwbSyncInput.main_ch, &rAwbMaster.rAWBSyncInput_N3D, sizeof(AWB_SYNC_INPUT_N3D_T));
    ::memcpy(&m_rAwbSyncInput.sub_ch, &rAwbSlave.rAWBSyncInput_N3D, sizeof(AWB_SYNC_INPUT_N3D_T));

#if 0
    rAwbSyncInput.main_ch.alg_gain      = rAwbMaster.rAWBSyncInput_N3D.rAlgGain;
    rAwbSyncInput.main_ch.curr_gain     = rAwbMaster.rAWBSyncInput_N3D.rCurrentGain;
    rAwbSyncInput.main_ch.target_gain   = rAwbMaster.rAWBSyncInput_N3D.rTargetGain;
    rAwbSyncInput.main_ch.m_i4CCT       = rAwbMaster.rAWBSyncInput_N3D.i4CCT;
    rAwbSyncInput.main_ch.m_i4LightMode = rAwbMaster.rAWBSyncInput_N3D.i4LightMode;
    rAwbSyncInput.main_ch.m_i4SceneLV   = rAwbMaster.rAWBSyncInput_N3D.i4SceneLV;
    rAwbSyncInput.main_ch.rLightProb    = rAwbMaster.rAWBSyncInput_N3D.rLightProb;
    rAwbSyncInput.sub_ch.alg_gain       = rAwbSlave.rAWBSyncInput_N3D.rAlgGain;
    rAwbSyncInput.sub_ch.curr_gain      = rAwbSlave.rAWBSyncInput_N3D.rCurrentGain;
    rAwbSyncInput.sub_ch.target_gain    = rAwbSlave.rAWBSyncInput_N3D.rTargetGain;
    rAwbSyncInput.sub_ch.m_i4CCT        = rAwbSlave.rAWBSyncInput_N3D.i4CCT;
    rAwbSyncInput.sub_ch.m_i4LightMode  = rAwbSlave.rAWBSyncInput_N3D.i4LightMode;
    rAwbSyncInput.sub_ch.m_i4SceneLV    = rAwbSlave.rAWBSyncInput_N3D.i4SceneLV;
    rAwbSyncInput.sub_ch.rLightProb     = rAwbSlave.rAWBSyncInput_N3D.rLightProb;
#endif
    CAM_LOGD("[%s] In Main: R(%d), G(%d), B(%d), Main2: R(%d), G(%d), B(%d)", __FUNCTION__,
        m_rAwbSyncInput.main_ch.curr_gain.i4R, m_rAwbSyncInput.main_ch.curr_gain.i4G, m_rAwbSyncInput.main_ch.curr_gain.i4B,
        m_rAwbSyncInput.sub_ch.curr_gain.i4R, m_rAwbSyncInput.sub_ch.curr_gain.i4G, m_rAwbSyncInput.sub_ch.curr_gain.i4B);
    m_pMTKSyncAwb->SyncAwbMain(&m_rAwbSyncInput, &m_rAwbSyncOutput);
    CAM_LOGD("[%s] Out Main: R(%d), G(%d), B(%d), Main2: R(%d), G(%d), B(%d)", __FUNCTION__,
        m_rAwbSyncOutput.main_ch.rAwbGain.i4R, m_rAwbSyncOutput.main_ch.rAwbGain.i4G, m_rAwbSyncOutput.main_ch.rAwbGain.i4B,
        m_rAwbSyncOutput.sub_ch.rAwbGain.i4R, m_rAwbSyncOutput.sub_ch.rAwbGain.i4G, m_rAwbSyncOutput.sub_ch.rAwbGain.i4B);

    CAM_LOGD("[%s] -", __FUNCTION__);

    return MTRUE;
}


MBOOL
Sync3AWrapper::
update(MINT32 i4Opt)
{
    CAM_LOGD("[%s] OPT(%d) +", __FUNCTION__, i4Opt);

    MINT32 i4MeMasterOnOff = 0;
    GET_PROP("vendor.debug.sync2a.me.m", "-1", i4MeMasterOnOff);
    if (i4MeMasterOnOff != -1)
    {
        GET_PROP("vendor.debug.sync2a.me.m.exp", "10000", m_rManualAeMaster.u4ExpTime);
        GET_PROP("vendor.debug.sync2a.me.m.gain", "1024", m_rManualAeMaster.u4AfeGain);
        GET_PROP("vendor.debug.sync2a.me.m.isp", "1024", m_rManualAeMaster.u4IspGain);
        m_rManualAeMaster.fgOnOff = i4MeMasterOnOff == 1 ? MTRUE : MFALSE;
    }

    MINT32 i4MeSlaveOnOff = 0;
    GET_PROP("vendor.debug.sync2a.me.s", "-1", i4MeSlaveOnOff);
    if (i4MeSlaveOnOff != -1)
    {
        GET_PROP("vendor.debug.sync2a.me.s.exp", "10000", m_rManualAeSlave.u4ExpTime);
        GET_PROP("vendor.debug.sync2a.me.s.gain", "1024", m_rManualAeSlave.u4AfeGain);
        GET_PROP("vendor.debug.sync2a.me.s.isp", "1024", m_rManualAeSlave.u4IspGain);
        m_rManualAeSlave.fgOnOff = i4MeSlaveOnOff == 1 ? MTRUE : MFALSE;
    }

    if (i4Opt & (ISync3A::E_SYNC3A_DO_AE | ISync3A::E_SYNC3A_DO_AE_PRECAP))
    {
        AE_MODE_CFG_T rAEInfoMaster, rAEInfoSlave;
        rAEInfoMaster.u4ExposureMode    = 0;   // us
        rAEInfoMaster.u4Eposuretime     = m_rAeSyncOutput.main_ch.Output.EvSetting.u4Eposuretime;
        rAEInfoMaster.u4AfeGain         = m_rAeSyncOutput.main_ch.Output.EvSetting.u4AfeGain;
        rAEInfoMaster.u4IspGain         = m_rAeSyncOutput.main_ch.Output.EvSetting.u4IspGain;
        rAEInfoMaster.u4RealISO         = m_rAeSyncOutput.main_ch.Output.u4ISO;
        rAEInfoMaster.u2FrameRate       = m_rAeSyncOutput.main_ch.Output.u2FrameRate;
        rAEInfoMaster.u4CWValue         = m_rAeSyncOutput.main_ch.Output.u4CWValue;
        rAEInfoMaster.i2FlareGain       = m_rAeSyncOutput.main_ch.Output.i2FlareGain;
        rAEInfoMaster.i2FlareOffset     = m_rAeSyncOutput.main_ch.Output.i2FlareOffset;
        if (m_rManualAeMaster.fgOnOff)
        {   // manual override for master
            rAEInfoMaster.u4Eposuretime = m_rManualAeMaster.u4ExpTime;
            rAEInfoMaster.u4AfeGain     = m_rManualAeMaster.u4AfeGain;
            rAEInfoMaster.u4IspGain     = m_rManualAeMaster.u4IspGain;
        }

        rAEInfoSlave.u4ExposureMode     = 0;   // us
        rAEInfoSlave.u4Eposuretime      = m_rAeSyncOutput.sub_ch.Output.EvSetting.u4Eposuretime;
        rAEInfoSlave.u4AfeGain          = m_rAeSyncOutput.sub_ch.Output.EvSetting.u4AfeGain;
        rAEInfoSlave.u4IspGain          = m_rAeSyncOutput.sub_ch.Output.EvSetting.u4IspGain;
        rAEInfoSlave.u4RealISO          = m_rAeSyncOutput.sub_ch.Output.u4ISO;
        rAEInfoSlave.u2FrameRate        = m_rAeSyncOutput.sub_ch.Output.u2FrameRate;
        rAEInfoSlave.u4CWValue          = m_rAeSyncOutput.sub_ch.Output.u4CWValue;
        rAEInfoSlave.i2FlareGain        = m_rAeSyncOutput.sub_ch.Output.i2FlareGain;
        rAEInfoSlave.i2FlareOffset      = m_rAeSyncOutput.sub_ch.Output.i2FlareOffset;
        if (m_rManualAeSlave.fgOnOff)
        {   // manual override for slave
            rAEInfoSlave.u4Eposuretime  = m_rManualAeSlave.u4ExpTime;
            rAEInfoSlave.u4AfeGain      = m_rManualAeSlave.u4AfeGain;
            rAEInfoSlave.u4IspGain      = m_rManualAeSlave.u4IspGain;
        }

        if (i4Opt & ISync3A::E_SYNC3A_DO_AE)
        {
            IAeMgr::getInstance().updatePreviewParams(m_i4Master, rAEInfoMaster, m_rAeSyncOutput.main_ch.Output.i4AEidxNext);
            IAeMgr::getInstance().updatePreviewParams(m_i4Slave, rAEInfoSlave, m_rAeSyncOutput.sub_ch.Output.i4AEidxNext);
        }
        if (i4Opt & ISync3A::E_SYNC3A_DO_AE_PRECAP)
        {
            IAeMgr::getInstance().updateCaptureParams(m_i4Master, rAEInfoMaster);
            IAeMgr::getInstance().updateCaptureParams(m_i4Slave, rAEInfoSlave);
        }
    }

    if (i4Opt & ISync3A::E_SYNC3A_DO_AWB)
    {
        AWB_SYNC_OUTPUT_N3D_T rAwbSyncMaster, rAwbSyncSlave;
        rAwbSyncMaster.rAWBGain = m_rAwbSyncOutput.main_ch.rAwbGain;
        rAwbSyncMaster.i4CCT    = m_rAwbSyncOutput.main_ch.i4CCT;
        rAwbSyncSlave.rAWBGain  = m_rAwbSyncOutput.sub_ch.rAwbGain;
        rAwbSyncSlave.i4CCT     = m_rAwbSyncOutput.sub_ch.i4CCT;

        IAwbMgr::getInstance().applyAWB(m_i4Master, rAwbSyncMaster);
        IAwbMgr::getInstance().applyAWB(m_i4Slave, rAwbSyncSlave);
    } else
    {

        AWB_OUTPUT_T rAwbMaster, rAwbSlave;

        IAwbMgr::getInstance().getAWBOutput(m_i4Master, rAwbMaster);
        IAwbMgr::getInstance().getAWBOutput(m_i4Slave, rAwbSlave);

        ::memcpy(&m_rAwbSyncInput.main_ch, &rAwbMaster.rAWBSyncInput_N3D, sizeof(AWB_SYNC_INPUT_N3D_T));
        ::memcpy(&m_rAwbSyncInput.sub_ch, &rAwbSlave.rAWBSyncInput_N3D, sizeof(AWB_SYNC_INPUT_N3D_T));


        AWB_SYNC_OUTPUT_N3D_T rAwbSyncMaster, rAwbSyncSlave;
        rAwbSyncMaster.rAWBGain = m_rAwbSyncInput.main_ch.curr_gain;
        rAwbSyncMaster.i4CCT    = m_rAwbSyncInput.main_ch.m_i4CCT;
        rAwbSyncSlave.rAWBGain  = m_rAwbSyncInput.sub_ch.curr_gain;
        rAwbSyncSlave.i4CCT     = m_rAwbSyncInput.sub_ch.m_i4CCT;

        IAwbMgr::getInstance().applyAWB(m_i4Master, rAwbSyncMaster);
        IAwbMgr::getInstance().applyAWB(m_i4Slave, rAwbSyncSlave);

    }

    CAM_LOGD("[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
Sync3AWrapper::
setDebugInfo(void* prDbgInfo) const
{
    CAM_LOGD("[%s] +", __FUNCTION__);

    N3D_DEBUG_INFO_T* prN3dDbgInfo = reinterpret_cast<N3D_DEBUG_INFO_T*>(prDbgInfo);
    if (!prN3dDbgInfo)
    {
        CAM_LOGE("prN3dDbgInfo is NULL!");
        return MFALSE;
    }
    m_pMTKSyncAe->SyncAeFeatureCtrl(SYNC_AE_FEATURE_GET_DEBUG_INFO, (void*) &prN3dDbgInfo->rAEDebugInfo, NULL);
    m_pMTKSyncAwb->SyncAwbFeatureCtrl(SYNC_AWB_FEATURE_GET_DEBUG_INFO, (void*) &prN3dDbgInfo->rAWBDebugInfo, NULL);

    CAM_LOGD("[%s] -", __FUNCTION__);
    return MTRUE;
}

SYNCAE_CAMERA_TYPE_ENUM
Sync3AWrapper::
querySensorType(MINT32 i4SensorDev)
{
    SensorStaticInfo sensorStaticInfo;
    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList)
    {
        CAM_LOGE("MAKE_HalSensorList() == NULL");
        return BAYER_SENSOR;
    }
    pHalSensorList->querySensorStaticInfo(i4SensorDev,&sensorStaticInfo);

    MUINT32 u4RawFmtType = sensorStaticInfo.rawFmtType; // SENSOR_RAW_MONO or SENSOR_RAW_Bayer
    CAM_LOGD("[%s] i4SensorDev(%d) u4RawFmtType(%d)\n", __FUNCTION__, i4SensorDev, u4RawFmtType);
    return (u4RawFmtType == SENSOR_RAW_MONO) ? MONO_SENSOR : BAYER_SENSOR;
}

/******************************************************************************
 *  Sync3A
 ******************************************************************************/
class Sync3A : public ISync3A
{
public:
    static Sync3A*              getInstance(MINT32 i4Id);

    virtual MBOOL               init(MINT32 i4Policy, MINT32 i4Master, MINT32 i4Slave, const char* strName);

    virtual MBOOL               uninit();

    virtual MINT32              sync(MINT32 i4Sensor, MINT32 i4Param, MVOID* pSttBuf);

    virtual MINT32              syncAF(MINT32 i4Sensor);

    virtual MVOID               enableSync(MBOOL fgOnOff);

    virtual MBOOL               isSyncEnable() const;

    virtual MINT32              getFrameCount() const {return m_i4SyncFrmCount;}

    virtual MBOOL               isAeStable() const {return m_pSync3AWrap->isAeStable();}

    virtual MINT32              getAeSchedule() const;

    virtual MBOOL               setDebugInfo(void* prDbgInfo) const {return m_pSync3AWrap->setDebugInfo(prDbgInfo);}

    virtual MVOID               enableSyncSupport(E_SYNC3A_SUPPORT iSupport);

    virtual MVOID               disableSyncSupport(E_SYNC3A_SUPPORT iSupport);

    virtual MINT32              getSyncSupport() const;

    virtual MBOOL               isAFSyncFinish();

    virtual MBOOL               isAESyncStable();

protected:
    Sync3A();
    virtual ~Sync3A();

    MBOOL                       wait(MINT32 i4Sensor);
    MBOOL                       post(MINT32 i4Sensor);
    MBOOL                       resetScheduler();

    mutable Mutex               m_Lock;
    Condition                   m_Condition;
    MBOOL                       m_fgSyncEnable;
    MBOOL                       m_fgChkSyncEnable;
    MINT32                      m_i4SyncSupport;
    MINT32                      m_i4Count;
    MINT32                      m_i4SyncAeCount;
    MINT32                      m_i4SyncFrmCount;
    MINT32                      m_i4OptFirstIn;
    MINT32                      m_i4Policy;
    MINT32                      m_i4Master;
    MINT32                      m_i4Slave;
    MINT32                      m_i4MasterMagic;
    MINT32                      m_i4SlaveMagic;
    StatisticBufInfo*           m_pMasterSttBuf;
    StatisticBufInfo*           m_pSlaveSttBuf;
    std::string                 m_strName;
    Sync3AWrapper*              m_pSync3AWrap;
    // AE control
    MINT32                      m_i4AeSchedule;
};

/******************************************************************************
 *  Sync3A Implementation
 ******************************************************************************/
Sync3A::
Sync3A()
    : m_Lock()
    , m_fgSyncEnable(MFALSE)
    , m_fgChkSyncEnable(MFALSE)
    , m_i4SyncSupport(3)
    , m_i4Count(0)
    , m_i4SyncAeCount(0)
    , m_i4SyncFrmCount(0)
    , m_i4OptFirstIn(0)
    , m_i4Policy(0)
    , m_i4Master(ESensorDevId_Main)
    , m_i4Slave(ESensorDevId_MainSecond)
    , m_i4MasterMagic(0)
    , m_i4SlaveMagic(0)
    , m_pMasterSttBuf(NULL)
    , m_pSlaveSttBuf(NULL)
    , m_pSync3AWrap(NULL)
    , m_i4AeSchedule(E_AE_IDLE)
{
}

Sync3A::
~Sync3A()
{}

Sync3A*
Sync3A::
getInstance(MINT32 i4Id)
{
    switch (i4Id)
    {
    case ISync3AMgr::E_SYNC3AMGR_CAPTURE:
        {
            static Sync3A _rSync3ACap;
            return &_rSync3ACap;
        }
    default:
    case ISync3AMgr::E_SYNC3AMGR_PRVIEW:
        {
            static Sync3A _rSync3A;
            return &_rSync3A;
        }
    }
}

MBOOL
Sync3A::
init(MINT32 i4Policy, MINT32 i4Master, MINT32 i4Slave, const char* strName)
{
    m_i4Count = 0;
    m_i4SyncAeCount = 0;
    m_i4SyncFrmCount = 0;
    m_i4OptFirstIn = 0;
    m_i4Policy = i4Policy;
    m_i4Master = i4Master;
    m_i4Slave = i4Slave;
    m_i4MasterMagic = 0;
    m_i4SlaveMagic = 0;
    m_fgSyncEnable = MFALSE;
    m_fgChkSyncEnable = MFALSE;
    m_i4AeSchedule = E_AE_IDLE;

    m_strName = strName;

    m_pSync3AWrap = Sync3AWrapper::getInstance();
    m_pSync3AWrap->init(i4Master, i4Slave);

    CAM_LOGD("[%s] %s: policy(%d), masterDev(%d), slaveDev(%d), m_pSync3AWrap(%p)",
        __FUNCTION__, m_strName.c_str(), i4Policy, i4Master, i4Slave, m_pSync3AWrap);
    return MTRUE;
}

MBOOL
Sync3A::
uninit()
{
    m_fgSyncEnable = MFALSE;

    m_pSync3AWrap->uninit();
    m_pSync3AWrap = NULL;

    CAM_LOGD("[%s] %s: OK", __FUNCTION__, m_strName.c_str());
    return MTRUE;
}

MVOID
Sync3A::
enableSync(MBOOL fgOnOff)
{
    Mutex::Autolock lock(m_Lock);

    if (fgOnOff == MFALSE)
    {
        if (m_i4Count < 0)
        {
            m_i4Count ++;
            m_Condition.signal();
            CAM_LOGD("[%s] %s: Disable Sync: post(%d)", __FUNCTION__, m_strName.c_str(), m_i4Count);
        }
    }
    CAM_LOGD("[%s] %s Sync OnOff(%d)", __FUNCTION__, m_strName.c_str(), fgOnOff);

    m_fgSyncEnable = fgOnOff;
}

MBOOL
Sync3A::
isSyncEnable() const
{
    return m_fgSyncEnable;
}

MVOID
Sync3A::
enableSyncSupport(E_SYNC3A_SUPPORT iSupport)
{
    CAM_LOGD("[%s] m_i4SyncSupport = %d, iSupport = %d", __FUNCTION__, m_i4SyncSupport, iSupport);
    m_i4SyncSupport |= iSupport;
    CAM_LOGD("[%s] m_i4SyncSupport = %d", __FUNCTION__, m_i4SyncSupport);
}

MVOID
Sync3A::
disableSyncSupport(E_SYNC3A_SUPPORT iSupport)
{
    CAM_LOGD("[%s] m_i4SyncSupport = %d, iSupport = %d", __FUNCTION__, m_i4SyncSupport, iSupport);
    m_i4SyncSupport &= (~iSupport);
    CAM_LOGD("[%s] m_i4SyncSupport = %d", __FUNCTION__, m_i4SyncSupport);
}

MINT32
Sync3A::
getSyncSupport() const
{
    return m_i4SyncSupport;
}

MBOOL
Sync3A::
isAFSyncFinish()
{
    MBOOL syncAF;
    GET_PROP("vendor.debug.syncaf.precapture", "1", syncAF);

    if((getSyncSupport() & E_SYNC3A_SUPPORT_AF) && syncAF)
    {
        CAM_LOGD("[%s] isFocusFinish = (%d,%d)", __FUNCTION__,
            IAfMgr::getInstance().isFocusFinish(m_i4Master),
            IAfMgr::getInstance().isFocusFinish(m_i4Slave));
        return (IAfMgr::getInstance().isFocusFinish(m_i4Master)
             && IAfMgr::getInstance().isFocusFinish(m_i4Slave));
    }
    return MTRUE;
}

MBOOL
Sync3A::
isAESyncStable()
{
    if((getSyncSupport() & E_SYNC3A_SUPPORT_AE))
    {
        CAM_LOGD("[%s] isFocusFinish = (%d,%d)", __FUNCTION__,
            IAeMgr::getInstance().IsAEStable(m_i4Master),
            IAeMgr::getInstance().IsAEStable(m_i4Slave));
        return IAeMgr::getInstance().IsAEStable(m_i4Master);
    }
    return MTRUE;
}

MINT32
Sync3A::
sync(MINT32 i4Sensor, MINT32 i4Param, MVOID* pSttBuf)
{
    MINT32 i4Ret = 0;
    MINT32 i4SemVal;

    Mutex::Autolock autoLock(m_Lock);
    i4SemVal = m_i4Count;

    if(i4Sensor == m_i4Master)
       m_pMasterSttBuf = reinterpret_cast<StatisticBufInfo*>(pSttBuf);
    else if(i4Sensor == m_i4Slave)
       m_pSlaveSttBuf = reinterpret_cast<StatisticBufInfo*>(pSttBuf);
    if (i4SemVal >= 0)
    {
        m_i4Count --;
        CAM_LOGD("[%s](0) %s: eSensor(%d) Wait(%d) i4Param(0x%08x)", __FUNCTION__, m_strName.c_str(), i4Sensor, m_i4Count, i4Param);
        i4Ret = 0;
    }
    else
    {
        m_i4Count ++;
        CAM_LOGD("[%s](2) %s: eSensor(%d) Post(%d) i4Param(0x%08x)", __FUNCTION__, m_strName.c_str(), i4Sensor, m_i4Count, i4Param);
        i4Ret = 1;
    }

    if (i4Ret == 0)
    {
        m_i4OptFirstIn = i4Param;
        wait(i4Sensor);
    }
    else
    {
        if ((i4Param|m_i4OptFirstIn) & ISync3A::E_SYNC3A_BYP_AE)
        {
            CAM_LOGD("%s: Ignore AEAWB: eSensor(%d), Opt(0x%08x), another Opt(0x%08x)", m_strName.c_str(), i4Sensor, i4Param, m_i4OptFirstIn);
            i4Param &= (~(ISync3A::E_SYNC3A_DO_AE|ISync3A::E_SYNC3A_DO_AWB));
        }
        else if (i4Param != m_i4OptFirstIn)
        {
            CAM_LOGE("%s: Phase Missmatch: eSensor(%d), Opt(0x%08x), another Opt(0x%08x)", m_strName.c_str(), i4Sensor, i4Param, m_i4OptFirstIn);
            resetScheduler();
            i4Param &= (~ISync3A::E_SYNC3A_DO_AE);
        }

        MINT32 i4BypSyncCalc = 0;
        GET_PROP("vendor.debug.sync2a.byp", "0", i4BypSyncCalc);

        // calculation
        CAM_LOGD("[%s](3) %s: Sync 2A: Sensor(%d) Calculation", __FUNCTION__, m_strName.c_str(), i4Sensor);

        // 2a sync do calculation
        MINT32 i4Opt = i4Param & (~i4BypSyncCalc);
        CAM_LOGD("[%s] i4Opt = %d", __FUNCTION__, i4Opt);
        if(!(m_i4SyncSupport & E_SYNC3A_SUPPORT_AE))
        {
            i4Opt &= (~ISync3A::E_SYNC3A_DO_AE);
            i4Opt &= (~ISync3A::E_SYNC3A_DO_AE_PRECAP);
        }
        if(!(m_i4SyncSupport & E_SYNC3A_SUPPORT_AWB))
            i4Opt &= (~ISync3A::E_SYNC3A_DO_AWB);
        CAM_LOGD("[%s] i4Opt = %d", __FUNCTION__, i4Opt);

        m_pSync3AWrap->syncCalc(i4Opt, m_pMasterSttBuf, m_pSlaveSttBuf);
        m_i4SyncAeCount++;
        m_i4AeSchedule = (m_i4AeSchedule + 1) % 3;
        if (m_i4AeSchedule == 1){ // AE calculation frame
            // 2A frame sync
            AE_MODE_CFG_T rMainPreviewParams;
            AE_MODE_CFG_T rMain2PreviewParams;
            IAeMgr::getInstance().getPreviewParams(m_i4Master, rMainPreviewParams);
            IAeMgr::getInstance().getPreviewParams(m_i4Slave, rMain2PreviewParams);
            AAASensorMgr::getInstance().updateStereoFrameSyncFps(m_i4Master, m_i4Slave,rMainPreviewParams.u4Eposuretime,rMain2PreviewParams.u4Eposuretime);
        }
        post(i4Sensor);
        m_pMasterSttBuf = NULL;
        m_pSlaveSttBuf = NULL;
    }

    return i4Ret;
}

MINT32
Sync3A::
syncAF(MINT32 i4Sensor)
{
    MINT32 i4Ret = 0, syncPos=0;
    if(i4Sensor == m_i4Master && (getSyncSupport() & E_SYNC3A_SUPPORT_AF))
    {
        MINT32 i4MfMasterOnOff = 0;
        GET_PROP("vendor.debug.sync2a.mf.m", "0", i4MfMasterOnOff);

        MINT32 i4MfSlaveOnOff = 0;
        GET_PROP("vendor.debug.sync2a.mf.s", "0", i4MfSlaveOnOff);

        CAM_LOGD("[%s](i4MfMasterOnOff,i4MfSlaveOnOff) = (%d,%d)", __FUNCTION__, i4MfMasterOnOff, i4MfSlaveOnOff);

        if(i4MfMasterOnOff)
        {
            MINT32 i4MfMasterPosition;
            GET_PROP("vendor.debug.sync2a.mf.m.pos", "0", i4MfMasterPosition);
            CAM_LOGD("[%s] i4MfMasterPosition = %d", __FUNCTION__, i4MfMasterPosition);
            IAfMgr::getInstance().setMFPos(m_i4Master, i4MfMasterPosition);
        }

        // prepare the data for AF sync
        // do AF algo for remapping position to Main2
        if(i4MfSlaveOnOff)
        {
            MINT32 i4MfSlavePosition;
            GET_PROP("vendor.debug.sync2a.mf.s.pos", "0", i4MfSlavePosition);
            CAM_LOGD("[%s] i4MfSlavePosition = %d", __FUNCTION__, i4MfSlavePosition);
            IAfMgr::getInstance().MoveLensTo(m_i4Slave, i4MfSlavePosition);
        }
        else
        {
            IAfMgr::getInstance().SyncAFSetMode(m_i4Slave,1);

            AF_SyncInfo_T sCamInfo;
            memset(&sCamInfo,  0, sizeof(sCamInfo));

            IAfMgr::getInstance().SyncAFGetMotorRange(m_i4Slave, sCamInfo);
            syncPos = IAfMgr::getInstance().SyncAFGetInfo(m_i4Master, sCamInfo);
            MY_LOG("[syncPos]%d [MPos]%d [Sdone]%d",
                syncPos,
                IAfMgr::getInstance().getAFPos(m_i4Master),
                IAfMgr::getInstance().isFocusFinish(m_i4Slave));
            if(syncPos!=0xFFFF)
            {
                IAfMgr::getInstance().SyncAFSetInfo(m_i4Slave, syncPos);
                IAfMgr::getInstance().MoveLensTo(m_i4Slave, syncPos);
            }
            else
                MY_LOG("[syncPos] err");

            IAfMgr::getInstance().SyncAFGetCalibPos(m_i4Slave, sCamInfo);
            IAfMgr::getInstance().SyncAFCalibPos(m_i4Master, sCamInfo);
        }
    }

    return i4Ret;
}

MBOOL
Sync3A::
wait(MINT32 i4Sensor)
{
    MBOOL fgRet = MTRUE;
    CAM_LOGD("[%s](1) %s: + eSensor(%d), m_fgSyncEnable(%d)", __FUNCTION__, m_strName.c_str(), i4Sensor, m_fgSyncEnable);
    if (m_fgSyncEnable)
    {
        m_Condition.wait(m_Lock);
    }

    if (!m_fgSyncEnable)
        fgRet = MFALSE;
    CAM_LOGD("[%s](5) %s: - eSensor(%d), m_fgSyncEnable(%d)", __FUNCTION__, m_strName.c_str(), i4Sensor, m_fgSyncEnable);
    return fgRet;
}

MBOOL
Sync3A::
post(MINT32 i4Sensor)
{
    CAM_LOGD("[%s](4) %s: eSensor(%d)", __FUNCTION__, m_strName.c_str(), i4Sensor);
    m_i4SyncFrmCount ++;
    m_Condition.signal();
    return MTRUE;
}

MBOOL
Sync3A::
resetScheduler()
{
    CAM_LOGD("[%s]", __FUNCTION__);
    m_i4SyncAeCount = 0;
    return MTRUE;
}

MINT32
Sync3A::
getAeSchedule() const
{
    if(m_i4AeSchedule == 0)
        return E_AE_AE_CALC;
    else
        return E_AE_IDLE;
}


/******************************************************************************
 *  ISync3AMgr Implementation
 ******************************************************************************/
class Sync3AMgr : public ISync3AMgr
{
public:
    /**
     * get singleton.
     */
    static Sync3AMgr*           getInstance();

    virtual ISync3A*            getSync3A(MINT32 i4Id) const;

    /**
     * Explicitly init 3A N3D Sync manager by MW.
     */
    virtual MBOOL               init(MINT32 i4Policy, MINT32 i4MasterIdx, MINT32 i4SlaveIdx);

    /**
     * Explicitly uninit 3A N3D Sync manager by MW.
     */
    virtual MBOOL               uninit();

    virtual MBOOL               isInit() const;

    virtual MBOOL               isActive() const;

    virtual MINT32              getMasterDev() const {return m_i4Master;}

    virtual MINT32              getSlaveDev() const {return m_i4Slave;}

    virtual MVOID               setAFState(MINT32 i4AfState);

    virtual MINT32              getAFState() const {return m_fgAfState;}

    virtual MVOID               setAFSyncMode(MINT32 i4AfSyncMode) {i4AfSyncMode;}

    virtual MVOID               enable() {}

    virtual MVOID               disable() {}

    virtual MVOID               updateInitParams() {}

    virtual MVOID               setManualControl(MBOOL bEnable) {bEnable;}

    virtual MBOOL               isManualControl(){return MFALSE;}

protected:
    Sync3AMgr()
        : m_fgIsActive(MFALSE)
        , m_fgAfState(MFALSE)
        , m_i4Master(ESensorDevId_Main)
        , m_i4Slave(ESensorDevId_MainSecond)
        , m_eCapMode(E_SYNC3AMGR_CAPMODE_2D)
        {}
    virtual ~Sync3AMgr(){}

    MBOOL                       m_fgIsActive;
    MBOOL                       m_fgAfState;
    MINT32                      m_i4Master;
    MINT32                      m_i4Slave;
    E_SYNC3AMGR_CAPMODE         m_eCapMode;
};

ISync3AMgr*
ISync3AMgr::
getInstance()
{
    return Sync3AMgr::getInstance();
}

Sync3AMgr*
Sync3AMgr::
getInstance()
{
    static Sync3AMgr _rSync3AMgr;
    return &_rSync3AMgr;
}

ISync3A*
Sync3AMgr::
getSync3A(MINT32 i4Id) const
{
    return Sync3A::getInstance(i4Id);
}

MBOOL
Sync3AMgr::
init(MINT32 i4Policy, MINT32 i4MasterIdx, MINT32 i4SlaveIdx)
{
    m_fgIsActive = MTRUE;

    IHalSensorList* pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList) return MFALSE;

    m_i4Master = pHalSensorList->querySensorDevIdx(i4MasterIdx);
    m_i4Slave = pHalSensorList->querySensorDevIdx(i4SlaveIdx);
    CAM_LOGD("[%s] Master(%d), Slave(%d)", __FUNCTION__, m_i4Master, m_i4Slave);

    Sync3A::getInstance(E_SYNC3AMGR_PRVIEW)->init(i4Policy, m_i4Master, m_i4Slave, "Preview");
    Sync3A::getInstance(E_SYNC3AMGR_CAPTURE)->init(i4Policy, m_i4Master, m_i4Slave, "Capture");

    // awb ROI for main2
    NSCamCustomSensor::SensorViewAngle_T rVangle = NSCamCustomSensor::getSensorViewAngle();


    if ((rVangle.Main2SensorVerFOV < rVangle.MainSensorVerFOV) ||
        (rVangle.Main2SensorHorFOV < rVangle.MainSensorHorFOV))
    {
        CAM_LOGD("[%s] Not necessary to config window MainFOV(%d,%d) Main2FOV(%d,%d)",
            __FUNCTION__, rVangle.MainSensorHorFOV, rVangle.MainSensorVerFOV, rVangle.Main2SensorHorFOV, rVangle.Main2SensorVerFOV);
    }
    else
    {
        MINT32 i;
        SensorStaticInfo rSensorStaticInfo;
        pHalSensorList->querySensorStaticInfo(m_i4Slave, &rSensorStaticInfo);
        MINT32 u4NumSensorModes = MIN(rSensorStaticInfo.SensorModeNum, (MINT32)ESensorMode_NUM);

        NSCam::MSize rSize[ESensorMode_NUM];
        rSize[ESensorMode_Preview].w    = rSensorStaticInfo.previewWidth;
        rSize[ESensorMode_Preview].h    = rSensorStaticInfo.previewHeight;
        rSize[ESensorMode_Capture].w    = rSensorStaticInfo.captureWidth;
        rSize[ESensorMode_Capture].h    = rSensorStaticInfo.captureHeight;
        rSize[ESensorMode_Video].w      = rSensorStaticInfo.videoWidth;
        rSize[ESensorMode_Video].h      = rSensorStaticInfo.videoHeight;
        rSize[ESensorMode_SlimVideo1].w = rSensorStaticInfo.video1Width;
        rSize[ESensorMode_SlimVideo1].h = rSensorStaticInfo.video1Height;
        rSize[ESensorMode_SlimVideo2].w = rSensorStaticInfo.video2Width;
        rSize[ESensorMode_SlimVideo2].h = rSensorStaticInfo.video2Height;

        CAM_LOGD("[%s] u4NumSensorModes(%d), prv(%d,%d), cap(%d,%d), vdo(%d,%d), vdo1(%d,%d), vdo2(%d,%d)", __FUNCTION__, u4NumSensorModes,
            rSensorStaticInfo.previewWidth,
            rSensorStaticInfo.previewHeight,
            rSensorStaticInfo.captureWidth,
            rSensorStaticInfo.captureHeight,
            rSensorStaticInfo.videoWidth,
            rSensorStaticInfo.videoHeight,
            rSensorStaticInfo.video1Width,
            rSensorStaticInfo.video1Height,
            rSensorStaticInfo.video2Width,
            rSensorStaticInfo.video2Height
            );

        for (i = 0; i < u4NumSensorModes; i++)
        {
            NSCam::MRect rCrop = get2AStatROI(
                rVangle.MainSensorVerFOV,
                rVangle.MainSensorHorFOV,
                rVangle.Main2SensorVerFOV,
                rVangle.Main2SensorHorFOV,
                rSize[i].w, rSize[i].h);

            IAwbMgr::getInstance().setAWBStatCropRegion(
                m_i4Slave,
                i,
                rCrop.p.x, rCrop.p.y, rCrop.s.w, rCrop.s.h);
        }
    }
    return MTRUE;
}

MBOOL
Sync3AMgr::
uninit()
{
    if (m_fgIsActive)
    {
        CAM_LOGD("[%s] Master(%d), Slave(%d)", __FUNCTION__, m_i4Master, m_i4Slave);
        Sync3A::getInstance(E_SYNC3AMGR_PRVIEW)->uninit();
        Sync3A::getInstance(E_SYNC3AMGR_CAPTURE)->uninit();
        m_fgIsActive = MFALSE;
    }
    return MTRUE;
}

MBOOL
Sync3AMgr::
isInit() const
{
    return m_fgIsActive;
}

MBOOL
Sync3AMgr::
isActive() const
{
    return m_fgIsActive;
}

MVOID
Sync3AMgr::
setAFState(MINT32 i4AfState)
{
    CAM_LOGD("[%s] i4AfState(%d)", __FUNCTION__, i4AfState);
    IAeMgr::getInstance().enableAEOneShotControl(m_i4Slave, (i4AfState != 0)?MTRUE:MFALSE);
    m_fgAfState = i4AfState;
}