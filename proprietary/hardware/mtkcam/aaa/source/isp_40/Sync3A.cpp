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

#include "MTKSyncAe.h"
#include "MTKSyncAwb.h"

#include "Local.h"

#if (!MT6763_API)
#include <ae_mgr_if.h>
#include <awb_mgr_if.h>
#include <isp_mgr.h>
#else
#include <../mt6763/ae_mgr/ae_mgr_if.h>
#include <../mt6763/awb_mgr/awb_mgr_if.h>
#include <../mt6763/isp_mgr/isp_mgr.h>
#endif

#include <sensor_mgr/aaa_sensor_mgr.h>

#include <nvbuf_util.h>
#include <cutils/properties.h>

#include "n3d_sync2a_tuning_param.h"
#include <camera_custom_imgsensor_cfg.h>
#include <mtkcam/drv/IHalSensor.h>
#if (!MT6763_API)
#include <isp_tuning_mgr.h>
#else
#include <../mt6763/isp_tuning/isp_tuning_mgr.h>
#endif
#include <isp_tuning.h>

#include <dbg_id_param.h>        // for DEBUG_XXX_KEY_ID
#include <dbg_cam_param.h>
#include <dbg_cam_n3d_param.h>

#include <string>
#include <math.h>

#include <dbg_aaa_param.h>
#include <dbg_af_param.h>
#include <camera_custom_nvram.h>
#include <af_param.h>
#include <af_feature.h>
#include <af_algo_if.h>
#include <af_mgr_if.h>
#include <StatisticBuf.h>
#include <ispif.h>
#include <flash_hal.h>
#include <flash_mgr.h>
#include <flicker_hal_if.h>

// for the FO of Wide standby mode
#include <camera_custom_dualzoom.h>


using namespace NS3Av3;
using namespace android;
using namespace NSIspTuningv3;

#define SEM_TIME_LIMIT_NS       1000000000L

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

    virtual VOID                syncAeSetting(MINT32 i4MasterIdx, MINT32 i4SlaveIdx);

    virtual MVOID               updateMasterSlave(MINT32 i4Master, MINT32 i4Slave);

    virtual MVOID               updateFrameSync(MBOOL bEnable);

protected:
    Sync3AWrapper();
    virtual ~Sync3AWrapper(){}

    virtual MBOOL               update(MINT32 i4Opt);
    SYNCAE_CAMERA_TYPE_ENUM     querySensorType(MINT32 i4SensorDev);
    MBOOL                       syncAeInit();
    MBOOL                       syncAwbInit();

    MBOOL                       syncAeMain(const StatisticBufInfo* pMasterSttBuf, const StatisticBufInfo* pSlaveSttBuf);
    MBOOL                       syncAwbMain(const StatisticBufInfo* pMasterSttBuf, const StatisticBufInfo* pSlaveSttBuf);

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
    MINT32                      m_Sync3ALogEnable;
};

/******************************************************************************
 *  Sync3AWrapper Implementation
 ******************************************************************************/
static MINT32 Complement2(MUINT32 value, MUINT32 digit)
{
    MINT32 Result;

    if (((value >> (digit - 1)) & 0x1) == 1) // negative
    {
    Result = 0 - (MINT32)((~value + 1) & ((1 << digit) - 1));
    }
    else
    {
    Result = (MINT32)(value & ((1 << digit) - 1));
    }

    return Result;
}


Sync3AWrapper::
Sync3AWrapper()
    : m_Lock()
    , m_i4User(0)
    , m_i4Master(1)
    , m_i4Slave(1)
    , m_fgMasterAeStable(MFALSE)
    , m_fgSlaveAeStable(MFALSE)
    , m_rAwbSyncInput()
    , m_rAwbSyncOutput()
    , m_rAeSyncOutput()
    , m_Sync3ALogEnable(0)
{
    m_pMTKSyncAe = NULL;
    m_pMTKSyncAwb = NULL;
    memset(&m_rAwbSyncInput, 0, sizeof(SYNC_AWB_INPUT_INFO_STRUCT));
    memset(&m_rAwbSyncOutput, 0, sizeof(SYNC_AWB_OUTPUT_INFO_STRUCT));
    memset(&m_rAeSyncOutput, 0, sizeof(SYNC_AE_OUTPUT_STRUCT));
    GET_PROP("vendor.debug.sync3AWrapper.log", "0", m_Sync3ALogEnable);
}

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
        CAM_LOGD("[%s] CCU_SYNC_DBG Create ICcuCtrl3ASync",__FUNCTION__);
        IAfMgr::getInstance().SyncAFSetMode(m_i4Slave,0xFE);
        IAfMgr::getInstance().SyncAFSetMode(m_i4Master,0xFF);
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
            CAM_LOGD("[%s] Still %d users \n", __FUNCTION__, m_i4User);
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
        CAM_LOGE("[%s] pMasterSttBuf = NULL(%p) || pSlaveSttBuf = NULL(%p)", __FUNCTION__, pMasterSttBuf, pSlaveSttBuf);
        return MFALSE;
    }
    if (i4Opt & (ISync3A::E_SYNC3A_DO_AE|ISync3A::E_SYNC3A_DO_AE_PRECAP))
    {
        if (!syncAeMain(pMasterSttBuf, pSlaveSttBuf))
            CAM_LOGE("[%s] syncAeMain fail", __FUNCTION__);
    }

    if (i4Opt & ISync3A::E_SYNC3A_DO_AWB)
    {
        if (!syncAwbMain(pMasterSttBuf, pSlaveSttBuf))
            CAM_LOGE("[%s] syncAwbMain fail", __FUNCTION__);
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

    ::memcpy(rAEInitData.SyncAeTuningParam.pDeltaBVtoRatioArray, &pSyncAEInitInfo->pDeltaBVtoRatioArray[0][0], sizeof(MUINT32)*2*30);
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

    const strSyncAWBInitInfo* pAwbInitInfo = getSyncAWBInitInfo();

    rAWBInitData.SyncScenario = SYNC_AWB_SCENARIO_N3D;

    NVRAM_CAMERA_3A_STRUCT* p3aNvram;
    NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_i4Master, (void*&)p3aNvram, MFALSE);
    rAWBInitData.main_param = p3aNvram->rAWBNVRAM[CAM_SCENARIO_PREVIEW];
    NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_i4Slave, (void*&)p3aNvram, MFALSE);
    rAWBInitData.sub_param = p3aNvram->rAWBNVRAM[CAM_SCENARIO_PREVIEW];

    rAWBInitData.SyncAwbTuningParam.SyncMode = pAwbInitInfo->syncAWBMethod;

    rAWBInitData.SyncAwbTuningParam.isFixMapLocation = pAwbInitInfo->isFixMapLocation;
    rAWBInitData.SyncAwbTuningParam.u4SyncAWB_fov_ratio = pAwbInitInfo->u4SyncAWB_fov_ratio;

    ::memcpy(&rAWBInitData.SyncAwbTuningParam.syncAWB_pp_method_tuning_param, &pAwbInitInfo->syncAWB_pp_method_tuning_param, sizeof(SYNC_AWB_ADV_PP_METHOD_TUNING_STRUCT));
    ::memcpy(&rAWBInitData.SyncAwbTuningParam.syncAWB_blend_method_tuning_param, &pAwbInitInfo->syncAWB_blend_method_tuning_param, sizeof(SYNC_AWB_BLEND_TUNING_STRUCT));
    ::memcpy(&rAWBInitData.SyncAwbTuningParam.syncAWB_wp_method_tuning_param, &pAwbInitInfo->syncAWB_wp_method_tuning_param, sizeof(SYNC_AWB_WHITEPOINTS_MAPPING_TUNING_STRUCT));
    ::memcpy(&rAWBInitData.SyncAwbTuningParam.syncAWB_smooth_param, &pAwbInitInfo->syncAWB_smooth_param, sizeof(SYNC_AWB_SMOOTH_TUNING_STRUCT));
    ::memcpy(&rAWBInitData.SyncAwbTuningParam.syncAWB_prefer_param, &pAwbInitInfo->syncAWB_prefer_param, sizeof(SYNC_AWB_PREFER_TUNING_STRUCT));
    ::memcpy(&rAWBInitData.SyncAwbTuningParam.syncAWB_prefer_lvbase, &pAwbInitInfo->syncAWB_prefer_lvbase, sizeof(SYNC_AWB_PREFER_LVBASE_STRUCT));

    m_pMTKSyncAwb->SyncAwbInit(reinterpret_cast<void*>(&rAWBInitData));

    CAM_LOGD("[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
Sync3AWrapper::
syncAeMain(const StatisticBufInfo* pMasterSttBuf, const StatisticBufInfo* pSlaveSttBuf)
{
    CAM_LOGD("[%s] MS(%d,%d)+", __FUNCTION__, m_i4Master, m_i4Slave);

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

    NVRAM_CAMERA_3A_STRUCT *pMasterNvram, *pSlaveNvram;
    NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_i4Master, (void*&)pMasterNvram, 0);
    NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_i4Slave, (void*&)pSlaveNvram, 0);

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
    rAeSyncInput.main_ch.Input.i4DeltaBV = IAeMgr::getInstance().getDeltaBV(m_i4Master);
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
    rAeSyncInput.main_ch.pAeNvram = &pMasterNvram->rAENVRAM[0];
    rAeSyncInput.main_ch.aao_buffer_info.pAEStatisticBuf = (MVOID*) pMasterSttBuf->getPart(AAOSepBuf); // reinterpret_cast<MVOID *>(IAAOBufMgr::getInstance().getCurrHwBuf(m_i4Master));
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
    rAeSyncInput.sub_ch.Input.i4DeltaBV = IAeMgr::getInstance().getDeltaBV(m_i4Slave);
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
    rAeSyncInput.sub_ch.pAeNvram = &pSlaveNvram->rAENVRAM[0];
    rAeSyncInput.sub_ch.aao_buffer_info.pAEStatisticBuf = (MVOID*) pSlaveSttBuf->getPart(AAOSepBuf); //reinterpret_cast<MVOID *>(IAAOBufMgr::getInstance().getCurrHwBuf(m_i4Slave));
    rAeSyncInput.sub_ch.aao_buffer_info.buf_line_size = IAeMgr::getInstance().getAAOLineByteSize(m_i4Slave);
    rAeSyncInput.sub_ch.wb_gain = rAwbSlave.rAWBSyncInput_N3D.rAlgGain;

    m_pMTKSyncAe->SyncAeMain(&rAeSyncInput, &m_rAeSyncOutput);

    CAM_LOGD("[%s] -", __FUNCTION__);

    return MTRUE;
}

