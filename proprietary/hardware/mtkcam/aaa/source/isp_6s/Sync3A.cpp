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

#define SYNC3A_TAG          "Sync3A"
#define SYNC3AWRAPPER_TAG   "Sync3AWrapper"
#define SYNC3AMGR_TAG       "Sync3AMgr"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <mtkcam/utils/std/Log.h>
#include <property_utils.h>
#include <aaa_types.h>
#include <ISync3A.h>
#include <aaa_scheduling_custom.h>

#include "aaa/MTKSyncAe.h"
#include "MTKSyncAwb.h"

#include "Local.h"
#include <ae_mgr/ae_mgr_if.h>
#include <awb_mgr/awb_mgr_if.h>
#include <isp_mgr/isp_mgr.h>
#include <sensor_mgr/aaa_sensor_mgr.h>

#include <nvbuf_util.h>

#include "n3d_sync2a_tuning_param.h"
#include <camera_custom_imgsensor_cfg.h>
#include <mtkcam/drv/IHalSensor.h>
#include <isp_tuning/isp_tuning_mgr.h>
#include <isp_tuning.h>

#include <debug_exif/dbg_id_param.h>        // for DEBUG_XXX_KEY_ID
#include <dbg_cam_param.h>
#include <dbg_cam_n3d_param.h>

#include <string>
#include <math.h>

#include <dbg_aaa_param.h>
#include <dbg_af_param.h>
#include <camera_custom_nvram.h>
#include <af_feature.h>
extern "C" {
#include <af_algo_if.h>
}
#include <af_mgr_if.h>
#include <StatisticBuf.h>
#include <ispif.h>
#include <flash_hal.h>
#include <flash_mgr.h>
#include <flicker_hal_if.h>
// for Sync AE of CCU version
#include <iccu_ctrl_3actrl.h>
#include <ccu_ext_interface/ccu_ae_extif.h>

// for the FO of Wide standby mode
#include <camera_custom_dualzoom.h>

// ResultPool
#include <IResultPool.h>

using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSCcuIf;

// SYNC3A_TAG
#define SYNC3A_LOGD(fmt, arg...) \
    do { \
        CAM_LOGD("[%s]" fmt, SYNC3A_TAG, ##arg); \
    }while(0)
#define SYNC3A_LOGW(fmt, arg...) \
    do { \
        CAM_LOGW("[%s]" fmt, SYNC3A_TAG, ##arg); \
    }while(0)
#define SYNC3A_LOGE(fmt, arg...) \
    do { \
        CAM_LOGE("[%s]" fmt, SYNC3A_TAG, ##arg); \
    }while(0)
#define SYNC3A_LOGD_IF(cond, fmt, arg...) \
    do { \
        if ( (cond) ){CAM_LOGD("[%s]" fmt, SYNC3A_TAG, ##arg); } \
    }while(0)
// SYNC3AWRAPPER_TAG
#define SYNC3AWRAPPER_LOGD(fmt, arg...) \
    do { \
        CAM_LOGD("[%s]" fmt, SYNC3AWRAPPER_TAG, ##arg); \
    }while(0)
#define SYNC3AWRAPPER_LOGW(fmt, arg...) \
    do { \
        CAM_LOGW("[%s]" fmt, SYNC3AWRAPPER_TAG, ##arg); \
    }while(0)
#define SYNC3AWRAPPER_LOGE(fmt, arg...) \
    do { \
        CAM_LOGE("[%s]" fmt, SYNC3AWRAPPER_TAG, ##arg); \
    }while(0)
#define SYNC3AWRAPPER_LOGD_IF(cond, fmt, arg...) \
    do { \
        if ( (cond) ){CAM_LOGD("[%s]" fmt, SYNC3AWRAPPER_TAG, ##arg); } \
    }while(0)
// SYNC3AMGR_TAG
#define SYNC3AMGR_LOGD(fmt, arg...) \
    do { \
        CAM_LOGD("[%s]" fmt, SYNC3AMGR_TAG, ##arg); \
    }while(0)
#define SYNC3AMGR_LOGW(fmt, arg...) \
    do { \
        CAM_LOGW("[%s]" fmt, SYNC3AMGR_TAG, ##arg); \
    }while(0)
#define SYNC3AMGR_LOGE(fmt, arg...) \
    do { \
        CAM_LOGE("[%s]" fmt, SYNC3AMGR_TAG, ##arg); \
    }while(0)
#define SYNC3AMGR_LOGD_IF(cond, fmt, arg...) \
    do { \
        if ( (cond) ){CAM_LOGD("[%s]" fmt, SYNC3AMGR_TAG, ##arg); } \
    }while(0)


#define SEM_TIME_LIMIT_NS       1000000000L

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

    width  = (int)( ::tan(main1_FOV_horizontal / 180.0f * (float)::acos(0.0f)) / ::tan(main2_FOV_horizontal / 180.0f * (float)::acos(0.0f)) * tg_size_w / 2 ) * 2 ;
    height = (int)( ::tan(main1_FOV_vertical / 180.0f * (float)::acos(0.0f)) / ::tan(main2_FOV_vertical / 180.0f * (float)::acos(0.0f)) * tg_size_h / 2 ) * 2 ;
    offset_x = ((( tg_size_w - width ) >> 2 ) << 1) ;
    offset_y = ((( tg_size_h - height) >> 2 ) << 1) ;

    CAM_LOGD("[%s] FOV1_V(%3.3f), FOV1_H(%3.3f), FOV2_V(%3.3f), FOV2_H(%3.3f), tg(%d, %d)", __FUNCTION__,
             main1_FOV_vertical, main1_FOV_horizontal, main2_FOV_vertical, main2_FOV_horizontal, tg_size_w, tg_size_h);

    CropResult.p.x = offset_x;
    CropResult.p.y = offset_y;

    CropResult.s.w = width;
    CropResult.s.h = height;

    return CropResult;
}

/******************************************************************************
 *  Sync3AMgr : This class is save stereo data
 ******************************************************************************/
class Sync3AMgr
{
public:
    /**
     * get singleton.
     */
    static Sync3AMgr*           getInstance();

    /**
     * @brief Explicitly init 3A N3D Sync manager by Sync3A class.
     * @brief this function need to call before sync3AWrapper
     * @brief because need to update stereo data
     */
    virtual MVOID               init(MINT32 i4MasterIdx, MINT32 i4SlaveIdx, MINT32 i4MasterDev, MINT32 i4SlaveDev);

    /**
     * @brief Explicitly uninit 3A N3D Sync manager by Sync3A class.
     */
    virtual MVOID               uninit();

    /**
     * @brief Functions for set new stereo parameter
     */
    virtual MVOID               setStereoParams(Stereo_Param_T const &rNewParam);

    /**
     * @brief Functions for get stereo parameter
     */
    virtual Stereo_Param_T      getStereoParams() const;

    /**
     * @brief Functions for get AWB master dev
     */
    virtual MINT32              getAWBMasterDev() const;

    /**
     * @brief Functions for get AWB slave dev
     */
    virtual MINT32              getAWBSlaveDev() const;

    /**
     * @brief Functions for update master and slave dev
     */
    virtual MVOID               updateMasterSlave(MINT32 i4MasterDev, MINT32 i4SlaveDev);

    /**
     * @brief Functions for get master dev
     */
    virtual MINT32              getMasterDev() const;

    /**
     * @brief Functions for get slave dev
     */
    virtual MINT32              getSlaveDev() const;

protected:
    Sync3AMgr();
    virtual ~Sync3AMgr() {}

    MINT32                      m_i4MasterDev;
    MINT32                      m_i4SlaveDev;
    Stereo_Param_T              m_rStereoParams;
    mutable std::mutex          m_LockSync3AMgr;
};

/******************************************************************************
 *  Sync3AMgr Implementation
 ******************************************************************************/
Sync3AMgr::
Sync3AMgr()
    : m_i4MasterDev(ESensorDevId_Main)
    , m_i4SlaveDev(ESensorDevId_Main)
    , m_rStereoParams()
    , m_LockSync3AMgr()
{}

Sync3AMgr*
Sync3AMgr::
getInstance()
{
    static Sync3AMgr _rSync3AMgr;
    return &_rSync3AMgr;
}

MVOID
Sync3AMgr::
init(MINT32 i4MasterIdx, MINT32 i4SlaveIdx, MINT32 i4MasterDev, MINT32 i4SlaveDev)
{
    std::lock_guard<std::mutex> lock(m_LockSync3AMgr);

    m_rStereoParams.i4MasterIdx = i4MasterIdx;
    m_rStereoParams.i4SlaveIdx = i4SlaveIdx;
    m_i4MasterDev = i4MasterDev;
    m_i4SlaveDev = i4SlaveDev;

    SYNC3AMGR_LOGD("[%s] M&S(Idx,Dev) : M(%d,%d)/S(%d,%d)", __FUNCTION__,
            m_rStereoParams.i4MasterIdx, m_i4MasterDev,
            m_rStereoParams.i4SlaveIdx, m_i4SlaveDev);
}

MVOID
Sync3AMgr::
uninit()
{
    std::lock_guard<std::mutex> lock(m_LockSync3AMgr);

    SYNC3AMGR_LOGD("[%s] M&S(Idx,Dev) : M(%d,%d)/S(%d,%d)", __FUNCTION__,
            m_rStereoParams.i4MasterIdx, m_i4MasterDev,
            m_rStereoParams.i4SlaveIdx, m_i4SlaveDev);

    m_rStereoParams.i4Sync2AMode = NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_NONE;
    m_rStereoParams.i4SyncAFMode = NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_OFF;
    m_rStereoParams.i4HwSyncMode = NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_OFF;
}

MINT32
Sync3AMgr::
getAWBMasterDev() const
{
    // TODO : query sensor type to decide which sensor is master.
    return NSIspTuning::ESensorDev_Main;
}

MINT32
Sync3AMgr::
getAWBSlaveDev() const
{
    // TODO : query sensor type to decide which sensor is slave.
    return NSIspTuning::ESensorDev_MainSecond;
}

MVOID
Sync3AMgr::
setStereoParams(Stereo_Param_T const &rNewParam)
{
    std::lock_guard<std::mutex> lock(m_LockSync3AMgr);
    SYNC3AMGR_LOGD("[%s] (MIdx,SIdx)(Sync2AMode,SyncAFMode,HwSyncMode)(IsByFrame,IsDummyFrame):(%d,%d)(%d,%d,%d)(%d,%d) -> (%d,%d)(%d,%d,%d)(%d,%d)", __FUNCTION__,
        m_rStereoParams.i4MasterIdx, m_rStereoParams.i4SlaveIdx, m_rStereoParams.i4Sync2AMode, m_rStereoParams.i4SyncAFMode, m_rStereoParams.i4HwSyncMode,
        m_rStereoParams.bIsByFrame, m_rStereoParams.bIsDummyFrame,
        rNewParam.i4MasterIdx, rNewParam.i4SlaveIdx, rNewParam.i4Sync2AMode, rNewParam.i4SyncAFMode, rNewParam.i4HwSyncMode,
        rNewParam.bIsByFrame, rNewParam.bIsDummyFrame);
    m_rStereoParams = rNewParam;
}

Stereo_Param_T
Sync3AMgr::
getStereoParams() const
{
    std::lock_guard<std::mutex> lock(m_LockSync3AMgr);
    return m_rStereoParams;
}

MVOID
Sync3AMgr::
updateMasterSlave(MINT32 i4MasterDev, MINT32 i4SlaveDev)
{
    std::lock_guard<std::mutex> lock(m_LockSync3AMgr);

    SYNC3AMGR_LOGD("[%s] M&S Dev: (%d,%d) -> (%d,%d)", __FUNCTION__,
            m_i4MasterDev, m_i4SlaveDev, i4MasterDev, i4SlaveDev);

    m_i4MasterDev = i4MasterDev;
    m_i4SlaveDev = i4SlaveDev;
}

MINT32
Sync3AMgr::
getMasterDev() const
{
    std::lock_guard<std::mutex> lock(m_LockSync3AMgr);
    return m_i4MasterDev;
}

MINT32
Sync3AMgr::
getSlaveDev() const
{
    std::lock_guard<std::mutex> lock(m_LockSync3AMgr);
    return m_i4SlaveDev;
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

    virtual MBOOL               init();

    virtual MBOOL               uninit();

    virtual MBOOL               syncCalc(MINT32 i4Opt, const StatisticBufInfo* pMasterSttBuf, const StatisticBufInfo* pSlaveSttBuf);

    virtual MBOOL               setDebugInfo(void* prDbgInfo) const;

    virtual MBOOL               isAeStable() const {return (m_fgMasterAeStable && m_fgSlaveAeStable);}

    //This is for CPU AE
    virtual VOID                syncAeSetting(MINT32 i4MasterDev, MINT32 i4SlaveDev);

    //This is for CCU AE
    virtual MBOOL               syncCCUAeSetting(MINT32 i4MasterIdx, MINT32 i4SlaveIdx, MINT32 i4MasterDev, MINT32 i4SlaveDev);

    virtual VOID                syncAwbSetting(MINT32 i4MasterIdx, MINT32 i4SlaveIdx);

    virtual MINT32              updateCcuSet3ASyncState();

    virtual MVOID               updateFrameSync(MBOOL bEnable);

protected:
    Sync3AWrapper();
    virtual ~Sync3AWrapper() {}

    virtual MBOOL               update(MINT32 i4Opt);
    SYNCAE_CAMERA_TYPE_ENUM     querySensorType(MINT32 i4SensorDev);
    MBOOL                       syncAeInit();
    MBOOL                       syncAwbInit();

    MBOOL                       syncAeMain(const StatisticBufInfo* pMasterSttBuf, const StatisticBufInfo* pSlaveSttBuf);
    MBOOL                       syncAwbMain(const StatisticBufInfo* pMasterSttBuf, const StatisticBufInfo* pSlaveSttBuf);

    mutable std::mutex          m_Lock;
    MINT32                      m_i4User;
    MINT32                      m_Sync3ALogEnable;

    MBOOL                       m_fgMasterAeStable;
    MBOOL                       m_fgSlaveAeStable;

    ManualAe                    m_rManualAeMaster;
    ManualAe                    m_rManualAeSlave;

    MTKSyncAe*                  m_pMTKSyncAe;
    MTKSyncAwb*                 m_pMTKSyncAwb;
    ICcuCtrl3ACtrl*             m_pCcuCtrl3ASync;

    SYNC_AWB_INPUT_INFO_STRUCT  m_rAwbSyncInput;
    SYNC_AWB_OUTPUT_INFO_STRUCT m_rAwbSyncOutput;
    SYNC_AE_OUTPUT_STRUCT       m_rAeSyncOutput;
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
    , m_Sync3ALogEnable(0)
    , m_fgMasterAeStable(MFALSE)
    , m_fgSlaveAeStable(MFALSE)
    , m_rManualAeMaster()
    , m_rManualAeSlave()
    , m_pMTKSyncAe(NULL)
    , m_pMTKSyncAwb(NULL)
    , m_pCcuCtrl3ASync(NULL)
{
    getPropInt("vendor.debug.sync3AWrapper.log", &m_Sync3ALogEnable, 0);
}

Sync3AWrapper*
Sync3AWrapper::getInstance()
{
    static Sync3AWrapper _rWrap;
    return &_rWrap;
}

MBOOL
Sync3AWrapper::
init()
{
    std::lock_guard<std::mutex> lock(m_Lock);

    if (m_i4User > 0)
    {

    }
    else
    {
        MINT32 i4MasterDev = Sync3AMgr::getInstance()->getMasterDev();
        MINT32 i4SlaveDev  = Sync3AMgr::getInstance()->getSlaveDev();
        SYNC3AWRAPPER_LOGD("[%s] User(%d), M&S Dev(%d,%d)", __FUNCTION__, m_i4User, i4MasterDev, i4SlaveDev);
        m_fgMasterAeStable = MFALSE;
        m_fgSlaveAeStable = MFALSE;
        syncAeInit();
        syncAwbInit();
        if(m_pCcuCtrl3ASync == NULL)
        {
            m_pCcuCtrl3ASync = ICcuCtrl3ACtrl::createInstance();
            SYNC3AWRAPPER_LOGD("[%s] CCU_SYNC_DBG Create ICcuCtrl3ACtrl", __FUNCTION__);
        }
        IAfMgr::getInstance(i4SlaveDev).SyncAFSetMode(0xFE);
        IAfMgr::getInstance(i4MasterDev).SyncAFSetMode(0xFF);
    }

    m_i4User++;

    return MTRUE;
}

MBOOL
Sync3AWrapper::
uninit()
{
    std::lock_guard<std::mutex> lock(m_Lock);

    if (m_i4User > 0)
    {
        // More than one user, so decrease one User.
        m_i4User--;

        if (m_i4User == 0) // There is no more User after decrease one User
        {
            SYNC3AWRAPPER_LOGD("[%s]", __FUNCTION__);
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
            SYNC3AWRAPPER_LOGD("[%s] Still %d users \n", __FUNCTION__, m_i4User);
        }
    }

    return MTRUE;
}

MBOOL
Sync3AWrapper::
syncCalc(MINT32 i4Opt, const StatisticBufInfo* pMasterSttBuf, const StatisticBufInfo* pSlaveSttBuf)
{
    if (pMasterSttBuf == NULL || pSlaveSttBuf == NULL)
    {
        SYNC3AWRAPPER_LOGE("[%s] pMasterSttBuf = NULL(%p) || pSlaveSttBuf = NULL(%p)", __FUNCTION__, pMasterSttBuf, pSlaveSttBuf);
        return MFALSE;
    }
    if (i4Opt & (ISync3A::E_SYNC3A_DO_AE | ISync3A::E_SYNC3A_DO_AE_PRECAP))
    {
        if (!syncAeMain(pMasterSttBuf, pSlaveSttBuf))
            SYNC3AWRAPPER_LOGE("[%s] syncAeMain fail", __FUNCTION__);
    }

    if (i4Opt & ISync3A::E_SYNC3A_DO_AWB)
    {
        if (!syncAwbMain(pMasterSttBuf, pSlaveSttBuf))
            SYNC3AWRAPPER_LOGE("[%s] syncAwbMain fail", __FUNCTION__);
    }

    return update(i4Opt);
}


MBOOL
Sync3AWrapper::
syncAeInit()
{
    MINT32 i4MasterDev = Sync3AMgr::getInstance()->getMasterDev();
    MINT32 i4SlaveDev  = Sync3AMgr::getInstance()->getSlaveDev();

    SYNC3AWRAPPER_LOGD("[%s] + M&S Dev(%d,%d)", __FUNCTION__, i4MasterDev, i4SlaveDev);

    m_pMTKSyncAe = MTKSyncAe::createInstance();
    if (!m_pMTKSyncAe)
    {
        SYNC3AWRAPPER_LOGE("MTKSyncAe::createInstance() fail");
        return MFALSE;
    }

    SYNC_AE_INIT_STRUCT rAEInitData;
    ::memset(&rAEInitData, 0x0, sizeof(SYNC_AE_INIT_STRUCT));  //  initialize, Set all to 0
    rAEInitData.SyncScenario = SYNC_AE_SCENARIO_N3D;
    rAEInitData.SyncNum = 2; //N3D 2 cam

    AWB_Calibration_Data AWBCalData = {512, 512, 512};   // todo remove ??

    const strSyncAEInitInfo* pSyncAEInitInfo = getSyncAEInitInfo();

    rAEInitData.SyncAeTuningParam.syncPolicy = (SYNC_AE_POLICY)pSyncAEInitInfo->SyncWhichEye;
    rAEInitData.SyncAeTuningParam.enFinerSyncOffset = pSyncAEInitInfo->isFinerSyncOffset;
    rAEInitData.SyncAeTuningParam.enPPGainComp = pSyncAEInitInfo->isDoGainRegression;
    rAEInitData.SyncAeTuningParam.u4FixSyncGain = pSyncAEInitInfo->FixSyncGain;
    rAEInitData.SyncAeTuningParam.u4RegressionType = pSyncAEInitInfo->u4RegressionType;

    rAEInitData.main_param.syncAECalData.Golden = AWBCalData;  // todo
    rAEInitData.main_param.syncAECalData.Unit = AWBCalData;    // todo
    rAEInitData.sub_param.syncAECalData.Golden = AWBCalData;   // todo
    rAEInitData.sub_param.syncAECalData.Unit = AWBCalData;     // todo
    rAEInitData.main_param.syncAECamType = querySensorType(i4MasterDev);
    rAEInitData.sub_param.syncAECamType = querySensorType(i4SlaveDev);

    ::memcpy(rAEInitData.SyncAeTuningParam.pDeltaBVtoRatioArray, &pSyncAEInitInfo->pDeltaBVtoRatioArray[0][0], sizeof(MUINT32) * 2 * 30);
    ::memcpy(rAEInitData.main_param.BVOffset, pSyncAEInitInfo->EVOffset_main, sizeof(MUINT32) * 2);
    ::memcpy(rAEInitData.sub_param.BVOffset, pSyncAEInitInfo->EVOffset_main2, sizeof(MUINT32) * 2);
    ::memcpy(rAEInitData.main_param.RGB2YCoef, pSyncAEInitInfo->RGB2YCoef_main, sizeof(MUINT32) * 3);
    ::memcpy(rAEInitData.sub_param.RGB2YCoef, pSyncAEInitInfo->RGB2YCoef_main2, sizeof(MUINT32) * 3);

    m_rAeSyncOutput.main_ch.u4SyncGain = 1024;
    m_rAeSyncOutput.sub_ch.u4SyncGain = 1024;
    NVRAM_CAMERA_3A_STRUCT* p3aNvram;
    NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, i4MasterDev, (void*&)p3aNvram, 0);
    rAEInitData.main_param.pAeNvram = &p3aNvram->AE[0];
    NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, i4SlaveDev, (void*&)p3aNvram, 0);
    rAEInitData.sub_param.pAeNvram = &p3aNvram->AE[0];

    m_pMTKSyncAe->SyncAeInit(reinterpret_cast<void*>(&rAEInitData));

    SYNC3AWRAPPER_LOGD("[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
Sync3AWrapper::
syncAwbInit()
{
    MINT32 i4MasterDev = Sync3AMgr::getInstance()->getMasterDev();
    MINT32 i4SlaveDev  = Sync3AMgr::getInstance()->getSlaveDev();
    MINT32 i4AWBMasterDev = Sync3AMgr::getInstance()->getAWBMasterDev();
    MINT32 i4AWBSlaveDev  = Sync3AMgr::getInstance()->getAWBSlaveDev();

    SYNC3AWRAPPER_LOGD("[%s] + M&S Dev(%d,%d), M&S AWB Dev(%d,%d)", __FUNCTION__, i4MasterDev, i4SlaveDev, i4AWBMasterDev, i4AWBSlaveDev);

    m_pMTKSyncAwb = MTKSyncAwb::createInstance();
    if (!m_pMTKSyncAwb)
    {
        SYNC3AWRAPPER_LOGE("MTKSyncAwb::createInstance() fail");
        return MFALSE;
    }

    SYNC_AWB_INIT_INFO_STRUCT rAWBInitData;
    ::memset(&rAWBInitData, 0x0, sizeof(SYNC_AWB_INIT_INFO_STRUCT));  //  initialize, Set all to 0

    const strSyncAWBInitInfo* pAwbInitInfo = getSyncAWBInitInfo();

    rAWBInitData.SyncScenario = SYNC_AWB_SCENARIO_N3D;

    NVRAM_CAMERA_3A_STRUCT* p3aNvram;
    NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, i4AWBMasterDev, (void*&)p3aNvram, MFALSE);
    rAWBInitData.main_param = p3aNvram->AWB[CAM_SCENARIO_PREVIEW];
    rAWBInitData.main_sync_param = p3aNvram->AWBSYNC;
    NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, i4AWBSlaveDev, (void*&)p3aNvram, MFALSE);
    rAWBInitData.sub_param = p3aNvram->AWB[CAM_SCENARIO_PREVIEW];
    rAWBInitData.sub_sync_param = p3aNvram->AWBSYNC;


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

    SYNC3AWRAPPER_LOGD("[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
Sync3AWrapper::
syncAeMain(const StatisticBufInfo* pMasterSttBuf, const StatisticBufInfo* pSlaveSttBuf)
{
    MINT32 i4MasterDev = Sync3AMgr::getInstance()->getMasterDev();
    MINT32 i4SlaveDev  = Sync3AMgr::getInstance()->getSlaveDev();

    SYNC3AWRAPPER_LOGD("[%s] + M&S Dev(%d,%d)", __FUNCTION__, i4MasterDev, i4SlaveDev);

    AWB_GAIN_T defGain = {512, 512, 512};   // todo remove ??

    SYNC_AE_INPUT_STRUCT rAeSyncInput;
    // rAeSyncInput.eSyncAEMode = SYNC_AE_PLINES_CONST_W;
    rAeSyncInput.eSyncAEMode = SYNC_AE_EXPTIME_CONST_W;
    // SYNC3AWRAPPER_LOGD("[%s] rAeSyncInput.eSyncAEMode = SYNC_AE_EXPTIME_CONST_W", __FUNCTION__);

    rAeSyncInput.SyncCamScenario = SYNC_AE_CAM_SCENARIO_PREVIEW;
    AE_MODE_CFG_T _a_rAEOutput_main, _a_rAEOutput_main2;
    strAETable strPreviewAEPlineTableMain, strCaptureAEPlineTabMain;
    strAETable strPreviewAEPlineTableMain2, strCaptureAEPlineTabMain2;
    strAETable strStrobeAEPlineTableMain, strStrobeAEPlineTableMain2;
    strAFPlineInfo strStrobeAFPlineTabMain, strStrobeAFPlineTabMain2;
    strFinerEvPline strPreviewAEFinverPlineTableMain, strCaptureAEFinverPlineTabMain;
    strFinerEvPline strPreviewAEFinverPlineTableMain2, strCaptureAEFinverPlineTabMain2;
    MUINT32 u4AEConditionMain, u4AEConditionMain2;

    AWB_OUTPUT_T rAwbMaster, rAwbSlave;

    NVRAM_CAMERA_3A_STRUCT *pMasterNvram, *pSlaveNvram;
    NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, i4MasterDev, (void*&)pMasterNvram, 0);
    NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, i4SlaveDev, (void*&)pSlaveNvram, 0);

    IAwbMgr::getInstance().getAWBOutput(i4MasterDev, rAwbMaster);
    IAwbMgr::getInstance().getAWBOutput(i4SlaveDev, rAwbSlave);

    AE_PERFRAME_INFO_T AEPerframeInfoMaster;
    AE_PERFRAME_INFO_T AEPerframeInfoSlave;
    IAeMgr::getInstance().getAEInfo(i4MasterDev, AEPerframeInfoMaster);
    IAeMgr::getInstance().getAEInfo(i4SlaveDev, AEPerframeInfoSlave);

    IAeMgr::getInstance().sendAECtrl(i4MasterDev, EAECtrl_GetCurrentPlineTable, reinterpret_cast<MINTPTR>(&(strPreviewAEPlineTableMain)), reinterpret_cast<MINTPTR>(&(strCaptureAEPlineTabMain)), reinterpret_cast<MINTPTR>(&(strStrobeAEPlineTableMain)), reinterpret_cast<MINTPTR>(&(strStrobeAFPlineTabMain)));
    IAeMgr::getInstance().sendAECtrl(i4SlaveDev, EAECtrl_GetCurrentPlineTable, reinterpret_cast<MINTPTR>(&(strPreviewAEPlineTableMain2)), reinterpret_cast<MINTPTR>(&(strCaptureAEPlineTabMain2)), reinterpret_cast<MINTPTR>(&(strStrobeAEPlineTableMain2)), reinterpret_cast<MINTPTR>(&(strStrobeAFPlineTabMain2)));
    IAeMgr::getInstance().sendAECtrl(i4MasterDev, EAECtrl_GetCurrentPlineTableF, reinterpret_cast<MINTPTR>(&(strPreviewAEFinverPlineTableMain)), NULL, NULL, NULL);
    IAeMgr::getInstance().sendAECtrl(i4SlaveDev, EAECtrl_GetCurrentPlineTableF, reinterpret_cast<MINTPTR>(&(strPreviewAEFinverPlineTableMain2)), NULL, NULL, NULL);

    m_fgMasterAeStable = AEPerframeInfoMaster.rAEISPInfo.bAEStable;
    m_fgSlaveAeStable  = AEPerframeInfoSlave.rAEISPInfo.bAEStable;

    MUINT32 i4AECondition = (AE_CONDITION_BACKLIGHT | AE_CONDITION_OVEREXPOSURE | AE_CONDITION_HIST_STRETCH | AE_CONDITION_SATURATION_CHECK | AE_CONDITION_FACEAE);

    u4AEConditionMain = (i4AECondition & AEPerframeInfoMaster.rAEUpdateInfo.u4AECondition);
    u4AEConditionMain2 = (i4AECondition & AEPerframeInfoSlave.rAEUpdateInfo.u4AECondition);

    // All we need is CCU_strAEOutput@ccu_ae_param.h
    // bAEStable     -> bAEStable@ccu_ae_param.h
    // EvSetting     -> EvSetting@ccu_ae_param.h
    // Bv            -> Bv@ccu_ae_param.h
    // u4AECondition -> u4AECondition@ccu_ae_param.h
    // i4DeltaBV     -> i4DeltaBV@ccu_ae_param.h
    // u4ISO         -> u4ISO@ccu_ae_param.h
    // u2FrameRate   -> u2FrameRate@ccu_ae_param.h
    // i2FlareOffset -> i2FlareOffset@ccu_ae_param.h
    // i2FlareGain -> i2FlareGain@ccu_ae_param.h
    // i2FaceDiffIndex -> i2FaceDiffIndex@ccu_ae_param.h
    // Except
    // rAeSyncInput.main_ch.u4SyncGain = 1024
    // rAeSyncInput.main_ch.u4ExposureMode ----> need to check
    // rAeSyncInput.main_ch.wb_gain ----> need to check
    // rAeSyncInput.main_ch.aao_buffer_info.buf_line_size = 780
    // rAeSyncInput.main_ch.aao_buffer_info.pAEStatisticBuf 780*90 aao buffer
    rAeSyncInput.main_ch.Input.bAEStable = AEPerframeInfoMaster.rAEISPInfo.bAEStable;

    rAeSyncInput.main_ch.Input.EvSetting.u4Eposuretime = AEPerframeInfoMaster.rAEISPInfo.u8P1Exposuretime_ns / 1000;
    rAeSyncInput.main_ch.Input.EvSetting.u4AfeGain = AEPerframeInfoMaster.rAEISPInfo.u4P1SensorGain;
    rAeSyncInput.main_ch.Input.EvSetting.u4IspGain = AEPerframeInfoMaster.rAEISPInfo.u4P1DGNGain;
    rAeSyncInput.main_ch.Input.EvSetting.uIris = 0;  // Iris fix
    rAeSyncInput.main_ch.Input.EvSetting.uSensorMode = 0;  // sensor mode don't change
    rAeSyncInput.main_ch.Input.EvSetting.uFlag = 0; // No hypersis
    rAeSyncInput.main_ch.Input.Bv = AEPerframeInfoMaster.rAEUpdateInfo.i4BVvalue_x10;
    rAeSyncInput.main_ch.Input.u4AECondition = u4AEConditionMain;
    rAeSyncInput.main_ch.Input.i4DeltaBV = AEPerframeInfoMaster.rAEUpdateInfo.i4DeltaBV;

    rAeSyncInput.main_ch.Input.u4ISO = AEPerframeInfoMaster.rAEISPInfo.u4P1RealISOValue;
    rAeSyncInput.main_ch.Input.u2FrameRate = AEPerframeInfoMaster.rAEUpdateInfo.u2FrameRate_x10;
    rAeSyncInput.main_ch.Input.i2FlareOffset = AEPerframeInfoMaster.rAEISPInfo.i2FlareOffset;
    rAeSyncInput.main_ch.Input.i2FlareGain = AEPerframeInfoMaster.rAEISPInfo.i2FlareGain;
    rAeSyncInput.main_ch.Input.i2FaceDiffIndex = AEPerframeInfoMaster.rAEUpdateInfo.i2AEFaceDiffIndex;
    rAeSyncInput.main_ch.Input.i4AEidxCurrent = AEPerframeInfoMaster.rAEUpdateInfo.i4AEidxCur;
    rAeSyncInput.main_ch.Input.i4AEidxCurrent = AEPerframeInfoMaster.rAEUpdateInfo.i4AEidxNext;
    rAeSyncInput.main_ch.Input.i4AEidxCurrent = AEPerframeInfoMaster.rAEUpdateInfo.i4AEidxNextF;
    rAeSyncInput.main_ch.Input.u4CWValue = AEPerframeInfoMaster.rAEUpdateInfo.u4CWValue;
    rAeSyncInput.main_ch.Input.u4AvgY = AEPerframeInfoMaster.rAEUpdateInfo.u4AvgY;
    rAeSyncInput.main_ch.u4SyncGain = m_rAeSyncOutput.main_ch.u4SyncGain;
    rAeSyncInput.main_ch.u4ExposureMode = AEPerframeInfoMaster.rAEUpdateInfo.u4ExposureMode;   // exposure time
    rAeSyncInput.main_ch.pAETable = &strPreviewAEPlineTableMain;
    rAeSyncInput.main_ch.pAEFinerEVPline = &strPreviewAEFinverPlineTableMain;
    rAeSyncInput.main_ch.pAeNvram = &pMasterNvram->AE[0];
    rAeSyncInput.main_ch.aao_buffer_info.pAEStatisticBuf = (MVOID*) pMasterSttBuf->mVa; // reinterpret_cast<MVOID *>(IAAOBufMgr::getInstance().getCurrHwBuf(m_i4MasterDev));
    rAeSyncInput.main_ch.u4FinverEVidx = AEPerframeInfoMaster.rAEISPInfo.u4AEFinerEVIdxBase;
    IAeMgr::getInstance().sendAECtrl(i4MasterDev, EAECtrl_GetAAOLineByteSize, reinterpret_cast<MINTPTR>(&(rAeSyncInput.main_ch.aao_buffer_info.buf_line_size)), NULL, NULL, NULL);
    rAeSyncInput.main_ch.wb_gain.i4B = rAwbMaster.rAWBSyncInput_N3D.rAlgGain.i4B;
    rAeSyncInput.main_ch.wb_gain.i4G = rAwbMaster.rAWBSyncInput_N3D.rAlgGain.i4G;
    rAeSyncInput.main_ch.wb_gain.i4R = rAwbMaster.rAWBSyncInput_N3D.rAlgGain.i4R;

    rAeSyncInput.sub_ch.Input.bAEStable = AEPerframeInfoSlave.rAEISPInfo.bAEStable;
    rAeSyncInput.sub_ch.Input.EvSetting.u4Eposuretime = _a_rAEOutput_main2.u4Eposuretime;
    rAeSyncInput.sub_ch.Input.EvSetting.u4AfeGain = _a_rAEOutput_main2.u4AfeGain;
    rAeSyncInput.sub_ch.Input.EvSetting.u4IspGain = _a_rAEOutput_main2.u4IspGain;
    rAeSyncInput.sub_ch.Input.EvSetting.uIris = 0;  // Iris fix
    rAeSyncInput.sub_ch.Input.EvSetting.uSensorMode = 0;  // sensor mode don't change
    rAeSyncInput.sub_ch.Input.EvSetting.uFlag = 0; // No hypersis
    rAeSyncInput.sub_ch.Input.Bv = AEPerframeInfoSlave.rAEUpdateInfo.i4BVvalue_x10;
    rAeSyncInput.sub_ch.Input.u4AECondition = u4AEConditionMain2;
    rAeSyncInput.sub_ch.Input.i4DeltaBV = AEPerframeInfoSlave.rAEUpdateInfo.i4DeltaBV;
    rAeSyncInput.sub_ch.Input.u4ISO = AEPerframeInfoMaster.rAEISPInfo.u4P1RealISOValue;
    rAeSyncInput.sub_ch.Input.u2FrameRate = _a_rAEOutput_main2.u2FrameRate;
    rAeSyncInput.sub_ch.Input.i2FlareOffset = _a_rAEOutput_main2.i2FlareOffset;
    rAeSyncInput.sub_ch.Input.i2FlareGain = _a_rAEOutput_main2.i2FlareGain;
    rAeSyncInput.sub_ch.Input.i2FaceDiffIndex = AEPerframeInfoSlave.rAEUpdateInfo.i2AEFaceDiffIndex;
    rAeSyncInput.sub_ch.Input.i4AEidxCurrent = AEPerframeInfoSlave.rAEUpdateInfo.i4AEidxCur;
    rAeSyncInput.sub_ch.Input.i4AEidxCurrent = AEPerframeInfoSlave.rAEUpdateInfo.i4AEidxNext;
    rAeSyncInput.sub_ch.Input.i4AEidxCurrent = AEPerframeInfoSlave.rAEUpdateInfo.i4AEidxNextF;
    rAeSyncInput.sub_ch.u4FinverEVidx = _a_rAEOutput_main2.u4AEFinerEVIdxBase;
    rAeSyncInput.sub_ch.Input.u4CWValue = _a_rAEOutput_main2.u4CWValue;
    rAeSyncInput.sub_ch.Input.u4AvgY = _a_rAEOutput_main2.u4AvgY;
    rAeSyncInput.sub_ch.u4SyncGain = m_rAeSyncOutput.sub_ch.u4SyncGain;
    rAeSyncInput.sub_ch.u4ExposureMode = _a_rAEOutput_main2.u4ExposureMode;   // exposure time
    rAeSyncInput.sub_ch.pAETable = &strPreviewAEPlineTableMain2;
    rAeSyncInput.sub_ch.pAEFinerEVPline = &strPreviewAEFinverPlineTableMain2;

    rAeSyncInput.sub_ch.pAeNvram = &pSlaveNvram->AE[0];
    rAeSyncInput.sub_ch.aao_buffer_info.pAEStatisticBuf = (MVOID*) pSlaveSttBuf->mVa; //reinterpret_cast<MVOID *>(IAAOBufMgr::getInstance().getCurrHwBuf(m_i4SlaveDev));
    IAeMgr::getInstance().sendAECtrl(i4SlaveDev, EAECtrl_GetAAOLineByteSize, reinterpret_cast<MINTPTR>(&(rAeSyncInput.sub_ch.aao_buffer_info.buf_line_size)), NULL, NULL, NULL);
    rAeSyncInput.sub_ch.wb_gain.i4B = rAwbSlave.rAWBSyncInput_N3D.rAlgGain.i4B;
    rAeSyncInput.sub_ch.wb_gain.i4G = rAwbSlave.rAWBSyncInput_N3D.rAlgGain.i4G;
    rAeSyncInput.sub_ch.wb_gain.i4R = rAwbSlave.rAWBSyncInput_N3D.rAlgGain.i4R;

    m_pMTKSyncAe->SyncAeMain(&rAeSyncInput, &m_rAeSyncOutput);

    SYNC3AWRAPPER_LOGD("[%s] -", __FUNCTION__);

    return MTRUE;
}

MBOOL
Sync3AWrapper::
syncAwbMain(const StatisticBufInfo* pMasterSttBuf, const StatisticBufInfo* pSlaveSttBuf)
{
    MINT32 i4MasterDev = Sync3AMgr::getInstance()->getMasterDev();
    MINT32 i4SlaveDev  = Sync3AMgr::getInstance()->getSlaveDev();
    MINT32 i4AWBMasterDev = Sync3AMgr::getInstance()->getAWBMasterDev();
    MINT32 i4AWBSlaveDev = Sync3AMgr::getInstance()->getAWBSlaveDev();
    SYNC3AWRAPPER_LOGD("[%s] + M&S (Dev/Stt)(%d/%p,%d/%p), M&S AWB Dev(%d,%d) +", __FUNCTION__, i4MasterDev, pMasterSttBuf, i4SlaveDev, pSlaveSttBuf, i4AWBMasterDev, i4AWBSlaveDev);

    AWB_OUTPUT_T rAwbMaster, rAwbSlave;
    AWB_AAO_CONFIG_Param_T rMasterAWBConfig, rSlaveAWBConfig;

    AWB_PARENT_BLK_STAT_T *rAwbParentStatMaster = &rAwbMaster.rAWBSyncInput_N3D.rAwbParentStatBlk[0][0];
    AWB_PARENT_BLK_STAT_T *rAwbParentStatSlave = &rAwbSlave.rAWBSyncInput_N3D.rAwbParentStatBlk[0][0];
    MINT32& iParentBlkNumX_Main = rAwbMaster.rAWBSyncInput_N3D.ParentBlkNumX;
    MINT32& iParentBlkNumY_Main = rAwbMaster.rAWBSyncInput_N3D.ParentBlkNumY;
    MINT32& iParentBlkNumX_Sub = rAwbSlave.rAWBSyncInput_N3D.ParentBlkNumX;
    MINT32& iParentBlkNumY_Sub = rAwbSlave.rAWBSyncInput_N3D.ParentBlkNumY;

    IAwbMgr::getInstance().getAWBOutput(i4AWBMasterDev, rAwbMaster);
    IAwbMgr::getInstance().getAWBOutput(i4AWBSlaveDev, rAwbSlave);

    IAwbMgr::getInstance().getAWBParentStat(i4AWBMasterDev, rAwbParentStatMaster, &iParentBlkNumX_Main, &iParentBlkNumY_Main);
    IAwbMgr::getInstance().getAWBParentStat(i4AWBSlaveDev, rAwbParentStatSlave, &iParentBlkNumX_Sub, &iParentBlkNumY_Sub);

    IAwbMgr::getInstance().getAAOConfig(i4AWBMasterDev, rMasterAWBConfig);
    IAwbMgr::getInstance().getAAOConfig(i4AWBSlaveDev, rSlaveAWBConfig);

    Stereo_Param_T rParams = Sync3AMgr::getInstance()->getStereoParams();
    switch (rParams.i4Sync2AMode)
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

    SYNC3AWRAPPER_LOGD("[%s] SYNC2A_MODE = %d", __FUNCTION__, m_rAwbSyncInput.feature_type);

    // 0 : main preview, 1 : sub preview
    if (i4MasterDev == Sync3AMgr::getInstance()->getAWBMasterDev())
    {
        m_rAwbSyncInput.display_cam = 0;
        m_rAwbSyncInput.main_ch.aao_buf_info.pAWBStatBuf = (MVOID*) pMasterSttBuf->mVa;
        m_rAwbSyncInput.sub_ch.aao_buf_info.pAWBStatBuf = (MVOID*) pSlaveSttBuf->mVa;
    }
    else
    {
        m_rAwbSyncInput.display_cam = 1;
        m_rAwbSyncInput.sub_ch.aao_buf_info.pAWBStatBuf = (MVOID*) pMasterSttBuf->mVa;
        m_rAwbSyncInput.main_ch.aao_buf_info.pAWBStatBuf = (MVOID*) pSlaveSttBuf->mVa;
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
    m_rAwbSyncInput.strLocationTransInfo.u4MainCamFovRatio = pAwbInitInfo->u4SyncAWB_fov_ratio;

    NSIspTuning::ISP_NVRAM_CCM_T ccm;
    IResultPool* pResultPoolMaster = NULL;
    ISPResultToMeta_T *pISPResultToMeta = NULL;
    pResultPoolMaster = IResultPool::getInstance(i4AWBMasterDev);
    MINT32 rHistoryReqMagic[HistorySize] = {0, 0, 0};
    if (pResultPoolMaster)
    {
        pResultPoolMaster->getHistory(rHistoryReqMagic);
        pISPResultToMeta = (ISPResultToMeta_T*)pResultPoolMaster->getResult(rHistoryReqMagic[1], E_ISP_RESULTTOMETA, __FUNCTION__);
        if (pISPResultToMeta == NULL)
        {
            SYNC3AWRAPPER_LOGW("[%s] get History (Req0) = (#%d)", __FUNCTION__, rHistoryReqMagic[0]);
            pISPResultToMeta = (ISPResultToMeta_T*)pResultPoolMaster->getResult(rHistoryReqMagic[0], E_ISP_RESULTTOMETA, __FUNCTION__);
        }
    }

    if (pISPResultToMeta)
        ccm = pISPResultToMeta->rCamInfo.rMtkCCM;
    else
        SYNC3AWRAPPER_LOGW("[%s] pISPResultToMeta %p", __FUNCTION__, pISPResultToMeta);
    if (sizeof(AWB_OUTPUT_T) == sizeof(AWB_OUTPUT_N3D_T))
    {
        memcpy(&m_rAwbSyncInput.main_ch.awb_output_struct, &rAwbMaster, sizeof(AWB_OUTPUT_T));
    }
    else
        SYNC3AWRAPPER_LOGE("AWB_OUTPUT_T != AWB_OUTPUT_N3D_T");
    m_rAwbSyncInput.main_ch.focus_distance = 0;                 // TODO : query from AF, (example : 10 cm)
    IAeMgr::getInstance().sendAECtrl(i4MasterDev, EAECtrl_GetAAOLineByteSize, reinterpret_cast<MINTPTR>(&(m_rAwbSyncInput.main_ch.aao_buf_info.stat_line_size)), NULL, NULL, NULL);// TODO
    m_rAwbSyncInput.main_ch.ccm.CCmatrix[0] = ((float)Complement2(ccm.cnv_1.bits.CCM_CNV_00, 13) / 512);
    m_rAwbSyncInput.main_ch.ccm.CCmatrix[1] = ((float)Complement2(ccm.cnv_1.bits.CCM_CNV_01, 13) / 512);
    m_rAwbSyncInput.main_ch.ccm.CCmatrix[2] = ((float)Complement2(ccm.cnv_2.bits.CCM_CNV_02, 13) / 512);
    m_rAwbSyncInput.main_ch.ccm.CCmatrix[3] = ((float)Complement2(ccm.cnv_3.bits.CCM_CNV_10, 13) / 512);
    m_rAwbSyncInput.main_ch.ccm.CCmatrix[4] = ((float)Complement2(ccm.cnv_3.bits.CCM_CNV_11, 13) / 512);
    m_rAwbSyncInput.main_ch.ccm.CCmatrix[5] = ((float)Complement2(ccm.cnv_4.bits.CCM_CNV_12, 13) / 512);
    m_rAwbSyncInput.main_ch.ccm.CCmatrix[6] = ((float)Complement2(ccm.cnv_5.bits.CCM_CNV_20, 13) / 512);
    m_rAwbSyncInput.main_ch.ccm.CCmatrix[7] = ((float)Complement2(ccm.cnv_5.bits.CCM_CNV_21, 13) / 512);
    m_rAwbSyncInput.main_ch.ccm.CCmatrix[8] = ((float)Complement2(ccm.cnv_6.bits.CCM_CNV_22, 13) / 512);

    IResultPool* pResultPoolSlave = NULL;
    pISPResultToMeta = NULL;
    pResultPoolSlave = IResultPool::getInstance(i4AWBSlaveDev);
    ::memset(rHistoryReqMagic, 0, sizeof(rHistoryReqMagic));
    if (pResultPoolSlave)
    {
        pResultPoolSlave->getHistory(rHistoryReqMagic);
        SYNC3AWRAPPER_LOGW("[%s] History (Req0, Req1, Req2) = (#%d, #%d, #%d)", __FUNCTION__, rHistoryReqMagic[0], rHistoryReqMagic[1], rHistoryReqMagic[2]);
        pISPResultToMeta = (ISPResultToMeta_T*)pResultPoolSlave->getResult(rHistoryReqMagic[1], E_ISP_RESULTTOMETA, __FUNCTION__);
        if (pISPResultToMeta == NULL)
        {
            SYNC3AWRAPPER_LOGW("[%s] get History (Req0) = (#%d)", __FUNCTION__, rHistoryReqMagic[0]);
            pISPResultToMeta = (ISPResultToMeta_T*)pResultPoolSlave->getResult(rHistoryReqMagic[0], E_ISP_RESULTTOMETA, __FUNCTION__);
        }
    }

    if (pISPResultToMeta)
        ccm = pISPResultToMeta->rCamInfo.rMtkCCM;
    else
        SYNC3AWRAPPER_LOGW("[%s] pISPResultToMeta %p", __FUNCTION__, pISPResultToMeta);
    if (sizeof(AWB_OUTPUT_T) == sizeof(AWB_OUTPUT_N3D_T))
    {
        memcpy(&m_rAwbSyncInput.sub_ch.awb_output_struct, &rAwbSlave, sizeof(AWB_OUTPUT_T));
    }
    else
        SYNC3AWRAPPER_LOGE("AWB_OUTPUT_T != AWB_OUTPUT_N3D_T");
    m_rAwbSyncInput.sub_ch.focus_distance = 0;                  // TODO : query from AF, (example : 10 cm)
    IAeMgr::getInstance().sendAECtrl(i4SlaveDev, EAECtrl_GetAAOLineByteSize, reinterpret_cast<MINTPTR>(&(m_rAwbSyncInput.sub_ch.aao_buf_info.stat_line_size)), NULL, NULL, NULL);// TODO
    m_rAwbSyncInput.sub_ch.ccm.CCmatrix[0] = ((float)Complement2(ccm.cnv_1.bits.CCM_CNV_00, 13) / 512);
    m_rAwbSyncInput.sub_ch.ccm.CCmatrix[1] = ((float)Complement2(ccm.cnv_1.bits.CCM_CNV_01, 13) / 512);
    m_rAwbSyncInput.sub_ch.ccm.CCmatrix[2] = ((float)Complement2(ccm.cnv_2.bits.CCM_CNV_02, 13) / 512);
    m_rAwbSyncInput.sub_ch.ccm.CCmatrix[3] = ((float)Complement2(ccm.cnv_3.bits.CCM_CNV_10, 13) / 512);
    m_rAwbSyncInput.sub_ch.ccm.CCmatrix[4] = ((float)Complement2(ccm.cnv_3.bits.CCM_CNV_11, 13) / 512);
    m_rAwbSyncInput.sub_ch.ccm.CCmatrix[5] = ((float)Complement2(ccm.cnv_4.bits.CCM_CNV_12, 13) / 512);
    m_rAwbSyncInput.sub_ch.ccm.CCmatrix[6] = ((float)Complement2(ccm.cnv_5.bits.CCM_CNV_20, 13) / 512);
    m_rAwbSyncInput.sub_ch.ccm.CCmatrix[7] = ((float)Complement2(ccm.cnv_5.bits.CCM_CNV_21, 13) / 512);
    m_rAwbSyncInput.sub_ch.ccm.CCmatrix[8] = ((float)Complement2(ccm.cnv_6.bits.CCM_CNV_22, 13) / 512);

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
    SYNC3AWRAPPER_LOGD_IF(m_Sync3ALogEnable, "[%s] In Main: R(%d), G(%d), B(%d), Main2: R(%d), G(%d), B(%d)", __FUNCTION__,
                rAwbMaster.rAWBSyncInput_N3D.rCurrentGain.i4R, rAwbMaster.rAWBSyncInput_N3D.rCurrentGain.i4G, rAwbMaster.rAWBInfo.rCurrentAWBGain.i4B,
                rAwbSlave.rAWBSyncInput_N3D.rCurrentGain.i4R, rAwbSlave.rAWBSyncInput_N3D.rCurrentGain.i4G, rAwbSlave.rAWBInfo.rCurrentAWBGain.i4B);
    if(m_pMTKSyncAwb)
        m_pMTKSyncAwb->SyncAwbMain(&m_rAwbSyncInput, &m_rAwbSyncOutput);
    SYNC3AWRAPPER_LOGD_IF(m_Sync3ALogEnable, "[%s] Out Main: R(%d), G(%d), B(%d), Main2: R(%d), G(%d), B(%d)", __FUNCTION__,
                m_rAwbSyncOutput.main_ch.rAwbGain.i4R, m_rAwbSyncOutput.main_ch.rAwbGain.i4G, m_rAwbSyncOutput.main_ch.rAwbGain.i4B,
                m_rAwbSyncOutput.sub_ch.rAwbGain.i4R, m_rAwbSyncOutput.sub_ch.rAwbGain.i4G, m_rAwbSyncOutput.sub_ch.rAwbGain.i4B);

    SYNC3AWRAPPER_LOGD("[%s] -", __FUNCTION__);

    return MTRUE;
}


MBOOL
Sync3AWrapper::
update(MINT32 i4Opt)
{
    MINT32 i4MasterDev = Sync3AMgr::getInstance()->getMasterDev();
    MINT32 i4SlaveDev  = Sync3AMgr::getInstance()->getSlaveDev();
    SYNC3AWRAPPER_LOGD("[%s] + M&S Dev(%d,%d), OPT(%d)", __FUNCTION__, i4MasterDev, i4SlaveDev, i4Opt);

    MINT32 i4MeMasterOnOff = 0;
    getPropInt("vendor.debug.sync2a.me.m", &i4MeMasterOnOff, -1);
    if (i4MeMasterOnOff != -1)
    {
        MINT32 i4ExpTime = 0;  // in micro second
        MINT32 i4AfeGain = 0;  // 1024 = 1x
        MINT32 i4IspGain = 0;  // 1024 = 1x
        getPropInt("vendor.debug.sync2a.me.m.exp", &i4ExpTime, 10000);
        getPropInt("vendor.debug.sync2a.me.m.gain", &i4AfeGain, 1024);
        getPropInt("vendor.debug.sync2a.me.m.isp", &i4IspGain, 1024);
        m_rManualAeMaster.u4ExpTime = static_cast<MUINT32>(i4ExpTime);
        m_rManualAeMaster.u4AfeGain = static_cast<MUINT32>(i4AfeGain);
        m_rManualAeMaster.u4IspGain = static_cast<MUINT32>(i4IspGain);


        m_rManualAeMaster.fgOnOff = i4MeMasterOnOff == 1 ? MTRUE : MFALSE;
        if (m_pCcuCtrl3ASync && m_pCcuCtrl3ASync->isSupport3ASync())
            IAeMgr::getInstance().sendAECtrl(i4MasterDev, EAECtrl_SetCCUManualControl, m_rManualAeMaster.u4ExpTime, m_rManualAeMaster.u4AfeGain, m_rManualAeMaster.u4IspGain, m_rManualAeMaster.fgOnOff);
    }

    MINT32 i4MeSlaveOnOff = 0;
    getPropInt("vendor.debug.sync2a.me.s", &i4MeSlaveOnOff, -1);
    if (i4MeSlaveOnOff != -1)
    {
        MINT32 i4ExpTime = 0;  // in micro second
        MINT32 i4AfeGain = 0;  // 1024 = 1x
        MINT32 i4IspGain = 0;  // 1024 = 1x
        getPropInt("vendor.debug.sync2a.me.s.exp", &i4ExpTime, 10000);
        getPropInt("vendor.debug.sync2a.me.s.gain", &i4AfeGain, 1024);
        getPropInt("vendor.debug.sync2a.me.s.isp", &i4IspGain, 1024);
        m_rManualAeSlave.u4ExpTime = static_cast<MUINT32>(i4ExpTime);
        m_rManualAeSlave.u4AfeGain = static_cast<MUINT32>(i4AfeGain);
        m_rManualAeSlave.u4IspGain = static_cast<MUINT32>(i4IspGain);

        m_rManualAeSlave.fgOnOff = i4MeSlaveOnOff == 1 ? MTRUE : MFALSE;
        if (m_pCcuCtrl3ASync && m_pCcuCtrl3ASync->isSupport3ASync())
            IAeMgr::getInstance().sendAECtrl(i4SlaveDev, EAECtrl_SetCCUManualControl, m_rManualAeSlave.u4ExpTime, m_rManualAeSlave.u4AfeGain, m_rManualAeSlave.u4IspGain, m_rManualAeSlave.fgOnOff);
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
            IAeMgr::getInstance().sendAECtrl(i4MasterDev, EAECtrl_SetPreviewParams, reinterpret_cast<MINTPTR>(&(rAEInfoMaster)), m_rAeSyncOutput.main_ch.Output.i4AEidxNext, m_rAeSyncOutput.main_ch.Output.i4AEidxNextF, NULL);
            IAeMgr::getInstance().sendAECtrl(i4SlaveDev, EAECtrl_SetPreviewParams, reinterpret_cast<MINTPTR>(&(rAEInfoSlave)), m_rAeSyncOutput.sub_ch.Output.i4AEidxNext, m_rAeSyncOutput.sub_ch.Output.i4AEidxNextF, NULL);
            IAeMgr::getInstance().sendAECtrl(i4MasterDev, EAECtrl_SetUnderExpdeltaBVIdx, m_rAeSyncOutput.main_ch.Output.i4DeltaBV, NULL, NULL, NULL);
            IAeMgr::getInstance().sendAECtrl(i4SlaveDev, EAECtrl_SetUnderExpdeltaBVIdx, m_rAeSyncOutput.sub_ch.Output.i4DeltaBV, NULL, NULL, NULL);
            IAeMgr::getInstance().sendAECtrl(i4MasterDev, EAECtrl_SetStereoDenoiseRatio, reinterpret_cast<MINTPTR>(&(m_rAeSyncOutput.main_ch.Output.u4DeltaBVRatio[0])), NULL, NULL, NULL);
            IAeMgr::getInstance().sendAECtrl(i4SlaveDev, EAECtrl_SetStereoDenoiseRatio, reinterpret_cast<MINTPTR>(&(m_rAeSyncOutput.sub_ch.Output.u4DeltaBVRatio[0])), NULL, NULL, NULL);
        }
        if (i4Opt & ISync3A::E_SYNC3A_DO_AE_PRECAP)
        {
            IAeMgr::getInstance().sendAECtrl(i4MasterDev, EAECtrl_SetCaptureParams, reinterpret_cast<MINTPTR>(&(rAEInfoMaster)), NULL, NULL, NULL);
            IAeMgr::getInstance().sendAECtrl(i4SlaveDev, EAECtrl_SetCaptureParams, reinterpret_cast<MINTPTR>(&(rAEInfoSlave)), NULL, NULL, NULL);
        }
    }

    if (i4Opt & ISync3A::E_SYNC3A_DO_AWB)
    {
        MINT32 i4Master = Sync3AMgr::getInstance()->getAWBMasterDev();
        MINT32 i4Slave = Sync3AMgr::getInstance()->getAWBSlaveDev();
        AWB_SYNC_OUTPUT_N3D_T rAwbSyncMaster, rAwbSyncSlave;
        rAwbSyncMaster.rAWBGain = m_rAwbSyncOutput.main_ch.rAwbGain;
        rAwbSyncMaster.i4CCT    = m_rAwbSyncOutput.main_ch.i4CCT;
        rAwbSyncSlave.rAWBGain  = m_rAwbSyncOutput.sub_ch.rAwbGain;
        rAwbSyncSlave.i4CCT     = m_rAwbSyncOutput.sub_ch.i4CCT;

        IAwbMgr::getInstance().applyAWB(i4Master, rAwbSyncMaster);
        IAwbMgr::getInstance().applyAWB(i4Slave, rAwbSyncSlave);
    }

    SYNC3AWRAPPER_LOGD("[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
Sync3AWrapper::
setDebugInfo(void* prDbgInfo) const
{
    SYNC3AWRAPPER_LOGD_IF(m_Sync3ALogEnable, "[%s] +", __FUNCTION__);

    N3D_DEBUG_INFO_T* prN3dDbgInfo = reinterpret_cast<N3D_DEBUG_INFO_T*>(prDbgInfo);
    if (!prN3dDbgInfo)
    {
        SYNC3AWRAPPER_LOGE("prN3dDbgInfo is NULL!");
        return MFALSE;
    }
    if (m_pMTKSyncAe)
        m_pMTKSyncAe->SyncAeFeatureCtrl(SYNC_AE_FEATURE_GET_DEBUG_INFO, (void*) &prN3dDbgInfo->rAEDebugInfo, NULL);
    if (m_pMTKSyncAwb)
        m_pMTKSyncAwb->SyncAwbFeatureCtrl(SYNC_AWB_FEATURE_GET_DEBUG_INFO, (void*) &prN3dDbgInfo->rAWBDebugInfo, NULL);
    if (m_pCcuCtrl3ASync && m_pCcuCtrl3ASync->isSupport3ASync())
        m_pCcuCtrl3ASync->getDebugInfo((N3D_AE_DEBUG_INFO_T *)&prN3dDbgInfo->rAEDebugInfo);

    SYNC3AWRAPPER_LOGD_IF(m_Sync3ALogEnable, "[%s] -", __FUNCTION__);
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
        SYNC3AWRAPPER_LOGE("MAKE_HalSensorList() == NULL");
        return BAYER_SENSOR;
    }
    pHalSensorList->querySensorStaticInfo(i4SensorDev, &sensorStaticInfo);

    MUINT32 u4RawFmtType = sensorStaticInfo.rawFmtType; // SENSOR_RAW_MONO or SENSOR_RAW_Bayer
    SYNC3AWRAPPER_LOGD("[%s] SensorDev(%d) u4RawFmtType(%d)\n", __FUNCTION__, i4SensorDev, u4RawFmtType);
    return (u4RawFmtType == SENSOR_RAW_MONO) ? MONO_SENSOR : BAYER_SENSOR;
}

MBOOL
Sync3AWrapper::
syncCCUAeSetting(MINT32 i4MasterIdx, MINT32 i4SlaveIdx, MINT32 i4MasterDev, MINT32 i4SlaveDev)
{
    if (m_pCcuCtrl3ASync && m_pCcuCtrl3ASync->isSupport3ASync())
    {
        struct ccu_sync_ae_settings_data syncAESettingData;
        Stereo_Param_T rParams = Sync3AMgr::getInstance()->getStereoParams();
        syncAESettingData.master_sensor_dev = i4MasterDev;
        syncAESettingData.master_sensor_idx = i4MasterIdx;
        SYNC3AWRAPPER_LOGD("[%s] CCU_SYNC_DBG MSG_TO_CCU_SYNC_AE_SETTING, M(Idx/Dev)(%d/%d)",__FUNCTION__, i4MasterIdx, i4MasterDev);
        m_pCcuCtrl3ASync->ccuControl(MSG_TO_CCU_SYNC_AE_SETTING, &syncAESettingData, NULL);
        return MTRUE;
    }
    else
        return MFALSE;
}

VOID
Sync3AWrapper::
syncAeSetting(MINT32 i4MasterDev, MINT32 i4SlaveDev)
{
    MINT32 i4MasterDevOld = Sync3AMgr::getInstance()->getMasterDev();
    MINT32 i4SlaveDevOld  = Sync3AMgr::getInstance()->getSlaveDev();
    SYNC3AWRAPPER_LOGD("[%s] + M&S old Dev(%d,%d), M&S new Dev(%d,%d) ", __FUNCTION__, i4MasterDevOld, i4SlaveDevOld, i4MasterDev, i4SlaveDev);

    SYNC_AE_DIRECT_INDEX_MAPPING_STRUCT rInput;
    SYNC_AE_DIRECT_MAP_RESULT_STRUCT    rOutput;
    AE_MODE_CFG_T rMasterPreviewInfo, rSlavePreviewInfo;

    // prepare master's input
    const strSyncAEInitInfo* pSyncAEInitInfo = getSyncAEInitInfo();
    strAETable strPreviewAEPlineTableMain, strCaptureAEPlineTabMain;
    strAETable strPreviewAEPlineTableMain2, strCaptureAEPlineTabMain2;
    strAETable strStrobeAEPlineTableMain, strStrobeAEPlineTableMain2;
    strAFPlineInfo strStrobeAFPlineTabMain, strStrobeAFPlineTabMain2;

    AE_PERFRAME_INFO_T AEPerframeInfoMaster;
    AE_PERFRAME_INFO_T AEPerframeInfoSlave;
    IAeMgr::getInstance().getAEInfo(i4MasterDevOld, AEPerframeInfoMaster);
    IAeMgr::getInstance().getAEInfo(i4SlaveDevOld, AEPerframeInfoSlave);

    IAeMgr::getInstance().sendAECtrl(i4MasterDevOld, EAECtrl_GetCurrentPlineTable, reinterpret_cast<MINTPTR>(&(strPreviewAEPlineTableMain)), reinterpret_cast<MINTPTR>(&(strCaptureAEPlineTabMain)), reinterpret_cast<MINTPTR>(&(strStrobeAEPlineTableMain)), reinterpret_cast<MINTPTR>(&(strStrobeAFPlineTabMain)));
    IAeMgr::getInstance().sendAECtrl(i4SlaveDevOld, EAECtrl_GetCurrentPlineTable, reinterpret_cast<MINTPTR>(&(strPreviewAEPlineTableMain2)), reinterpret_cast<MINTPTR>(&(strCaptureAEPlineTabMain2)), reinterpret_cast<MINTPTR>(&(strStrobeAEPlineTableMain2)), reinterpret_cast<MINTPTR>(&(strStrobeAFPlineTabMain2)));

    NVRAM_CAMERA_3A_STRUCT* pMaster3aNvram = NULL;
    NVRAM_CAMERA_3A_STRUCT* pSlave3aNvram = NULL;
    NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, i4MasterDev, (void*&)pMaster3aNvram, 0);
    NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, i4SlaveDev, (void*&)pSlave3aNvram, 0);

    rInput.MasterCamAeIdx = AEPerframeInfoMaster.rAEUpdateInfo.i4AEidxNext;
    rInput.MasterCamAeIdxF = AEPerframeInfoMaster.rAEUpdateInfo.i4AEidxNextF;
    rInput.MasterFinerEVIdxBase = AEPerframeInfoMaster.rAEISPInfo.u4AEFinerEVIdxBase;
    rInput.SlaveFinerEVIdxBase = AEPerframeInfoSlave.rAEISPInfo.u4AEFinerEVIdxBase;
    rInput.MasterCamBVOffset = pMaster3aNvram->AE[0].rCCTConfig.i4BVOffset;
    rInput.SlaveCamBVOffset = pSlave3aNvram->AE[0].rCCTConfig.i4BVOffset;
    rInput.pMasterAETable = &strPreviewAEPlineTableMain;
    rInput.pSlaveAETable = &strPreviewAEPlineTableMain2;
    SYNC3AWRAPPER_LOGD("[%s] M(%d) : AeIdx(%d)/BVOffset(%d)/idxBase(%d), S(%d) : BVOffset(%d)/idxBase(%d)", __FUNCTION__,
             i4MasterDev, rInput.MasterCamAeIdx, rInput.MasterCamBVOffset, rInput.MasterFinerEVIdxBase,
             i4SlaveDev, rInput.SlaveCamBVOffset, rInput.SlaveFinerEVIdxBase);

    rMasterPreviewInfo.u4ExposureMode = AEPerframeInfoMaster.rAEUpdateInfo.u4ExposureMode;
    rMasterPreviewInfo.u2FrameRate = AEPerframeInfoMaster.rAEUpdateInfo.u2FrameRate_x10;
    rMasterPreviewInfo.u4CWValue = AEPerframeInfoMaster.rAEUpdateInfo.u4CWValue;
    rMasterPreviewInfo.u4AvgY = AEPerframeInfoMaster.rAEUpdateInfo.u4AvgY;
    rMasterPreviewInfo.u4RealISO = AEPerframeInfoMaster.rAEISPInfo.u4P1RealISOValue;
    rMasterPreviewInfo.i2FlareOffset = AEPerframeInfoMaster.rAEISPInfo.i2FlareOffset;
    rMasterPreviewInfo.i2FlareGain = AEPerframeInfoMaster.rAEISPInfo.i2FlareGain;
    rMasterPreviewInfo.u4AEFinerEVIdxBase = AEPerframeInfoMaster.rAEISPInfo.u4AEFinerEVIdxBase;;

    // get new setting
    m_pMTKSyncAe->SyncAeFeatureCtrl(SYNC_AE_FEATURE_GET_MAPPING_INDEX, &rInput, &rOutput);

    rMasterPreviewInfo.u4Eposuretime = rOutput.EVSetting.u4Eposuretime;
    rMasterPreviewInfo.u4AfeGain = rOutput.EVSetting.u4AfeGain;
    rMasterPreviewInfo.u4IspGain = rOutput.EVSetting.u4IspGain;
    SYNC3AWRAPPER_LOGD("[%s] Exp(%d), Afe(%d), Isp(%d)", __FUNCTION__, rMasterPreviewInfo.u4Eposuretime, rMasterPreviewInfo.u4AfeGain, rMasterPreviewInfo.u4IspGain);
    // update output to slave
    IAeMgr::getInstance().sendAECtrl(i4SlaveDev, EAECtrl_SetPreviewParams, reinterpret_cast<MINTPTR>(&(rMasterPreviewInfo)), rOutput.u4SlaveAEindex, rOutput.u4SlaveAEindexF, NULL);
    IAeMgr::getInstance().sendAECtrl(i4SlaveDev, EAECtrl_SetExposureSetting, rMasterPreviewInfo.u4Eposuretime, rMasterPreviewInfo.u4AfeGain, rMasterPreviewInfo.u4IspGain, NULL);
}

VOID
Sync3AWrapper::
syncAwbSetting(MINT32 i4MasterDev, MINT32 i4SlaveDev)
{
    SYNC_AWB_DIRECT_GAIN_MAPPING_STRUCT rInput = {0};

    //SYNC_AWB_OUTPUT_INFO_STRUCT rOutput;

    //AWB_CALIBRATION_DATA_T rMasterAWBCalData, rSlaveAWBCalData;

    AWB_OUTPUT_T rAwbMaster = {0}, rAwbSlave = {0};

    IAwbMgr::getInstance().getAWBOutput(i4MasterDev, rAwbMaster);

    //IAwbMgr::getInstance().getAWBUnitGain(i4MasterDev, rMasterAWBCalData);
    //IAwbMgr::getInstance().getAWBUnitGain(i4SlaveDev, rSlaveAWBCalData);
/*
    Stereo_Param_T rParams = ISync3AMgr::getInstance()->getStereoParams();
    switch (rParams.i4Sync2AMode)
    {
    case NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_VSDOF:
        rInput.feature_type = DUAL_CAM_FEATURE_VSDOF;
        break;
    case NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DENOISE:
        rInput.feature_type = DUAL_CAM_FEATURE_BAYER_MONO;
        break;
    case NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DUAL_ZOOM:
        rInput.feature_type = DUAL_CAM_FEATURE_WIDE_TELE;
        break;
    }
*/

    NVRAM_CAMERA_3A_STRUCT* pMaster3aNvram = NULL;
    NVRAM_CAMERA_3A_STRUCT* pSlave3aNvram = NULL;
    NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, i4MasterDev, (void*&)pMaster3aNvram, 0);
    NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, i4SlaveDev, (void*&)pSlave3aNvram, 0);

    rInput.EnStandbyMode = (DUALZOOM_WIDE_STANDY_EN) ? TRUE : FALSE;

    //SYNC3AWRAPPER_LOGD("[%s] MasterCam = %d, Unit Gain (R/G/B) = (%d,%d,%d), Golden Gain (R/G/B) = (%d,%d,%d) ", __FUNCTION__,i4MasterDev,
    //    rMasterAWBCalData.rUnitGain.i4R,rMasterAWBCalData.rUnitGain.i4G,rMasterAWBCalData.rUnitGain.i4B,
    //    rMasterAWBCalData.rGoldenGain.i4R,rMasterAWBCalData.rGoldenGain.i4G,rMasterAWBCalData.rGoldenGain.i4B);

   //SYNC3AWRAPPER_LOGD("[%s] SlaveCam  = %d, Unit Gain (R/G/B) = (%d,%d,%d), Golden Gain (R/G/B) = (%d,%d,%d) ", __FUNCTION__,i4SlaveDev,
   //     rSlaveAWBCalData.rUnitGain.i4R,rSlaveAWBCalData.rUnitGain.i4G,rSlaveAWBCalData.rUnitGain.i4B,
   //     rSlaveAWBCalData.rGoldenGain.i4R,rSlaveAWBCalData.rGoldenGain.i4G,rSlaveAWBCalData.rGoldenGain.i4B);

    SYNC3AWRAPPER_LOGD("[%s] MasterCam = %d, AWB = %d/%d ", __FUNCTION__,i4MasterDev,
        rAwbMaster.rAWBSyncInput_N3D.rCurrentGain,rAwbMaster.rAWBSyncInput_N3D.rStandbySyncGain);

    if(i4MasterDev == ISync3AMgr::getInstance()->getAWBMasterDev())
        rInput.i4Display_cam = 0;
    else
        rInput.i4Display_cam = 1;

    rInput.MasterCalibrationData = (pMaster3aNvram->AWB[CAM_SCENARIO_PREVIEW]).rAlgoCalParam.rCalData;
    rInput.SlaveCalibrationData  = (pSlave3aNvram->AWB[CAM_SCENARIO_PREVIEW]).rAlgoCalParam.rCalData;

    if (sizeof(AWB_OUTPUT_N3D_T) == sizeof(AWB_OUTPUT_T))
    {
        memcpy(&rInput.rAWBSyncData, &rAwbMaster, sizeof(AWB_OUTPUT_N3D_T));
    }
    else
    {
        SYNC3AWRAPPER_LOGE("AWB_OUTPUT_T != AWB_OUTPUT_N3D_T");
    }


    // get new setting
    m_pMTKSyncAwb->SyncAwbFeatureCtrl(SYNC_AWB_FEATURE_GET_MAPPING_GAIN, &rInput, &m_rAwbSyncOutput);

    SYNC3AWRAPPER_LOGD_IF(m_Sync3ALogEnable, "[%s] Out Main: R(%d), G(%d), B(%d), Main2: R(%d), G(%d), B(%d)", __FUNCTION__,
                m_rAwbSyncOutput.main_ch.rAwbGain.i4R, m_rAwbSyncOutput.main_ch.rAwbGain.i4G, m_rAwbSyncOutput.main_ch.rAwbGain.i4B,
                m_rAwbSyncOutput.sub_ch.rAwbGain.i4R, m_rAwbSyncOutput.sub_ch.rAwbGain.i4G, m_rAwbSyncOutput.sub_ch.rAwbGain.i4B);

    // update output to slave
    AWB_SYNC_OUTPUT_N3D_T rAwbSyncSlave = {0};
    rAwbSyncSlave.rAWBGain          = m_rAwbSyncOutput.sub_ch.rAwbGain;
    rAwbSyncSlave.i4CCT             = m_rAwbSyncOutput.sub_ch.i4CCT;
    rAwbSyncSlave.rSyncAWBGain      = m_rAwbSyncOutput.sub_ch.rAwbGain;
    rAwbSyncSlave.i4XR              = m_rAwbSyncInput.sub_ch.awb_output_struct.rAWBSyncInput_N3D.i4XR;
    rAwbSyncSlave.i4YR              = m_rAwbSyncInput.sub_ch.awb_output_struct.rAWBSyncInput_N3D.i4YR;
    rAwbSyncSlave.i4PreviewCam      = m_rAwbSyncInput.sub_ch.awb_output_struct.rAWBSyncOutput_N3D.i4PreviewCam;
    IAwbMgr::getInstance().applyAWB(i4SlaveDev, rAwbSyncSlave);
}

MINT32
Sync3AWrapper::
updateCcuSet3ASyncState()
{
    MINT32 i4Ret = MFALSE;

    if (m_pCcuCtrl3ASync && m_pCcuCtrl3ASync->isSupport3ASync())
    {
        MINT32 i4MasterDev = Sync3AMgr::getInstance()->getMasterDev();
        MINT32 i4SlaveDev  = Sync3AMgr::getInstance()->getSlaveDev();

        struct ccu_3a_sync_state syncState;
        syncState.mode = SYNC;
        syncState.master_sensor_dev = (ESensorDev_T) i4MasterDev;

        Stereo_Param_T rParams = Sync3AMgr::getInstance()->getStereoParams();
        syncState.master_sensor_idx = rParams.i4MasterIdx;
        syncState.master_cam_type = (querySensorType(i4MasterDev) == MONO_SENSOR) ? CCU_MONO_SENSOR : CCU_BAYER_SENSOR;
        syncState.slave_cam_type = (querySensorType(i4SlaveDev) == MONO_SENSOR) ? CCU_MONO_SENSOR : CCU_BAYER_SENSOR;

        switch (rParams.i4Sync2AMode)
        {
        case NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_VSDOF:
            syncState.feature = BAYER_BAYER;
            break;
        case NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DENOISE:
            syncState.feature = BAYER_MONO;
            break;
        case NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DUAL_ZOOM:
            syncState.feature = WIDE_TELE;
            break;
        default:
            syncState.feature = BAYER_BAYER;
            break;
        }
        SYNC3AWRAPPER_LOGD("[%s] CCU_SYNC_DBG MSG_TO_CCU_SET_3A_SYNC_STATE, mode(%d), feature(%d). master_dev(%d) master_Idx(%d)",
                 __FUNCTION__, syncState.mode, syncState.feature, syncState.master_sensor_dev, syncState.master_sensor_idx);
        m_pCcuCtrl3ASync->ccuControl(MSG_TO_CCU_SET_3A_SYNC_STATE, &syncState, NULL);
        i4Ret = MTRUE;
    }

    return i4Ret;
}

VOID
Sync3AWrapper::
updateFrameSync(MBOOL bEnable)
{
    if(m_pCcuCtrl3ASync == NULL)
    {
        m_pCcuCtrl3ASync = ICcuCtrl3ACtrl::createInstance();
        SYNC3AWRAPPER_LOGD("[%s] CCU_SYNC_DBG Create ICcuCtrl3ACtrl", __FUNCTION__);
    }
    if (m_pCcuCtrl3ASync && m_pCcuCtrl3ASync->isSupportFrameSync())
    {
        if (bEnable)
        {
            m_pCcuCtrl3ASync->ccuControl(MSG_TO_CCU_START_FRAME_SYNC, NULL, NULL);
            SYNC3AWRAPPER_LOGD("[%s] CCU_SYNC_DBG MSG_TO_CCU_START_FRAME_SYNC", __FUNCTION__);
        }
        else
        {
            m_pCcuCtrl3ASync->ccuControl(MSG_TO_CCU_STOP_FRAME_SYNC, NULL, NULL);
            SYNC3AWRAPPER_LOGD("[%s] CCU_SYNC_DBG MSG_TO_CCU_STOP_FRAME_SYNC", __FUNCTION__);
        }
    }
    else
        SYNC3AWRAPPER_LOGD("[%s] ICcuCtrl3ACtrl is NULL", __FUNCTION__);
}


/******************************************************************************
 *  Sync3A
 ******************************************************************************/
class Sync3A : public ISync3A
{
public:
    /**
     * @brief Create instance of Sync3A
     */
    static Sync3A*              getInstance();

    /**
     * @brief Control Sync3AWrapper and Sync3AMgr life cycle and initial flow.
     * @param [in] i4Policy
     * @param [in] i4Master
     * @param [in] i4Slave
     * @param [in] strName
     */
    virtual MBOOL               init(MINT32 i4MasterIdx, MINT32 i4SlaveIdx, const char* strName);

    virtual MBOOL               uninit();

    virtual MINT32              sync(MINT32 i4SensorDev, MINT32 i4Param, MVOID* pSttBuf, MINT32 i4FrameNum);

    virtual MINT32              syncAF(MINT32 i4SensorDev, MBOOL initLens);

    virtual MVOID               enableSync(MBOOL fgOnOff);

    virtual MBOOL               isSyncEnable() const;

    virtual MBOOL               isAeStable() const;

    virtual MINT32              getAeSchedule() const;

    virtual MBOOL               setDebugInfo(void* prDbgInfo) const;

    virtual MVOID               enableSyncSupport(E_SYNC3A_SUPPORT iSupport);

    virtual MVOID               disableSyncSupport(E_SYNC3A_SUPPORT iSupport);

    virtual MINT32              getSyncSupport() const;

    virtual MBOOL               isAFSyncFinish();

    virtual MBOOL               is2ASyncFinish();

    virtual MBOOL               isAESyncStable();

    virtual MVOID               sync2ASetting(MINT32 i4MasterIdx, MINT32 i4SlaveIdx);

    virtual MVOID               updateMasterSlave(MINT32 i4MasterDev, MINT32 i4SlaveDev);

    virtual MVOID               updateFrameSync(MBOOL bEnable);

    virtual MVOID               DevCount(MBOOL bEnable, MINT32 i4SensorDev);

    virtual MBOOL               isInit() const;

    virtual MBOOL               isActive() const;

    virtual MINT32              getMasterDev() const;

    virtual MINT32              getSlaveDev() const;

    virtual MVOID               setAFState(MINT32 i4AfState);

    virtual MVOID               setStereoParams(Stereo_Param_T const &rNewParam);

    virtual MINT32              getFrmSyncOpt() const;

    virtual MINT32              getAWBMasterDev() const;

    virtual MINT32              getAWBSlaveDev() const;

    virtual MVOID               setSwitchCamCount(MBOOL bEnable);

    virtual MINT32              getSwitchCamCount() const {return m_SwitchCamCount;}

    virtual MINT32              sendSync3ACtrl(ESync3ACtrl_T eSync3ACtrl, MINTPTR iArg1, MINTPTR iArg2);

protected:
    Sync3A();
    virtual ~Sync3A();

    MINT32                      querySensorType(MINT32 i4SensorDev);
    MBOOL                       wait(MINT32 i4SensorDev);
    MBOOL                       post(MINT32 i4SensorDev);
    MBOOL                       resetScheduler();

    mutable std::mutex          m_Lock;
    mutable std::mutex          m_LockSync3AWrap;
    mutable std::mutex          m_LockSync3AWait;
    MBOOL                       m_fgSyncEnable;
    MINT32                      m_i4SyncSupport;
    MINT32                      m_i4Count;
    MINT32                      m_i4OptFirstIn;
    StatisticBufInfo*           m_pMasterSttBuf;
    StatisticBufInfo*           m_pSlaveSttBuf;
    std::string                 m_strName;
    Sync3AWrapper*              m_pSync3AWrap;
    // AE control
    MINT32                      m_i4AeSchedule;
    // AWB control
    MINT32                      m_i4AwbSyncCount;
    MINT32                      m_Sync3ALogEnable;
    sem_t                       m_semSync;

    MBOOL                       m_fgIsActive;
    MINT32                      m_i4MasterSensorType;
    MINT32                      m_i4SlaveSensorType;
    MBOOL                       m_fgAfState;
    AF_SyncInfo_T               m_sCamInfoHistWide;
    AF_SyncInfo_T               m_sCamInfoHistTele;
    MINT32                      m_i4DevCount;
    //This is for m_fgIsActive
    mutable std::mutex          m_LockIsActive;
    MINT32                      m_i4WaitFramNum;
    MINT32                      m_SwitchCamCount;
    MINT32                      m_IsStart;
};

/******************************************************************************
 *  Sync3A Implementation
 ******************************************************************************/
Sync3A::
Sync3A()
    : m_Lock()
    , m_LockSync3AWrap()
    , m_LockSync3AWait()
    , m_fgSyncEnable(MFALSE)
    , m_i4SyncSupport(3)
    , m_i4Count(0)
    , m_i4OptFirstIn(0)
    , m_pMasterSttBuf(NULL)
    , m_pSlaveSttBuf(NULL)
    , m_pSync3AWrap(NULL)
    , m_i4AeSchedule(0)
    , m_i4AwbSyncCount(0)
    , m_Sync3ALogEnable(0)
    , m_fgIsActive(MFALSE)
    , m_i4MasterSensorType(0)
    , m_i4SlaveSensorType(0)
    , m_fgAfState(MFALSE)
    , m_i4DevCount(0)
    , m_LockIsActive()
    , m_i4WaitFramNum(0)
    , m_SwitchCamCount(2)
    , m_IsStart(1)
{
    getPropInt("vendor.debug.sync3A.log", &m_Sync3ALogEnable, 0);
}

Sync3A::
~Sync3A()
{}

ISync3A*
ISync3A::
getInstance()
{
    return Sync3A::getInstance();
}

Sync3A*
Sync3A::
getInstance()
{
    static Sync3A _rSync3A;
    return &_rSync3A;
}

MBOOL
Sync3A::
init(MINT32 i4MasterIdx, MINT32 i4SlaveIdx, const char * strName)
{
    std::lock_guard<std::mutex> lock_wrap(m_LockSync3AWrap);
    std::lock_guard<std::mutex> lock_active(m_LockIsActive);

    m_fgIsActive = MTRUE;
    m_i4Count = 0;
    m_i4OptFirstIn = 0;
    m_fgSyncEnable = MFALSE;
    m_i4AeSchedule = 0;
    m_i4SyncSupport = E_SYNC3A_SUPPORT_AE | E_SYNC3A_SUPPORT_AWB;
    m_pMasterSttBuf = NULL;
    m_pSlaveSttBuf = NULL;
    m_strName = strName;

    MINT32 i4Ret = sem_init(&m_semSync, 0, 0);
    if(i4Ret != 0)
        SYNC3A_LOGE("m_semSync sem_init fail");

    // Query Master and slave Dev by Idx
    IHalSensorList* pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList) return MFALSE;

    MINT32 i4MasterDev = pHalSensorList->querySensorDevIdx(i4MasterIdx);
    MINT32 i4SlaveDev = pHalSensorList->querySensorDevIdx(i4SlaveIdx);
    m_i4MasterSensorType = querySensorType(i4MasterDev);
    m_i4SlaveSensorType = querySensorType(i4SlaveDev);

    // Init Sync3AMgr
    // Call Sync3AMgr init before Sync3AWrapper. Because update master and slave dev to Sync3AMgr
    Sync3AMgr::getInstance()->init(i4MasterIdx, i4SlaveIdx, i4MasterDev, i4SlaveDev);

    // Create Sync3AWrap & Sync3AMgr instance
    if (m_pSync3AWrap == NULL)
        m_pSync3AWrap = Sync3AWrapper::getInstance();
    // Init Sync3AWrap
    if (m_pSync3AWrap)
        m_pSync3AWrap->init();

    // Flash control setting
    FlashHal::getInstance(i4MasterDev)->setInCharge(MTRUE);
    FlashHal::getInstance(i4SlaveDev)->setInCharge(MFALSE);
    FlashMgr::getInstance(i4MasterDev)->setInCharge(MTRUE);
    FlashMgr::getInstance(i4SlaveDev)->setInCharge(MFALSE);

    // Flicker control life cycle
    IFlickerHal::getInstance(i4MasterDev)->stop();
    IFlickerHal::getInstance(i4SlaveDev)->stop();
    IFlickerHal::getInstance(i4MasterDev)->start(FLK_ATTACH_PRIO_HIGH);
    IFlickerHal::getInstance(i4SlaveDev)->start(FLK_ATTACH_PRIO_LOW);

    // Enable sync flag
    enableSync(MTRUE);

    // AWB AE AF control setting
    MBOOL isMono = MFALSE;
    MINT32 IsSupportAF = MFALSE;
    AFStaticInfo_T staticInfo;
    IAfMgr::getInstance(i4MasterDev).getStaticInfo(staticInfo, LOG_TAG);
    MINT32 IsSupportAFMaster = staticInfo.isAfSupport;
    IAfMgr::getInstance(i4SlaveDev).getStaticInfo(staticInfo, LOG_TAG);
    MINT32 IsSupportAFSlave = staticInfo.isAfSupport;
    IsSupportAF = (IsSupportAFMaster & IsSupportAFSlave);

    if ( m_i4MasterSensorType == SENSOR_RAW_MONO)
    {
        IAwbMgr::getInstance().disableAWB(i4MasterDev);
        IAfMgr::getInstance(i4MasterDev).SyncSetOffMode();
        isMono = MTRUE;
    } else if (m_i4SlaveSensorType == SENSOR_RAW_MONO)
    {
        IAwbMgr::getInstance().disableAWB(i4SlaveDev);
        IAfMgr::getInstance(i4SlaveDev).SyncSetOffMode();
        isMono = MTRUE;
    }

    if (isMono)
    {
        // disable AWB sync
        disableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AWB);

        // enable AF sync
        if(IsSupportAF)
            enableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AF);
        else
            disableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AF);
    }
    else
        // enable AWB sync
        enableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AWB);

    ICcuCtrl3ACtrl* pCcuCtrl3ASync = ICcuCtrl3ACtrl::createInstance();
    if (pCcuCtrl3ASync && pCcuCtrl3ASync->isSupport3ASync())
        // disable AE sync
        disableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AE);
    pCcuCtrl3ASync->destroyInstance();
    pCcuCtrl3ASync = NULL;

    // AWB control setting
    // awb ROI for main2
    //NSCamCustomSensor::SensorViewAngle_T rVangle = NSCamCustomSensor::getSensorViewAngle();
    NSCamCustomSensor::CUSTOM_CFG* pCustomMainCFG = NSCamCustomSensor::getCustomConfig(IMGSENSOR_SENSOR_IDX_MAIN);
    NSCamCustomSensor::CUSTOM_CFG* pCustomMain2CFG = NSCamCustomSensor::getCustomConfig(IMGSENSOR_SENSOR_IDX_MAIN2);

    if ((pCustomMain2CFG->verticalFov < pCustomMainCFG->verticalFov) ||
            (pCustomMain2CFG->horizontalFov < pCustomMainCFG->horizontalFov))
    {
        SYNC3A_LOGD("[%s] Not necessary to config window MainFOV(%d,%d) Main2FOV(%d,%d)",
                 __FUNCTION__, pCustomMainCFG->horizontalFov, pCustomMainCFG->verticalFov, pCustomMain2CFG->horizontalFov , pCustomMain2CFG->verticalFov);
    }
    else
    {
        MINT32 i;
        SensorStaticInfo rSensorStaticInfo;
        pHalSensorList->querySensorStaticInfo(i4SlaveDev, &rSensorStaticInfo);
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

        SYNC3A_LOGD("[%s] u4NumSensorModes(%d), prv(%d,%d), cap(%d,%d), vdo(%d,%d), vdo1(%d,%d), vdo2(%d,%d)", __FUNCTION__, u4NumSensorModes,
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
                                     pCustomMainCFG->verticalFov,
                                     pCustomMainCFG->horizontalFov,
                                     pCustomMain2CFG->verticalFov,
                                     pCustomMain2CFG->horizontalFov,
                                     rSize[i].w, rSize[i].h);

            IAwbMgr::getInstance().setAWBStatCropRegion(
                i4SlaveDev,
                i,
                rCrop.p.x, rCrop.p.y, rCrop.s.w, rCrop.s.h);
        }
    }

    SYNC3A_LOGD("[%s] %s: Master(Dev,Idx,SensorType)(%d, %d, %d), Slave(Dev,Idx,SensorType)(%d, %d, %d), m_pSync3AWrap(%p)",
             __FUNCTION__, m_strName.c_str(), i4MasterDev, i4MasterIdx, m_i4MasterSensorType, i4SlaveDev, i4SlaveIdx, m_i4SlaveSensorType, m_pSync3AWrap);

    return MTRUE;
}

MBOOL
Sync3A::
uninit()
{
    std::lock_guard<std::mutex> lock_wrap(m_LockSync3AWrap);
    std::lock_guard<std::mutex> lock_active(m_LockIsActive);

    if(m_fgIsActive)
    {
        MINT32 i4MasterDev = Sync3AMgr::getInstance()->getMasterDev();
        MINT32 i4SlaveDev  = Sync3AMgr::getInstance()->getSlaveDev();
        SYNC3A_LOGD("[%s] + M&S Dev(%d,%d)", __FUNCTION__, i4MasterDev, i4SlaveDev);

        enableSync(MFALSE);

        if (m_pSync3AWrap)
        {
            m_pSync3AWrap->uninit();
            m_pSync3AWrap = NULL;
        }

        Sync3AMgr::getInstance()->uninit();
        m_fgIsActive = MFALSE;

        m_pMasterSttBuf = NULL;
        m_pSlaveSttBuf = NULL;
        m_fgSyncEnable = MFALSE;

        MBOOL bEnableAEStereoManualPline = MFALSE;
        IAeMgr::getInstance().sendAECtrl(i4MasterDev, EAECtrl_EnableAEStereoManualPline, bEnableAEStereoManualPline, NULL, NULL, NULL);
        IAeMgr::getInstance().sendAECtrl(i4SlaveDev, EAECtrl_EnableAEStereoManualPline, bEnableAEStereoManualPline, NULL, NULL, NULL);

        // reset Master AF sync status
        AF_SyncInfo_T sCamInfo;
        memset(&sCamInfo, 0, sizeof(sCamInfo));
        IAfMgr::getInstance(i4MasterDev).SyncAFGetInfo( sCamInfo);

        // reset aaa sensor status
        AAASensorMgr::getInstance().disableFrameSync();

        SYNC3A_LOGD("[%s] - %s: OK", __FUNCTION__, m_strName.c_str());
    }
    return MTRUE;
}

MVOID
Sync3A::
enableSync(MBOOL fgOnOff)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    if (fgOnOff == MFALSE)
    {
        if (m_i4Count < 0)
        {
            m_i4Count ++;
            ::sem_post(&m_semSync);
            SYNC3A_LOGD("[%s] %s: Disable Sync: post(%d)", __FUNCTION__, m_strName.c_str(), m_i4Count);
        }
    }
    else
    {
        m_i4AwbSyncCount = 3;
        MINT32 i4Ret = sem_init(&m_semSync, 0, 0);
        if(i4Ret != 0)
            SYNC3A_LOGE("m_semSync sem_init fail");
    }
    SYNC3A_LOGD("[%s] %s Sync OnOff(%d)", __FUNCTION__, m_strName.c_str(), fgOnOff);
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
    SYNC3A_LOGD("[%s] m_i4SyncSupport (%d -> %d), iSupport(%d)", __FUNCTION__, i4OldSyncSupport, m_i4SyncSupport, iSupport);
}

MVOID
Sync3A::
disableSyncSupport(E_SYNC3A_SUPPORT iSupport)
{
    MINT32 i4OldSyncSupport = m_i4SyncSupport;
    m_i4SyncSupport &= (~iSupport);
    SYNC3A_LOGD("[%s] m_i4SyncSupport (%d -> %d), iSupport(%d)", __FUNCTION__, i4OldSyncSupport, m_i4SyncSupport, iSupport);
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
    if ( (getSyncSupport() & E_SYNC3A_SUPPORT_AF) && isSyncEnable() )
    {
        MINT32 i4MasterDev = Sync3AMgr::getInstance()->getMasterDev();
        MINT32 i4SlaveDev  = Sync3AMgr::getInstance()->getSlaveDev();

        AFResult_T rAFResultFromMgrMaster;
        AFResult_T rAFResultFromMgrSlave;
        IAfMgr::getInstance(i4MasterDev).getResult(rAFResultFromMgrMaster);
        IAfMgr::getInstance(i4SlaveDev).getResult(rAFResultFromMgrSlave);

        SYNC3A_LOGD("[%s] M&S Dev(%d,%d) isFocusFinish = (%d,%d)", __FUNCTION__,
                 i4MasterDev, i4SlaveDev,
                 rAFResultFromMgrMaster.isFocusFinish,
                 rAFResultFromMgrSlave.isFocusFinish);
        return (rAFResultFromMgrMaster.isFocusFinish && rAFResultFromMgrSlave.isFocusFinish);
    }
    return MFALSE;
}

MBOOL
Sync3A::
is2ASyncFinish()
{
    if ( (getSyncSupport() & (E_SYNC3A_SUPPORT_AE | E_SYNC3A_SUPPORT_AWB)) && isSyncEnable() )
    {
        SYNC3A_LOGD_IF(m_i4AwbSyncCount, "[%s] m_i4AwbSyncCount = (%d)", __FUNCTION__, m_i4AwbSyncCount);
        if (m_i4AwbSyncCount == 0)
            return MTRUE;
    }
    return MFALSE;
}

MBOOL
Sync3A::
isAESyncStable()
{
    if ((getSyncSupport() & E_SYNC3A_SUPPORT_AE))
    {
        MINT32 i4MasterDev = Sync3AMgr::getInstance()->getMasterDev();
        MINT32 i4SlaveDev  = Sync3AMgr::getInstance()->getSlaveDev();

        AE_PERFRAME_INFO_T AEPerframeInfoMaster;
        AE_PERFRAME_INFO_T AEPerframeInfoSlave;
        IAeMgr::getInstance().getAEInfo(i4MasterDev, AEPerframeInfoMaster);
        IAeMgr::getInstance().getAEInfo(i4SlaveDev, AEPerframeInfoSlave);

        SYNC3A_LOGD("[%s] M&S Dev(%d,%d) bAEStable = (%d,%d)", __FUNCTION__,
                 i4MasterDev, i4SlaveDev,
                 AEPerframeInfoMaster.rAEISPInfo.bAEStable,
                 AEPerframeInfoSlave.rAEISPInfo.bAEStable);
        return AEPerframeInfoMaster.rAEISPInfo.bAEStable;
    }
    return MTRUE;
}

MINT32
Sync3A::
sync(MINT32 i4SensorDev, MINT32 i4Param, MVOID* pSttBuf, MINT32 i4FrameNum)
{
    MINT32 i4Ret = 0;
    MINT32 i4SemVal;

    MINT32 i4MasterDev = Sync3AMgr::getInstance()->getMasterDev();
    MINT32 i4SlaveDev  = Sync3AMgr::getInstance()->getSlaveDev();

    if (i4SensorDev == i4MasterDev)
        m_pMasterSttBuf = reinterpret_cast<StatisticBufInfo*>(pSttBuf);
    else if (i4SensorDev == i4SlaveDev)
        m_pSlaveSttBuf = reinterpret_cast<StatisticBufInfo*>(pSttBuf);

    //if (!(i4Param & E_SYNC3A_DO_SW_SYNC))
    if (MFALSE)
    {
        MBOOL bIsHwSync = MFALSE;
        if (i4Param & E_SYNC3A_DO_HW_SYNC)
            bIsHwSync = MTRUE;

        MINT32 i4BypSyncCalc = 0;
        getPropInt("vendor.debug.sync2a.byp", &i4BypSyncCalc, 0);
        // Slave do AE sync
        if ( (i4SensorDev == i4SlaveDev && !bIsHwSync) ||
                (i4SensorDev == i4MasterDev && bIsHwSync) )
        {
            MINT32 i4Opt = i4Param & (~i4BypSyncCalc);
            MINT32 i4OldOpt = i4Opt;
            // if disable AE sync, remove AE opt
            if (!(m_i4SyncSupport & E_SYNC3A_SUPPORT_AE))
            {
                i4Opt &= (~ISync3A::E_SYNC3A_DO_AE);
                i4Opt &= (~ISync3A::E_SYNC3A_DO_AE_PRECAP);
            }
            // remove AWB opt
            i4Opt &= (~ISync3A::E_SYNC3A_DO_AWB);
            SYNC3A_LOGD("[%s] m_i4SyncSupport(%d), AE i4Opt(%d -> %d)", __FUNCTION__, m_i4SyncSupport, i4OldOpt, i4Opt);

            m_LockSync3AWrap.lock();
            if (m_pSync3AWrap)
                m_pSync3AWrap->syncCalc(i4Opt, m_pMasterSttBuf, m_pSlaveSttBuf);
            m_LockSync3AWrap.unlock();

            if (bIsHwSync && (m_i4AeSchedule == 1)) // AE calculation frame
            {
                // 2A frame sync
                AE_PERFRAME_INFO_T AEPerframeInfoMain;
                AE_PERFRAME_INFO_T AEPerframeInfoMain2;
                IAeMgr::getInstance().getAEInfo(i4MasterDev, AEPerframeInfoMain);
                IAeMgr::getInstance().getAEInfo(i4SlaveDev, AEPerframeInfoMain2);
                AAASensorMgr::getInstance().updateStereoFrameSyncFps(i4MasterDev, i4SlaveDev, AEPerframeInfoMain.rAEISPInfo.u8P1Exposuretime_ns / 1000, AEPerframeInfoMain2.rAEISPInfo.u8P1Exposuretime_ns / 1000);
            }
        }
        if (i4SensorDev == i4MasterDev)
        {
            m_i4AeSchedule = (m_i4AeSchedule + 1) % 3;
        }

        // Slave do AWB sync
        if (i4SensorDev != Sync3AMgr::getInstance()->getAWBMasterDev())
        {
            MINT32 i4Opt = i4Param & (~i4BypSyncCalc);
            MINT32 i4OldOpt = i4Opt;
            // remove AE opt
            i4Opt &= (~ISync3A::E_SYNC3A_DO_AE);
            i4Opt &= (~ISync3A::E_SYNC3A_DO_AE_PRECAP);
            // if disable AWB sync, remove AWB opt
            if (!(m_i4SyncSupport & E_SYNC3A_SUPPORT_AWB))
                i4Opt &= (~ISync3A::E_SYNC3A_DO_AWB);
            SYNC3A_LOGD("[%s] m_i4SyncSupport(%d), AWB i4Opt(%d -> %d)", __FUNCTION__, m_i4SyncSupport, i4OldOpt, i4Opt);

            m_LockSync3AWrap.lock();
            if (m_pSync3AWrap)
                m_pSync3AWrap->syncCalc(i4Opt, m_pMasterSttBuf, m_pSlaveSttBuf);
            m_LockSync3AWrap.unlock();

            if (m_i4AwbSyncCount > 0)
                m_i4AwbSyncCount--;
        }
    } else
    {
        {
            std::lock_guard<std::mutex> autoLock(m_Lock);
            i4SemVal = m_i4Count;
            if (i4SemVal >= 0)
            {
                m_i4Count --;
                i4Ret = 0;

                //Save wait_FrameNum
                if(i4SensorDev == i4MasterDev)
                   m_i4WaitFramNum = i4FrameNum;
                else if(i4SensorDev == i4SlaveDev)
                   m_i4WaitFramNum = i4FrameNum;
                SYNC3A_LOGD_IF(m_Sync3ALogEnable, "[%s](0) %s: eSensor(%d) Wait(%d) WaitFrm(#%d) i4Param(0x%08x)", __FUNCTION__, m_strName.c_str(), i4SensorDev, m_i4Count, m_i4WaitFramNum, i4Param);
            }
            else
            {
                //compare post_FrameNum with wait_FrameNum
                if(i4FrameNum == m_i4WaitFramNum)
                {
                    m_i4Count ++;
                    SYNC3A_LOGD_IF(m_Sync3ALogEnable, "[%s](2) %s: eSensor(%d) Post(%d) PostFrm(#%d) i4Param(0x%08x)", __FUNCTION__, m_strName.c_str(), i4SensorDev, m_i4Count, i4FrameNum, i4Param);
                    i4Ret = 1;
                }
                else
                {
                    SYNC3A_LOGD_IF(m_Sync3ALogEnable, "[%s](2) %s: eSensor(%d) Post(%d) PostFrm(#%d) i4Param(0x%08x)", __FUNCTION__, m_strName.c_str(), i4SensorDev, m_i4Count, i4FrameNum, i4Param);
                    i4Ret = -1;
                }
            }
        }

        if (i4Ret == 0)
        {
            m_i4OptFirstIn = i4Param;
            wait(i4SensorDev);
        }
        else if(i4Ret == 1)
        {
            if ((i4Param | m_i4OptFirstIn) & ISync3A::E_SYNC3A_BYP_AE)
            {
                SYNC3A_LOGD("%s: Ignore AEAWB: eSensor(%d), Opt(0x%08x), another Opt(0x%08x)", m_strName.c_str(), i4SensorDev, i4Param, m_i4OptFirstIn);
                i4Param &= (~(ISync3A::E_SYNC3A_DO_AE | ISync3A::E_SYNC3A_DO_AWB));
            }
            else if((i4Param|m_i4OptFirstIn) & ISync3A::E_SYNC3A_DO_AE)
            {
                i4Param |= (ISync3A::E_SYNC3A_DO_AE);
            }

            MINT32 i4BypSyncCalc = 0;
            getPropInt("vendor.debug.sync2a.byp", &i4BypSyncCalc, 0);

            // calculation
            SYNC3A_LOGD_IF(m_Sync3ALogEnable, "[%s](3) %s: Sync 2A: Sensor(%d) Calculation", __FUNCTION__, m_strName.c_str(), i4SensorDev);

            // 2a sync do calculation
            MINT32 i4Opt = i4Param & (~i4BypSyncCalc);
            SYNC3A_LOGD("[%s] i4Opt = %d", __FUNCTION__, i4Opt);
            if(!(m_i4SyncSupport & E_SYNC3A_SUPPORT_AE))
            {
                i4Opt &= (~ISync3A::E_SYNC3A_DO_AE);
                i4Opt &= (~ISync3A::E_SYNC3A_DO_AE_PRECAP);
            }
            if(!(m_i4SyncSupport & E_SYNC3A_SUPPORT_AWB))
                i4Opt &= (~ISync3A::E_SYNC3A_DO_AWB);
            SYNC3A_LOGD("[%s] i4Opt = %d", __FUNCTION__, i4Opt);

            m_LockSync3AWrap.lock();
            if (m_pSync3AWrap)
                m_pSync3AWrap->syncCalc(i4Opt, m_pMasterSttBuf, m_pSlaveSttBuf);
            m_LockSync3AWrap.unlock();

            m_i4AeSchedule = (m_i4AeSchedule + 1) % 3;
            if (m_i4AwbSyncCount > 0)
                m_i4AwbSyncCount--;

            if (m_i4AeSchedule == 1) { // AE calculation frame
                // 2A frame sync
                AE_PERFRAME_INFO_T AEPerframeInfoMain;
                AE_PERFRAME_INFO_T AEPerframeInfoMain2;
                IAeMgr::getInstance().getAEInfo(i4MasterDev, AEPerframeInfoMain);
                IAeMgr::getInstance().getAEInfo(i4SlaveDev, AEPerframeInfoMain2);
                AAASensorMgr::getInstance().updateStereoFrameSyncFps(i4MasterDev, i4SlaveDev, AEPerframeInfoMain.rAEISPInfo.u8P1Exposuretime_ns / 1000, AEPerframeInfoMain2.rAEISPInfo.u8P1Exposuretime_ns / 1000);
            }
            post(i4SensorDev);
            m_pMasterSttBuf = NULL;
            m_pSlaveSttBuf = NULL;
            m_i4WaitFramNum = 0;
        }
        else
            SYNC3A_LOGW("[%s] Case1: PostFrm(#%d) isn't equre WaitFrm(#%d)\n", __FUNCTION__, i4FrameNum, m_i4WaitFramNum);
    }
    return i4Ret;
}

MINT32
Sync3A::
syncAF(MINT32 i4SensorDev, MBOOL initLens)
{

    MINT32 i4MasterDev = Sync3AMgr::getInstance()->getMasterDev();
    MINT32 i4SlaveDev  = Sync3AMgr::getInstance()->getSlaveDev();
    MINT32 i4Ret = 0, syncPos = 0;
    if ((i4SensorDev == i4MasterDev && (getSyncSupport() & E_SYNC3A_SUPPORT_AF)) || initLens)
    {
        MINT32 i4MfMasterOnOff = 0;
        getPropInt("vendor.debug.sync2a.mf.m", &i4MfMasterOnOff, 0);

        MINT32 i4MfSlaveOnOff = 0;
        getPropInt("vendor.debug.sync2a.mf.s", &i4MfSlaveOnOff, 0);

        SYNC3A_LOGD("[%s](i4MfMasterOnOff,i4MfSlaveOnOff) = (%d,%d)", __FUNCTION__, i4MfMasterOnOff, i4MfSlaveOnOff);

        if (i4MfMasterOnOff)
        {
            MINT32 i4MfMasterPosition;
            getPropInt("vendor.debug.sync2a.mf.m.pos", &i4MfMasterPosition, 0);
            SYNC3A_LOGD("[%s] i4MfMasterPosition = %d", __FUNCTION__, i4MfMasterPosition);
            IAfMgr::getInstance(i4MasterDev).SyncSetMFPos( i4MfMasterPosition); // master : move lens via mf control
        }

        // prepare the data for AF sync
        // do AF algo for remapping position to Main2
        if (i4MfSlaveOnOff)
        {
            MINT32 i4MfSlavePosition;
            getPropInt("vendor.debug.sync2a.mf.s.pos", &i4MfSlavePosition, 0);
            SYNC3A_LOGD("[%s] i4MfSlavePosition = %d", __FUNCTION__, i4MfSlavePosition);
            IAfMgr::getInstance(i4SlaveDev).SyncMoveLens( i4MfSlavePosition);  // slave : move lens directly
        }
        else
        {
            Stereo_Param_T rParams = Sync3AMgr::getInstance()->getStereoParams();
            MINT32 sync2AMode = 0;
            switch (rParams.i4Sync2AMode)
            {
            case NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_VSDOF:
                sync2AMode=1;
                break;
            case NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DENOISE:
                sync2AMode=2;
                break;
            case NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DUAL_ZOOM:
                sync2AMode=3;
                break;
            default:
                sync2AMode=0;
                break;
            }
            AF_SyncInfo_T sCamInfo;
            memset(&sCamInfo,  0, sizeof(sCamInfo));
            if (initLens)
            {
                sCamInfo.i4SyncStatus = SYNC_STATUS_INITLENS;
            }

            AFStaticInfo_T staticInfo;
            IAfMgr::getInstance(i4MasterDev).getStaticInfo(staticInfo, LOG_TAG);
            if (staticInfo.isCCUAF)
            {
                // CCU version SyncAF
                MINT32 slaveDev = 0, slaveIdx = 0;
                IAfMgr::getInstance(i4SlaveDev).SyncAFGetSensorInfoForCCU(slaveDev, slaveIdx);
                IAfMgr::getInstance(i4MasterDev).SyncAFProcess(slaveDev, slaveIdx, sync2AMode, sCamInfo);
            }
            else
            {
                // CPU version SyncAF
                IAfMgr::getInstance(i4SlaveDev).SyncAFSetMode(sync2AMode);
                IAfMgr::getInstance(i4SlaveDev).SyncAFGetMotorRange( sCamInfo);
                syncPos = IAfMgr::getInstance(i4MasterDev).SyncAFGetInfo( sCamInfo);
                if (syncPos != 0xFFFF)
                {
                    IAfMgr::getInstance(i4SlaveDev).SyncAFSetInfo( syncPos, sCamInfo);
                    IAfMgr::getInstance(i4SlaveDev).SyncMoveLens( syncPos);
                }
                else
                    SYNC3A_LOGD("[syncPos] err");

                IAfMgr::getInstance(i4SlaveDev).SyncAFGetCalibPos( sCamInfo);
                IAfMgr::getInstance(i4MasterDev).SyncAFCalibPos( sCamInfo);
            }
        }
    }

    return i4Ret;
}

MBOOL
Sync3A::
wait(MINT32 i4SensorDev)
{
    MBOOL fgRet = MTRUE;
    SYNC3A_LOGD("[%s](1) %s: + eSensor(%d), m_fgSyncEnable(%d)", __FUNCTION__, m_strName.c_str(), i4SensorDev, m_fgSyncEnable);
    if (m_fgSyncEnable)
    {
        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
            SYNC3A_LOGE("[%s] error in clock_gettime! Please check\n", __FUNCTION__);

        ts.tv_sec  += SEM_TIME_LIMIT_NS/1000000000;
        ts.tv_nsec += SEM_TIME_LIMIT_NS%1000000000;
        if (ts.tv_nsec >= 1000000000)
        {
            ts.tv_nsec -= 1000000000;
            ts.tv_sec += 1;
        }
        int s = ::sem_timedwait(&m_semSync, &ts);
        if (s == -1)
        {
            if (errno == ETIMEDOUT)
            {
                SYNC3A_LOGD("[%s] %s: eSensor(%d), TimeOut(%d ns)", __FUNCTION__, m_strName.c_str(), i4SensorDev, SEM_TIME_LIMIT_NS);
            }
            else
                SYNC3A_LOGD("[%s] %s: eSensor(%d) sem_timedwait() errno = %d\n", __FUNCTION__, m_strName.c_str(), i4SensorDev, errno);
        }
    }

    if (!m_fgSyncEnable)
        fgRet = MFALSE;
    SYNC3A_LOGD("[%s](5) %s: - eSensor(%d), m_fgSyncEnable(%d)", __FUNCTION__, m_strName.c_str(), i4SensorDev, m_fgSyncEnable);
    return fgRet;
}

MBOOL
Sync3A::
post(MINT32 i4SensorDev)
{
    SYNC3A_LOGD("[%s](4) %s: eSensor(%d)", __FUNCTION__, m_strName.c_str(), i4SensorDev);
    ::sem_post(&m_semSync);
    return MTRUE;
}

MBOOL
Sync3A::
resetScheduler()
{
    SYNC3A_LOGD("[%s]", __FUNCTION__);
    m_i4AeSchedule = 0;
    return MTRUE;
}

MBOOL
Sync3A::
isAeStable() const
{
    std::lock_guard<std::mutex> lock_wrap(m_LockSync3AWrap);

    if (m_pSync3AWrap)
        return m_pSync3AWrap->isAeStable();

    return MFALSE;
}

MBOOL
Sync3A::
setDebugInfo(void* prDbgInfo) const
{
    std::lock_guard<std::mutex> lock_wrap(m_LockSync3AWrap);

    if (m_pSync3AWrap)
        return m_pSync3AWrap->setDebugInfo(prDbgInfo);

    return MFALSE;
}

MINT32
Sync3A::
getAeSchedule() const
{
    if (m_i4AeSchedule == 0)
        return E_AE_AE_CALC;
    else
        return E_AE_IDLE;
}

MVOID
Sync3A::
sync2ASetting(MINT32 i4MasterIdx, MINT32 i4SlaveIdx)
{
    // sync2ASetting and updateMasterSlave deadlock
    // MW call this API only, so don't add mutex(m_LockSync3AWrap)
    // Triple camera will change index, so need updateMasterSlave.
    // std::lock_guard<std::mutex> lock_wrap(m_LockSync3AWrap);
    // m_pSync3AWrap don't use this,because racing condition with uninit

    Sync3AWrapper* pSync3AWrap = Sync3AWrapper::getInstance();

    if (pSync3AWrap)
    {
        IHalSensorList* pHalSensorList = MAKE_HalSensorList();
        if (!pHalSensorList) return;

        MINT32 i4MasterDev = pHalSensorList->querySensorDevIdx(i4MasterIdx);
        MINT32 i4SlaveDev = pHalSensorList->querySensorDevIdx(i4SlaveIdx);
        SYNC3A_LOGD("[%s] M(Idx/Dev)(%d/%d) S(Idx/Dev)(%d/%d)", __FUNCTION__, i4MasterIdx, i4MasterDev, i4SlaveIdx, i4SlaveDev);
        pSync3AWrap->init();
        MBOOL bIsSupportCCU = pSync3AWrap->syncCCUAeSetting(i4MasterIdx, i4SlaveIdx, i4MasterDev, i4SlaveDev);
        if(!bIsSupportCCU)
            pSync3AWrap->syncAeSetting(i4MasterDev, i4SlaveDev);

        pSync3AWrap->syncAwbSetting(i4MasterDev, i4SlaveDev);
        pSync3AWrap->uninit();
    }
}

MVOID
Sync3A::
updateMasterSlave(MINT32 i4MasterDev, MINT32 i4SlaveDev)
{
    std::lock_guard<std::mutex> lock_wrap(m_LockSync3AWrap);
    Sync3AMgr::getInstance()->updateMasterSlave(i4MasterDev, i4SlaveDev);
}

MVOID
Sync3A::
updateFrameSync(MBOOL bEnable)
{
    std::lock_guard<std::mutex> lock_wrap(m_LockSync3AWrap);
    if (m_pSync3AWrap)
        m_pSync3AWrap->updateFrameSync(bEnable);
}

MVOID
Sync3A::
DevCount(MBOOL bEnable, MINT32 i4SensorDev)
{
    SYNC3A_LOGD("[%s] bEnable(%d) m_i4DevCount(%d) i4SensorDev(%d) +", __FUNCTION__, bEnable, m_i4DevCount, i4SensorDev);
    if (bEnable)
        m_i4DevCount++;
    else
        m_i4DevCount--;
    SYNC3A_LOGD("[%s] bEnable(%d) m_i4DevCount(%d) i4SensorDev(%d) -", __FUNCTION__, bEnable, m_i4DevCount, i4SensorDev);
}

MBOOL
Sync3A::
isInit() const
{
    std::lock_guard<std::mutex> lock_active(m_LockIsActive);
    return m_fgIsActive;
}

MBOOL
Sync3A::
isActive() const
{
    std::lock_guard<std::mutex> lock_active(m_LockIsActive);
    //return (m_fgIsActive && m_i4DevCount == 2);
    return (m_fgIsActive);
}

MINT32
Sync3A::
getMasterDev() const
{
    MINT32 i4MasterDev = Sync3AMgr::getInstance()->getMasterDev();
    return i4MasterDev;
}

MINT32
Sync3A::
getSlaveDev() const
{
    MINT32 i4SlaveDev  = Sync3AMgr::getInstance()->getSlaveDev();
    return i4SlaveDev;
}


MVOID
Sync3A::
setAFState(MINT32 i4AfState)
{
    if (m_fgAfState != i4AfState)
    {
        MINT32 i4SlaveDev = Sync3AMgr::getInstance()->getSlaveDev();
        SYNC3A_LOGD("[%s] State(%d -> %d)", __FUNCTION__, m_fgAfState, i4AfState);
        MBOOL bEnableAEOneShotControl = (i4AfState != 0) ? MTRUE : MFALSE;
        IAeMgr::getInstance().sendAECtrl(i4SlaveDev, EAECtrl_EnableAEOneShotControl, bEnableAEOneShotControl, NULL, NULL, NULL);
        m_fgAfState = i4AfState;
    }
}

MVOID
Sync3A::
setStereoParams(Stereo_Param_T const &rNewParam)
{
    MINT32 i4Sync2AMode = rNewParam.i4Sync2AMode;
    MINT32 i4SyncAFMode = rNewParam.i4SyncAFMode;
    MINT32 i4HwSyncMode = rNewParam.i4HwSyncMode;
    MINT32 i4MasterIdx = rNewParam.i4MasterIdx;
    MINT32 i4Sync3ASwitchOn = rNewParam.i4Sync3ASwitchOn;

    if( (m_SwitchCamCount != 2) && (i4Sync3ASwitchOn == NS3Av3::E_SYNC3A_NOTIFY::E_SYNC3A_NOTIFY_SWITCH_ON) )
    {
        m_SwitchCamCount++;
        SYNC3A_LOGD("[%s] m_SwitchCamCount(%d)", __FUNCTION__, m_SwitchCamCount);
    }

    // get old Stereo Params & update new Stereo Params
    Stereo_Param_T rOldStereoParams;
    rOldStereoParams = Sync3AMgr::getInstance()->getStereoParams();
    Sync3AMgr::getInstance()->setStereoParams(rNewParam);

    MINT32 i4MasterDev = Sync3AMgr::getInstance()->getMasterDev();
    MINT32 i4SlaveDev  = Sync3AMgr::getInstance()->getSlaveDev();

    if (i4MasterIdx != 0 || i4SlaveIdx != 0)
    {
        IHalSensorList* pHalSensorList = MAKE_HalSensorList();
        if (!pHalSensorList) return;

        MINT32 newMasterDev = pHalSensorList->querySensorDevIdx(i4MasterIdx);
        MINT32 newSlaveDev = pHalSensorList->querySensorDevIdx(i4SlaveIdx);
        if (i4MasterDev != newMasterDev || i4SlaveDev != newSlaveDev)
        {
            SYNC3A_LOGD("[%s] M&S : old(%d,%d), new(%d,%d)", __FUNCTION__,
                     i4MasterDev, i4SlaveDev,
                     newMasterDev, newSlaveDev);
            // update new master and slave dev
            i4MasterDev = newMasterDev;
            i4SlaveDev = newSlaveDev;

            //W+T
            FlashHal::getInstance(i4MasterDev)->setInCharge(MTRUE);
            FlashHal::getInstance(i4SlaveDev)->setInCharge(MFALSE);
            FlashMgr::getInstance(i4MasterDev)->setInCharge(MTRUE);
            FlashMgr::getInstance(i4SlaveDev)->setInCharge(MFALSE);

            IFlickerHal::getInstance(i4MasterDev)->stop();
            IFlickerHal::getInstance(i4SlaveDev)->stop();
            IFlickerHal::getInstance(i4MasterDev)->start(FLK_ATTACH_PRIO_HIGH);
            IFlickerHal::getInstance(i4SlaveDev)->start(FLK_ATTACH_PRIO_LOW);

            IAfMgr::getInstance(i4SlaveDev).SyncAFSetMode(0xFE);
            IAfMgr::getInstance(i4MasterDev).SyncAFSetMode(0xFF);

            updateMasterSlave(i4MasterDev, i4SlaveDev);
            if(m_pSync3AWrap && ((i4Sync3ASwitchOn == NS3Av3::E_SYNC3A_NOTIFY::E_SYNC3A_NOTIFY_SWITCH_ON) || (m_SwitchCamCount == 2)))
                m_pSync3AWrap->updateCcuSet3ASyncState();
        }
        //W+T store master history and sent slave history
        if (rOldStereoParams.i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DUAL_ZOOM)
        {
            if (i4MasterDev == ESensorDevId_Main)
            {
                if (i4MasterDev == newMasterDev || i4SlaveDev == newSlaveDev)
                {
                    IAfMgr::getInstance(i4MasterDev).SyncAFGetInfo( m_sCamInfoHistWide);// store master_Wide cam info
                    IAfMgr::getInstance(i4MasterDev).SyncAFSetInfo( 0xFFFF, m_sCamInfoHistTele);// sent slave history to master
                }
            }
            else if (i4MasterDev == ESensorDevId_MainSecond)
            {
                if (i4MasterDev == newMasterDev || i4SlaveDev == newSlaveDev)
                {
                    IAfMgr::getInstance(i4MasterDev).SyncAFGetInfo( m_sCamInfoHistTele);// store master_Tele cam info
                    IAfMgr::getInstance(i4MasterDev).SyncAFSetInfo( 0xFFFF, m_sCamInfoHistWide);// sent slave history to master
                }
            }
        }

        AWB_SYNC_INPUT_N3D_T AWBSyncInfo = {};
        AWBSyncInfo.i4SyncMode  = i4Sync2AMode;
        AWBSyncInfo.i4MasterDev = i4MasterDev;
        AWBSyncInfo.i4SyncEn    = isActive();
        AWBSyncInfo.ZoomRatio   = i4ZoomRatio;
        IAwbMgr::getInstance().setAWBSyncInfo(i4MasterDev, AWBSyncInfo);
        IAwbMgr::getInstance().setAWBSyncInfo(i4SlaveDev, AWBSyncInfo);
    }

    if (i4Sync2AMode != NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_IDLE && rOldStereoParams.i4Sync2AMode != i4Sync2AMode)
    {
        SYNC3A_LOGD("[%s] Sync2AMode : old(%d), new(%d)", __FUNCTION__, rOldStereoParams.i4Sync2AMode, i4Sync2AMode);

        if (i4Sync2AMode != NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_NONE)
        {
            SYNC3A_LOGD("[%s] enableAEStereoManualPline master(%d)/slave(%d):1", __FUNCTION__, i4MasterDev, i4SlaveDev);
            MBOOL bEnableAEStereoManualPline = MTRUE;
            IAeMgr::getInstance().sendAECtrl(i4MasterDev, EAECtrl_EnableAEStereoManualPline, bEnableAEStereoManualPline, NULL, NULL, NULL);
            IAeMgr::getInstance().sendAECtrl(i4SlaveDev, EAECtrl_EnableAEStereoManualPline, bEnableAEStereoManualPline, NULL, NULL, NULL);
        }
        else
        {
            SYNC3A_LOGD("[%s] enableAEStereoManualPline master(%d)/slave(%d):0", __FUNCTION__,i4MasterDev, i4SlaveDev);
            MBOOL bEnableAEStereoManualPline = MFALSE;
            IAeMgr::getInstance().sendAECtrl(i4MasterDev, EAECtrl_EnableAEStereoManualPline, bEnableAEStereoManualPline, NULL, NULL, NULL);
            IAeMgr::getInstance().sendAECtrl(i4SlaveDev, EAECtrl_EnableAEStereoManualPline, bEnableAEStereoManualPline, NULL, NULL, NULL);
        }

        // Use New Sync2AMode
        if (i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DUAL_ZOOM)
        {
#if DUALZOOM_WIDE_STANDY_EN
            disableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AWB);
            disableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AWB);
#else
            enableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AWB);
            enableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AWB);
#endif
        }
        ICcuCtrl3ACtrl* pCcuCtrl3ASync = ICcuCtrl3ACtrl::createInstance();
        if (i4Sync2AMode != NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_NONE)
        {
            if(m_pSync3AWrap && ((i4Sync3ASwitchOn == NS3Av3::E_SYNC3A_NOTIFY::E_SYNC3A_NOTIFY_SWITCH_ON) || (m_SwitchCamCount == 2)))
                m_pSync3AWrap->updateCcuSet3ASyncState();
        }
        else
        {
            if (pCcuCtrl3ASync)
            {
                struct ccu_3a_sync_state syncState;
                syncState.mode = FREE_RUN;
                SYNC3A_LOGD("[%s] CCU_SYNC_DBG MSG_TO_CCU_SET_3A_SYNC_STATE to FREE_RUN", __FUNCTION__);
                pCcuCtrl3ASync->ccuControl(MSG_TO_CCU_SET_3A_SYNC_STATE, &syncState, NULL);
            }
        }
        pCcuCtrl3ASync->destroyInstance();
        pCcuCtrl3ASync = NULL;
    }

    if (i4SyncAFMode != NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_IDLE && rOldStereoParams.i4SyncAFMode != i4SyncAFMode)
    {
        SYNC3A_LOGD("[%s] SyncAFMode : old(%d), new(%d)", __FUNCTION__, rOldStereoParams.i4SyncAFMode, i4SyncAFMode);
        // Use New SyncAFMode
        if (i4SyncAFMode == NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_ON)
        {
            if (i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DUAL_ZOOM)
            {
                AFResult_T rAFResultFromMgrMaster;
                IAfMgr::getInstance(i4MasterDev).getResult(rAFResultFromMgrMaster);
                if (rAFResultFromMgrMaster.isFocusFinish)
                    syncAF(i4MasterDev, MTRUE);
            }
            else
            {
                enableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AF);
                enableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AF);
            }
        } else if (i4SyncAFMode == NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_OFF)
        {
            disableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AF);
            disableSyncSupport(ISync3A::E_SYNC3A_SUPPORT_AF);

            // reset Master AF sync status
            AF_SyncInfo_T sCamInfo;
            memset(&sCamInfo, 0, sizeof(sCamInfo));
            IAfMgr::getInstance(i4MasterDev).SyncAFGetInfo( sCamInfo);
        }
    }

    if (i4HwSyncMode != NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_IDLE && rOldStereoParams.i4HwSyncMode != i4HwSyncMode)
    {
        SYNC3A_LOGD("[%s] SyncHwMode : old(%d), new(%d)", __FUNCTION__, rOldStereoParams.i4HwSyncMode, i4HwSyncMode);
        // Use New HwSyncMode
        if (i4HwSyncMode == NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_OFF)
        {
            AAASensorMgr::getInstance().disableFrameSync();
            updateFrameSync(MFALSE);
        }
        else
            updateFrameSync(MTRUE);
    }
}