MBOOL
Sync3AWrapper::
syncAwbMain(const StatisticBufInfo* pMasterSttBuf, const StatisticBufInfo* pSlaveSttBuf)
{
    MINT32 i4Master = ISync3AMgr::getInstance()->getAWBMasterDev();
    MINT32 i4Slave = ISync3AMgr::getInstance()->getAWBSlaveDev();
    CAM_LOGD("[%s] MS(%d/%p,%d/%p), AWB(%d,%d) +", __FUNCTION__, m_i4Master, pMasterSttBuf, m_i4Slave, pSlaveSttBuf, i4Master, i4Slave);

    AWB_OUTPUT_T rAwbMaster, rAwbSlave;
    AWB_AAO_CONFIG_Param_T rMasterAWBConfig, rSlaveAWBConfig;

    AWB_PARENT_BLK_STAT_T& rAwbParentStatMaster = rAwbMaster.rAWBSyncInput_N3D.rAwbParentStatBlk;
    AWB_PARENT_BLK_STAT_T& rAwbParentStatSlave = rAwbSlave.rAWBSyncInput_N3D.rAwbParentStatBlk;
    MINT32& iParentBlkNumX_Main = rAwbMaster.rAWBSyncInput_N3D.ParentBlkNumX;
    MINT32& iParentBlkNumY_Main = rAwbMaster.rAWBSyncInput_N3D.ParentBlkNumY;
    MINT32& iParentBlkNumX_Sub = rAwbSlave.rAWBSyncInput_N3D.ParentBlkNumX;
    MINT32& iParentBlkNumY_Sub = rAwbSlave.rAWBSyncInput_N3D.ParentBlkNumY;

    IAwbMgr::getInstance().getAWBOutput(i4Master, rAwbMaster);
    IAwbMgr::getInstance().getAWBOutput(i4Slave, rAwbSlave);

    IAwbMgr::getInstance().getAWBParentStat(i4Master, rAwbParentStatMaster, &iParentBlkNumX_Main, &iParentBlkNumY_Main);
    IAwbMgr::getInstance().getAWBParentStat(i4Slave, rAwbParentStatSlave, &iParentBlkNumX_Sub, &iParentBlkNumY_Sub);

    IAwbMgr::getInstance().getAAOConfig(i4Master, rMasterAWBConfig);
    IAwbMgr::getInstance().getAAOConfig(i4Slave, rSlaveAWBConfig);

    Stereo_Param_T rParams = ISync3AMgr::getInstance()->getStereoParams();
    switch(rParams.i4Sync2AMode)
    {
        case NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_VSDOF:
            m_rAwbSyncInput.feature_type = DUAL_CAM_FEATURE_VSDOF;
            break;
        case NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DENOISE:
            m_rAwbSyncInput.feature_type = DUAL_CAM_FEATURE_BAYER_MONO;
            break;
        case NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DUAL_ZOOM:
            m_rAwbSyncInput.feature_type = DUAL_CAM_FEATURE_WIDE_TELE;
            break;
    }

    // 0 : main preview, 1 : sub preview
    if(m_i4Master == ISync3AMgr::getInstance()->getAWBMasterDev())
    {
        m_rAwbSyncInput.display_cam = 0;
        m_rAwbSyncInput.main_ch.aao_buf_info.pAWBStatBuf = pMasterSttBuf->getPart(AAOSepLSCBuf);
        m_rAwbSyncInput.sub_ch.aao_buf_info.pAWBStatBuf = pSlaveSttBuf->getPart(AAOSepLSCBuf);
    }
    else
    {
        m_rAwbSyncInput.display_cam = 1;
        m_rAwbSyncInput.sub_ch.aao_buf_info.pAWBStatBuf = pMasterSttBuf->getPart(AAOSepLSCBuf);
        m_rAwbSyncInput.main_ch.aao_buf_info.pAWBStatBuf = pSlaveSttBuf->getPart(AAOSepLSCBuf);
    }
    m_rAwbSyncInput.zoom_ratio = 32;    // TODO

    m_rAwbSyncInput.strLocationTransInfo.AAOWinConfig_main.width = rMasterAWBConfig.width;
    m_rAwbSyncInput.strLocationTransInfo.AAOWinConfig_main.height = rMasterAWBConfig.height;
    m_rAwbSyncInput.strLocationTransInfo.AAOWinConfig_main.offset_x = rMasterAWBConfig.offset_x;
    m_rAwbSyncInput.strLocationTransInfo.AAOWinConfig_main.offset_y = rMasterAWBConfig.offset_y;
    m_rAwbSyncInput.strLocationTransInfo.AAOWinConfig_main.size_x = rMasterAWBConfig.size_x;
    m_rAwbSyncInput.strLocationTransInfo.AAOWinConfig_main.size_y = rMasterAWBConfig.size_y;
    m_rAwbSyncInput.strLocationTransInfo.AAOWinConfig_main.num_x = rMasterAWBConfig.num_x;
    m_rAwbSyncInput.strLocationTransInfo.AAOWinConfig_main.num_y = rMasterAWBConfig.num_y;

    m_rAwbSyncInput.strLocationTransInfo.AAOWinConfig_main2.width = rSlaveAWBConfig.width;
    m_rAwbSyncInput.strLocationTransInfo.AAOWinConfig_main2.height = rSlaveAWBConfig.height;
    m_rAwbSyncInput.strLocationTransInfo.AAOWinConfig_main2.offset_x = rSlaveAWBConfig.offset_x;
    m_rAwbSyncInput.strLocationTransInfo.AAOWinConfig_main2.offset_y = rSlaveAWBConfig.offset_y;
    m_rAwbSyncInput.strLocationTransInfo.AAOWinConfig_main2.size_x = rSlaveAWBConfig.size_x;
    m_rAwbSyncInput.strLocationTransInfo.AAOWinConfig_main2.size_y = rSlaveAWBConfig.size_y;
    m_rAwbSyncInput.strLocationTransInfo.AAOWinConfig_main2.num_x = rSlaveAWBConfig.num_x;
    m_rAwbSyncInput.strLocationTransInfo.AAOWinConfig_main2.num_y = rSlaveAWBConfig.num_y;

    const strSyncAWBInitInfo* pAwbInitInfo = getSyncAWBInitInfo();
    m_rAwbSyncInput.strLocationTransInfo.isFixMapLocation = pAwbInitInfo->isFixMapLocation;
    m_rAwbSyncInput.strLocationTransInfo.u4CamFovRatio = pAwbInitInfo->u4SyncAWB_fov_ratio;

    NSIspTuning::ISP_NVRAM_CCM_T ccm;
    NSIspTuningv3::ISP_MGR_CCM::getInstance(static_cast<NSIspTuning::ESensorDev_T>(i4Master)).get(ccm);
    m_rAwbSyncInput.main_ch.awb_output_struct = rAwbMaster;
    m_rAwbSyncInput.main_ch.focus_distance = 0;                 // TODO : query from AF, (example : 10 cm)
    m_rAwbSyncInput.main_ch.aao_buf_info.stat_line_size = 480;    // TODO
    m_rAwbSyncInput.main_ch.ccm.CCmatrix[0] = ((float)Complement2(ccm.cnv_1.bits.G2G_CNV_00,13) / 512);
    m_rAwbSyncInput.main_ch.ccm.CCmatrix[1] = ((float)Complement2(ccm.cnv_1.bits.G2G_CNV_01,13) / 512);
    m_rAwbSyncInput.main_ch.ccm.CCmatrix[2] = ((float)Complement2(ccm.cnv_2.bits.G2G_CNV_02,13) / 512);
    m_rAwbSyncInput.main_ch.ccm.CCmatrix[3] = ((float)Complement2(ccm.cnv_3.bits.G2G_CNV_10,13) / 512);
    m_rAwbSyncInput.main_ch.ccm.CCmatrix[4] = ((float)Complement2(ccm.cnv_3.bits.G2G_CNV_11,13) / 512);
    m_rAwbSyncInput.main_ch.ccm.CCmatrix[5] = ((float)Complement2(ccm.cnv_4.bits.G2G_CNV_12,13) / 512);
    m_rAwbSyncInput.main_ch.ccm.CCmatrix[6] = ((float)Complement2(ccm.cnv_5.bits.G2G_CNV_20,13) / 512);
    m_rAwbSyncInput.main_ch.ccm.CCmatrix[7] = ((float)Complement2(ccm.cnv_5.bits.G2G_CNV_21,13) / 512);
    m_rAwbSyncInput.main_ch.ccm.CCmatrix[8] = ((float)Complement2(ccm.cnv_6.bits.G2G_CNV_22,13) / 512);

    NSIspTuningv3::ISP_MGR_CCM::getInstance(static_cast<NSIspTuning::ESensorDev_T>(i4Slave)).get(ccm);
    m_rAwbSyncInput.sub_ch.awb_output_struct = rAwbSlave;
    m_rAwbSyncInput.sub_ch.focus_distance = 0;                  // TODO : query from AF, (example : 10 cm)
    m_rAwbSyncInput.sub_ch.aao_buf_info.stat_line_size = 480;     // TODO
    m_rAwbSyncInput.sub_ch.ccm.CCmatrix[0] = ((float)Complement2(ccm.cnv_1.bits.G2G_CNV_00,13) / 512);
    m_rAwbSyncInput.sub_ch.ccm.CCmatrix[1] = ((float)Complement2(ccm.cnv_1.bits.G2G_CNV_01,13) / 512);
    m_rAwbSyncInput.sub_ch.ccm.CCmatrix[2] = ((float)Complement2(ccm.cnv_2.bits.G2G_CNV_02,13) / 512);
    m_rAwbSyncInput.sub_ch.ccm.CCmatrix[3] = ((float)Complement2(ccm.cnv_3.bits.G2G_CNV_10,13) / 512);
    m_rAwbSyncInput.sub_ch.ccm.CCmatrix[4] = ((float)Complement2(ccm.cnv_3.bits.G2G_CNV_11,13) / 512);
    m_rAwbSyncInput.sub_ch.ccm.CCmatrix[5] = ((float)Complement2(ccm.cnv_4.bits.G2G_CNV_12,13) / 512);
    m_rAwbSyncInput.sub_ch.ccm.CCmatrix[6] = ((float)Complement2(ccm.cnv_5.bits.G2G_CNV_20,13) / 512);
    m_rAwbSyncInput.sub_ch.ccm.CCmatrix[7] = ((float)Complement2(ccm.cnv_5.bits.G2G_CNV_21,13) / 512);
    m_rAwbSyncInput.sub_ch.ccm.CCmatrix[8] = ((float)Complement2(ccm.cnv_6.bits.G2G_CNV_22,13) / 512);

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
        rAwbMaster.rAWBSyncInput_N3D.rCurrentGain.i4R, rAwbMaster.rAWBSyncInput_N3D.rCurrentGain.i4G, rAwbMaster.rAWBInfo.rCurrentAWBGain.i4B,
        rAwbSlave.rAWBSyncInput_N3D.rCurrentGain.i4R, rAwbSlave.rAWBSyncInput_N3D.rCurrentGain.i4G, rAwbSlave.rAWBInfo.rCurrentAWBGain.i4B);
    m_pMTKSyncAwb->SyncAwbMain(&m_rAwbSyncInput, &m_rAwbSyncOutput);
    CAM_LOGD_IF(m_Sync3ALogEnable, "[%s] Out Main: R(%d), G(%d), B(%d), Main2: R(%d), G(%d), B(%d)", __FUNCTION__,
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
            IAeMgr::getInstance().updateAEUnderExpdeltaBVIdx(m_i4Master, m_rAeSyncOutput.main_ch.Output.i4DeltaBV);
            IAeMgr::getInstance().updateAEUnderExpdeltaBVIdx(m_i4Slave, m_rAeSyncOutput.sub_ch.Output.i4DeltaBV);
            IAeMgr::getInstance().updateStereoDenoiseRatio(m_i4Master, (MINT32*) &m_rAeSyncOutput.main_ch.Output.u4DeltaBVRatio[0]);
            IAeMgr::getInstance().updateStereoDenoiseRatio(m_i4Slave, (MINT32*) &m_rAeSyncOutput.sub_ch.Output.u4DeltaBVRatio[0]);
        }
        if (i4Opt & ISync3A::E_SYNC3A_DO_AE_PRECAP)
        {
            IAeMgr::getInstance().updateCaptureParams(m_i4Master, rAEInfoMaster);
            IAeMgr::getInstance().updateCaptureParams(m_i4Slave, rAEInfoSlave);
        }
    }

    if (i4Opt & ISync3A::E_SYNC3A_DO_AWB)
    {
        MINT32 i4Master = ISync3AMgr::getInstance()->getAWBMasterDev();
        MINT32 i4Slave = ISync3AMgr::getInstance()->getAWBSlaveDev();
        AWB_SYNC_OUTPUT_N3D_T rAwbSyncMaster, rAwbSyncSlave;
        rAwbSyncMaster.rAWBGain = m_rAwbSyncOutput.main_ch.rAwbGain;
        rAwbSyncMaster.i4CCT    = m_rAwbSyncOutput.main_ch.i4CCT;
        rAwbSyncSlave.rAWBGain  = m_rAwbSyncOutput.sub_ch.rAwbGain;
        rAwbSyncSlave.i4CCT     = m_rAwbSyncOutput.sub_ch.i4CCT;

        IAwbMgr::getInstance().applyAWB(i4Master, rAwbSyncMaster);
        IAwbMgr::getInstance().applyAWB(i4Slave, rAwbSyncSlave);
    }

    CAM_LOGD("[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
Sync3AWrapper::
setDebugInfo(void* prDbgInfo) const
{
    CAM_LOGD_IF(m_Sync3ALogEnable, "[%s] +", __FUNCTION__);

    N3D_DEBUG_INFO_T* prN3dDbgInfo = reinterpret_cast<N3D_DEBUG_INFO_T*>(prDbgInfo);
    if (!prN3dDbgInfo)
    {
        CAM_LOGE("prN3dDbgInfo is NULL!");
        return MFALSE;
    }
    if(m_pMTKSyncAe)
        m_pMTKSyncAe->SyncAeFeatureCtrl(SYNC_AE_FEATURE_GET_DEBUG_INFO, (void*) &prN3dDbgInfo->rAEDebugInfo, NULL);
    if(m_pMTKSyncAwb)
        m_pMTKSyncAwb->SyncAwbFeatureCtrl(SYNC_AWB_FEATURE_GET_DEBUG_INFO, (void*) &prN3dDbgInfo->rAWBDebugInfo, NULL);

    CAM_LOGD_IF(m_Sync3ALogEnable, "[%s] -", __FUNCTION__);
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

VOID
Sync3AWrapper::
syncAeSetting(MINT32 i4MasterDev, MINT32 i4SlaveDev)
{
    SYNC_AE_DIRECT_INDEX_MAPPING_STRUCT rInput;
    SYNC_AE_DIRECT_MAP_RESULT_STRUCT    rOutput;
    AE_MODE_CFG_T rMasterPreviewInfo;
    FrameOutputParam_T rMasterFrameParams;

    // prepare master's input
    const strSyncAEInitInfo* pSyncAEInitInfo = getSyncAEInitInfo();
    strAETable strPreviewAEPlineTableMain, strCaptureAEPlineTabMain;
    strAETable strPreviewAEPlineTableMain2, strCaptureAEPlineTabMain2;
    strAETable strStrobeAEPlineTableMain, strStrobeAEPlineTableMain2;
    strAFPlineInfo strStrobeAFPlineTabMain, strStrobeAFPlineTabMain2;

    IAeMgr::getInstance().getPreviewParams(i4MasterDev, rMasterPreviewInfo);
    IAeMgr::getInstance().getRTParams(i4MasterDev, rMasterFrameParams);
    IAeMgr::getInstance().getCurrentPlineTable(i4MasterDev, strPreviewAEPlineTableMain, strCaptureAEPlineTabMain, strStrobeAEPlineTableMain, strStrobeAFPlineTabMain);
    IAeMgr::getInstance().getCurrentPlineTable(i4SlaveDev, strPreviewAEPlineTableMain2, strCaptureAEPlineTabMain2, strStrobeAEPlineTableMain2, strStrobeAFPlineTabMain2);

    NVRAM_CAMERA_3A_STRUCT* pMaster3aNvram = NULL;
    NVRAM_CAMERA_3A_STRUCT* pSlave3aNvram = NULL;
    NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, i4MasterDev, (void*&)pMaster3aNvram, 0);
    NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, i4SlaveDev, (void*&)pSlave3aNvram, 0);

    rInput.MasterCamAeIdx = rMasterFrameParams.u4AEIndex;
    rInput.MasterCamBVOffset = pMaster3aNvram->rAENVRAM[0].rCCTConfig.i4BVOffset;
    rInput.SlaveCamBVOffset = pSlave3aNvram->rAENVRAM[0].rCCTConfig.i4BVOffset;
    rInput.pMasterAETable = &strPreviewAEPlineTableMain;
    rInput.pSlaveAETable = &strPreviewAEPlineTableMain2;
    CAM_LOGD("[%s] M(%d) : AeIdx(%d)/BVOffset(%d), S(%d) : BVOffset(%d)", __FUNCTION__,
        i4MasterDev, rInput.MasterCamAeIdx, rInput.MasterCamBVOffset,
        i4SlaveDev, rInput.SlaveCamBVOffset);

    // get new setting
    m_pMTKSyncAe->SyncAeFeatureCtrl(SYNC_AE_FEATURE_GET_MAPPING_INDEX, &rInput, &rOutput);

    rMasterPreviewInfo.u4Eposuretime = rOutput.EVSetting.u4Eposuretime;
    rMasterPreviewInfo.u4AfeGain = rOutput.EVSetting.u4AfeGain;
    rMasterPreviewInfo.u4IspGain = rOutput.EVSetting.u4IspGain;
    CAM_LOGD("[%s] Exp(%d), Afe(%d), Isp(%d)", __FUNCTION__, rMasterPreviewInfo.u4Eposuretime, rMasterPreviewInfo.u4AfeGain, rMasterPreviewInfo.u4IspGain);
    // update output to slave
    IAeMgr::getInstance().updatePreviewParams(i4SlaveDev, rMasterPreviewInfo, rOutput.u4SlaveAEindex);
    IAeMgr::getInstance().setExp(i4SlaveDev, rMasterPreviewInfo.u4Eposuretime);
    IAeMgr::getInstance().setAfe(i4SlaveDev, rMasterPreviewInfo.u4AfeGain);
    IAeMgr::getInstance().setIsp(i4SlaveDev, rMasterPreviewInfo.u4IspGain);
}

VOID
Sync3AWrapper::
updateMasterSlave(MINT32 i4Master, MINT32 i4Slave)
{
    m_i4Master = i4Master;
    m_i4Slave = i4Slave;
}

VOID
Sync3AWrapper::
updateFrameSync(MBOOL /*bEnable*/)
{
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

    virtual MINT32              sync(MINT32 i4Sensor, MINT32 i4Param, MVOID* pSttBuf, MINT32 i4FrameNum);

    virtual MINT32              syncAF(MINT32 i4Sensor, MBOOL initLens);

    virtual MVOID               enableSync(MBOOL fgOnOff);

    virtual MBOOL               isSyncEnable() const;

    virtual MINT32              getFrameCount() const {return m_i4SyncFrmCount;}

    virtual MBOOL               isAeStable() const;

    virtual MINT32              getAeSchedule() const;

    virtual MBOOL               setDebugInfo(void* prDbgInfo) const;

    virtual MVOID               enableSyncSupport(E_SYNC3A_SUPPORT iSupport);

    virtual MVOID               disableSyncSupport(E_SYNC3A_SUPPORT iSupport);

    virtual MINT32              getSyncSupport() const;

    virtual MBOOL               isAFSyncFinish();

    virtual MBOOL               is2ASyncFinish();

    virtual MBOOL               isAESyncStable();

    virtual MBOOL               isPerframeAE(){return 0;}    // fixing o1.mp1 build error

    virtual MVOID               sync2ASetting(MINT32 i4MasterIdx, MINT32 i4SlaveIdx);

    virtual MVOID               updateMasterSlave(MINT32 i4Master, MINT32 i4Slave);

    virtual MVOID               updateFrameSync(MBOOL bEnable);

    virtual MBOOL               getRemappingAeSetting(void* pAeSetting) const { return 0; }; // Only use for ISP 3.0, slave need to remapping AE setting

protected:
    Sync3A();
    virtual ~Sync3A();

    MBOOL                       wait(MINT32 i4Sensor);
    MBOOL                       post(MINT32 i4Sensor);
    MBOOL                       resetScheduler();

    mutable Mutex               m_Lock;
    mutable Mutex               m_LockSync3AWrap;
    Condition                   m_Condition;
    MBOOL                       m_fgSyncEnable;
    MBOOL                       m_fgChkSyncEnable;
    MINT32                      m_i4SyncSupport;
    MINT32                      m_i4Count;
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
    // AWB control
    MINT32                      m_i4AwbSyncCount;
    MINT32                      m_Sync3ALogEnable;
    MINT32                      m_i4WaitFramNum;
};