MINT32
Sync3A::
getFrmSyncOpt() const
{
    MINT32 opt = 0;

    Stereo_Param_T rStereoParams;
    rStereoParams = Sync3AMgr::getInstance()->getStereoParams();

    if (rStereoParams.i4HwSyncMode == NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_ON)
    {
        opt |= ISync3A::E_SYNC3A_DO_HW_SYNC;
        if ( rStereoParams.i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_VSDOF ||
                rStereoParams.i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DENOISE ||
                rStereoParams.i4Sync2AMode == NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_DUAL_ZOOM)
        {
            opt |= ISync3A::E_SYNC3A_DO_SW_SYNC;
        }
    }
    return opt;
}

MINT32
Sync3A::
getAWBMasterDev() const
{
    return Sync3AMgr::getInstance()->getAWBMasterDev();
}

MINT32
Sync3A::
getAWBSlaveDev() const
{
    return Sync3AMgr::getInstance()->getAWBSlaveDev();
}

MVOID
Sync3AMgr::
setSwitchCamCount(MBOOL bEnable)
{
    if(bEnable)
        m_SwitchCamCount = 0;
    SYNC3A_LOGD("[%s] m_SwitchCamCount(%d)", __FUNCTION__, m_SwitchCamCount);
}

MINT32
Sync3A::
sendSync3ACtrl(ESync3ACtrl_T eSync3ACtrl, MINTPTR iArg1, MINTPTR iArg2)
{
    MINT32 i4Ret = 0;
    switch (eSync3ACtrl)
    {
        case ESync3ACtrl_SetIsStart:
            {
                MINT32 i4IsStart = (MINT32)iArg1;
                SYNC3A_LOGD("[%s] m_IsStart(%d->%d)", __FUNCTION__, m_IsStart, i4IsStart);
                m_IsStart = (MINT32)iArg1;
                i4Ret = MTRUE;
            }
            break;
        case ESync3ACtrl_GetIsStart:
            {
                SYNC3A_LOGD("[%s] m_IsStart(%d)", __FUNCTION__, m_IsStart);
                *(reinterpret_cast<MINT32*>(iArg1)) = (MINT32)(m_IsStart);
                i4Ret = MTRUE;
            }
            break;
        default:
            SYNC3A_LOGW("[%s] Unsupport Command(%d)", __FUNCTION__, eSync3ACtrl);
            return MFALSE;
    }
    return i4Ret;
}

MINT32
Sync3A::
querySensorType(MINT32 i4SensorDev)
{
    SensorStaticInfo sensorStaticInfo;
    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList)
    {
        SYNC3A_LOGE("MAKE_HalSensorList() == NULL");
        return 0;
    }
    pHalSensorList->querySensorStaticInfo(i4SensorDev, &sensorStaticInfo);

    return sensorStaticInfo.rawFmtType; // SENSOR_RAW_MONO or SENSOR_RAW_Bayer
}