/******************************************************************************
 *  Sync3A Implementation
 ******************************************************************************/
Sync3A::
Sync3A()
    : m_Lock()
    , m_LockSync3AWrap()
    , m_fgSyncEnable(MFALSE)
    , m_fgChkSyncEnable(MFALSE)
    , m_i4SyncSupport(3)
    , m_i4Count(0)
    , m_i4SyncFrmCount(0)
    , m_i4OptFirstIn(0)
    , m_i4Policy(0)
    , m_i4Master(ESensorDevId_Main)
    , m_i4Slave(ESensorDevId_Main)
    , m_i4MasterMagic(0)
    , m_i4SlaveMagic(0)
    , m_pMasterSttBuf(NULL)
    , m_pSlaveSttBuf(NULL)
    , m_pSync3AWrap(NULL)
    , m_i4AeSchedule(0)
    , m_i4AwbSyncCount(0)
    , m_Sync3ALogEnable(0)
    , m_i4WaitFramNum(0)
{
    GET_PROP("vendor.debug.sync3A.log", "0", m_Sync3ALogEnable);
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
    Mutex::Autolock lock(m_LockSync3AWrap);

    m_i4Count = 0;
    m_i4SyncFrmCount = 0;
    m_i4OptFirstIn = 0;
    m_i4Policy = i4Policy;
    m_i4Master = i4Master;
    m_i4Slave = i4Slave;
    m_i4MasterMagic = 0;
    m_i4SlaveMagic = 0;
    m_fgSyncEnable = MFALSE;
    m_fgChkSyncEnable = MFALSE;
    m_i4AeSchedule = 0;
    m_i4SyncSupport = E_SYNC3A_SUPPORT_AE | E_SYNC3A_SUPPORT_AWB;
    m_pMasterSttBuf = NULL;
    m_pSlaveSttBuf = NULL;

    m_strName = strName;

    if(m_pSync3AWrap == NULL)
        m_pSync3AWrap = Sync3AWrapper::getInstance();

    if(m_pSync3AWrap)
        m_pSync3AWrap->init(i4Master, i4Slave);

    CAM_LOGD("[%s] %s: policy(%d), masterDev(%d), slaveDev(%d), m_pSync3AWrap(%p)",
        __FUNCTION__, m_strName.c_str(), i4Policy, i4Master, i4Slave, m_pSync3AWrap);
    return MTRUE;
}

MBOOL
Sync3A::
uninit()
{
    Mutex::Autolock lock(m_LockSync3AWrap);

    m_fgSyncEnable = MFALSE;

    if(m_pSync3AWrap)
    {
        m_pSync3AWrap->uninit();
        m_pSync3AWrap = NULL;
    }
    m_pMasterSttBuf = NULL;
    m_pSlaveSttBuf = NULL;

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
    else
        m_i4AwbSyncCount = 3;
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
    MINT32 i4OldSyncSupport = m_i4SyncSupport;
    m_i4SyncSupport |= iSupport;
    CAM_LOGD("[%s] m_i4SyncSupport (%d -> %d), iSupport(%d)", __FUNCTION__, i4OldSyncSupport, m_i4SyncSupport, iSupport);
}

MVOID
Sync3A::
disableSyncSupport(E_SYNC3A_SUPPORT iSupport)
{
    MINT32 i4OldSyncSupport = m_i4SyncSupport;
    m_i4SyncSupport &= (~iSupport);
    CAM_LOGD("[%s] m_i4SyncSupport (%d -> %d), iSupport(%d)", __FUNCTION__, i4OldSyncSupport, m_i4SyncSupport, iSupport);
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
    if(getSyncSupport() & E_SYNC3A_SUPPORT_AF)
    {
        CAM_LOGD("[%s] isFocusFinish = (%d,%d)", __FUNCTION__,
            IAfMgr::getInstance().isFocusFinish(m_i4Master),
            IAfMgr::getInstance().isFocusFinish(m_i4Slave));
        return (IAfMgr::getInstance().isFocusFinish(m_i4Master)
             && IAfMgr::getInstance().isFocusFinish(m_i4Slave));
    }
    return MFALSE;
}

MBOOL
Sync3A::
is2ASyncFinish()
{
    if( getSyncSupport() & (E_SYNC3A_SUPPORT_AE | E_SYNC3A_SUPPORT_AWB))
    {
        CAM_LOGD_IF(m_i4AwbSyncCount, "[%s] m_i4AwbSyncCount = (%d)", __FUNCTION__, m_i4AwbSyncCount);
        if(m_i4AwbSyncCount == 0)
            return MTRUE;
    }
    return MFALSE;
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
sync(MINT32 i4Sensor, MINT32 i4Param, MVOID* pSttBuf, MINT32 i4FrameNum)
{
    MINT32 i4Ret = 0;
    MINT32 i4SemVal;
    static MINT32 i4TimeCount = 0;

    Mutex::Autolock autoLock(m_Lock);
    i4SemVal = m_i4Count;

    if(i4Sensor == m_i4Master)
       m_pMasterSttBuf = reinterpret_cast<StatisticBufInfo*>(pSttBuf);
    else if(i4Sensor == m_i4Slave)
       m_pSlaveSttBuf = reinterpret_cast<StatisticBufInfo*>(pSttBuf);

    if(!(i4Param & E_SYNC3A_DO_SW_SYNC))
    {
        MBOOL bIsHwSync = MFALSE;
        if(i4Param & E_SYNC3A_DO_HW_SYNC)
            bIsHwSync = MTRUE;

        MINT32 i4BypSyncCalc = 0;
        GET_PROP("vendor.debug.sync2a.byp", "0", i4BypSyncCalc);
        // Slave do AE sync
        if( (i4Sensor == m_i4Slave && !bIsHwSync) ||
            (i4Sensor == m_i4Master && bIsHwSync) )
        {
            MINT32 i4Opt = i4Param & (~i4BypSyncCalc);
            MINT32 i4OldOpt = i4Opt;
            // if disable AE sync, remove AE opt
            if(!(m_i4SyncSupport & E_SYNC3A_SUPPORT_AE))
            {
                i4Opt &= (~ISync3A::E_SYNC3A_DO_AE);
                i4Opt &= (~ISync3A::E_SYNC3A_DO_AE_PRECAP);
            }
            // remove AWB opt
            i4Opt &= (~ISync3A::E_SYNC3A_DO_AWB);
            CAM_LOGD("[%s] m_i4SyncSupport(%d), AE i4Opt(%d -> %d)", __FUNCTION__, m_i4SyncSupport, i4OldOpt, i4Opt);

            m_LockSync3AWrap.lock();
            if(m_pSync3AWrap)
                m_pSync3AWrap->syncCalc(i4Opt, m_pMasterSttBuf, m_pSlaveSttBuf);
            m_LockSync3AWrap.unlock();

            if (bIsHwSync && (m_i4AeSchedule == 1)) // AE calculation frame
            {
                // 2A frame sync
                AE_MODE_CFG_T rMainPreviewParams;
                AE_MODE_CFG_T rMain2PreviewParams;
                IAeMgr::getInstance().getPreviewParams(m_i4Master, rMainPreviewParams);
                IAeMgr::getInstance().getPreviewParams(m_i4Slave, rMain2PreviewParams);
                AAASensorMgr::getInstance().updateStereoFrameSyncFps(m_i4Master, m_i4Slave,rMainPreviewParams.u4Eposuretime,rMain2PreviewParams.u4Eposuretime);
            }
        }
        if(i4Sensor == m_i4Master)
        {
            m_i4AeSchedule = (m_i4AeSchedule + 1) % 3;
        }

        // Slave do AWB sync
        if(i4Sensor != ISync3AMgr::getInstance()->getAWBMasterDev())
        {
            MINT32 i4Opt = i4Param & (~i4BypSyncCalc);
            MINT32 i4OldOpt = i4Opt;
            // remove AE opt
            i4Opt &= (~ISync3A::E_SYNC3A_DO_AE);
            i4Opt &= (~ISync3A::E_SYNC3A_DO_AE_PRECAP);
            // if disable AWB sync, remove AWB opt
            if(!(m_i4SyncSupport & E_SYNC3A_SUPPORT_AWB))
                i4Opt &= (~ISync3A::E_SYNC3A_DO_AWB);
            CAM_LOGD("[%s] m_i4SyncSupport(%d), AWB i4Opt(%d -> %d)", __FUNCTION__, m_i4SyncSupport, i4OldOpt, i4Opt);

            m_LockSync3AWrap.lock();
            if(m_pSync3AWrap)
                m_pSync3AWrap->syncCalc(i4Opt, m_pMasterSttBuf, m_pSlaveSttBuf);
            m_LockSync3AWrap.unlock();

            if(m_i4AwbSyncCount > 0)
                m_i4AwbSyncCount--;
        }
    } else
    {
        if (i4SemVal >= 0)
        {
            m_i4Count --;
            i4Ret = 0;

            //Save wait_FrameNum
            if(i4Sensor == m_i4Master)
               m_i4WaitFramNum = i4FrameNum;
            else if(i4Sensor == m_i4Slave)
               m_i4WaitFramNum = i4FrameNum;
            CAM_LOGD_IF(m_Sync3ALogEnable, "[%s](0) %s: eSensor(%d) Wait(%d) WaitFrm(#%d) i4Param(0x%08x)", __FUNCTION__, m_strName.c_str(), i4Sensor, m_i4Count, m_i4WaitFramNum, i4Param);
        }
        else
        {
            //compare post_FrameNum with wait_FrameNum
            if(i4FrameNum == m_i4WaitFramNum)
            {
                m_i4Count ++;
                CAM_LOGD_IF(m_Sync3ALogEnable, "[%s](2) %s: eSensor(%d) Post(%d) PostFrm(#%d) i4Param(0x%08x)", __FUNCTION__, m_strName.c_str(), i4Sensor, m_i4Count, i4FrameNum, i4Param);
                i4Ret = 1;
            }
            else
            {
                CAM_LOGW("[%s] Case1: PostFrm(#%d) isn't equal to WaitFrm(#%d)\n", __FUNCTION__, i4FrameNum, m_i4WaitFramNum);
                i4TimeCount++;
                if (i4TimeCount > 3)
                {
                    m_i4Count ++;
                    i4TimeCount =0;
                }
            }
        }

        if (i4Ret == 0)
        {
            m_i4OptFirstIn = i4Param;
            wait(i4Sensor);
        }
        else if(i4Ret == 1)
        {
            if ((i4Param|m_i4OptFirstIn) & ISync3A::E_SYNC3A_BYP_AE)
            {
                CAM_LOGD("%s: Ignore AEAWB: eSensor(%d), Opt(0x%08x), another Opt(0x%08x)", m_strName.c_str(), i4Sensor, i4Param, m_i4OptFirstIn);
                i4Param &= (~(ISync3A::E_SYNC3A_DO_AE|ISync3A::E_SYNC3A_DO_AWB));
            }
            else if((i4Param|m_i4OptFirstIn) & ISync3A::E_SYNC3A_DO_AE)
            {
                i4Param |= (ISync3A::E_SYNC3A_DO_AE);
            }

            MINT32 i4BypSyncCalc = 0;
            GET_PROP("vendor.debug.sync2a.byp", "0", i4BypSyncCalc);

            // calculation
            CAM_LOGD_IF(m_Sync3ALogEnable, "[%s](3) %s: Sync 2A: Sensor(%d) Calculation", __FUNCTION__, m_strName.c_str(), i4Sensor);

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

            m_LockSync3AWrap.lock();
            if(m_pSync3AWrap)
                m_pSync3AWrap->syncCalc(i4Opt, m_pMasterSttBuf, m_pSlaveSttBuf);
            m_LockSync3AWrap.unlock();

            m_i4AeSchedule = (m_i4AeSchedule + 1) % 3;
            if(m_i4AwbSyncCount > 0)
                m_i4AwbSyncCount--;

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
            m_i4WaitFramNum = 0;
        }
        else
            CAM_LOGW("[%s] Case1: PostFrm(#%d) isn't equre WaitFrm(#%d)\n", __FUNCTION__, i4FrameNum, m_i4WaitFramNum);
    }
    return i4Ret;
}

MINT32
Sync3A::
syncAF(MINT32 i4Sensor, MBOOL initLens)
{
    MINT32 i4Ret = 0, syncPos=0;
    if((i4Sensor == m_i4Master && (getSyncSupport() & E_SYNC3A_SUPPORT_AF)) || initLens)
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
            // 0 : off, 1 : VSDOF, 2, Denoise/Dual zoom
            Stereo_Param_T rParams = ISync3AMgr::getInstance()->getStereoParams();
            switch(rParams.i4Sync2AMode)
            {
                case NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_VSDOF:
                    IAfMgr::getInstance().SyncAFSetMode(m_i4Slave,1);
                    break;
                case NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DENOISE:
                    IAfMgr::getInstance().SyncAFSetMode(m_i4Slave,2);
                    break;
                case NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DUAL_ZOOM:
                    IAfMgr::getInstance().SyncAFSetMode(m_i4Slave,3);
                    break;
                default:
                    IAfMgr::getInstance().SyncAFSetMode(m_i4Slave,0);
                    break;
            }

            AF_SyncInfo_T sCamInfo;
            memset(&sCamInfo,  0, sizeof(sCamInfo));

            if(initLens)
                sCamInfo.i4SyncStatus = SYNC_STATUS_INITLENS;

            IAfMgr::getInstance().SyncAFGetMotorRange(m_i4Slave, sCamInfo);
            syncPos = IAfMgr::getInstance().SyncAFGetInfo(m_i4Master, sCamInfo);
            CAM_LOGD("[syncPos]%d [MPos]%d [Sdone]%d",
                syncPos,
                IAfMgr::getInstance().getAFPos(m_i4Master),
                IAfMgr::getInstance().isFocusFinish(m_i4Slave));
            if(syncPos!=0xFFFF)
            {
#if (!MT6763_API) && (!CAM3_3A_ISP_40_PLATFORM_MIX_VER_1)
                IAfMgr::getInstance().SyncAFSetInfo(m_i4Slave, syncPos, sCamInfo);
#else
                IAfMgr::getInstance().SyncAFSetInfo(m_i4Slave, syncPos);
#endif
                IAfMgr::getInstance().MoveLensTo(m_i4Slave, syncPos);
            }
            else
                CAM_LOGD("[syncPos] err");

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
        MINT32 i4Ret = m_Condition.waitRelative(m_Lock, SEM_TIME_LIMIT_NS);
        if(i4Ret != 0)
            CAM_LOGD("[%s] %s: eSensor(%d), i4Ret(%d), TimeOut(%d ns)", __FUNCTION__, m_strName.c_str(), i4Sensor, i4Ret, SEM_TIME_LIMIT_NS);
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
    CAM_LOGD_IF(m_Sync3ALogEnable, "[%s](4) %s: eSensor(%d)", __FUNCTION__, m_strName.c_str(), i4Sensor);
    m_i4SyncFrmCount ++;
    m_Condition.signal();
    return MTRUE;
}

MBOOL
Sync3A::
resetScheduler()
{
    CAM_LOGD("[%s]", __FUNCTION__);
    m_i4AeSchedule = 0;
    return MTRUE;
}

MBOOL
Sync3A::
isAeStable() const
{
    Mutex::Autolock lock(m_LockSync3AWrap);

    if(m_pSync3AWrap)
        return m_pSync3AWrap->isAeStable();

    return MFALSE;
}

MBOOL
Sync3A::
setDebugInfo(void* prDbgInfo) const
{
    Mutex::Autolock lock(m_LockSync3AWrap);

    if(m_pSync3AWrap)
        return m_pSync3AWrap->setDebugInfo(prDbgInfo);

    return MFALSE;
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

MVOID
Sync3A::
sync2ASetting(MINT32 i4MasterIdx, MINT32 i4SlaveIdx)
{
    Mutex::Autolock lock(m_LockSync3AWrap);

    if(m_pSync3AWrap == NULL)
        m_pSync3AWrap = Sync3AWrapper::getInstance();
    if(m_pSync3AWrap)
    {
        IHalSensorList* pHalSensorList = MAKE_HalSensorList();
        if (!pHalSensorList) return;

        MINT32 i4MasterDev = pHalSensorList->querySensorDevIdx(i4MasterIdx);
        MINT32 i4SlaveDev = pHalSensorList->querySensorDevIdx(i4SlaveIdx);

        m_pSync3AWrap->init(i4MasterDev, i4SlaveDev);
        m_pSync3AWrap->syncAeSetting(i4MasterDev, i4SlaveDev);
        m_pSync3AWrap->uninit();
    }
}

MVOID
Sync3A::
updateMasterSlave(MINT32 i4Master, MINT32 i4Slave)
{
    m_i4Master = i4Master;
    m_i4Slave = i4Slave;
    Mutex::Autolock lock(m_LockSync3AWrap);
    if(m_pSync3AWrap)
        m_pSync3AWrap->updateMasterSlave(i4Master,i4Slave);
}

MVOID
Sync3A::
updateFrameSync(MBOOL /*bEnable*/)
{
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

    virtual MVOID               DevCount(MBOOL bEnable, MINT32 i4SensorDev);

    virtual MBOOL               isInit() const;

    virtual MBOOL               isActive() const;

    virtual MINT32              getMasterDev() const {return m_i4Master;}

    virtual MINT32              getSlaveDev() const {return m_i4Slave;}

    virtual MVOID               setAFState(MINT32 i4AfState);

    virtual MINT32              getAFState() const {return m_fgAfState;}

    virtual MVOID               setStereoParams(Stereo_Param_T const &rNewParam);

    virtual Stereo_Param_T      getStereoParams() const {return m_rStereoParams;}

    virtual MINT32              getFrmSyncOpt() const;

    virtual MINT32              getAWBMasterDev() const;

    virtual MINT32              getAWBSlaveDev() const;

    // only for 97, fixing build error
    virtual MVOID               setAFSyncMode(MINT32){return ;}

    virtual MVOID               enable(){return ;}

    virtual MVOID               disable(){return ;}

    virtual MVOID               updateInitParams(){return ;}

    virtual MVOID               setManualControl(MBOOL){return ;}

    virtual MBOOL               isManualControl(){return 0;}

private:
    virtual MINT32              querySensorType(MINT32 i4SensorDev);

protected:
    Sync3AMgr()
        : m_fgIsActive(MFALSE)
        , m_i4DevCount(0)
        , m_fgAfState(MFALSE)
        , m_i4Master(ESensorDevId_Main)
        , m_i4Slave(ESensorDevId_Main)
        , m_i4MasterSensorType(0)
        , m_i4SlaveSensorType(0)
        , m_eCapMode(E_SYNC3AMGR_CAPMODE_2D)
        {}
    virtual ~Sync3AMgr(){}

    MBOOL                       m_fgIsActive;
    MINT32                      m_i4DevCount;
    MBOOL                       m_fgAfState;
    MINT32                      m_i4Master;
    MINT32                      m_i4Slave;
    MINT32                      m_i4MasterSensorType;
    MINT32                      m_i4SlaveSensorType;
    E_SYNC3AMGR_CAPMODE         m_eCapMode;
    Stereo_Param_T              m_rStereoParams;
    AF_SyncInfo_T               m_sCamInfoHistWide;
    AF_SyncInfo_T               m_sCamInfoHistTele;
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

    SensorStaticInfo sensorStaticInfo;
    IHalSensorList* pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList) return MFALSE;

    m_i4Master = pHalSensorList->querySensorDevIdx(i4MasterIdx);
    m_i4Slave = pHalSensorList->querySensorDevIdx(i4SlaveIdx);
    m_i4MasterSensorType = querySensorType(m_i4Master);
    m_i4SlaveSensorType = querySensorType(m_i4Slave);

    FlashHal::getInstance(m_i4Master)->setInCharge(MTRUE);
    FlashHal::getInstance(m_i4Slave)->setInCharge(MFALSE);
    FlashMgr::getInstance(m_i4Master)->setInCharge(MTRUE);
    FlashMgr::getInstance(m_i4Slave)->setInCharge(MFALSE);

    IFlickerHal::getInstance(m_i4Master)->stop();
    IFlickerHal::getInstance(m_i4Slave)->stop();
    IFlickerHal::getInstance(m_i4Master)->start(FLK_ATTACH_PRIO_HIGH);
    IFlickerHal::getInstance(m_i4Slave)->start(FLK_ATTACH_PRIO_LOW);

    CAM_LOGD("[%s] Master(%d,%d), Slave(%d,%d)", __FUNCTION__,
        m_i4Master, m_i4MasterSensorType,
        m_i4Slave, m_i4SlaveSensorType);

    Sync3A::getInstance(E_SYNC3AMGR_PRVIEW)->init(i4Policy, m_i4Master, m_i4Slave, "Preview");
    Sync3A::getInstance(E_SYNC3AMGR_CAPTURE)->init(i4Policy, m_i4Master, m_i4Slave, "Capture");
    Sync3A::getInstance(E_SYNC3AMGR_PRVIEW)->enableSync(MTRUE);
    Sync3A::getInstance(E_SYNC3AMGR_CAPTURE)->enableSync(MTRUE);

    // AWB control setting
    MBOOL isMono = MFALSE;
    if( m_i4MasterSensorType == SENSOR_RAW_MONO)
    {
        IAwbMgr::getInstance().disableAWB(m_i4Master);
        IAfMgr::getInstance().setAFMode(m_i4Master, MTK_CONTROL_AF_MODE_OFF);
        isMono = MTRUE;
    } else if(m_i4SlaveSensorType == SENSOR_RAW_MONO)
    {
        IAwbMgr::getInstance().disableAWB(m_i4Slave);
        IAfMgr::getInstance().setAFMode(m_i4Slave, MTK_CONTROL_AF_MODE_OFF);
        isMono = MTRUE;
    }

    if(isMono)
    {
        // disable AWB sync
        Sync3A::getInstance(E_SYNC3AMGR_PRVIEW)->disableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AWB);
        Sync3A::getInstance(E_SYNC3AMGR_CAPTURE)->disableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AWB);

        // enable AF sync
        Sync3A::getInstance(E_SYNC3AMGR_PRVIEW)->enableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AF);
        Sync3A::getInstance(E_SYNC3AMGR_CAPTURE)->enableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AF);
    }
    else
    {
        // enable AWB sync
        Sync3A::getInstance(E_SYNC3AMGR_PRVIEW)->enableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AWB);
        Sync3A::getInstance(E_SYNC3AMGR_CAPTURE)->enableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AWB);
    }

    // awb ROI for main2
    auto MainSensorVerFOV = NSCamCustomSensor::getSensorViewAngle_V((IMGSENSOR_SENSOR_IDX)i4MasterIdx);
    auto MainSensorHorFOV = NSCamCustomSensor::getSensorViewAngle_H((IMGSENSOR_SENSOR_IDX)i4MasterIdx);
    auto Main2SensorVerFOV = NSCamCustomSensor::getSensorViewAngle_V((IMGSENSOR_SENSOR_IDX)i4SlaveIdx);
    auto Main2SensorHorFOV = NSCamCustomSensor::getSensorViewAngle_H((IMGSENSOR_SENSOR_IDX)i4SlaveIdx);

    if ((Main2SensorVerFOV < MainSensorVerFOV) ||
        (Main2SensorHorFOV < MainSensorHorFOV))
    {
        CAM_LOGD("[%s] Not necessary to config window MainFOV(%d,%d) Main2FOV(%d,%d)",
            __FUNCTION__, MainSensorHorFOV, MainSensorVerFOV, Main2SensorHorFOV, Main2SensorVerFOV);
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
                MainSensorVerFOV,
                MainSensorHorFOV,
                Main2SensorVerFOV,
                Main2SensorHorFOV,
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
        Sync3A::getInstance(E_SYNC3AMGR_PRVIEW)->enableSync(MFALSE);
        Sync3A::getInstance(E_SYNC3AMGR_CAPTURE)->enableSync(MFALSE);
        Sync3A::getInstance(E_SYNC3AMGR_PRVIEW)->uninit();
        Sync3A::getInstance(E_SYNC3AMGR_CAPTURE)->uninit();

        m_rStereoParams.i4Sync2AMode = NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_NONE;
        m_rStereoParams.i4SyncAFMode = NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_OFF;
        m_rStereoParams.i4HwSyncMode = NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_OFF;
        m_fgIsActive = MFALSE;

        IAeMgr::getInstance().enableAEStereoManualPline(m_i4Master, MFALSE);
        IAeMgr::getInstance().enableAEStereoManualPline(m_i4Slave, MFALSE);

        // reset Master AF sync status
        AF_SyncInfo_T sCamInfo;
        memset(&sCamInfo, 0, sizeof(sCamInfo));
        IAfMgr::getInstance().SyncAFGetInfo(m_i4Master, sCamInfo);

        // reset aaa sensor status
        AAASensorMgr::getInstance().disableFrameSync();
    }
    return MTRUE;
}

MVOID
Sync3AMgr::
DevCount(MBOOL bEnable, MINT32 i4SensorDev)
{
    CAM_LOGD("[%s] bEnable(%d) m_i4DevCount(%d) i4SensorDev(%d) +", __FUNCTION__, bEnable, m_i4DevCount, i4SensorDev);
    if(bEnable)
        m_i4DevCount++;
    else
        m_i4DevCount--;
    CAM_LOGD("[%s] bEnable(%d) m_i4DevCount(%d) i4SensorDev(%d) -", __FUNCTION__, bEnable, m_i4DevCount, i4SensorDev);
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
    return (m_fgIsActive && m_i4DevCount == 2);
}

MVOID
Sync3AMgr::
setAFState(MINT32 i4AfState)
{
    if(m_fgAfState != i4AfState)
    {
        CAM_LOGD("[%s] State(%d -> %d)", __FUNCTION__, m_fgAfState, i4AfState);
        IAeMgr::getInstance().enableAEOneShotControl(m_i4Slave, (i4AfState != 0)?MTRUE:MFALSE);
        m_fgAfState = i4AfState;
    }
}

MVOID
Sync3AMgr::
setStereoParams(Stereo_Param_T const &rNewParam)
{
    MINT32 i4Sync2AMode = rNewParam.i4Sync2AMode;
    MINT32 i4SyncAFMode = rNewParam.i4SyncAFMode;
    MINT32 i4HwSyncMode = rNewParam.i4HwSyncMode;
    MINT32 i4MasterIdx = rNewParam.i4MasterIdx;
    MINT32 i4SlaveIdx = rNewParam.i4SlaveIdx;

    if(i4MasterIdx != 0 || i4SlaveIdx != 0)
    {
        IHalSensorList* pHalSensorList = MAKE_HalSensorList();
        if (!pHalSensorList) return;

        MINT32 newMasterDev = pHalSensorList->querySensorDevIdx(i4MasterIdx);
        MINT32 newSlaveDev = pHalSensorList->querySensorDevIdx(i4SlaveIdx);
        if(m_i4Master != newMasterDev || m_i4Slave != newSlaveDev)
        {
            CAM_LOGD("[%s] M&S : old(%d,%d), new(%d,%d)", __FUNCTION__,
                m_i4Master, m_i4Slave,
                newMasterDev, newSlaveDev);
            m_i4Master = newMasterDev;
            m_i4Slave = newSlaveDev;

            m_rStereoParams.i4MasterIdx = i4MasterIdx;
            m_rStereoParams.i4SlaveIdx= i4SlaveIdx;
            //W+T
            FlashHal::getInstance(m_i4Master)->setInCharge(MTRUE);
            FlashHal::getInstance(m_i4Slave)->setInCharge(MFALSE);
            FlashMgr::getInstance(m_i4Master)->setInCharge(MTRUE);
            FlashMgr::getInstance(m_i4Slave)->setInCharge(MFALSE);

            IFlickerHal::getInstance(m_i4Master)->stop();
            IFlickerHal::getInstance(m_i4Slave)->stop();
            IFlickerHal::getInstance(m_i4Master)->start(FLK_ATTACH_PRIO_HIGH);
            IFlickerHal::getInstance(m_i4Slave)->start(FLK_ATTACH_PRIO_LOW);

            IAfMgr::getInstance().SyncAFSetMode(m_i4Slave,0xFE);
            IAfMgr::getInstance().SyncAFSetMode(m_i4Master,0xFF);

            Sync3A::getInstance(E_SYNC3AMGR_PRVIEW)->updateMasterSlave(m_i4Master, m_i4Slave);
            Sync3A::getInstance(E_SYNC3AMGR_CAPTURE)->updateMasterSlave(m_i4Master, m_i4Slave);
        }
#if (!MT6763_API) && (!CAM3_3A_ISP_40_PLATFORM_MIX_VER_1)
        //W+T store master history and sent slave history
        if(m_rStereoParams.i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DUAL_ZOOM)
        {
            if(m_i4Master == ESensorDevId_Main)
            {
                if(m_i4Master == newMasterDev || m_i4Slave == newSlaveDev)
                {
                    IAfMgr::getInstance().SyncAFGetInfo(m_i4Master, Sync3AMgr::getInstance()->m_sCamInfoHistWide);// store master_Wide cam info
                    IAfMgr::getInstance().SyncAFSetInfo(m_i4Master, 0xFFFF, m_sCamInfoHistTele);// sent slave history to master
                }
            }
            else if(m_i4Master == ESensorDevId_MainSecond)
            {
                if(m_i4Master == newMasterDev || m_i4Slave == newSlaveDev)
                {
                    IAfMgr::getInstance().SyncAFGetInfo(m_i4Master, Sync3AMgr::getInstance()->m_sCamInfoHistTele);// store master_Tele cam info
                    IAfMgr::getInstance().SyncAFSetInfo(m_i4Master, 0xFFFF, m_sCamInfoHistWide);// sent slave history to master
                }
            }
        }
#endif
    }

    if(i4Sync2AMode != NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_IDLE && m_rStereoParams.i4Sync2AMode != i4Sync2AMode)
    {
        CAM_LOGD("[%s] Sync2AMode : old(%d), new(%d)", __FUNCTION__, m_rStereoParams.i4Sync2AMode, i4Sync2AMode);

        if(i4Sync2AMode != E_SYNC2A_MODE_NONE)
        {
            CAM_LOGD("[%s] enableAEStereoManualPline master(%d)/slave(%d):1", __FUNCTION__,m_i4Master, m_i4Slave);
            IAeMgr::getInstance().enableAEStereoManualPline(m_i4Master, MTRUE);
            IAeMgr::getInstance().enableAEStereoManualPline(m_i4Slave, MTRUE);
        }
        else
        {
            CAM_LOGD("[%s] enableAEStereoManualPline (%d)/slave(%d):0", __FUNCTION__,m_i4Master, m_i4Slave);
            IAeMgr::getInstance().enableAEStereoManualPline(m_i4Master, MFALSE);
            IAeMgr::getInstance().enableAEStereoManualPline(m_i4Slave, MFALSE);
        }

        m_rStereoParams.i4Sync2AMode = static_cast<NS3Av3::E_SYNC2A_MODE>(i4Sync2AMode);
        if(m_rStereoParams.i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DUAL_ZOOM)
        {
#if DUALZOOM_WIDE_STANDY_EN
            Sync3A::getInstance(E_SYNC3AMGR_PRVIEW)->disableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AWB);
            Sync3A::getInstance(E_SYNC3AMGR_CAPTURE)->disableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AWB);
#else
            Sync3A::getInstance(E_SYNC3AMGR_PRVIEW)->enableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AWB);
            Sync3A::getInstance(E_SYNC3AMGR_CAPTURE)->enableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AWB);
#endif
        }
    }

    if(i4SyncAFMode != NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_IDLE && m_rStereoParams.i4SyncAFMode != i4SyncAFMode)
    {
        CAM_LOGD("[%s] SyncAFMode : old(%d), new(%d)", __FUNCTION__, m_rStereoParams.i4SyncAFMode, i4SyncAFMode);
        m_rStereoParams.i4SyncAFMode = static_cast<NS3Av3::E_SYNCAF_MODE>(i4SyncAFMode);
        if(m_rStereoParams.i4SyncAFMode == NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_ON)
        {
            if(m_rStereoParams.i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DUAL_ZOOM)
            {
                if(IAfMgr::getInstance().isFocusFinish(m_i4Master))
                    Sync3A::getInstance(E_SYNC3AMGR_PRVIEW)->syncAF(m_i4Master,MTRUE);
            }
            else
            {
                Sync3A::getInstance(E_SYNC3AMGR_PRVIEW)->enableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AF);
                Sync3A::getInstance(E_SYNC3AMGR_CAPTURE)->enableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AF);
            }
        } else if(m_rStereoParams.i4SyncAFMode == NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_OFF)
        {
            Sync3A::getInstance(E_SYNC3AMGR_PRVIEW)->disableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AF);
            Sync3A::getInstance(E_SYNC3AMGR_CAPTURE)->disableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AF);

            // reset Master AF sync status
            AF_SyncInfo_T sCamInfo;
            memset(&sCamInfo, 0, sizeof(sCamInfo));
            IAfMgr::getInstance().SyncAFGetInfo(m_i4Master, sCamInfo);
        }
    }

    if(i4HwSyncMode != NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_IDLE && m_rStereoParams.i4HwSyncMode != i4HwSyncMode)
    {
        CAM_LOGD("[%s] SyncHwMode : old(%d), new(%d)", __FUNCTION__, m_rStereoParams.i4HwSyncMode, i4HwSyncMode);
        m_rStereoParams.i4HwSyncMode = static_cast<NS3Av3::E_HW_FRM_SYNC_MODE>(i4HwSyncMode);
        if(m_rStereoParams.i4HwSyncMode == NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_OFF)
        {
            AAASensorMgr::getInstance().disableFrameSync();
            Sync3A::getInstance(E_SYNC3AMGR_PRVIEW)->updateFrameSync(MFALSE);
        }
        else
            Sync3A::getInstance(E_SYNC3AMGR_PRVIEW)->updateFrameSync(MTRUE);
    }
}

MINT32
Sync3AMgr::
getFrmSyncOpt() const
{
    MINT32 opt = 0;
    if (m_rStereoParams.i4HwSyncMode == NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_ON)
    {
        opt |= ISync3A::E_SYNC3A_DO_HW_SYNC;
        if( m_rStereoParams.i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_VSDOF ||
            m_rStereoParams.i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DENOISE ||
            m_rStereoParams.i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DUAL_ZOOM)
        {
            opt |= ISync3A::E_SYNC3A_DO_SW_SYNC;
        }
    }
    return opt;
}

MINT32
Sync3AMgr::
getAWBMasterDev() const
{
    // TODO : query sensor type to decide which sensor is master.
    return m_i4Master;
}

MINT32
Sync3AMgr::
getAWBSlaveDev() const
{
    // TODO : query sensor type to decide which sensor is slave.
    return m_i4Slave;
}



MINT32
Sync3AMgr::
querySensorType(MINT32 i4SensorDev)
{
    SensorStaticInfo sensorStaticInfo;
    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList)
    {
        CAM_LOGE("MAKE_HalSensorList() == NULL");
        return 0;
    }
    pHalSensorList->querySensorStaticInfo(i4SensorDev,&sensorStaticInfo);

    return sensorStaticInfo.rawFmtType; // SENSOR_RAW_MONO or SENSOR_RAW_Bayer
}

