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
#define LOG_TAG "HalIspImp"

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>

//For Metadata
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include "aaa_utils.h"


#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>

#include <mtkcam/drv/IHalSensor.h>

#include <mutex>
#include <faces.h>

//tuning utils
#include <mtkcam/utils/TuningUtils/FileReadRule.h>
#include <SttBufQ.h>
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>
#include <mtkcam/utils/sys/IFileCache.h>

#include "../lsc_mgr/ILscTsf.h"
#include <lsc/ILscTbl.h>

#include <sys/stat.h>

#include <isp_tuning_buf.h>

#include <dip_reg.h>

#include <mtkcam/utils/exif/IBaseCamExif.h>
#include <isp_mgr.h>
#include <mtkcam3/feature/3dnr/3dnr_isp_defs.h>

#include <mtkcam/feature/eis/eis_ext.h>

// Index manager
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>
#include <nvbuf_util.h>
#include <EModule_string.h>
#include <EApp_string.h>
#include <EIspProfile_string.h>
#include <ESensorMode_string.h>
#include "camera_custom_msdk.h"
#include <mtkcam/utils/hw/HwTransform.h>

#include<mtkcam/drv/iopipe/CamIO/Cam_Notify.h>

using namespace NSCam;
using namespace NSCamHW;

#include "HalIspImp.h"

using namespace NS3Av3;
using namespace NSCam::TuningUtils;


#define MY_INST NS3Av3::INST_T<HalIspImp>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_ISP_HAL(String) \
          do { \
              aee_system_exception( \
                  "HalISP", \
                  NULL, \
                  DB_OPT_DEFAULT, \
                  String); \
          } while(0)
#else
#define AEE_ASSERT_ISP_HAL(String)
#endif

#define GET_PROP(prop, dft, val)\
{\
   val = property_get_int32(prop,dft);\
}

#define HALISP_LOG_SET_P1       (1<<0)
#define HALISP_LOG_SET_P2       (2<<0)
#define HALISP_LOG_SETPARAM_P2  (3<<0)
#define HALISP_LOG_GET_P1       (4<<0)
#define HALISP_LOG_GET_P2       (5<<0)
#define HALISP_LOG_CONVERT_P2   (6<<0)

MUINT32 HalIspImp::m_u4LogEn = 0;

static void
_dumpAAO(const char* filename, MUINT32 SensorDev, MINT32 MagicNumberRequest)
{
    android::sp<IFileCache> fptr;

    android::sp<ISttBufQ> pSttBufQ;
    android::sp<ISttBufQ::DATA> pData;
    int N;

    pSttBufQ = ISttBufQ::getInstance(SensorDev);
    if (!pSttBufQ.get()) {
        CAM_LOGW("ISttBufQ::getInstance(SensorDev=%d) error!!", SensorDev);
        goto lbExit;
    }

    pData = pSttBufQ->deque_byMagicNumberRequest(MagicNumberRequest);
    if (!pData.get()) {
        CAM_LOGW("pSttBufQ->GetByMagicNumberRequest(%d) error!!", MagicNumberRequest);
        goto lbExit;
    }

    fptr = IFileCache::open(filename);
    if (fptr.get()==NULL) {
        CAM_LOGW("open file(%s) error!!", filename);
        goto lbExit;
    }

    N = pData->AAO.size();
    if ((int)fptr->write(pData->AAO.editArray(), N) != N) {
        CAM_LOGW("file(%s) write error!!", filename);
        goto lbExit;
    }

lbExit:
    if(pData.get()) {
        pSttBufQ->enque_first(pData);
    }
}

static
inline MBOOL
_dumpDebugInfo(const char* filename, const IMetadata& metaExif)
{
    android::sp<IFileCache> fid = IFileCache::open(filename);
    if (fid.get())
    {
        IMetadata::Memory p3ADbg;
        if (IMetadata::getEntry<IMetadata::Memory>(&metaExif, MTK_3A_EXIF_DBGINFO_AAA_DATA, p3ADbg))
        {
            CAM_LOGD("[%s] %s, 3A(%p, %d)", __FUNCTION__, filename, p3ADbg.array(), (MINT32)p3ADbg.size());
            MUINT8 hdr[6] = {0, 0, 0xFF, 0xE6, 0, 0};
            MUINT16 size = (MUINT16)(p3ADbg.size()+2);
            hdr[4] = (size >> 8); // big endian
            hdr[5] = size & 0xFF;
            fid->write(hdr, 6);
            fid->write(p3ADbg.array(), p3ADbg.size());
        }
        IMetadata::Memory pIspDbg;
        if (IMetadata::getEntry<IMetadata::Memory>(&metaExif, MTK_3A_EXIF_DBGINFO_ISP_DATA, pIspDbg))
        {
            CAM_LOGD("[%s] %s, ISP(%p, %d)", __FUNCTION__, filename, pIspDbg.array(), (MINT32)pIspDbg.size());
            MUINT8 hdr[4] = {0xFF, 0xE7, 0, 0};
            MUINT16 size = (MUINT16)(pIspDbg.size()+2);
            hdr[2] = (size >> 8);
            hdr[3] = size & 0xFF;
            fid->write(hdr, 4);
            fid->write(pIspDbg.array(), pIspDbg.size());
        }
        return MTRUE;
    } else {
        CAM_LOGW("IFileCache open fail <%s>", filename);
        return MFALSE;
    }
}

static MBOOL _isFileExist(const char* file) {
  struct stat buffer;
  return (stat (file, &buffer) == 0);
}

/*******************************************************************************
* implementations
********************************************************************************/
HalIspImp*
HalIspImp::
createInstance(MINT32 const i4SensorIdx, const char* strUser)
{
    GET_PROP("vendor.debug.halisp.log", 0, m_u4LogEn);
    CAM_LOGD("[%s] sensorIdx(%d) %s", __FUNCTION__, i4SensorIdx, strUser);

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
        return MFALSE;
    }

    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<HalIspImp>(i4SensorIdx);
    } );

    if(rSingleton.instance) {
        (rSingleton.instance)->init(strUser);
    }
    return rSingleton.instance.get();

}



HalIspImp::
HalIspImp(MINT32 const i4SensorIdx)
    : m_Users(0)
    , m_Lock()
    , m_i4SensorIdx(i4SensorIdx)
    , m_i4SensorDev(0)
    , m_pTuning(NULL)
    , m_pCamIO(NULL)
    , m_pResultPoolObj(NULL)
    , m_i4SensorMode(0)
    , m_i4TgWidth(1000)
    , m_i4TgHeight(1000)
    , m_i4faceNum(0)
    , m_bFaceDetectEnable(0)
    , m_eIspProfile(NSIspTuning::EIspProfile_Preview)
    , m_i4Magic(-1)
    , m_i4CopyLscP1En(0)
    , m_rResultMtx()
    , m_u1ColorCorrectMode(MTK_COLOR_CORRECTION_MODE_FAST)
    , m_u1IsGetExif(0)
    , m_bIsCapEnd(0)
    , m_bDbgInfoEnable(MFALSE)
    , m_i4SubsampleCount(1)
    , mpHalISPP1DrvCbHub(NULL)
{
    // query SensorDev from HalSensorList
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList)
        CAM_LOGE("[%s] MAKE HalSensorList fail", __FUNCTION__);
    else
        m_i4SensorDev = pHalSensorList->querySensorDevIdx(i4SensorIdx);

    CAM_LOGD("[%s] sensorIdx(0x%04x) sensorDev(%d)", __FUNCTION__, i4SensorIdx, m_i4SensorDev);
}

MVOID
HalIspImp::
destroyInstance(const char* strUser)
{
    CAM_LOGD("[%s]+ sensorIdx(%d)  User(%s)", __FUNCTION__, m_i4SensorIdx, strUser);
    uninit(strUser);
    CAM_LOGD("[%s]- ", __FUNCTION__);
}

MINT32
HalIspImp::
config(const ConfigInfo_T& rConfigInfo)
{
    CAM_LOGD("[%s] config i4SubsampleCount(%d)", __FUNCTION__, rConfigInfo.i4SubsampleCount);

    if (m_i4SubsampleCount != rConfigInfo.i4SubsampleCount){
        CAM_LOGD("[%s] m_i4SubsampleCount(%d, %d)", __FUNCTION__, m_i4SubsampleCount, rConfigInfo.i4SubsampleCount);
        m_i4SubsampleCount = rConfigInfo.i4SubsampleCount;
        // TuningMgr uninit
        if (m_pTuning)
        {
            CAM_LOGD("[%s] m_pTuning uninit +", __FUNCTION__);
            m_pTuning->uninit(m_i4SensorDev);
            m_pTuning = NULL;
            CAM_LOGD("[%s] m_pTuning uninit -", __FUNCTION__);
        }
       // TuningMgr init
        if (m_pTuning == NULL)
        {
            CAM_LOGD("[%s] m_pTuning init +", __FUNCTION__);
            AAA_TRACE_D("TUNING init");
            m_pTuning = &IspTuningMgr::getInstance();
            if (!m_pTuning->init(m_i4SensorDev, m_i4SensorIdx, rConfigInfo.i4SubsampleCount))
            {
                CAM_LOGE("Fail to init IspTuningMgr (%d,%d)", m_i4SensorDev, m_i4SensorIdx);
                AEE_ASSERT_ISP_HAL("Fail to init IspTuningMgr");
                AAA_TRACE_END_D;
                return MFALSE;
            }
            CAM_LOGD("[%s] m_pTuning init -", __FUNCTION__);
            AAA_TRACE_END_D;
        }
        //InitLCS();
        //querySensorStaticInfo(); //Add this, will NE
    }
    else
    {
        //reinit P1 TuningMgr
        MBOOL ret = m_pTuning->reinitP1TuningMgr(m_i4SensorDev, m_i4SubsampleCount);
        CAM_LOGE_IF( (ret == MFALSE),"[%s] reinitP1TuningMgr Fail", __FUNCTION__);
    }

    // NormalIOPipe create instance
    if (m_pCamIO == NULL)
    {
        m_pCamIO = (INormalPipe*)INormalPipeUtils::get()->createDefaultNormalPipe(m_i4SensorIdx, LOG_TAG);
        if (m_pCamIO == NULL)
        {
            CAM_LOGE("Fail to create NormalPipe");
            return MFALSE;
        }
    }

    updateTGInfo(); // Get m_i4TgWidth, m_i4TgHeight

    MUINT32 u4ISPRawWidth, u4ISPRawHeight;
    m_pCamIO->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_BIN_INFO,
                        (MINTPTR)&u4ISPRawWidth, (MINTPTR)&u4ISPRawHeight, 0);

    MBOOL bFrontalBin = (m_i4TgWidth == (MINT32)u4ISPRawWidth && m_i4TgHeight == (MINT32)u4ISPRawHeight) ? MFALSE : MTRUE;
    m_pTuning->setSensorMode(m_i4SensorDev, m_i4SensorMode, bFrontalBin, u4ISPRawWidth, u4ISPRawHeight);

    // Get RawAB/C Info
    MUINT32 ERawPath;
    m_pCamIO->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_CUR_RAW,
                        (MINTPTR)&ERawPath, 0, 0);
    m_pTuning->setRawPath(m_i4SensorDev, ERawPath);

    // P1 YUV Port Info
    m_pTuning->setP1DirectYUV_Port(m_i4SensorDev, rConfigInfo.u4P1DirectYUV_Port);

    //Need change
    m_pTuning->setIspProfile(m_i4SensorDev, m_eIspProfile);

    m_pTuning->notifyRPGEnable(m_i4SensorDev, MTRUE);   // apply awb gain for init stat

    const AEResultInfo_T  *pAEResultInfo = (AEResultInfo_T*)m_pResultPoolObj->getResult(ConfigMagic, E_AE_RESULTINFO, __FUNCTION__);
    if(pAEResultInfo)
    {
        CAM_LOGD_IF(1, "[%s] u4DGNGain(%d)", __FUNCTION__, pAEResultInfo->AEPerframeInfo.rAEISPInfo.u4P1DGNGain);
        m_pTuning->setAEInfo2ISP(m_i4SensorDev, pAEResultInfo->AEPerframeInfo.rAEISPInfo);
    }else
        CAM_LOGD("[%s] pAEResultInfo(%p)", __FUNCTION__, pAEResultInfo);

    const AWBResultInfo_T *pAWBResultInfo = (AWBResultInfo_T*)m_pResultPoolObj->getResult(ConfigMagic, E_AWB_RESULTINFO4ISP, __FUNCTION__);
    if(pAWBResultInfo)
        m_pTuning->setAWBInfo2ISP(m_i4SensorDev, pAWBResultInfo->AWBInfo4ISP);
    else
        CAM_LOGD("[%s] pAWBResultInfo(%p)", __FUNCTION__, pAWBResultInfo);

    RequestSet_T rRequestSet;
    rRequestSet.vNumberSet.clear();
    for (MINT32 i4Num = 0; i4Num < m_i4SubsampleCount; i4Num++)
        rRequestSet.vNumberSet.push_back(ConfigMagic);

    m_pTuning->validatePerFrameP1(m_i4SensorDev, rRequestSet);

    CAM_LOGD("[%s]- After Bin Raw Size(%dx%d)", __FUNCTION__, u4ISPRawWidth, u4ISPRawHeight);
    return MTRUE;
}


MVOID
HalIspImp::
WT_Reconfig(MVOID* pInput)
{
    // check user count
    std::lock_guard<std::mutex> lock(m_Lock);

    TUNINGCB_INPUT_INFO *apConfigInfo  = (TUNINGCB_INPUT_INFO *)pInput;

    MBOOL bFrontalBin = (m_i4TgWidth == apConfigInfo->RRZ_IN_CROP.src_w
                         && m_i4TgHeight == apConfigInfo->RRZ_IN_CROP.src_h) ? MFALSE : MTRUE;

    m_pTuning->setSensorMode(m_i4SensorDev, m_i4SensorMode, bFrontalBin,
                             apConfigInfo->RRZ_IN_CROP.src_w, apConfigInfo->RRZ_IN_CROP.src_h);

    MUINT32 ERawPath = apConfigInfo->m_hwModule;
    m_pTuning->setRawPath(m_i4SensorDev, ERawPath);

    MUINT32 u4MagicNum = apConfigInfo->Magic;

    CAM_LOGD("[%s] reconfig with magic num=%d, frontal bin=%d", __FUNCTION__, u4MagicNum, bFrontalBin);

    const AEResultInfo_T  *pAEResultInfo = (AEResultInfo_T*)m_pResultPoolObj->getResult(u4MagicNum, E_AE_RESULTINFO, __FUNCTION__);
    if(pAEResultInfo){
        CAM_LOGD_IF(1, "[%s] u4DGNGain(%d)", __FUNCTION__, pAEResultInfo->AEPerframeInfo.rAEISPInfo.u4P1DGNGain);
        m_pTuning->getInstance().setAEInfo2ISP(m_i4SensorDev, pAEResultInfo->AEPerframeInfo.rAEISPInfo);
    }
    else{
        CAM_LOGD("[%s] pAEResultInfo(%p)", __FUNCTION__, pAEResultInfo);
    }

    const AWBResultInfo_T *pAWBResultInfo = (AWBResultInfo_T*)m_pResultPoolObj->getResult(u4MagicNum, E_AWB_RESULTINFO4ISP, __FUNCTION__);
    if(pAWBResultInfo){
        m_pTuning->setAWBInfo2ISP(m_i4SensorDev, pAWBResultInfo->AWBInfo4ISP);
    }
    else{
        CAM_LOGD("[%s] pAWBResultInfo(%p)", __FUNCTION__, pAWBResultInfo);
    }

    RequestSet_T rRequestSet;
    rRequestSet.vNumberSet.clear();
    for (MINT32 i4Num = 0; i4Num < m_i4SubsampleCount; i4Num++)
        rRequestSet.vNumberSet.push_back(u4MagicNum);

    m_pTuning->validatePerFrameP1(m_i4SensorDev, rRequestSet, MTRUE);
}

MVOID
HalIspImp::
setSensorMode(MINT32 i4SensorMode)
{
    CAM_LOGD("[%s] mode(%d)", __FUNCTION__, i4SensorMode);
    m_i4SensorMode = i4SensorMode;
}

MBOOL
HalIspImp::
init(const char* strUser)
{
    MBOOL dbgInfoEnable;
#if (IS_BUILD_USER)
    dbgInfoEnable = 0;
#else
    dbgInfoEnable = 1;
#endif

    GET_PROP("vendor.debug.camera.dbginfo", dbgInfoEnable, m_bDbgInfoEnable);

    GET_PROP("vendor.debug.camera.copy.p1.lsc", 0, m_i4CopyLscP1En);

    CAM_LOGD("[%s] m_Users: %d, SensorDev %d, index %d \n", __FUNCTION__, std::atomic_load((&m_Users)), m_i4SensorDev, m_i4SensorIdx);

    // check user count
    std::lock_guard<std::mutex> lock(m_Lock);

    if (m_Users > 0)
    {
        CAM_LOGD("[%s] %d has created \n", __FUNCTION__, std::atomic_load((&m_Users)));
        MINT32 ret __unused = std::atomic_fetch_add((&m_Users), 1);
        return MTRUE;
    }


    // TuningMgr init
    if (m_pTuning == NULL)
    {
        AAA_TRACE_D("TUNING init");
        m_pTuning = &IspTuningMgr::getInstance();
        if (!m_pTuning->init(m_i4SensorDev, m_i4SensorIdx))
        {
            CAM_LOGE("Fail to init IspTuningMgr (%d,%d)", m_i4SensorDev, m_i4SensorIdx);
            AEE_ASSERT_ISP_HAL("Fail to init IspTuningMgr");
            AAA_TRACE_END_D;
            return MFALSE;
        }
        AAA_TRACE_END_D;
    }

    if(m_pResultPoolObj == NULL)
        m_pResultPoolObj = IResultPool::getInstance(m_i4SensorDev);
    if(m_pResultPoolObj == NULL)
        CAM_LOGE("ResultPool getInstance fail");

    querySensorStaticInfo();

    MINT32 ret __unused = std::atomic_fetch_add((&m_Users), 1);
    return MTRUE;
}

MBOOL
HalIspImp::
uninit(const char* strUser)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    // If no more users, return directly and do nothing.
    if (m_Users <= 0)
    {
        return MTRUE;
    }
    CAM_LOGD("[%s] m_Users: %d \n", __FUNCTION__, std::atomic_load((&m_Users)));

    // More than one user, so decrease one User.
    MINT32 ret __unused = std::atomic_fetch_sub((&m_Users), 1);

    if (m_Users == 0) // There is no more User after decrease one User
    {
        // TuningMgr uninit
        if (m_pTuning)
        {
            m_pTuning->uninit(m_i4SensorDev);
            m_pTuning = NULL;
        }

        //====== Destroy CbHub Driver ======
        if(mpHalISPP1DrvCbHub != NULL)
        {
            delete mpHalISPP1DrvCbHub;
            mpHalISPP1DrvCbHub = NULL;
        }

        CAM_LOGD("[%s] done\n", __FUNCTION__);

    }
    else    // There are still some users.
    {
        CAM_LOGD("[%s] Still %d users \n", __FUNCTION__, std::atomic_load((&m_Users)));
    }
    return MTRUE;
}

MBOOL
HalIspImp::
start()
{
    CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SET_P1, "[%s] +", __FUNCTION__);

    IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_NOTIFY_START, 0, 0);

    CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SET_P1, "[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
HalIspImp::
stop()
{
    CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SET_P1, "[%s] +", __FUNCTION__);

    IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_NOTIFY_STOP, 0, 0);
    // NormalIOPipe destroy instance
    if (m_pCamIO != NULL)
    {
        m_pCamIO->destroyInstance(LOG_TAG);
        m_pCamIO = NULL;
    }
#if 0
    // TuningMgr uninit
    if (m_pTuning)
    {
        m_pTuning->uninit(m_i4SensorDev);
        m_pTuning = NULL;
    }
#endif
    CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SET_P1, "[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
HalIspImp::
setP1Isp(const vector<MetaSet_T*>& requestQ, MBOOL const fgForce/*MINT32 const i4SensorDev, RequestSet_T const RequestSet, MetaSet_T& control, MBOOL const fgForce, MINT32 i4SubsampleIdex*/)
{
    // check user count
    std::lock_guard<std::mutex> lock(m_Lock);

    AAA_TRACE_D("setP1ISP");
    MetaSet_T* it = requestQ[0];
    MINT32 i4FrmId = it->MagicNum;
    if (i4FrmId <= 0)   QUERY_ENTRY_SINGLE(it->halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, i4FrmId);
    MINT32 i4FrmId4SMVR = 0;
    m_i4Magic = i4FrmId;

    CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SET_P1, "[%s] + i4FrmId(%d) fgForce(%d)", __FUNCTION__, i4FrmId, fgForce);
    /*****************************
     *     Special flow - InitReq
     *     Warning : MW set one request only, unlike set three request to HAL3A
     *               So ISP_profile will wrong, this issue need to fix from MW
     *****************************/
    if(fgForce)
    {
        MetaSet_T* it = requestQ[0];
        RequestSet_T rRequestSet;
        rRequestSet.vNumberSet.clear();
        rRequestSet.vNumberSet.push_back(i4FrmId);
        NS3Av3::ParamIspProfile_T _3AProf(NSIspTuning::EIspProfile_Preview, i4FrmId, 1, MTRUE, NS3Av3::ParamIspProfile_T::EParamValidate_All, rRequestSet);
        _3AProf.i4MagicNum = i4FrmId;
        IspTuningMgr::getInstance().setIspProfile(m_i4SensorDev, NSIspTuning::EIspProfile_Preview);
        validateP1(_3AProf);
        getCurrResult(i4FrmId);
        AAA_TRACE_END_D;
        return MTRUE;
    }
    /*****************************
     *     Parse ISP Param
     *****************************/

    RequestSet_T rRequestSet;
    rRequestSet.vNumberSet.clear();
    MUINT8 u1CapIntent = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
    MUINT8 u1ColorCorrectMode = MTK_COLOR_CORRECTION_MODE_FAST;
    const IMetadata& _appmeta = it->appMeta;
    const IMetadata& _halmeta = it->halMeta;
    MUINT8 u1IspProfile = 0xFF;
    MINT32 i4DisableP1=0;

    for (MINT32 i = 0; i < requestQ.size(); i++)
    {
        MetaSet_T* it = requestQ[i];
        i4FrmId = it->MagicNum;
        if (i4FrmId <= 0)   QUERY_ENTRY_SINGLE(it->halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, i4FrmId);
        m_i4Magic = i4FrmId;
        u1CapIntent = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
        u1ColorCorrectMode = MTK_COLOR_CORRECTION_MODE_FAST;
        u1IspProfile = 0xFF;
        const IMetadata& _appmeta = it->appMeta;
        const IMetadata& _halmeta = it->halMeta;

        QUERY_ENTRY_SINGLE(_halmeta, MTK_3A_ISP_PROFILE, u1IspProfile);

        CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SET_P1, "[%s] i4FrmId(%d)", __FUNCTION__, i4FrmId);
        rRequestSet.vNumberSet.push_back(std::max(0, i4FrmId));

        // Dual PDAF support for EngMode
        QUERY_ENTRY_SINGLE(_halmeta, MTK_HAL_REQUEST_PASS1_DISABLE, i4DisableP1);
        if (!QUERY_ENTRY_SINGLE(_halmeta, MTK_HAL_REQUEST_REQUIRE_EXIF, m_u1IsGetExif))
            m_u1IsGetExif = 0;

        for (MINT32 j = 0; j < _appmeta.count(); j++)
        {
            IMetadata::IEntry entry = _appmeta.entryAt(j);
            mtk_camera_metadata_tag_t tag = (mtk_camera_metadata_tag_t)entry.tag();
            // convert metadata tag into 3A settings.
            switch (tag)
            {
            case MTK_CONTROL_MODE:  // dynamic
                {
                    MUINT8 u1ControlMode = entry.itemAt(0, Type2Type< MUINT8 >());
                    IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_NOTIFY_CONTROL_MODE, (MINTPTR)&u1ControlMode, 0);
                }
                break;
            case MTK_CONTROL_CAPTURE_INTENT:
                {
                    u1CapIntent = entry.itemAt(0, Type2Type< MUINT8 >());
                }
                break;
            // ISP
            case MTK_CONTROL_EFFECT_MODE:
                {
                    MUINT8 u1EffectMode = entry.itemAt(0, Type2Type< MUINT8 >());
                    IspTuningMgr::getInstance().setEffect(m_i4SensorDev,u1EffectMode);
                }
                break;
            case MTK_CONTROL_SCENE_MODE:
                {
                    MUINT32 u4SceneMode = entry.itemAt(0, Type2Type< MUINT8 >());
                    IspTuningMgr::getInstance().setSceneMode(m_i4SensorDev,u4SceneMode);
                }
                break;
            case MTK_EDGE_MODE:
                {
                    MUINT8 u1EdgeMode = entry.itemAt(0, Type2Type< MUINT8 >());
                    IspTuningMgr::getInstance().setEdgeMode(m_i4SensorDev,u1EdgeMode);
                }
                break;
            case MTK_NOISE_REDUCTION_MODE:
                {
                    MUINT8 u1NRMode = entry.itemAt(0, Type2Type< MUINT8 >());
                    IspTuningMgr::getInstance().setNoiseReductionMode(m_i4SensorDev,u1NRMode);
                }
                break;
            // Color correction
            case MTK_COLOR_CORRECTION_MODE:
                {
                    u1ColorCorrectMode = entry.itemAt(0, Type2Type<MUINT8>());
                    m_u1ColorCorrectMode = u1ColorCorrectMode;
                    IspTuningMgr::getInstance().setColorCorrectionMode(m_i4SensorDev,u1ColorCorrectMode);
                }
                break;
            case MTK_COLOR_CORRECTION_TRANSFORM:
                {
                    MFLOAT fColorCorrectMat[9];
                    for (MINT32 k = 0; k < 9; k++)
                    {
                        MRational rMat = entry.itemAt(k, Type2Type<MRational>());
                        fColorCorrectMat[k] = (0.0f != rMat.denominator) ? (MFLOAT)rMat.numerator / rMat.denominator : 0.0f;
                    }
                    IspTuningMgr::getInstance().setColorCorrectionTransform(m_i4SensorDev,
                    fColorCorrectMat[0], fColorCorrectMat[1], fColorCorrectMat[2],
                    fColorCorrectMat[3], fColorCorrectMat[4], fColorCorrectMat[5],
                    fColorCorrectMat[6], fColorCorrectMat[7], fColorCorrectMat[8]);
                }
                break;
            }
        }

    }

    MSize targetSize(0,0);
    QUERY_ENTRY_SINGLE(_halmeta, MTK_3A_ISP_MDP_TARGET_SIZE, targetSize);
    CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SET_P2, "[%s] MTK_3A_ISP_MDP_TARGET_SIZE(%d, %d)", __FUNCTION__, targetSize.w, targetSize.h);

    // ISP profile
    if (u1IspProfile == 0xFF){
        switch (u1CapIntent)
        {
            case MTK_CONTROL_CAPTURE_INTENT_PREVIEW:
            case MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG:
                m_eIspProfile = NSIspTuning::EIspProfile_Preview;
                break;
            case MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD:
            case MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT:
                m_eIspProfile = NSIspTuning::EIspProfile_Video;
                break;
            default:
                m_eIspProfile = NSIspTuning::EIspProfile_Preview;
                break;
        }
        CAM_LOGE("[%s] No ISP Profile: CaptureIntent: %d, Default Profile: %d", __FUNCTION__, u1CapIntent, m_eIspProfile);
    }
    else{

        NSIspTuning::EIspProfile_T eIspProfile = static_cast<NSIspTuning::EIspProfile_T>(u1IspProfile);
        CAM_LOGD_IF(m_eIspProfile != eIspProfile, "[%s] eIspProfile %d -> %d", __FUNCTION__, m_eIspProfile, eIspProfile);
        m_eIspProfile = eIspProfile;
    }
    IspTuningMgr::getInstance().setIspProfile(m_i4SensorDev,m_eIspProfile);

    MBOOL bIMGO_RawType = MFALSE;
    QUERY_ENTRY_SINGLE(_halmeta, MTK_P1NODE_RAW_TYPE, bIMGO_RawType);
    IspTuningMgr::getInstance().notifyIMGOType(m_i4SensorDev, bIMGO_RawType);

    MINT32 i4RequestNumber=0;
    QUERY_ENTRY_SINGLE(_halmeta, MTK_PIPELINE_REQUEST_NUMBER, i4RequestNumber);
    IspTuningMgr::getInstance().setRequestNumber(m_i4SensorDev, i4RequestNumber);

    //Need perframe call??
    IspTuningMgr::getInstance().notifyRPGEnable(m_i4SensorDev, MTRUE);

    IspTuningBufCtrl::getInstance(m_i4SensorDev)->clearP1Buffer();

    if(m_i4SubsampleCount>1)
    {
        AllResult_T *pAllResult = m_pResultPoolObj->getAllResult(i4FrmId);
        i4FrmId4SMVR = pAllResult->rOld3AInfo.i4ConvertMagic[0];
    }
    else
        i4FrmId4SMVR = i4FrmId;
    /*****************************
     *     Set AWBInfo to ISP
     *****************************/
    AWBResultInfo_T *m_pAWBResultInfo = (AWBResultInfo_T*)m_pResultPoolObj->getResult(i4FrmId4SMVR, E_AWB_RESULTINFO4ISP, __FUNCTION__);
    if(m_pAWBResultInfo)
        IspTuningMgr::getInstance().setAWBInfo2ISP(m_i4SensorDev, m_pAWBResultInfo->AWBInfo4ISP);
    /*****************************
     *     Set AEInfo to ISP
     *****************************/
    const AEResultInfo_T  *pAEResultInfo = (AEResultInfo_T*)m_pResultPoolObj->getResult(i4FrmId4SMVR, E_AE_RESULTINFO, __FUNCTION__);
    if(pAEResultInfo)
    {
        CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SET_P1, "[%s] u4DGNGain(%d)", __FUNCTION__, pAEResultInfo->AEPerframeInfo.rAEISPInfo.u4P1DGNGain);
        IspTuningMgr::getInstance().setAEInfo2ISP(m_i4SensorDev, pAEResultInfo->AEPerframeInfo.rAEISPInfo);
    }else
        CAM_LOGD("[%s] pAEResultInfo(%p)", __FUNCTION__, pAEResultInfo);

    /*****************************
     *     ISP Validate
     *****************************/
    AAA_TRACE_D("P1_VLD");
    AAA_TRACE_ISP(P1_VLD);
    rRequestSet.fgDisableP1 = i4DisableP1;
    NS3Av3::ParamIspProfile_T _3AProf(m_eIspProfile, i4FrmId, 0, MTRUE, NS3Av3::ParamIspProfile_T::EParamValidate_All, rRequestSet);

    /*****************************
     *     3A HAL info to ISP
     *****************************/
    const HALResultToMeta_T* pHALResult = (HALResultToMeta_T*)m_pResultPoolObj->getResult(i4FrmId4SMVR, E_HAL_RESULTTOMETA, __FUNCTION__);
    if(pHALResult)
    {
        rRequestSet.fgKeep = pHALResult->fgKeep;
        m_bIsCapEnd = pHALResult->fgKeep;
        setFDEnable(pHALResult->fgFdEnable);
        CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SET_P1, "[%s] pHALResult->i4ZoomRatio(%d)", __FUNCTION__, pHALResult->i4ZoomRatio);
        IspTuningMgr::getInstance().setZoomRatio(m_i4SensorDev, pHALResult->i4ZoomRatio);
    }
    CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SET_P1, "[%s] rRequestSet.fgKeep(%d)", __FUNCTION__, rRequestSet.fgKeep);

    if (_3AProf.rRequestSet.vNumberSet[0] > 0)
    {
        CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SET_P1, "[%s] validateP1 rRequestSet.vNumberSet[0](%d)", __FUNCTION__, _3AProf.rRequestSet.vNumberSet[0]);
        validateP1(_3AProf);
    }
    AAA_TRACE_END_ISP;
    AAA_TRACE_END_D;

    /*****************************
     *     Get ISP Result
     *****************************/
    std::vector<MINT32> rNumberSet = rRequestSet.vNumberSet;
    std::vector<MINT32>::iterator it4Magic;
    MINT32 i4MagicNum = 0;
    for (it4Magic = rNumberSet.begin(); it4Magic != rNumberSet.end(); it4Magic++)
    {
        i4MagicNum = (*it4Magic);
        getCurrResult(i4MagicNum);
    }
    CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SET_P1, "[%s] - i4FrmId(%d) i4FrmId4SMVR(%d)", __FUNCTION__, i4FrmId, i4FrmId4SMVR);
    AAA_TRACE_END_D;
    return MTRUE;
}

MBOOL
HalIspImp::
setP2Isp(MINT32 flowType, const MetaSet_T& control, TuningParam* pTuningBuf, MetaSet_T* pResult)
{
    AAA_TRACE_D("setP2ISP");

    MINT32 i4P2En = 0, defaultValue = 1;
    const NSIspTuning::RAWIspCamInfo *pCaminfoBuf = NULL;
    MUINT32 u4readDump = 0;
    FileReadRule rule;

#if CAM3_DEFAULT_ISP
    defaultValue = 0;
#endif
    GET_PROP("vendor.debug.hal3av3.p2", defaultValue, i4P2En);

    MINT32 i4Ret = -1;

    if (i4P2En == 0 || pTuningBuf == NULL)
    {
        CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SET_P2, "[%s] IT: flowType(%d), pTuningBuf(%p)", __FUNCTION__, flowType, pTuningBuf);
    }
    else
    {
        AAA_TRACE_HAL(P2ParsingMetadata);
        ResultP2_T rResultP2;
        P2Param_T rNewP2Param;
        NSIspTuning::ISP_INFO_T rIspInfo;
        IMetadata::Memory pCaminfoMeta;
        NSCam::IMetadata rOutExifMeta;

        auto bCamInfoMeta = IMetadata::getEntry<IMetadata::Memory>(&control.halMeta, MTK_PROCESSOR_CAMINFO, pCaminfoMeta);
        if (bCamInfoMeta) {
            pCaminfoBuf = (const NSIspTuning::RAWIspCamInfo *)(pCaminfoMeta.array());
        }

        QUERY_ENTRY_SINGLE(control.halMeta, MTK_HAL_REQUEST_REQUIRE_EXIF,       rNewP2Param.u1Exif);
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_HAL_REQUEST_DUMP_EXIF,          rNewP2Param.u1DumpExif);
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_STEREO_FEATURE_DENOISE_MODE,    rNewP2Param.i4DenoiseMode);
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM,      rNewP2Param.i4MagicNum);
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_PIPELINE_FRAME_NUMBER,          rNewP2Param.i4FrmNo);
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_PIPELINE_REQUEST_NUMBER,        rNewP2Param.i4ReqNo);
        if (!control.halMeta.entryFor(MTK_3A_REPEAT_RESULT).isEmpty())
            QUERY_ENTRY_SINGLE(control.halMeta, MTK_3A_REPEAT_RESULT,           rNewP2Param.u1RepeatResult);
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_ISP_P2_IN_IMG_FMT,              rNewP2Param.i4P2InImgFmt);
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_ISP_P2_TUNING_UPDATE_MODE,      rNewP2Param.u1P2TuningUpdate);
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_ISP_P2_IN_IMG_RES_REVISED,      rNewP2Param.ResizeYUV);

        CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SET_P2, "[%s] u1RepeatResult(%d): i4MagicNum(%d)", __FUNCTION__, rNewP2Param.u1RepeatResult, rNewP2Param.i4MagicNum);

        QUERY_ENTRY_SINGLE(control.halMeta, MTK_3A_PGN_ENABLE,                  rNewP2Param.u1PGN);
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_3A_ISP_PROFILE,                 rNewP2Param.u1IspProfile);
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_P1NODE_RAW_TYPE,                rNewP2Param.i4RawType);
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_HAL_REQUEST_BRIGHTNESS_MODE,    rNewP2Param.i4BrightnessMode);
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_HAL_REQUEST_CONTRAST_MODE,      rNewP2Param.i4ContrastMode);
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_HAL_REQUEST_HUE_MODE,           rNewP2Param.i4HueMode);
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_HAL_REQUEST_SATURATION_MODE,    rNewP2Param.i4SaturationMode);
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_HAL_REQUEST_EDGE_MODE,          rNewP2Param.i4halEdgeMode);
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_HAL_REQUEST_ISP_PIPELINE_MODE,  rNewP2Param.bEnableNDD);

        MBOOL bDumpNDD = 0;
        GET_PROP("vendor.debug.camera.dump.campipe", 0, bDumpNDD);

        CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SET_P2, "[%s] Frm(%d), Req(%d), NDD meta(%d), adb(%d), profile(%d)", __FUNCTION__, rNewP2Param.i4FrmNo, rNewP2Param.i4ReqNo, rNewP2Param.bEnableNDD, bDumpNDD, rNewP2Param.u1IspProfile);

        rNewP2Param.bEnableNDD = (rNewP2Param.bEnableNDD || bDumpNDD);

        QUERY_ENTRY_SINGLE(control.appMeta, MTK_CONTROL_CAPTURE_INTENT,         rNewP2Param.u1CapIntent);
        QUERY_ENTRY_SINGLE(control.appMeta, MTK_TONEMAP_MODE,                   rNewP2Param.u1TonemapMode);
        QUERY_ENTRY_SINGLE(control.appMeta, MTK_EDGE_MODE,                      rNewP2Param.u1appEdgeMode);
        QUERY_ENTRY_SINGLE(control.appMeta, MTK_NOISE_REDUCTION_MODE,           rNewP2Param.u1NrMode);
        QUERY_ENTRY_SINGLE(control.appMeta, MTK_SENSOR_SENSITIVITY,             rNewP2Param.i4ISO);

        QUERY_ENTRY_SINGLE(control.halMeta, MTK_3A_ISP_MDP_TARGET_SIZE, rNewP2Param.targetSize);
        CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SET_P2, "[%s] MTK_3A_ISP_MDP_TARGET_SIZE(%d, %d)", __FUNCTION__, rNewP2Param.targetSize.w, rNewP2Param.targetSize.h);

        MBOOL HasExif = QUERY_ENTRY_SINGLE(control.halMeta, MTK_3A_EXIF_METADATA, rNewP2Param.rexifMeta);

        NSCam::IMetadata::Memory rpdbgIsp;
        MBOOL bResult3aMetaExif = MFALSE;

        if (pResult){
            if (QUERY_ENTRY_SINGLE(pResult->halMeta, MTK_3A_EXIF_METADATA, rNewP2Param.rexifMeta)) {
                QUERY_ENTRY_SINGLE<NSCam::IMetadata::Memory>(rNewP2Param.rexifMeta, MTK_3A_EXIF_DBGINFO_ISP_DATA, rpdbgIsp);
                rNewP2Param.rpdbgIsp = &rpdbgIsp;
                bResult3aMetaExif = MTRUE;
            }
        }

        if (!bResult3aMetaExif){
            if (QUERY_ENTRY_SINGLE(control.halMeta, MTK_3A_EXIF_METADATA, rNewP2Param.rexifMeta)) {
                QUERY_ENTRY_SINGLE<NSCam::IMetadata::Memory>(rNewP2Param.rexifMeta, MTK_3A_EXIF_DBGINFO_ISP_DATA, rpdbgIsp);
                rNewP2Param.rpdbgIsp = &rpdbgIsp;
            }
        }

        P2Info_T rP2Info;
        MINT32 isLmvValid = 0;
        if(!QUERY_ENTRY_SINGLE(control.halMeta, MTK_LMV_VALIDITY, isLmvValid))
        {
            CAM_LOGD("[%s] MTK_LMV_VALIDITY query fail",__FUNCTION__);
        }
        if(isLmvValid)
        {
            MBOOL isGMVValid=MFALSE;
            MINT32 GMV_X=0, GMV_Y=0, GMVCONF_X=0, GMVCONF_Y=0, GMVMAX=0;
            isGMVValid |= QUERY_ENTRY_SINGLE_BY_IDX(control.halMeta, MTK_EIS_REGION, GMV_X, EIS_REGION_INDEX_GMVX);
            isGMVValid |= QUERY_ENTRY_SINGLE_BY_IDX(control.halMeta, MTK_EIS_REGION, GMV_Y, EIS_REGION_INDEX_GMVY);
            isGMVValid |= QUERY_ENTRY_SINGLE_BY_IDX(control.halMeta, MTK_EIS_REGION, GMVCONF_X, EIS_REGION_INDEX_CONFX);
            isGMVValid |= QUERY_ENTRY_SINGLE_BY_IDX(control.halMeta, MTK_EIS_REGION, GMVCONF_Y, EIS_REGION_INDEX_CONFY);
            isGMVValid |= QUERY_ENTRY_SINGLE_BY_IDX(control.halMeta, MTK_EIS_REGION, GMVMAX, EIS_REGION_INDEX_MAX_GMV);
            CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SET_P2, "[%s] isGMVValid(%d) GMV = (%d, %d, %d, %d, %d) ",
                __FUNCTION__,
                isGMVValid,
                GMV_X,
                GMV_Y,
                GMVCONF_X,
                GMVCONF_Y,
                GMVMAX);
            rP2Info.AF_GMV_X = GMV_X;
            rP2Info.AF_GMV_Y = GMV_Y;
            rP2Info.AF_GMV_Conf_X = GMVCONF_X;
            rP2Info.AF_GMV_Conf_Y = GMVCONF_Y;
            rP2Info.AF_GMV_Max = GMVMAX;
        }
//------------------------------------Extract Pointer to Metadata---------------------------------------------

        #define FETCH_ENTRY_SINGLE(VAR, TYPE, TAG) \
            TYPE VAR;\
            QUERY_ENTRY_SINGLE<TYPE>(control.halMeta, TAG, VAR); \
            rNewP2Param.VAR = &VAR;

        FETCH_ENTRY_SINGLE(rpSclCropRect,   MRect,              MTK_3A_PRV_CROP_REGION);
        //FETCH_ENTRY_SINGLE(rpP1Crop,        MRect,             MTK_P1NODE_SCALAR_CROP_REGION);  //TG Domain
        FETCH_ENTRY_SINGLE(rpP1Crop,        MRect,              MTK_P1NODE_BIN_CROP_REGION);  //After FBin Domain
        FETCH_ENTRY_SINGLE(rpP2Crop,        MRect,              MTK_ISP_P2_CROP_REGION);
        FETCH_ENTRY_SINGLE(rpRzSize,        MSize,              MTK_P1NODE_RESIZER_SIZE);
        FETCH_ENTRY_SINGLE(rpP2OriginSize,  MSize,              MTK_ISP_P2_ORIGINAL_SIZE);
        FETCH_ENTRY_SINGLE(rpP2RzSize,      MSize,              MTK_ISP_P2_RESIZER_SIZE);
        FETCH_ENTRY_SINGLE(rpLscData,       IMetadata::Memory,  MTK_LSC_TBL_DATA);
        FETCH_ENTRY_SINGLE(rpTsfData,       IMetadata::Memory,  MTK_LSC_TSF_DATA);
        FETCH_ENTRY_SINGLE(rpTsfDumpNo,     IMetadata::Memory,  MTK_LSC_TSF_DUMP_NO);
        FETCH_ENTRY_SINGLE(rpRzInSize,      MSize,              MTK_P1NODE_BIN_SIZE);  //After FBin Domain

        #undef FETCH_ENTRY_SINGLE


        if (rNewP2Param.rpSclCropRect)
        {
            // crop info for AE
            rNewP2Param.rScaleCropRect.i4Xoffset = rNewP2Param.rpSclCropRect->p.x;
            rNewP2Param.rScaleCropRect.i4Yoffset = rNewP2Param.rpSclCropRect->p.y;
            rNewP2Param.rScaleCropRect.i4Xwidth  = rNewP2Param.rpSclCropRect->s.w;
            rNewP2Param.rScaleCropRect.i4Yheight = rNewP2Param.rpSclCropRect->s.h;

            rP2Info.rSclCropRectl.p.x = rNewP2Param.rpSclCropRect->p.x;
            rP2Info.rSclCropRectl.p.y = rNewP2Param.rpSclCropRect->p.y;
            rP2Info.rSclCropRectl.s.w = rNewP2Param.rpSclCropRect->s.w;
            rP2Info.rSclCropRectl.s.h = rNewP2Param.rpSclCropRect->s.h;
            CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SET_P2, "[%s] Pass2 info AE SCL CROP(%d,%d,%d,%d)",
                __FUNCTION__, rNewP2Param.rpSclCropRect->p.x, rNewP2Param.rpSclCropRect->p.y, rNewP2Param.rpSclCropRect->s.w, rNewP2Param.rpSclCropRect->s.h);
        }
        else
        {
            CAM_LOGW("[%s] HalMeta not send preview crop region", __FUNCTION__);
        }
        m_pResultPoolObj->updateP2Info(rP2Info);
#if 0 //Pass2 not use
        if (rNewP2Param.rpSclCropRect)
        {
            mPrvCropRegion.p.x = rNewP2Param.rpSclCropRect->p.x;
            mPrvCropRegion.p.y = rNewP2Param.rpSclCropRect->p.y;
            mPrvCropRegion.s.w = rNewP2Param.rpSclCropRect->s.w;
            mPrvCropRegion.s.h = rNewP2Param.rpSclCropRect->s.h;

            // crop info for AE
            rNewP2Param.rScaleCropRect.i4Xoffset = rNewP2Param.rpSclCropRect->p.x;
            rNewP2Param.rScaleCropRect.i4Yoffset = rNewP2Param.rpSclCropRect->p.y;
            rNewP2Param.rScaleCropRect.i4Xwidth  = rNewP2Param.rpSclCropRect->s.w;
            rNewP2Param.rScaleCropRect.i4Yheight = rNewP2Param.rpSclCropRect->s.h;

            // crop info for AF
            CameraArea_T& rArea = mAfParams.rScaleCropArea;
            MINT32 i4TgWidth = 0;
            MINT32 i4TgHeight = 0;
            mpHal3aObj->queryTgSize(i4TgWidth,i4TgHeight);

            rArea.i4Left   = mPrvCropRegion.p.x;
            rArea.i4Top    = mPrvCropRegion.p.y;
            rArea.i4Right  = mPrvCropRegion.p.x + mPrvCropRegion.s.w;
            rArea.i4Bottom = mPrvCropRegion.p.y + mPrvCropRegion.s.h;
            rArea = _transformArea(mi4SensorIdx, mi4SensorMode, rArea);
            rArea = _clipArea(i4TgWidth, i4TgHeight, rArea);

            CAM_LOGD_IF(1, "[%s] AE SCL CROP(%d,%d,%d,%d) AF SCL CROP(%d,%d,%d,%d)",
                    __FUNCTION__, rNewP2Param.rScaleCropRect.i4Xoffset, rNewP2Param.rScaleCropRect.i4Yoffset, rNewP2Param.rScaleCropRect.i4Xwidth, rNewP2Param.rScaleCropRect.i4Yheight,
                    rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom);
        }
#endif
        IMetadata::IEntry entryRed = control.appMeta.entryFor(MTK_TONEMAP_CURVE_RED);
        if (entryRed.tag() != IMetadata::IEntry::BAD_TAG)
        {
            rNewP2Param.u4TonemapCurveRedSize = entryRed.count();
            rNewP2Param.pTonemapCurveRed = (const MFLOAT*)entryRed.data();
            //rNewP2Param.pTonemapCurveRed = pTonemapRed;
        }

        IMetadata::IEntry entryGreen = control.appMeta.entryFor(MTK_TONEMAP_CURVE_GREEN);
        if (entryGreen.tag() != IMetadata::IEntry::BAD_TAG)
        {
            rNewP2Param.u4TonemapCurveGreenSize = entryGreen.count();
            rNewP2Param.pTonemapCurveGreen = (const MFLOAT*)entryGreen.data();
            //rNewP2Param.pTonemapCurveGreen = pTonemapGreen;
        }

        IMetadata::IEntry entryBlue = control.appMeta.entryFor(MTK_TONEMAP_CURVE_BLUE);
        if (entryBlue.tag() != IMetadata::IEntry::BAD_TAG)
        {
            rNewP2Param.u4TonemapCurveBlueSize = entryBlue.count();
            rNewP2Param.pTonemapCurveBlue = (const MFLOAT*)entryBlue.data();
            //rNewP2Param.pTonemapCurveBlue = pTonemapBlue;
        }
        AAA_TRACE_END_HAL;

        CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SET_P2,"[%s] FrameId(%d), HueMode(%d),BrightnessMode(%d), ContrastMode(%d), SaturationMode(%d), i4EdgeMode(%d)"
                , __FUNCTION__, rNewP2Param.i4MagicNum, rNewP2Param.i4HueMode, rNewP2Param.i4BrightnessMode, rNewP2Param.i4ContrastMode, rNewP2Param.i4SaturationMode, rNewP2Param.i4halEdgeMode);

        CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SET_P2, "[%s] MTK_TONEMAP_MODE(%d), MagicNum(%d) Rsize(%d) Gsize(%d) Bsize(%d)",
                    __FUNCTION__, rNewP2Param.u1TonemapMode, rNewP2Param.i4MagicNum,
                    rNewP2Param.u4TonemapCurveRedSize, rNewP2Param.u4TonemapCurveGreenSize, rNewP2Param.u4TonemapCurveBlueSize);

//=========================================================================================================
//                                                                              Metadata extraction complete
//=========================================================================================================
        rIspInfo.isCapture = 0
                    || rNewP2Param.u1CapIntent == MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT
                    || rNewP2Param.u1CapIntent == MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG
                    || rNewP2Param.u1CapIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE
                    ;
        extract(&rIspInfo.hint, &control.halMeta);
        rIspInfo.hint.SensorDev = m_i4SensorDev;

        m_pResultPoolObj->lockLastInfo();
        LastInfo_T vLastInfo = m_pResultPoolObj->getLastInfo();
        // Restore caminfo
        if (pCaminfoBuf == NULL && vLastInfo.mBackupCamInfo_copied)
        {
            vLastInfo.mBackupCamInfo.rCropRzInfo.sTGout.w = m_i4TgWidth;
            vLastInfo.mBackupCamInfo.rCropRzInfo.sTGout.h = m_i4TgHeight;
            vLastInfo.mBackupCamInfo.rMapping_Info.eSensorMode = static_cast<ESensorMode_T>(m_i4SensorMode);
            vLastInfo.mBackupCamInfo.rMapping_Info.eIspProfile = static_cast<NSIspTuning::EIspProfile_T>(rNewP2Param.u1IspProfile);

            pCaminfoBuf = &vLastInfo.mBackupCamInfo;
            CAM_LOGD("[%s] Restore caminfo,copied(%d)/mode(%d)/profile(%d)/FrmId(%d)/TG(%d,%d)",__FUNCTION__, vLastInfo.mBackupCamInfo_copied,
                    vLastInfo.mBackupCamInfo.rMapping_Info.eSensorMode,
                    vLastInfo.mBackupCamInfo.rMapping_Info.eIspProfile,
                     rNewP2Param.i4MagicNum, m_i4TgWidth, m_i4TgHeight);
        }
        m_pResultPoolObj->unlockLastInfo();

        if (pCaminfoBuf)
        {
            AAA_TRACE_HAL(CopyCaminfo);
            ::memcpy(&rIspInfo.rCamInfo, pCaminfoBuf, sizeof(NSIspTuning::RAWIspCamInfo));

            //NR3D Params
            const NSCam::NR3D::NR3DIspParam *pNR3DBuf = NULL;
            IMetadata::Memory pNR3DMeta;
            auto bNR3DMeta = IMetadata::getEntry<IMetadata::Memory>(&control.halMeta, MTK_3A_ISP_NR3D_HW_PARAMS, pNR3DMeta);
            if (bNR3DMeta) {
                pNR3DBuf = (const NSCam::NR3D::NR3DIspParam *)(pNR3DMeta.array());
                if(pNR3DBuf) ::memcpy(&rIspInfo.rCamInfo.NR3D_Data, pNR3DBuf, sizeof(NSCam::NR3D::NR3DIspParam));
            }

            QUERY_ENTRY_SINGLE(control.halMeta, MTK_3A_ISP_DISABLE_NR, rNewP2Param.bBypassNR);

            //Feature Control Overwrite
            QUERY_ENTRY_SINGLE(control.halMeta, MTK_3A_ISP_BYPASS_LCE, rNewP2Param.bBypassLCE);

            AAA_TRACE_END_HAL;

            rIspInfo.rCamInfo.bEnableNDD = rNewP2Param.bEnableNDD;

            setISPInfo(rNewP2Param, rIspInfo, 0);

            if(!rIspInfo.rCamInfo.bBypassLCE){

                if (pTuningBuf->pLcsBuf == NULL)
                {
                     CAM_LOGD("[%s] [-No Lcso Buffer ]", __FUNCTION__);
                     rIspInfo.rCamInfo.bBypassLCE = MTRUE;
                }
            }

            CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SET_P2, "[%s] bBypassLCE %d",__FUNCTION__, rIspInfo.rCamInfo.bBypassLCE);

            if (rule.isREADEnable("ISPHAL"))
                _readDump(pTuningBuf, control, pResult, &rIspInfo, (MINT32)E_Lsc_Output);

            MUINT32 u4ManualMode = 0;
#if HAL3A_TEST_OVERRIDE
            GET_PROP("vendor.debug.hal3av3.testp2", 0, u4ManualMode);
            _test_p2(u4ManualMode, mParams, rNewP2Param);
#endif

            setP2Params(rNewP2Param, &rResultP2);

            // set ISP Profile to file naming hint
            rIspInfo.hint.IspProfile = rIspInfo.rCamInfo.rMapping_Info.eIspProfile;

            CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SET_P2, "[%s]+ sensorDev(%d), key(%09d), #(%d), flow(%d), ispProfile(%d), rpg(%d), pTuningBuf(%p)",
                __FUNCTION__, m_i4SensorDev, rIspInfo.i4UniqueKey, rIspInfo.rCamInfo.u4Id, flowType, rIspInfo.rCamInfo.rMapping_Info.eIspProfile, rIspInfo.rCamInfo.fgRPGEnable, pTuningBuf);

            if (rNewP2Param.u1Exif && pResult) {
                UPDATE_ENTRY_SINGLE(pResult->halMeta, MTK_3A_EXIF_METADATA, rNewP2Param.rexifMeta);
            }

            if (rNewP2Param.u1Exif && m_bDbgInfoEnable)
            {
                // Must fill ResultPool with standard metadata

                if (m_bDbgInfoEnable) {
                    if(/*rIspInfo.rCamInfo.rMapping_Info.eIspProfile != EIspProfile_Capture_MultiPass_HWNR && */rNewP2Param.rpdbgIsp) {
                        // after got p1 debug info
                        rResultP2.vecDbgIspP2.appendArray(rNewP2Param.rpdbgIsp->array(), rNewP2Param.rpdbgIsp->size());
                    }

                    // generate P2 tuning and get result including P2 debug info
                    generateP2(flowType, rIspInfo, pTuningBuf, &rResultP2);

                    // after got p1 debug info, append p2 debug info, and then put to result
                    if (rResultP2.vecDbgIspP2.size()) {
                        UPDATE_ENTRY_SINGLE<MINT32>(rNewP2Param.rexifMeta, MTK_3A_EXIF_DBGINFO_ISP_KEY, ISP_DEBUG_KEYID);
                        IMetadata::Memory dbgIsp;
                        dbgIsp.appendVector(rResultP2.vecDbgIspP2);
                        UPDATE_ENTRY_SINGLE(rNewP2Param.rexifMeta, MTK_3A_EXIF_DBGINFO_ISP_DATA, dbgIsp);
                    }

#if 0
                    // multi-pass NR debug info
                    if (rResultP2.vecDbgIspP2_MultiP.size())
                    {
                        if (pResult)
                        {
                            QUERY_ENTRY_SINGLE(pResult->halMeta, MTK_3A_EXIF_METADATA, rNewP2Param.rexifMeta);
                        }
                        UPDATE_ENTRY_SINGLE<MINT32>(rNewP2Param.rexifMeta, MTK_POSTNR_EXIF_DBGINFO_NR_KEY, DEBUG_EXIF_MID_CAM_RESERVE1);
                        IMetadata::Memory dbgIsp;
                        dbgIsp.appendVector(rResultP2.vecDbgIspP2_MultiP);
                        UPDATE_ENTRY_SINGLE(rNewP2Param.rexifMeta, MTK_POSTNR_EXIF_DBGINFO_NR_DATA, dbgIsp);
                    }
#endif
                    if (pResult) {
                        UPDATE_ENTRY_SINGLE(pResult->halMeta, MTK_3A_EXIF_METADATA, rNewP2Param.rexifMeta);
                    }

                    MINT32 CaptureDump = property_get_int32("vendor.debug.camera.dump.p2.debuginfo", 0);
                    MINT32 PreviewDump = property_get_int32("vendor.debug.camera.dump.isp.preview", 0);
                    if (((CaptureDump && rIspInfo.isCapture && (rNewP2Param.bEnableNDD || (rNewP2Param.i4P2InImgFmt != EYuv2Yuv)))
                        || PreviewDump || rNewP2Param.u1DumpExif))
                    {
                        char filename[512] = "";

                        if (rIspInfo.rCamInfo.u1P2TuningUpdate == 3 || rIspInfo.rCamInfo.u1P2TuningUpdate == 6){
                            genFileName_TUNING(filename, sizeof(filename), &rIspInfo.hint,"lpcnrout1");
                        } else if (rIspInfo.rCamInfo.u1P2TuningUpdate == 4 || rIspInfo.rCamInfo.u1P2TuningUpdate == 7){
                            genFileName_TUNING(filename, sizeof(filename), &rIspInfo.hint, "lpcnrout2");
                        } else
                            genFileName_TUNING(filename, sizeof(filename), &rIspInfo.hint);
                        _dumpDebugInfo(filename, rNewP2Param.rexifMeta);
                    }

                    MINT32 dumpAAO        = property_get_int32("vendor.debug.camera.AAO.dump", 0);
                    MINT32 dumpAAOPreview = property_get_int32("vendor.debug.camera.AAO.dump.preview", 0);
                    if ((dumpAAO && rIspInfo.isCapture) || dumpAAOPreview)
                    {
                        char filename[512] = "";
                        char temp[512];
                        genFileName_HW_AAO(filename, sizeof(filename), &rIspInfo.hint);

                        sprintf(temp, "/sdcard/camera_dump/captue_end_aao_%d.hw_aao", rNewP2Param.i4MagicNum);
                        if(_isFileExist(temp))
                        {
                            int result;
                            result= rename(temp, filename);
                            if ( result == 0 )
                                CAM_LOGD("Renamed success: %s", filename);
                            else
                                CAM_LOGD("Renamed fail: %s", temp);
                        }
                        else
                            _dumpAAO(filename, m_i4SensorDev, rNewP2Param.i4MagicNum);
                    }
                }
            }else {
                // generate P2 tuning only
                generateP2(flowType, rIspInfo, pTuningBuf, NULL);
            }

#if 0 // HAL3 ReprocessCaptureTest#testReprocessRequestKeys failed
            if (pResult)
            {
                UPDATE_ENTRY_SINGLE(pResult->appMeta, MTK_EDGE_MODE, static_cast<MUINT8>(rIspInfo.rCamInfo.eEdgeMode));
                UPDATE_ENTRY_SINGLE(pResult->appMeta, MTK_NOISE_REDUCTION_MODE, static_cast<MUINT8>(rIspInfo.rCamInfo.eNRMode));
            }
#endif
            CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SET_P2, "[%s]- OK(%p, %p)", __FUNCTION__, pTuningBuf->pRegBuf, pTuningBuf->pLsc2Buf);
            i4Ret = 0;
        }
        else if(rNewP2Param.u1IspProfile == EIspProfile_YUV_Reprocess){
            rIspInfo.rCamInfo.bBypassLCE = MTRUE;

            setISPInfo(rNewP2Param, rIspInfo, 1);

            if (pResult){
                UPDATE_ENTRY_SINGLE(pResult->appMeta, MTK_EDGE_MODE, static_cast<MUINT8>(rIspInfo.rCamInfo.eEdgeMode));
                UPDATE_ENTRY_SINGLE(pResult->appMeta, MTK_NOISE_REDUCTION_MODE, static_cast<MUINT8>(rIspInfo.rCamInfo.eNRMode));
                UPDATE_ENTRY_SINGLE(pResult->halMeta, MTK_3A_EXIF_METADATA, getReprocStdExif(control));
            }

            generateP2(flowType, rIspInfo, pTuningBuf, NULL);

            i4Ret = 0;
        }
        else
        {
            CAM_LOGE("[%s] NG (no caminfo)", __FUNCTION__);
            i4Ret = -1;
        }


        if (rNewP2Param.u1TonemapMode != MTK_TONEMAP_MODE_FAST && rNewP2Param.u1TonemapMode != MTK_TONEMAP_MODE_HIGH_QUALITY)
        {
            getP2Result(rNewP2Param, &rResultP2);
            convertP2ResultToMeta(rResultP2, pResult);
        }

        AAA_TRACE_HAL(ReadDump);
        if (rule.isREADEnable("ISPHAL")){
            _readDump(pTuningBuf, control, pResult, &rIspInfo, (MINT32)E_LPCNR_Output);
            _readDump(pTuningBuf, control, pResult, &rIspInfo, (MINT32)E_Tuning_Output);
            _readDump(pTuningBuf, control, pResult, &rIspInfo, (MINT32)E_MFB_Output);
            _readDump(pTuningBuf, control, pResult, &rIspInfo, (MINT32)E_LTM_CURVE_Output);
            _readDump(pTuningBuf, control, pResult, &rIspInfo, (MINT32)E_YNR_MAP_Output);
        }
        AAA_TRACE_END_HAL;

        if (((rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_Capture) ||
            (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_Capture_DCE) ||
            (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_Capture_DSDN) ||
            (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_Before_Blend) ||
            (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_MFB) ||
            (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_Single) ||
            (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_After_Blend) ||
            (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_AINR_Single) ||
            (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_AINR_MainYUV) ||
            (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_AINR_Main))  &&
             rule.isDumpEnable("ISPHAL")){
               MUINT32 u4RegCnt_start = 0; //start address count
               MUINT32 u4RegCnt = 0; // reg count in exif

               sendIspCtrl(EISPCtrl_GetP2TuningInfo, (MINTPTR)&u4RegCnt, (MINTPTR)&u4RegCnt_start);

               if (pTuningBuf->pRegBuf != NULL)
                {
                    const char *ispProfileName = IspTuningMgr::getIspProfileName(rIspInfo.rCamInfo.rMapping_Info.eIspProfile);
                    char strTuningFile[512] = {'\0'};
                    sprintf(strTuningFile, "/data/vendor/camera_dump/%09d-%04d-%04d-%d-%s.p2buf", rIspInfo.hint.UniqueKey, rIspInfo.hint.RequestNo, rIspInfo.hint.FrameNo, m_i4SensorDev, ispProfileName);
                    if (rIspInfo.rCamInfo.u1P2TuningUpdate == 3 || rIspInfo.rCamInfo.u1P2TuningUpdate == 6){
                        sprintf(strTuningFile, "/data/vendor/camera_dump/%09d-%04d-%04d-%d-%s-%s.p2buf"
                            , rIspInfo.hint.UniqueKey, rIspInfo.hint.RequestNo, rIspInfo.hint.FrameNo, m_i4SensorDev, "lpcnrout1", ispProfileName);
                    } else if (rIspInfo.rCamInfo.u1P2TuningUpdate == 4 || rIspInfo.rCamInfo.u1P2TuningUpdate == 7){
                        sprintf(strTuningFile, "/data/vendor/camera_dump/%09d-%04d-%04d-%d-%s-%s.p2buf"
                            , rIspInfo.hint.UniqueKey, rIspInfo.hint.RequestNo, rIspInfo.hint.FrameNo, m_i4SensorDev, "lpcnrout2", ispProfileName);
                    } else {
                        sprintf(strTuningFile, "/data/vendor/camera_dump/%09d-%04d-%04d-%d-%s.p2buf"
                            , rIspInfo.hint.UniqueKey, rIspInfo.hint.RequestNo, rIspInfo.hint.FrameNo, m_i4SensorDev, ispProfileName);
                    }

                    FILE* fidTuning = fopen(strTuningFile, "wb");
                    if (fidTuning)
                    {
                         CAM_LOGD("[p2buf] %s pRegBuf size(%d)",__FUNCTION__, strTuningFile, u4RegCnt*4);
                         fwrite((MUINT32*)pTuningBuf->pRegBuf+u4RegCnt_start, u4RegCnt*4, 1, fidTuning);
                         fclose(fidTuning);
                    }
                }
           }
    }

    AAA_TRACE_END_D;
    return i4Ret;
}

MBOOL
HalIspImp::
setFdGamma(MINT32 i4Index, MINT32* pTonemapCurveRed, MUINT32 u4size){
    return MTRUE;
}

MINT32
HalIspImp::
sendIspCtrl(EISPCtrl_T eISPCtrl, MINTPTR iArg1, MINTPTR iArg2)
{
    MINT32 i4Ret = 0;
    switch (eISPCtrl)
    {
        // ----------------------------------ISP----------------------------------
        case EISPCtrl_GetIspGamma:
            IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_GET_ISP_GAMMA, iArg1, iArg2);
            break;
        case EISPCtrl_ValidatePass1:
            {
                MINT32 i4Magic = (MINT32)iArg1;
                NSIspTuning::EIspProfile_T prof = static_cast<NSIspTuning::EIspProfile_T>(iArg2);
                RequestSet_T rRequestSet;
                rRequestSet.vNumberSet.clear();
                rRequestSet.vNumberSet.push_back(i4Magic);
                NS3Av3::ParamIspProfile_T _3AProf(prof, i4Magic, 1, MTRUE, NS3Av3::ParamIspProfile_T::EParamValidate_All, rRequestSet);
                _3AProf.i4MagicNum = i4Magic;
                IspTuningMgr::getInstance().setIspProfile(m_i4SensorDev, prof);
                validateP1(_3AProf);
            }
            break;
        case EISPCtrl_SetIspProfile:
            IspTuningMgr::getInstance().setIspProfile(m_i4SensorDev, static_cast<NSIspTuning::EIspProfile_T>(iArg1));
            m_eIspProfile = static_cast<NSIspTuning::EIspProfile_T>(iArg1);
            break;
        case EISPCtrl_GetOBOffset:
            {
                const ISPResultToMeta_T *pISPResult = (const ISPResultToMeta_T*)m_pResultPoolObj->getResult(m_i4Magic, E_ISP_RESULTTOMETA, __FUNCTION__);
                CAM_LOGD("[%s], GetOBOffset ISPResult(%p) at MagicNum(%d)", __FUNCTION__, pISPResult, m_i4Magic);
                if (!pISPResult)
                {
                    MINT32 rHistoryReqMagic[HistorySize] = {0,0,0};
                    m_pResultPoolObj->getHistory(rHistoryReqMagic);
                    pISPResult = (const ISPResultToMeta_T*)m_pResultPoolObj->getResult(rHistoryReqMagic[1], E_ISP_RESULTTOMETA, __FUNCTION__);
                    CAM_LOGD("[%s], Get ISPResult(%p) at MagicNum(%d) Instead", __FUNCTION__, pISPResult, rHistoryReqMagic[1]);
                }
                MINT32 *OBOffset = reinterpret_cast<MINT32*>(iArg1);

                OBOffset[0] =  pISPResult->rCamInfo.rOBC_OFST[0];
                OBOffset[1] =  pISPResult->rCamInfo.rOBC_OFST[1];
                OBOffset[2] =  pISPResult->rCamInfo.rOBC_OFST[2];
                OBOffset[3] =  pISPResult->rCamInfo.rOBC_OFST[3];
            }
            break;
        case EISPCtrl_SetOperMode:
            MINT32 i4OperMode;
            i4OperMode = IspTuningMgr::getInstance().getOperMode(m_i4SensorDev);
            if(i4OperMode != EOperMode_Meta)
                i4Ret = IspTuningMgr::getInstance().setOperMode(m_i4SensorDev, iArg1);
            CAM_LOGD("[%s] prev_mode(%d), new_mode(%ld)", __FUNCTION__, i4OperMode, (long)iArg1);
            break;
        case EISPCtrl_GetOperMode:
            *(reinterpret_cast<MUINT32*>(iArg1)) = IspTuningMgr::getInstance().getOperMode(m_i4SensorDev);
            break;
        case EISPCtrl_GetMfbSize:
            IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_GET_MFB_SIZE, iArg1, iArg2);
            break;
        case EISPCtrl_GetLtmCurve:
            IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_GET_LTM_CURVE_SIZE, iArg1, iArg2);
            break;
        case EISPCtrl_GetP2TuningInfo:
            IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_GET_P2_BUFFER_SIZE, iArg1, iArg2);
            break;
        case EISPCtrl_SetLcsoParam:
            getCurrLCSResult(*(ISP_LCS_OUT_INFO_T*)iArg1);
            break;
        case EISPCtrl_NotifyCCU:
            IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_NOTIFY_CCU_START, iArg1, iArg2);
            break;
        case EISPCtrl_WTSwitch:
            WT_Reconfig((MVOID*)iArg1);
            break;
                // --------------------------------- LCE ---------------------------------
        case EISPCtrl_GetLCEGain:
            IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_GET_LCE_GAIN, iArg1, 0);
            break;
        case EISPCtrl_GetAINRParam:
            IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_GetAINRParam, iArg1, iArg2);
            break;
        default:
            CAM_LOGD("[%s] Unsupport Command(%d)", __FUNCTION__, eISPCtrl);
            return MFALSE;
    }
    return MTRUE;
}

MINT32
HalIspImp::
attachCb(/*IHal3ACb::ECb_T eId, IHal3ACb* pCb*/)
{
    return MTRUE;
}

MINT32
HalIspImp::
detachCb(/*IHal3ACb::ECb_T eId, IHal3ACb* pCb*/)
{
    return MTRUE;
}

MVOID
HalIspImp::
setFDEnable(MBOOL fgEnable)
{
    CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SET_P1, "[%s] fgEnable(%d)", __FUNCTION__, fgEnable);
    m_bFaceDetectEnable = fgEnable;
    IspTuningMgr::getInstance().setFDEnable(m_i4SensorDev, fgEnable);
    if (!m_bFaceDetectEnable)
        m_i4faceNum = 0;
}

MBOOL
HalIspImp::
setFDInfo(MVOID* prFaces)
{
    //TODO
    setFDInfo(prFaces, prFaces);
    return MTRUE;
}

MBOOL
HalIspImp::
setFDInfo(MVOID* prFaces, MVOID* prAFFaces)
{
    CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SET_P1, "[%s] m_bFaceDetectEnable(%d)", __FUNCTION__, m_bFaceDetectEnable);
    if (m_bFaceDetectEnable)
    {
        MtkCameraFaceMetadata *pFaces = (MtkCameraFaceMetadata *)prAFFaces;
        m_i4faceNum = pFaces->number_of_faces;

        IspTuningMgr::getInstance().setFDInfo(m_i4SensorDev, prAFFaces, m_i4TgWidth, m_i4TgHeight);
    }
    return MTRUE;
}

MBOOL
HalIspImp::
setOTInfo(MVOID* prOT)
{
    return MTRUE;
}

MINT32
HalIspImp::
updateTGInfo()
{
    //Before wait for VSirq of IspDrv, we need to query IHalsensor for the current TG info
    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList)
    {
        CAM_LOGE("MAKE_HalSensorList() == NULL");
        return MFALSE;
    }
    const char* const callerName = "HalISPQueryTG";
    IHalSensor* pHalSensor = pHalSensorList->createSensor(callerName, m_i4SensorIdx);
    //Note that Middleware has configured sensor before
    SensorDynamicInfo senInfo;
    MINT32 i4SensorDevId = pHalSensor->querySensorDynamicInfo(m_i4SensorDev, &senInfo);
    pHalSensor->destroyInstance(callerName);

    CAM_LOGD("m_i4SensorDev = %d, senInfo.TgInfo = %d\n", m_i4SensorDev, senInfo.TgInfo);

    MINT32 i4TgInfo = senInfo.TgInfo; //now, TG info is obtained! TG1, TG2 or TG3

    m_pCamIO->sendCommand( NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_TG_OUT_SIZE, (MINTPTR)&m_i4TgWidth, (MINTPTR)&m_i4TgHeight, 0);
    CAM_LOGD("[%s] TG size(%d,%d)", __FUNCTION__, m_i4TgWidth, m_i4TgHeight);

    IspTuningMgr::getInstance().setTGInfo(m_i4SensorDev, i4TgInfo, m_i4TgWidth, m_i4TgHeight);

    return MTRUE;
}

MVOID
HalIspImp::
querySensorStaticInfo()
{
    //Before phone boot up (before opening camera), we can query IHalsensor for the sensor static info (EX: MONO or Bayer)
    SensorStaticInfo sensorStaticInfo;
    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList)
    {
        CAM_LOGE("MAKE_HalSensorList() == NULL");
        return;
    }
    pHalSensorList->querySensorStaticInfo(m_i4SensorDev,&sensorStaticInfo);

    MUINT32 u4RawFmtType = sensorStaticInfo.rawFmtType; // SENSOR_RAW_MONO or SENSOR_RAW_Bayer

    // 3A/ISP mgr can query sensor static information here
    IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_NOTIFY_SENSOR_TYPE, u4RawFmtType, 0);
}

MBOOL
HalIspImp::
validateP1(const ParamIspProfile_T& rParamIspProfile)
{
    m_pTuning->validatePerFrameP1(m_i4SensorDev, rParamIspProfile.rRequestSet);

    return MTRUE;
}

MBOOL
HalIspImp::
setISPInfo(P2Param_T const &rNewP2Param, NSIspTuning::ISP_INFO_T &rIspInfo, MINT32 type)
{
    AAA_TRACE_HAL(setISPInfo);
    // type == 0 would do the all set
    if (type < 1) {
        rIspInfo.rCamInfo.bBypassLCE                        = rNewP2Param.bBypassLCE;
        rIspInfo.rCamInfo.i4P2InImgFmt                      = static_cast<EP2IN_FMT_T>(rNewP2Param.i4P2InImgFmt);
        rIspInfo.rCamInfo.u1P2TuningUpdate                  = static_cast<EP2UPDATE_MODE>(rNewP2Param.u1P2TuningUpdate);

        rIspInfo.rCamInfo.bBypassNR                         = rNewP2Param.bBypassNR;

        rIspInfo.rCamInfo.eEdgeMode                         = static_cast<mtk_camera_metadata_enum_android_edge_mode_t>(rNewP2Param.u1appEdgeMode);
        rIspInfo.rCamInfo.eToneMapMode                      = static_cast<mtk_camera_metadata_enum_android_tonemap_mode_t>(rNewP2Param.u1TonemapMode);

        //rIspInfo.rCamInfo.rIspUsrSelectLevel.eIdx_Edge      = static_cast<EIndex_Isp_Edge_T>(rNewP2Param.i4halEdgeMode);
        //rIspInfo.rCamInfo.rIspUsrSelectLevel.eIdx_Bright    = static_cast<EIndex_Isp_Brightness_T>(rNewP2Param.i4BrightnessMode);
        //rIspInfo.rCamInfo.rIspUsrSelectLevel.eIdx_Contrast  = static_cast<EIndex_Isp_Contrast_T>(rNewP2Param.i4ContrastMode);
        //rIspInfo.rCamInfo.rIspUsrSelectLevel.eIdx_Hue       = static_cast<EIndex_Isp_Hue_T>(rNewP2Param.i4HueMode);
        //rIspInfo.rCamInfo.rIspUsrSelectLevel.eIdx_Sat       = static_cast<EIndex_Isp_Saturation_T>(rNewP2Param.i4SaturationMode);

        rIspInfo.rCamInfo.rIspUsrSelectLevel.eIdx_Edge     = MTK_CONTROL_ISP_EDGE_MIDDLE;
        rIspInfo.rCamInfo.rIspUsrSelectLevel.eIdx_Bright   = MTK_CONTROL_ISP_BRIGHTNESS_MIDDLE;
        rIspInfo.rCamInfo.rIspUsrSelectLevel.eIdx_Contrast = MTK_CONTROL_ISP_CONTRAST_MIDDLE;
        rIspInfo.rCamInfo.rIspUsrSelectLevel.eIdx_Hue      = MTK_CONTROL_ISP_HUE_MIDDLE;
        rIspInfo.rCamInfo.rIspUsrSelectLevel.eIdx_Sat      = MTK_CONTROL_ISP_SATURATION_MIDDLE;
    }

    if (type < 2) {
        //     _reprocess  part

        rIspInfo.rCamInfo.fgRPGEnable = !rNewP2Param.u1PGN;

        if (rNewP2Param.u1IspProfile != 255)
        {
            rIspInfo.rCamInfo.rMapping_Info.eIspProfile = static_cast<NSIspTuning::EIspProfile_T>(rNewP2Param.u1IspProfile);
        }
        else
        {
            switch (rNewP2Param.u1CapIntent)
            {
            case MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD:
            case MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT:
                rIspInfo.rCamInfo.rMapping_Info.eIspProfile = NSIspTuning::EIspProfile_Video;
                break;
            case MTK_CONTROL_CAPTURE_INTENT_PREVIEW:
            case MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG:
                rIspInfo.rCamInfo.rMapping_Info.eIspProfile = NSIspTuning::EIspProfile_Preview;
                break;
            case MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE:
                if(rIspInfo.rCamInfo.fgRPGEnable){
                    rIspInfo.rCamInfo.rMapping_Info.eIspProfile = NSIspTuning::EIspProfile_Preview;
                }else{
                    rIspInfo.rCamInfo.rMapping_Info.eIspProfile = NSIspTuning::EIspProfile_Capture;
                }
                break;
            }
        }

        rIspInfo.rCamInfo.i4RawType = static_cast<ERawType_T>(rNewP2Param.i4RawType);

        if (((!rIspInfo.rCamInfo.fgRPGEnable) && (rNewP2Param.i4RawType == NSIspTuning::ERawType_Pure)) || m_i4CopyLscP1En == 1)
        {
            if (rNewP2Param.rpLscData)
                rIspInfo.rLscData = std::vector<MUINT8>((rNewP2Param.rpLscData)->array(), (rNewP2Param.rpLscData)->array()+(rNewP2Param.rpLscData)->size());
            else
                CAM_LOGD("[%s] No shading entry in metadata\n", __FUNCTION__);

            if (rNewP2Param.rpTsfData)
                rIspInfo.rTsfData = std::vector<MUINT8>((rNewP2Param.rpTsfData)->array(), (rNewP2Param.rpTsfData)->array()+(rNewP2Param.rpTsfData)->size());
            else
            {
                CAM_LOGD("[%s] No Tsfs output entry in metadata\n", __FUNCTION__);
            }
        }

        MINT32 _u4LSCDumpEn = property_get_int32("vendor.debug.lsc_mgr.log", 0);
        if(_u4LSCDumpEn == 4095)
        {
            if (rNewP2Param.rpTsfDumpNo)
            {
                std::vector<MUINT8> rLscDumpNo = std::vector<MUINT8>((rNewP2Param.rpTsfDumpNo)->array(), (rNewP2Param.rpTsfDumpNo)->array()+(rNewP2Param.rpTsfDumpNo)->size());

                CAM_LOGD("[%s] Copy Tsf dump %s to hint\n", __FUNCTION__, (char *)&rLscDumpNo[0]);
                strncpy(rIspInfo.hint.additStr, (char *)&rLscDumpNo[0], rLscDumpNo.size());
            }
            else
            {
                CAM_LOGD("[%s] No Tsf dump no entry in metadata\n", __FUNCTION__);
            }
        }

        if(rIspInfo.rLscData.size())
        {
            MUINT32 mu4DumpLscP1En=0;
            MUINT32 mu4DumpLscP1CapEn=0;
            GET_PROP("vendor.debug.camera.dump.p1.lsc", 0, mu4DumpLscP1En);
            GET_PROP("vendor.debug.camera.dump.cap.lsc", 0, mu4DumpLscP1CapEn);
            if(mu4DumpLscP1En || mu4DumpLscP1CapEn && rIspInfo.isCapture && (rNewP2Param.bEnableNDD || (rNewP2Param.i4P2InImgFmt != EYuv2Yuv)))
            {
                char filename[256] = {'\0'};
                rIspInfo.hint.IspProfile = rIspInfo.rCamInfo.rMapping_Info.eIspProfile;
                genFileName_LSC(filename, sizeof(filename), &rIspInfo.hint);
                android::sp<IFileCache> fidLscDump;
                fidLscDump = IFileCache::open(filename);
                if (fidLscDump->write(&rIspInfo.rLscData[0], rIspInfo.rLscData.size()) != rIspInfo.rLscData.size())
                {
                    CAM_LOGD("[%s] write error %s", __FUNCTION__, filename);
                }
            }
        }

        if(rIspInfo.rTsfData.size())
        {
            MUINT32 mu4DumpTsfP1En=0;
            MUINT32 mu4DumpTsfP1CapEn=0;
            GET_PROP("vendor.debug.camera.dump.p1.tsfo", 0, mu4DumpTsfP1En);
            GET_PROP("vendor.debug.camera.dump.cap.tsfo", 0, mu4DumpTsfP1CapEn);
            if(mu4DumpTsfP1En || mu4DumpTsfP1CapEn && rIspInfo.isCapture && (rNewP2Param.bEnableNDD || (rNewP2Param.i4P2InImgFmt != EYuv2Yuv)))
            {
                char filename[256] = "";  //Coverity: Uninitialized variables
                rIspInfo.hint.IspProfile = rIspInfo.rCamInfo.rMapping_Info.eIspProfile;
                genFileName_TSF(filename, sizeof(filename), &rIspInfo.hint);
                android::sp<IFileCache> fidLscDump;
                fidLscDump = IFileCache::open(filename);
                if (fidLscDump->write(&rIspInfo.rTsfData[0], rIspInfo.rTsfData.size()) != rIspInfo.rTsfData.size())
                {
                    CAM_LOGD("[%s] write error %s", __FUNCTION__, filename);
                }
            }
        }

        rIspInfo.i4UniqueKey = rNewP2Param.i4UniqueKey;
        rIspInfo.rCamInfo.targetSize = rNewP2Param.targetSize;

        //Coverity: Null pointer dereferences
        if(rNewP2Param.rpRzInSize == nullptr ){
            CAM_LOGE("[%s] ERROR: rNewP2Param.rpRzInSize is a null pointer!", __FUNCTION__);
            return MFALSE;
        }

        if (rNewP2Param.rpP1Crop && rNewP2Param.rpRzSize && rNewP2Param.rpRzInSize &&
            (rIspInfo.rCamInfo.fgRPGEnable))  //||
             //rIspInfo.rCamInfo.eIspProfile == NSIspTuning::EIspProfile_N3D_Denoise  ||
             //rIspInfo.rCamInfo.eIspProfile == NSIspTuning::EIspProfile_N3D_Denoise_toGGM))
        {
            rIspInfo.rCamInfo.rCropRzInfo.rRRZcrop.p.x  = rNewP2Param.rpP1Crop->p.x;
            rIspInfo.rCamInfo.rCropRzInfo.rRRZcrop.p.y  = rNewP2Param.rpP1Crop->p.y;
            rIspInfo.rCamInfo.rCropRzInfo.rRRZcrop.s.w  = rNewP2Param.rpP1Crop->s.w;
            rIspInfo.rCamInfo.rCropRzInfo.rRRZcrop.s.h  = rNewP2Param.rpP1Crop->s.h;
            rIspInfo.rCamInfo.rCropRzInfo.sRRZout.w     = rNewP2Param.rpRzSize->w;
            rIspInfo.rCamInfo.rCropRzInfo.sRRZout.h     = rNewP2Param.rpRzSize->h;
            rIspInfo.rCamInfo.rCropRzInfo.fgRRZOnOff    = MTRUE;

            rIspInfo.rCamInfo.rCropRzInfo.sRRZin.w      = rNewP2Param.rpRzInSize->w;
            rIspInfo.rCamInfo.rCropRzInfo.sRRZin.h      = rNewP2Param.rpRzInSize->h;
        }
        else if(rIspInfo.rCamInfo.i4RawType == NSIspTuning::ERawType_Pure){
            rIspInfo.rCamInfo.rCropRzInfo.rRRZcrop.p.x  = 0;
            rIspInfo.rCamInfo.rCropRzInfo.rRRZcrop.p.y  = 0;
            rIspInfo.rCamInfo.rCropRzInfo.rRRZcrop.s.w  = rIspInfo.rCamInfo.rCropRzInfo.sTGout.w;
            rIspInfo.rCamInfo.rCropRzInfo.rRRZcrop.s.h  = rIspInfo.rCamInfo.rCropRzInfo.sTGout.h;
            rIspInfo.rCamInfo.rCropRzInfo.sRRZout.w     = rIspInfo.rCamInfo.rCropRzInfo.sTGout.w;
            rIspInfo.rCamInfo.rCropRzInfo.sRRZout.h     = rIspInfo.rCamInfo.rCropRzInfo.sTGout.h;
            rIspInfo.rCamInfo.rCropRzInfo.fgRRZOnOff    = MFALSE;

            rIspInfo.rCamInfo.rCropRzInfo.sRRZin.w      = rIspInfo.rCamInfo.rCropRzInfo.sTGout.w;
            rIspInfo.rCamInfo.rCropRzInfo.sRRZin.h      = rIspInfo.rCamInfo.rCropRzInfo.sTGout.h;
        }
        else{
            rIspInfo.rCamInfo.rCropRzInfo.rRRZcrop.p.x  = 0;
            rIspInfo.rCamInfo.rCropRzInfo.rRRZcrop.p.y  = 0;
            rIspInfo.rCamInfo.rCropRzInfo.rRRZcrop.s.w  = rNewP2Param.rpRzInSize->w;
            rIspInfo.rCamInfo.rCropRzInfo.rRRZcrop.s.h  = rNewP2Param.rpRzInSize->h;
            rIspInfo.rCamInfo.rCropRzInfo.sRRZout.w     = rNewP2Param.rpRzInSize->w;
            rIspInfo.rCamInfo.rCropRzInfo.sRRZout.h     = rNewP2Param.rpRzInSize->h;
            rIspInfo.rCamInfo.rCropRzInfo.fgRRZOnOff    = MFALSE;

            rIspInfo.rCamInfo.rCropRzInfo.sRRZin.w      = rNewP2Param.rpRzInSize->w;
            rIspInfo.rCamInfo.rCropRzInfo.sRRZin.h      = rNewP2Param.rpRzInSize->h;
        }

        if((rNewP2Param.rpRzInSize->w != rIspInfo.rCamInfo.rCropRzInfo.sTGout.w) ||
           (rNewP2Param.rpRzInSize->h != rIspInfo.rCamInfo.rCropRzInfo.sTGout.h))
        {
            rIspInfo.rCamInfo.rCropRzInfo.fgFBinOnOff   = MTRUE;
        }
        else
        {
            rIspInfo.rCamInfo.rCropRzInfo.fgFBinOnOff   = MFALSE;
        }

        if(rNewP2Param.i4P2InImgFmt ==EYuv2Yuv){

            MUINT32 ResizeYUV_W = rNewP2Param.ResizeYUV & 0x0000FFFF;
            MUINT32 ResizeYUV_H = rNewP2Param.ResizeYUV >> 16;


            if( ResizeYUV_W != 0 && ResizeYUV_H !=0){
                if((ResizeYUV_W != rIspInfo.rCamInfo.rCropRzInfo.sRRZout.w) ||
                (ResizeYUV_H != rIspInfo.rCamInfo.rCropRzInfo.sRRZout.h)){
                rIspInfo.rCamInfo.rCropRzInfo.sRRZout.w = ResizeYUV_W;
                rIspInfo.rCamInfo.rCropRzInfo.sRRZout.h = ResizeYUV_H;
                rIspInfo.rCamInfo.rCropRzInfo.fgRRZOnOff = MTRUE;
                }
            }
        }

#if 1
        // CRZ temporarily disable, so rIspP2CropInfo align RRZ info rCropRzInfo
        rIspInfo.rIspP2CropInfo = rIspInfo.rCamInfo.rCropRzInfo;

#else
        if (rNewP2Param.rpP2OriginSize && rNewP2Param.rpP2Crop && rNewP2Param.rpP2RzSize)
        {
            rIspInfo.rIspP2CropInfo.i4FullW     = rNewP2Param.rpP2OriginSize->w;
            rIspInfo.rIspP2CropInfo.i4FullH     = rNewP2Param.rpP2OriginSize->h;
            rIspInfo.rIspP2CropInfo.i4OfstX     = rNewP2Param.rpP2Crop->p.x;
            rIspInfo.rIspP2CropInfo.i4OfstY     = rNewP2Param.rpP2Crop->p.y;
            rIspInfo.rIspP2CropInfo.i4Width     = rNewP2Param.rpP2Crop->s.w;
            rIspInfo.rIspP2CropInfo.i4Height    = rNewP2Param.rpP2Crop->s.h;
            rIspInfo.rIspP2CropInfo.i4RzWidth   = rNewP2Param.rpP2RzSize->w;
            rIspInfo.rIspP2CropInfo.i4RzHeight  = rNewP2Param.rpP2RzSize->h;
            rIspInfo.rIspP2CropInfo.fgOnOff     = MTRUE;
        }
        else
        {
            rIspInfo.rIspP2CropInfo.i4FullW     = rIspInfo.rCamInfo.rCropRzInfo.i4FullW;
            rIspInfo.rIspP2CropInfo.i4FullH     = rIspInfo.rCamInfo.rCropRzInfo.i4FullH;
            rIspInfo.rIspP2CropInfo.i4OfstX     = 0;
            rIspInfo.rIspP2CropInfo.i4OfstY     = 0;
            rIspInfo.rIspP2CropInfo.i4Width     = rIspInfo.rCamInfo.rCropRzInfo.i4FullW;
            rIspInfo.rIspP2CropInfo.i4Height    = rIspInfo.rCamInfo.rCropRzInfo.i4FullH;
            rIspInfo.rIspP2CropInfo.i4RzWidth   = rIspInfo.rCamInfo.rCropRzInfo.i4FullW;
            rIspInfo.rIspP2CropInfo.i4RzHeight  = rIspInfo.rCamInfo.rCropRzInfo.i4FullH;
            rIspInfo.rIspP2CropInfo.fgOnOff     = MFALSE;
        }
#endif

        rIspInfo.rCamInfo.eEdgeMode = static_cast<mtk_camera_metadata_enum_android_edge_mode_t>(rNewP2Param.u1appEdgeMode);
        rIspInfo.rCamInfo.eNRMode = static_cast<mtk_camera_metadata_enum_android_noise_reduction_mode_t>(rNewP2Param.u1NrMode);


        if (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_YUV_Reprocess)
        {
            rIspInfo.rCamInfo.rAEInfo.u4P2RealISOValue= rNewP2Param.i4ISO;
            rIspInfo.rCamInfo.eIdx_Scene = static_cast<NSIspTuning::EIndex_Scene_T>(0);  //MTK_CONTROL_SCENE_MODE_DISABLED
            rIspInfo.rCamInfo.rMapping_Info.eSensorMode = NSIspTuning::ESensorMode_Capture;
        }
    }

    AAA_TRACE_END_HAL;

    return MTRUE;
}


MBOOL
HalIspImp::
setP2Params(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2)
{
    AAA_TRACE_HAL(setP2Params);
#if 0 //Not use
    if( rNewP2Param.rScaleCropRect.i4Xwidth != 0 && rNewP2Param.rScaleCropRect.i4Yheight != 0 )
        IAeMgr::getInstance().setZoomWinInfo(m_i4SensorDev, rNewP2Param.rScaleCropRect.i4Xoffset,rNewP2Param.rScaleCropRect.i4Yoffset,rNewP2Param.rScaleCropRect.i4Xwidth,rNewP2Param.rScaleCropRect.i4Yheight);
    if( rNewP2Param.rScaleCropRect.i4Xwidth != 0 && rNewP2Param.rScaleCropRect.i4Yheight != 0 )
        IAwbMgr::getInstance().setZoomWinInfo(m_i4SensorDev, rNewP2Param.rScaleCropRect.i4Xoffset,rNewP2Param.rScaleCropRect.i4Yoffset,rNewP2Param.rScaleCropRect.i4Xwidth,rNewP2Param.rScaleCropRect.i4Yheight);
    // AE for Denoise OB2
    IAeMgr::getInstance().enableStereoDenoiseRatio(m_i4SensorDev, rNewP2Param.i4DenoiseMode);
#endif

    //ISP
    IspTuningMgr::getInstance().setIspUserIdx_Bright(m_i4SensorDev, rNewP2Param.i4BrightnessMode);
    IspTuningMgr::getInstance().setIspUserIdx_Hue(m_i4SensorDev, rNewP2Param.i4HueMode);
    IspTuningMgr::getInstance().setIspUserIdx_Sat(m_i4SensorDev, rNewP2Param.i4SaturationMode);
    IspTuningMgr::getInstance().setIspUserIdx_Edge(m_i4SensorDev, rNewP2Param.i4halEdgeMode);
    IspTuningMgr::getInstance().setIspUserIdx_Contrast(m_i4SensorDev, rNewP2Param.i4ContrastMode);

    //IspTuningMgr::getInstance().setToneMapMode(m_i4SensorDev, rNewP2Param.u1TonemapMode);
    if (rNewP2Param.u1TonemapMode == MTK_TONEMAP_MODE_CONTRAST_CURVE)
    {
        MINT32 i = 0;
        std::vector<MFLOAT> vecIn, vecOut;
        MINT32 i4Cnt = rNewP2Param.u4TonemapCurveRedSize/ 2;
        vecIn.resize(i4Cnt);
        vecOut.resize(i4Cnt);
        MFLOAT* pArrayIn = &(vecIn[0]);
        MFLOAT* pArrayOut = &(vecOut[0]);
        const MFLOAT* pCurve = rNewP2Param.pTonemapCurveRed;
        for (i = i4Cnt; i != 0; i--)
        {
            MFLOAT x, y;
            x = *pCurve++;
            y = *pCurve++;
            *pArrayIn++ = x;
            *pArrayOut++ = y;
            pResultP2->vecTonemapCurveRed.push_back(x);
            pResultP2->vecTonemapCurveRed.push_back(y);
            CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SETPARAM_P2, "[Red]#%d(%f,%f)", rNewP2Param.i4MagicNum, x, y);
        }
        IspTuningMgr::getInstance().setTonemapCurve_Red(m_i4SensorDev, &(vecIn[0]), &(vecOut[0]), &i4Cnt);

        i4Cnt = rNewP2Param.u4TonemapCurveGreenSize/ 2;
        vecIn.resize(i4Cnt);
        vecOut.resize(i4Cnt);
        pArrayIn = &(vecIn[0]);
        pArrayOut = &(vecOut[0]);
        pCurve = rNewP2Param.pTonemapCurveGreen;
        for (i = i4Cnt; i != 0; i--)
        {
            MFLOAT x, y;
            x = *pCurve++;
            y = *pCurve++;
            *pArrayIn++ = x;
            *pArrayOut++ = y;
            pResultP2->vecTonemapCurveGreen.push_back(x);
            pResultP2->vecTonemapCurveGreen.push_back(y);
            CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SETPARAM_P2, "[Green]#%d(%f,%f)", rNewP2Param.i4MagicNum, x, y);
        }
        IspTuningMgr::getInstance().setTonemapCurve_Green(m_i4SensorDev, &(vecIn[0]), &(vecOut[0]), &i4Cnt);

        i4Cnt = rNewP2Param.u4TonemapCurveBlueSize/ 2;
        vecIn.resize(i4Cnt);
        vecOut.resize(i4Cnt);
        pArrayIn = &(vecIn[0]);
        pArrayOut = &(vecOut[0]);
        pCurve = rNewP2Param.pTonemapCurveBlue;
        for (i = i4Cnt; i != 0; i--)
        {
            MFLOAT x, y;
            x = *pCurve++;
            y = *pCurve++;
            *pArrayIn++ = x;
            *pArrayOut++ = y;
            pResultP2->vecTonemapCurveBlue.push_back(x);
            pResultP2->vecTonemapCurveBlue.push_back(y);
            CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SETPARAM_P2, "[Blue]#%d(%f,%f)", rNewP2Param.i4MagicNum, x, y);
        }
        IspTuningMgr::getInstance().setTonemapCurve_Blue(m_i4SensorDev, &(vecIn[0]), &(vecOut[0]), &i4Cnt);
    }
    AAA_TRACE_END_HAL;

    return MTRUE;
}

MBOOL
HalIspImp::
generateP2(MINT32 flowType, const NSIspTuning::ISP_INFO_T& rIspInfo, void* pTuningBuf, ResultP2_T* pResultP2)
{
    std::lock_guard<std::mutex> lock(m_P2Mtx);

    void* pRegBuf = ((TuningParam*)pTuningBuf)->pRegBuf;
    CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SETPARAM_P2, "[%s] + flow(%d), buf(%p)", __FUNCTION__, flowType, pRegBuf);


    AAA_TRACE_ISP(P2_VLD);
    IspTuningMgr::getInstance().validatePerFrameP2(m_i4SensorDev, flowType, rIspInfo, pTuningBuf);
    AAA_TRACE_END_ISP;

#if CAM3_LSC_FEATURE_EN
    ILscBuf* pLscBuf = NSIspTuning::ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->getP2Buf();
    if (pLscBuf)
        ((TuningParam*)pTuningBuf)->pLsc2Buf = pLscBuf->getBuf();
    else
        ((TuningParam*)pTuningBuf)->pLsc2Buf = NULL;
#endif

    if (!((dip_x_reg_t*)pRegBuf)->DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_LCE_D1_EN){
        ((TuningParam*)pTuningBuf)->pLcsBuf = NULL;
    }

    ((TuningParam*)pTuningBuf)->pBpc2Buf = IspTuningMgr::getInstance().getDMGItable(m_i4SensorDev, rIspInfo.rCamInfo.fgRPGEnable);

    IspTuningBufCtrl::getInstance(m_i4SensorDev)->updateHint((void*)(&rIspInfo.hint), rIspInfo.rCamInfo.u4Id);

    // debug info
    if (pResultP2)
    {
#if 0
        if (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Capture_MultiPass_HWNR)
        {
            CAM_LOGD_IF(m_3ALogEnable, "[%s] get debug info p2 for Multi_Pass_NR #(%d)", __FUNCTION__, rIspInfo.rCamInfo.u4Id);
            if (0 == pResultP2->vecDbgIspP2_MultiP.size())
            {
                CAM_LOGD_IF(m_3ALogEnable, "[%s] Need to allocate P2 result for Multi_Pass_NR", __FUNCTION__);
                pResultP2->vecDbgIspP2_MultiP.resize(sizeof(DEBUG_RESERVEA_INFO_T));
            }
            DEBUG_RESERVEA_INFO_T& rIspExifDebugInfo = *reinterpret_cast<DEBUG_RESERVEA_INFO_T*>(pResultP2->vecDbgIspP2_MultiP.editArray());
            IspTuningMgr::getInstance().getDebugInfo_MultiPassNR(m_i4SensorDev, rIspInfo, rIspExifDebugInfo, pRegBuf);
        }
#endif
        CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SETPARAM_P2, "[%s] get debug info p2 #(%d)", __FUNCTION__, rIspInfo.rCamInfo.u4Id);
        if (0 == pResultP2->vecDbgIspP2.size())
        {
            CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SETPARAM_P2, "[%s] Need to allocate P2 result", __FUNCTION__);
            pResultP2->vecDbgIspP2.resize(sizeof(AAA_DEBUG_INFO2_T));
        }

        AAA_DEBUG_INFO2_T& rDbg3AInfo2 = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(pResultP2->vecDbgIspP2.editArray());
        NSIspExifDebug::IspExifDebugInfo_T& rIspExifDebugInfo = rDbg3AInfo2.rISPDebugInfo;
        IspTuningMgr::getInstance().getDebugInfoP2(m_i4SensorDev, rIspInfo, rIspExifDebugInfo, pTuningBuf);

        MBOOL bDump = ::property_get_int32("vendor.debug.tuning.dump_capture", 0);
        if (!rIspInfo.rCamInfo.fgRPGEnable && bDump)
        {
            char filename[512] = "";
            sprintf(filename, "/sdcard/debug/p2dbg_dump_capture-%04d.bin", rIspInfo.i4UniqueKey);
            FILE* fp = fopen(filename, "wb");
            if (fp)
            {
                ::fwrite(rIspExifDebugInfo.P2RegInfo.regDataP2, sizeof(rIspExifDebugInfo.P2RegInfo.regDataP2), 1, fp);
            }
            if (fp)
                fclose(fp);
        }
    }
    //update mapping info
    CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_SETPARAM_P2, "[%s] -", __FUNCTION__);

    return MTRUE;
}

MBOOL
HalIspImp::
getP2Result(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2)
{
    if (rNewP2Param.u1TonemapMode != MTK_TONEMAP_MODE_CONTRAST_CURVE)
    {
        // Tonemap
        pResultP2->vecTonemapCurveRed.clear();
        pResultP2->vecTonemapCurveGreen.clear();
        pResultP2->vecTonemapCurveBlue.clear();

        MINT32 i = 0;
        MFLOAT *pIn, *pOut;
        MINT32 i4NumPt;
        IspTuningMgr::getInstance().getTonemapCurve_Blue(m_i4SensorDev, pIn, pOut, &i4NumPt);
        for (i = 0; i < i4NumPt; i++)
        {
            CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_GET_P2, "[%s][Blue](%f,%f)", __FUNCTION__, *pIn, *pOut);
            pResultP2->vecTonemapCurveBlue.push_back(*pIn++);
            pResultP2->vecTonemapCurveBlue.push_back(*pOut++);
        }
        IspTuningMgr::getInstance().getTonemapCurve_Green(m_i4SensorDev, pIn, pOut, &i4NumPt);
        for (i = 0; i < i4NumPt; i++)
        {
            CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_GET_P2, "[%s][Green](%f,%f)", __FUNCTION__, *pIn, *pOut);
            pResultP2->vecTonemapCurveGreen.push_back(*pIn++);
            pResultP2->vecTonemapCurveGreen.push_back(*pOut++);
        }
        IspTuningMgr::getInstance().getTonemapCurve_Red(m_i4SensorDev, pIn, pOut, &i4NumPt);
        for (i = 0; i < i4NumPt; i++)
        {
            CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_GET_P2, "[%s][Red](%f,%f)", __FUNCTION__, *pIn, *pOut);
            pResultP2->vecTonemapCurveRed.push_back(*pIn++);
            pResultP2->vecTonemapCurveRed.push_back(*pOut++);
        }
        CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_GET_P2,"[%s] rsize(%d) gsize(%d) bsize(%d)"
            ,__FUNCTION__, (MINT32)pResultP2->vecTonemapCurveRed.size(), (MINT32)pResultP2->vecTonemapCurveGreen.size(), (MINT32)pResultP2->vecTonemapCurveBlue.size());

    }
    return MTRUE;
}

MINT32
HalIspImp::
convertP2ResultToMeta(const ResultP2_T& rResultP2, MetaSet_T* pResult) const
{
    MBOOL fgLog = (HALISP_LOG_CONVERT_P2 & m_u4LogEn);
    MINT32 i4Size = 0;
    if (pResult != NULL)
    {
        // tonemap
        i4Size = rResultP2.vecTonemapCurveBlue.size();
        if (i4Size)
        {
            const MFLOAT* pCurve = &(rResultP2.vecTonemapCurveBlue[0]);
            UPDATE_ENTRY_ARRAY(pResult->appMeta, MTK_TONEMAP_CURVE_BLUE, pCurve, i4Size);
            CAM_LOGD_IF(fgLog, "[%s] B size(%d), P0(%f,%f), P_end(%f,%f)", __FUNCTION__, i4Size, pCurve[0], pCurve[1], pCurve[i4Size-2], pCurve[i4Size-1]);
        }
        i4Size = rResultP2.vecTonemapCurveGreen.size();
        if (i4Size)
        {
            const MFLOAT* pCurve = &(rResultP2.vecTonemapCurveGreen[0]);
            UPDATE_ENTRY_ARRAY(pResult->appMeta, MTK_TONEMAP_CURVE_GREEN, pCurve, i4Size);
            CAM_LOGD_IF(fgLog, "[%s] G size(%d), P0(%f,%f), P_end(%f,%f)", __FUNCTION__, i4Size, pCurve[0], pCurve[1], pCurve[i4Size-2], pCurve[i4Size-1]);
        }
        i4Size = rResultP2.vecTonemapCurveRed.size();
        if (i4Size)
        {
            const MFLOAT* pCurve = &(rResultP2.vecTonemapCurveRed[0]);
            UPDATE_ENTRY_ARRAY(pResult->appMeta, MTK_TONEMAP_CURVE_RED, pCurve, i4Size);
            CAM_LOGD_IF(fgLog, "[%s] R size(%d), P0(%f,%f), P_end(%f,%f)", __FUNCTION__, i4Size, pCurve[0], pCurve[1], pCurve[i4Size-2], pCurve[i4Size-1]);
        }
    }
    return 0;
}

MBOOL
HalIspImp::
_readDump(TuningParam* pTuningBuf, const MetaSet_T& control, MetaSet_T* pResult, ISP_INFO_T* pIspInfo, MINT32 i4Format)
{
#if 1
    FileReadRule rule;
    MINT32 i4RetSize;

    if (!rule.isREADEnable("ISPHAL") && pIspInfo == NULL)
        return MFALSE;
    if ((pIspInfo->rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_Capture) ||
        (pIspInfo->rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_Capture_DCE) ||
        (pIspInfo->rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_Capture_DSDN) ||
        (pIspInfo->rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_After_Blend) ||
        (pIspInfo->rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_MFB) ||
        (pIspInfo->rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_Single) ||
        (pIspInfo->rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_Before_Blend) ||
        (pIspInfo->rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_AINR_Main) ||
        (pIspInfo->rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_AINR_MainYUV) ||
        (pIspInfo->rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_AINR_Single))
    {
        MINT32 i4ReqNo = 0;
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_PIPELINE_REQUEST_NUMBER, i4ReqNo);
        if (QUERY_ENTRY_SINGLE(control.halMeta, MTK_HAL_REQUEST_INDEX_BSS, i4ReqNo)){
            CAM_LOGD("[%s] MTK_HAL_REQUEST_INDEX_BSS(%d)",__FUNCTION__, i4ReqNo);
        }
        char strDump[512] = {'\0'};
        std::string strispProfileName_prefix = "EIspProfile_";
        std::string strispProfileName_prefix_postfix = IspTuningMgr::getIspProfileName(pIspInfo->rCamInfo.rMapping_Info.eIspProfile);
        std::string strispProfileName = strispProfileName_prefix + strispProfileName_prefix_postfix;
        MUINT32 u4P2TuningBufferSize = queryTuningSize();

        CAM_LOGD("[%s] i4ReqNo(%d) ispProfileName(%s) format(%d)",__FUNCTION__, i4ReqNo, strispProfileName.c_str(), i4Format);

        if ((i4Format == E_Tuning_Output) || (i4Format == E_LPCNR_Output))
        {
            MUINT32 u4RegCnt_start = 0; //start address count
            MUINT32 u4RegCnt = 0; // reg count in exif
            MBOOL bIsLpcnr1 = (pIspInfo->rCamInfo.u1P2TuningUpdate == ELPCNR_8Bit_Pass1 || pIspInfo->rCamInfo.u1P2TuningUpdate == ELPCNR_10Bit_Pass1);
            MBOOL bIsLpcnr2 = (pIspInfo->rCamInfo.u1P2TuningUpdate == ELPCNR_8Bit_Pass2 || pIspInfo->rCamInfo.u1P2TuningUpdate == ELPCNR_10Bit_Pass2);

            sendIspCtrl(EISPCtrl_GetP2TuningInfo, (MINTPTR)&u4RegCnt, (MINTPTR)&u4RegCnt_start);

            if (i4Format == E_Tuning_Output) {
                if (!(bIsLpcnr1 || bIsLpcnr2))
                    rule.getFile_P2TUNING(i4ReqNo, strispProfileName.c_str(), strDump, 512, "ISPHAL");
            }
            else if (i4Format == E_LPCNR_Output){
                if (bIsLpcnr1)
                    rule.getFile_LPCNR_TUNING(i4ReqNo, strispProfileName.c_str(), strDump, 512, "lpcnrout1");
                if (bIsLpcnr2)
                    rule.getFile_LPCNR_TUNING(i4ReqNo, strispProfileName.c_str(), strDump, 512, "lpcnrout2");
            }

            FILE* fidTuning = fopen(strDump, "rb");
            if (fidTuning)
            {
                if (u4P2TuningBufferSize >= (u4RegCnt_start+u4RegCnt)*4)
                {
                    CAM_LOGD("[%s] i4Format(%d) %s pRegBuf size(%d), u4RegCnt(%d), u4RegCnt_start(%d)",__FUNCTION__, i4Format, strDump, u4P2TuningBufferSize, u4RegCnt, u4RegCnt_start);
                    i4RetSize = 0;
                    i4RetSize =  fread((MUINT32*)pTuningBuf->pRegBuf+u4RegCnt_start, u4RegCnt*4, 1, fidTuning);
                    if (1 != i4RetSize)
                        CAM_LOGD("[%s] i4Format(%d) %s fread fail \n",__FUNCTION__, i4Format, strDump);
                } else {
                    CAM_LOGD("[%s] i4Format(%d) u4RegCnt_end*4(%d) > P2 Buffer Size(%d) ",
                        __FUNCTION__, i4Format, strDump, ((u4RegCnt_start+u4RegCnt))*4, u4P2TuningBufferSize);
                }
                fclose(fidTuning);
            }
        }

        if (i4Format == E_Lsc_Output)
        {
            rule.getFile_LSC(i4ReqNo, strispProfileName.c_str(), strDump, 512, "ISPHAL");
            FILE* fidLscRead = fopen(strDump, "rb");
            if (fidLscRead){
                CAM_LOGD("[%s] %s ILscTbl::lscdata",__FUNCTION__, strDump, pIspInfo->rLscData.size());
                i4RetSize = 0;
                i4RetSize =  fread(pIspInfo->rLscData.data(), pIspInfo->rLscData.size(), 1, fidLscRead);
                if (1 != i4RetSize)
                        CAM_LOGD("[%s] i4Format(%d) %s fread fail \n",__FUNCTION__, i4Format, strDump);
                fclose(fidLscRead);
            }
        }

        if (i4Format == E_MFB_Output && (pIspInfo->rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_MFB))
        {
            MINT32 i4MfbSize = 0;
            sendIspCtrl(EISPCtrl_GetMfbSize, (MINTPTR)&i4MfbSize, (MINTPTR)NULL);
            if (i4MfbSize == 0)
                return MTRUE;
            rule.getFile_MFB(i4ReqNo, strispProfileName.c_str(), strDump, 512);
            FILE* fidTuning = fopen(strDump, "rb");
            if (fidTuning)
            {
                CAM_LOGD("[%s] %s pMfbBuf size(%d)",__FUNCTION__, strDump, i4MfbSize);
                i4RetSize = 0;
                i4RetSize =  fread(pTuningBuf->pMfbBuf, i4MfbSize, 1, fidTuning);
                if (1 != i4RetSize)
                        CAM_LOGD("[%s] i4Format(%d) %s fread fail \n",__FUNCTION__, i4Format, strDump);
                fclose(fidTuning);
            }
        }

        if (i4Format == E_LTM_CURVE_Output)
        {
            MUINT32 u4RegCnt_start = 0; //start address count
            MUINT32 u4RegCnt = 0; // reg count in exif

            sendIspCtrl(EISPCtrl_GetLtmCurve, (MINTPTR)&u4RegCnt, (MINTPTR)&u4RegCnt_start);
            rule.getFile_LTMCURVE(i4ReqNo, strispProfileName.c_str(), strDump, 512, "ISPHAL");

            FILE* fidTuning = fopen(strDump, "rb");
            if (fidTuning)
            {
                if (u4P2TuningBufferSize >= (u4RegCnt_start+u4RegCnt)*4)
                {
                    CAM_LOGD("[%s] %s pRegBuf size(%d), LtmCurveCnt(%d), u4LtmCurveCnt_start(%d)",__FUNCTION__, strDump, u4P2TuningBufferSize, u4RegCnt, u4RegCnt_start);
                     i4RetSize = 0;
                     i4RetSize = fread((MUINT32*)pTuningBuf->pRegBuf+u4RegCnt_start, u4RegCnt*4, 1, fidTuning);
                     if (1 != i4RetSize)
                        CAM_LOGD("[%s] i4Format(%d) %s fread fail \n",__FUNCTION__, i4Format, strDump);

                } else {
                    CAM_LOGD("[%s] LTM_end(%d) > P2 Buffer Size(%d) ",
                        __FUNCTION__, strDump, ((u4RegCnt_start+u4RegCnt))*4, u4P2TuningBufferSize);
                }
                fclose(fidTuning);
            }

        }

        if (i4Format == E_YNR_MAP_Output)
        {
            if (pTuningBuf->pFaceAlphaBuf != NULL){
                rule.getFile_YNR_MAP(i4ReqNo, strispProfileName.c_str(), (IImageBuffer*)pTuningBuf->pFaceAlphaBuf, "ISPHAL", m_i4SensorIdx);
                CAM_LOGD("[readdump] facemap read done");
            }
        }

    }
#endif
    return MTRUE;
}

IMetadata
HalIspImp::
getReprocStdExif(const MetaSet_T& control)
{
    IMetadata rMetaExif;

    MFLOAT fFNum = 0.0f;
    MFLOAT fFocusLength = 0.0f;
    MUINT8 u1AWBMode = 0;
    MINT32 u4LightSource = 0;
    MUINT8 u1SceneMode = 0;
    MINT32 u4ExpProgram = 0;
    MINT32 u4SceneCapType = 0;
    MUINT8 u1FlashState = 0;
    MINT32 u4FlashLightTimeus = 0;
    MINT32 u4AEComp = 0;
    MINT32 i4AEExpBias = 0;
    MINT32 u4AEISOSpeed = 0;
    MINT64 u8CapExposureTime = 0;
    QUERY_ENTRY_SINGLE(control.appMeta, MTK_LENS_APERTURE, fFNum);
    QUERY_ENTRY_SINGLE(control.appMeta, MTK_LENS_FOCAL_LENGTH , fFocusLength);
    QUERY_ENTRY_SINGLE(control.appMeta, MTK_CONTROL_AWB_MODE, u1AWBMode);

    //LightSource
    switch (u1AWBMode)
    {
        case MTK_CONTROL_AWB_MODE_AUTO:
        case MTK_CONTROL_AWB_MODE_WARM_FLUORESCENT:
        case MTK_CONTROL_AWB_MODE_TWILIGHT:
        case MTK_CONTROL_AWB_MODE_INCANDESCENT:
            u4LightSource = eLightSourceId_Other;
            break;
        case MTK_CONTROL_AWB_MODE_DAYLIGHT:
            u4LightSource = eLightSourceId_Daylight;
            break;
        case MTK_CONTROL_AWB_MODE_FLUORESCENT:
            u4LightSource = eLightSourceId_Fluorescent;
            break;
#if 0
        case MTK_CONTROL_AWB_MODE_TUNGSTEN:
            u4LightSource = eLightSourceId_Tungsten;
            break;
#endif
        case MTK_CONTROL_AWB_MODE_CLOUDY_DAYLIGHT:
            u4LightSource = eLightSourceId_Cloudy;
            break;
        case MTK_CONTROL_AWB_MODE_SHADE:
            u4LightSource = eLightSourceId_Shade;
            break;
        default:
            u4LightSource = eLightSourceId_Other;
            break;
        }

    QUERY_ENTRY_SINGLE(control.appMeta, MTK_CONTROL_SCENE_MODE, u1SceneMode);

    //EXP_Program
    switch (u1SceneMode)
    {
        case MTK_CONTROL_SCENE_MODE_PORTRAIT:
            u4ExpProgram = eExpProgramId_Portrait;
            break;
        case MTK_CONTROL_SCENE_MODE_LANDSCAPE:
            u4ExpProgram = eExpProgramId_Landscape;
            break;
        default:
            u4ExpProgram = eExpProgramId_NotDefined;
            break;
    }

    //SCENE_CAP_TYPE
    switch (u1SceneMode)
    {
        case MTK_CONTROL_SCENE_MODE_DISABLED:
        case MTK_CONTROL_SCENE_MODE_NORMAL:
        case MTK_CONTROL_SCENE_MODE_NIGHT_PORTRAIT:
        case MTK_CONTROL_SCENE_MODE_THEATRE:
        case MTK_CONTROL_SCENE_MODE_BEACH:
        case MTK_CONTROL_SCENE_MODE_SNOW:
        case MTK_CONTROL_SCENE_MODE_SUNSET:
        case MTK_CONTROL_SCENE_MODE_STEADYPHOTO:
        case MTK_CONTROL_SCENE_MODE_FIREWORKS:
        case MTK_CONTROL_SCENE_MODE_SPORTS:
        case MTK_CONTROL_SCENE_MODE_PARTY:
        case MTK_CONTROL_SCENE_MODE_CANDLELIGHT:
            u4SceneCapType = eCapTypeId_Standard;
            break;
        case MTK_CONTROL_SCENE_MODE_PORTRAIT:
            u4SceneCapType = eCapTypeId_Portrait;
            break;
        case MTK_CONTROL_SCENE_MODE_LANDSCAPE:
            u4SceneCapType = eCapTypeId_Landscape;
            break;
        case MTK_CONTROL_SCENE_MODE_NIGHT:
            u4SceneCapType = eCapTypeId_Night;
            break;
        default:
            u4SceneCapType = eCapTypeId_Standard;
            break;
    }

    //FlashTimeUs
    QUERY_ENTRY_SINGLE(control.appMeta, MTK_FLASH_STATE, u1FlashState);
    if (u1FlashState == MTK_FLASH_STATE_FIRED){
        u4FlashLightTimeus = 30000;
    }

    //AE_EXP_BIAS

    IMetadata mMetaStaticInfo = m_pResultPoolObj->getMetaStaticInfo();
    QUERY_ENTRY_SINGLE(control.appMeta, MTK_CONTROL_AE_EXPOSURE_COMPENSATION, u4AEComp );
    // AE Comp Step
    MFLOAT fExpCompStep= 0.0f;
    MRational rStep;
    if (QUERY_ENTRY_SINGLE(mMetaStaticInfo, MTK_CONTROL_AE_COMPENSATION_STEP, rStep))
    {
        fExpCompStep = (MFLOAT) rStep.numerator / rStep.denominator;
        CAM_LOGD("[%s] ExpCompStep(%3.3f), (%d/%d)", __FUNCTION__, fExpCompStep, rStep.numerator, rStep.denominator);
    }
    i4AEExpBias = fExpCompStep*u4AEComp*10;


    QUERY_ENTRY_SINGLE(control.appMeta, MTK_SENSOR_EXPOSURE_TIME, u8CapExposureTime);
    QUERY_ENTRY_SINGLE(control.appMeta, MTK_SENSOR_SENSITIVITY, u4AEISOSpeed);

    UPDATE_ENTRY_SINGLE<MINT32>(rMetaExif, MTK_3A_EXIF_FNUMBER,              fFNum*FNUMBER_PRECISION/*rExifInfo.u4FNumber*/);
    UPDATE_ENTRY_SINGLE<MINT32>(rMetaExif, MTK_3A_EXIF_FOCAL_LENGTH,         fFocusLength*1000/*rExifInfo.u4FocalLength*/);
    UPDATE_ENTRY_SINGLE<MINT32>(rMetaExif, MTK_3A_EXIF_AWB_MODE,             u1AWBMode);
    UPDATE_ENTRY_SINGLE<MINT32>(rMetaExif, MTK_3A_EXIF_LIGHT_SOURCE,         u4LightSource);
    UPDATE_ENTRY_SINGLE<MINT32>(rMetaExif, MTK_3A_EXIF_EXP_PROGRAM,          u4ExpProgram);
    UPDATE_ENTRY_SINGLE<MINT32>(rMetaExif, MTK_3A_EXIF_SCENE_CAP_TYPE,       u4SceneCapType);
    UPDATE_ENTRY_SINGLE<MINT32>(rMetaExif, MTK_3A_EXIF_FLASH_LIGHT_TIME_US,  u4FlashLightTimeus);
    UPDATE_ENTRY_SINGLE<MINT32>(rMetaExif, MTK_3A_EXIF_AE_METER_MODE,        (MINT32)eMeteringMode_Average);
    UPDATE_ENTRY_SINGLE<MINT32>(rMetaExif, MTK_3A_EXIF_AE_EXP_BIAS,          i4AEExpBias);
    UPDATE_ENTRY_SINGLE<MINT32>(rMetaExif, MTK_3A_EXIF_CAP_EXPOSURE_TIME,    u8CapExposureTime/1000);
    UPDATE_ENTRY_SINGLE<MINT32>(rMetaExif, MTK_3A_EXIF_AE_ISO_SPEED,         u4AEISOSpeed);

    return rMetaExif;
}

unsigned int
HalIspImp::
queryTuningSize()
{
    return sizeof(dip_x_reg_t);
}

MINT32
HalIspImp::
dumpIsp(MINT32 flowType, const MetaSet_T& control, TuningParam* pTuningBuf, MetaSet_T* pResult)
{
#if 0
    MUINT32 u4readDump = 0;
    FileReadRule rule;
    if (!rule.isDumpEnable("ISPHAL"))
        return MFALSE;
    IMetadata::Memory pCaminfoBuf;
    auto bCamInfoBuf = IMetadata::getEntry<IMetadata::Memory>(&control.halMeta, MTK_PROCESSOR_CAMINFO, pCaminfoBuf);
    NSIspTuning::ISP_INFO_T rIspInfo;
    MINT32 i4IspProfile = -1;
    MUINT8 u1IspProfile = NSIspTuning::EIspProfile_Preview;
    MUINT32 u4DebugInfo = 0;

    extract(&rIspInfo.hint, &control.halMeta);

    if (QUERY_ENTRY_SINGLE(control.halMeta, MTK_3A_ISP_PROFILE, u1IspProfile)){
        i4IspProfile = u1IspProfile;
    }

    P2Param_T rNewP2Param;
    QUERY_ENTRY_SINGLE(control.appMeta, MTK_CONTROL_CAPTURE_INTENT, rNewP2Param.u1CapIntent);
    setISPInfo(rNewP2Param, rIspInfo, 0);

    if (bCamInfoBuf)
    {
     if ((rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_Capture) ||
         (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_Before_Blend) ||
         (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_MFB) ||
         (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_Single) ||
         (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_After_Blend)){
            if (pTuningBuf->pRegBuf != NULL)
             {
                 const char *ispProfileName = IspTuningMgr::getIspProfileName(rIspInfo.rCamInfo.rMapping_Info.eIspProfile);
                 char strTuningFile[512] = {'\0'};
                 sprintf(strTuningFile, "/sdcard/camera_dump/%09d-%04d-%04d-%d-%s.p2buf", rIspInfo.hint.UniqueKey, rIspInfo.hint.FrameNo, rIspInfo.hint.RequestNo, m_i4SensorDev, ispProfileName);
                 FILE* fidTuning = fopen(strTuningFile, "wb");
                 if (fidTuning)
                 {
                      CAM_LOGD("[%s] %s pRegBuf size(%d)",__FUNCTION__, strTuningFile, queryTuningSize());
                      fwrite(pTuningBuf->pRegBuf, queryTuningSize(), 1, fidTuning);
                      fclose(fidTuning);
                 }
             }
        }
    }
#endif

    return MTRUE;
    //return MTRUE;
}

MINT32
HalIspImp::
get(MUINT32 frmId, MetaSet_T& result)
{
    AAA_TRACE_D("HalISPGet");
    AAA_TRACE_HAL(HalISPGet);
    CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_GET_P1, "[%s] sensorDev(%d), sensorIdx(%d) R(%d)", __FUNCTION__, m_i4SensorDev, m_i4SensorIdx, frmId);

    /*****************************
     *     To get ISP Result Pointer
     *****************************/
    MINT32 i4Ret = MTRUE;
    // Vector pointer
    AllResult_T *pAllResult = m_pResultPoolObj->getAllResult(frmId);
    if (pAllResult == NULL)
    {
        AAA_TRACE_END_HAL;
        AAA_TRACE_END_D;
        return MFALSE;
    }
    // Get result pointer if validateP1 true. Otherwise false
    const ISPResultToMeta_T* pISPResult = (ISPResultToMeta_T*)m_pResultPoolObj->getResult(frmId, E_ISP_RESULTTOMETA, __FUNCTION__);
    const LCSOResultToMeta_T* pLCSOResult = (LCSOResultToMeta_T*)m_pResultPoolObj->getResult(frmId, E_LCSO_RESULTTOMETA, __FUNCTION__);
//Special flow - InitReq, so not to return -1
#if 0
    if(pISPResult == NULL)
    {
        // ResultPool - Fail to get the specified result, use current.
        // ex:convert 27, get 25 fail, history 25/26/27, use current 27
        MY_LOGE("[%s] Fail to get the specified result", __FUNCTION__);
        MINT32 rHistoryReqMagic[HistorySize] = {0,0,0};
        m_pResultPoolObj->getHistory(rHistoryReqMagic);

        MY_LOGW("[%s] History (Req0, Req1, Req2) = (#%d, #%d, #%d)", __FUNCTION__, rHistoryReqMagic[0], rHistoryReqMagic[1], rHistoryReqMagic[2]);

        for(MINT32 i = (HistorySize-1); i >= 0 ; i--)
        {
            if(rHistoryReqMagic[i] != 0 && rHistoryReqMagic[i] != frmId)
            {
                pAllResult  = m_pResultPoolObj->getAllResult(rHistoryReqMagic[i]);
                pISPResult  = (ISPResultToMeta_T*)m_pResultPoolObj->getResult((rHistoryReqMagic[i]), E_ISP_RESULTTOMETA, __FUNCTION__);
            }
            if(pISPResult != NULL)
            {
                MY_LOGW("[%s] Use Current-MetaResult historyMagic[%d]:%d", __FUNCTION__, i, rHistoryReqMagic[i]);
                break;
            }
        }
    }
#endif
    if(pISPResult == NULL)
    {
        MY_LOGW("[%s] Not find result to conver metadata(#%d)", __FUNCTION__, frmId);
        AAA_TRACE_END_HAL;
        AAA_TRACE_END_D;
        //Special flow - InitReq, so not to return -1
        //return (-1);
    }

    NSIspTuning::RAWIspCamInfo tempCamInfo;

    if(pISPResult) {
        tempCamInfo = pISPResult->rCamInfo;

        //write back to caminfo
        IspTuningMgr::getInstance().setCCUInfo2ISP(m_i4SensorDev, tempCamInfo.rCCU_Result);
    }

    if(pLCSOResult){
        tempCamInfo.rLCS_Info = pLCSOResult->rLcsOutInfo;
    }
    else{
        CAM_LOGE("No LCS in Result Pool, FrmID: %d", frmId);
    }

    /*****************************
     *     Convert ISP Result to Metadata
     *****************************/


    MBOOL bP1_EXIF_Valid = MFALSE;
    IMetadata metaExif;
    // protect vector before use vector
    std::lock_guard<std::mutex> Vec_lock(pAllResult->LockVecResult);
    if(pAllResult->vecExifInfo.size() && pAllResult->vecDbgIspInfo.size() > 0){

        QUERY_ENTRY_SINGLE(result.halMeta, MTK_3A_EXIF_METADATA, metaExif);

        bP1_EXIF_Valid = MTRUE;
    }

    if(pISPResult)
    {
        UPDATE_MEMORY(result.halMeta, MTK_ISP_ATMS_MAPPING_INFO, tempCamInfo.rMapping_Info);

        UPDATE_MEMORY(result.halMeta, MTK_PROCESSOR_CAMINFO, tempCamInfo);

        UPDATE_ENTRY_SINGLE(result.halMeta, MTK_FEATURE_FACE_APPLIED_GAMMA, tempCamInfo.rFdInfo.FaceGGM_Idx);

        // color correction matrix
        if (pAllResult->vecColorCorrectMat.size())
        {
            const MFLOAT* pfMat = &(pAllResult->vecColorCorrectMat[0]);
            IMetadata::IEntry entry(MTK_COLOR_CORRECTION_TRANSFORM);
            for (MINT32 k = 0; k < 9; k++)
            {
                MRational rMat;
                MFLOAT fVal = *pfMat++;
                rMat.numerator = fVal*512;
                rMat.denominator = 512;
                entry.push_back(rMat, Type2Type<MRational>());
                CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_GET_P1, "[%s] Mat[%d] = (%3.6f, %d)", __FUNCTION__, k, fVal, rMat.numerator);
            }
            result.appMeta.update(MTK_COLOR_CORRECTION_TRANSFORM, entry);
        }

        if(bP1_EXIF_Valid)
        {
            // debug exif
            AAA_DEBUG_INFO2_T& rDbg3AInfo2 = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(pAllResult->vecDbgIspInfo.editArray());
            NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo = rDbg3AInfo2.rISPDebugInfo;
            // P1 ISP
            IspTuningMgr::getInstance().setHLRDebugInfo4CCU(m_i4SensorDev, tempCamInfo.rCCU_Result.HLR.HLR_EN ,tempCamInfo.rCCU_Result.HLR.HLR_Data , rDbgIspInfo);
            IspTuningMgr::getInstance().setLTMnLTMSDebugInfo4CCU(m_i4SensorDev, tempCamInfo.rCCU_Result.LTM.LTM_EN ,
                                                                                tempCamInfo.rCCU_Result.LTM.P1_LTM_Reg[0],
                                                                                tempCamInfo.rCCU_Result.LTM.P1_LTM_Reg[1],
                                                                                tempCamInfo.rCCU_Result.LTM.P1_LTMS_Reg,
                                                                                rDbgIspInfo);
        }
    }
    // Need to over-write CCU data to ISP EXIF
    // Update to resultPool buffer
    const CCUResultInfo_T* pCCUResult = (CCUResultInfo_T*)m_pResultPoolObj->getResult(frmId, E_CCU_RESULTINFO4OVERWRITE, __FUNCTION__);
    if(pCCUResult)
    {
        // protect vector before use vector
        if(bP1_EXIF_Valid)
        {
            // debug exif
            AAA_DEBUG_INFO2_T& rDbg3AInfo2 = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(pAllResult->vecDbgIspInfo.editArray());
            NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo = rDbg3AInfo2.rISPDebugInfo;
            // P1 ISP
            IspTuningMgr::getInstance().setDGNDebugInfo4CCU(m_i4SensorDev, pCCUResult->u4Rto, pCCUResult->u4P1DGNGain, rDbgIspInfo);
        }

        // Need to over-write CCU AEInfo data to CamInfo
        // Update to resultPool buffer
        const AEResultInfo_T  *pAEResultInfo = (AEResultInfo_T*)m_pResultPoolObj->getResult(frmId, E_AE_RESULTINFO, __FUNCTION__);
        if(pAEResultInfo)
        {
            ISPResultToMeta_T     rISPResult;
            if (pISPResult)
                rISPResult.rCamInfo = pISPResult->rCamInfo;
            ::memcpy(&rISPResult.rCamInfo.rAEInfo, &pAEResultInfo->AEPerframeInfo.rAEISPInfo, sizeof(AE_ISP_INFO_T));
            m_pResultPoolObj->updateResult(LOG_TAG, frmId, E_ISP_RESULTTOMETA, &rISPResult);
        }
        else
            MY_LOGE("[%s] R(%d) pAEResultInfo is NULL", __FUNCTION__, frmId);
    }

    if(bP1_EXIF_Valid){
        // debug info
        IMetadata::Memory dbgIspP1;
        dbgIspP1.appendVector(pAllResult->vecDbgIspInfo);
        UPDATE_ENTRY_SINGLE(metaExif, MTK_3A_EXIF_DBGINFO_ISP_DATA, dbgIspP1);
        UPDATE_ENTRY_SINGLE(result.halMeta, MTK_3A_EXIF_METADATA, metaExif);
    }

    CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_GET_P1, "[%s] - pAllResult:%p", __FUNCTION__, pAllResult);
    AAA_TRACE_END_HAL;
    AAA_TRACE_END_D;
    return i4Ret;
}

MINT32
HalIspImp::
getCur(MUINT32 frmId, MetaSet_T& result)
{
    AAA_TRACE_HAL(AdapterGetCur);
    CAM_LOGD("[%s] sensorDev(%d), sensorIdx(%d) R(%d)", __FUNCTION__, m_i4SensorDev, m_i4SensorIdx, frmId);
    MINT32 i4Ret = 0;
    AllResult_T *pAllResult = NULL;
    MINT32 i4Validate= 0;

    /*****************************
     *     get 2A CurResult from ResultPool
     *****************************/
    // ResultPool - 1. get result (dynamic) of x from 3AMgr
    i4Ret = getResultCur(frmId);//req/stt:5/2
    // ResultPool - 2. Use sttMagic to get ResultCur
    if(1 == i4Ret)
    {
        CAM_LOGD("[%s] get R[%d]", __FUNCTION__, frmId);
        pAllResult = m_pResultPoolObj->getAllResultCur(frmId);
        i4Validate = m_pResultPoolObj->isValidateCur(frmId, E_ISP_RESULTTOMETA);
    }

    /*****************************
     *     get special result
     *****************************/
    if(-1 == i4Ret)
    {
        // ResultPool - Get History
        MINT32 rHistoryReqMagic[HistorySize] = {0,0,0};
        m_pResultPoolObj->getHistory(rHistoryReqMagic);
        CAM_LOGW("[%s] History (Req0, Req1, Req2) = (#%d, #%d, #%d), Fail to get R[%d], current result will be obtained(%d).", __FUNCTION__, rHistoryReqMagic[0], rHistoryReqMagic[1], rHistoryReqMagic[2], frmId, rHistoryReqMagic[2]);
        pAllResult = m_pResultPoolObj->getAllResultLastCur(rHistoryReqMagic[2]);//get the last request EX: req/stt:4/1
        i4Validate = m_pResultPoolObj->isValidate(rHistoryReqMagic[2], E_ISP_RESULTTOMETA);
    }
    if(-2 == i4Ret)
    {
        CAM_LOGW("[%s] Fail to get R[%d], result will be obtained.", __FUNCTION__, frmId);
        pAllResult = m_pResultPoolObj->getAllResult(frmId);//req/stt:2/X
        i4Validate = m_pResultPoolObj->isValidate(frmId, E_ISP_RESULTTOMETA);
    }

    /*****************************
     *     convert result to metadata and update metedata to MW
     *****************************/

    const ISPResultToMeta_T* pISPResult = NULL;
    if(pAllResult->ModuleResultAddr[E_ISP_RESULTTOMETA]->isValidate())
        pISPResult = ( (ISPResultToMeta_T*)(pAllResult->ModuleResultAddr[E_ISP_RESULTTOMETA]->read()) );
    const LCSOResultToMeta_T* pLCSOResult = (LCSOResultToMeta_T*)m_pResultPoolObj->getResult(frmId, E_LCSO_RESULTTOMETA, __FUNCTION__);

    if(i4Validate == MTRUE && pISPResult != NULL)
    {
        NSIspTuning::RAWIspCamInfo tempCamInfo;

        tempCamInfo = pISPResult->rCamInfo;

        if(pLCSOResult){
            tempCamInfo.rLCS_Info = pLCSOResult->rLcsOutInfo;
        }
        else{
            CAM_LOGE("No LCS in Result Pool, FrmID: %d", frmId);
        }

        UPDATE_MEMORY(result.halMeta, MTK_ISP_ATMS_MAPPING_INFO, tempCamInfo.rMapping_Info);

        UPDATE_MEMORY(result.halMeta, MTK_PROCESSOR_CAMINFO, tempCamInfo);

        UPDATE_ENTRY_SINGLE(result.halMeta, MTK_FEATURE_FACE_APPLIED_GAMMA, tempCamInfo.rFdInfo.FaceGGM_Idx);

        // color correction matrix
        if (pAllResult->vecColorCorrectMat.size())
        {
            const MFLOAT* pfMat = &(pAllResult->vecColorCorrectMat[0]);
            IMetadata::IEntry entry(MTK_COLOR_CORRECTION_TRANSFORM);
            for (MINT32 k = 0; k < 9; k++)
            {
                MRational rMat;
                MFLOAT fVal = *pfMat++;
                rMat.numerator = fVal*512;
                rMat.denominator = 512;
                entry.push_back(rMat, Type2Type<MRational>());
                CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_GET_P1, "[%s] Mat[%d] = (%3.6f, %d)", __FUNCTION__, k, fVal, rMat.numerator);
            }
            result.appMeta.update(MTK_COLOR_CORRECTION_TRANSFORM, entry);
        }
    }

#if 0
    if(pISPResult != NULL)
    {
        //update CCU info to p1caminfo
        IspTuningMgr::getInstance().setCCUInfo2ISP(m_i4SensorDev, pISPResult->rCamInfo.rCCU_Result);
        CAM_LOGD("update ccu valid %d, value %d",
            pISPResult->rCamInfo.rCCU_Result.LTM.LTM_Valid,
            pISPResult->rCamInfo.rCCU_Result.LTM.LTM_Curve.lut[150].val);
    }
#endif

    MBOOL bP1_EXIF_Valid = MFALSE;
    IMetadata metaExif;
    // protect vector before use vector
    std::lock_guard<std::mutex> Vec_lock(pAllResult->LockVecResult);
    if(pAllResult->vecExifInfo.size() && pAllResult->vecDbgIspInfo.size() > 0){

        QUERY_ENTRY_SINGLE(result.halMeta, MTK_3A_EXIF_METADATA, metaExif);

        bP1_EXIF_Valid = MTRUE;
    }

    if(bP1_EXIF_Valid){
        // debug info
        IMetadata::Memory dbgIspP1;
        dbgIspP1.appendVector(pAllResult->vecDbgIspInfo);
        UPDATE_ENTRY_SINGLE(metaExif, MTK_3A_EXIF_DBGINFO_ISP_DATA, dbgIspP1);
        UPDATE_ENTRY_SINGLE(result.halMeta, MTK_3A_EXIF_METADATA, metaExif);
    }

    CAM_LOGD("[%s] - Validate:%d", __FUNCTION__, i4Validate);

    AAA_TRACE_END_HAL;
    return 0;
}

MINT32
HalIspImp::
getResultCur(MINT32 i4FrmId)
{
    std::unique_lock<std::mutex> autoLock(m_rResultMtx);
    MINT32 i4ResultWaitCnt = 3;

    MINT32 i4Ret = 1;

    // ResultPool - get Current All Result with SttMagic
    // 1. Use SttMagic need to judge isValidate.
    // 2. If result validate, can get result address.

    AllResult_T *pAllResult = m_pResultPoolObj->getAllResultCur(i4FrmId);
    MINT32 i4Validate = 0;
    i4Validate = m_pResultPoolObj->isValidateCur(i4FrmId, E_ISP_RESULTTOMETA);

    if(i4Validate == 0) //isp result not ready, block p1 setisp result
    {
        // ResultPool - Wait to get Current All Result with SttMagic
        while (i4ResultWaitCnt)
        {
            CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_GET_P1, "[%s] wait result #(%d) i4ResultWaitCnt(%d)", __FUNCTION__, i4FrmId, i4ResultWaitCnt);
            m_rResultCond.wait_for(autoLock, std::chrono::nanoseconds(1000000000));
            CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_GET_P1, "[%s] wait result done #(%d), i4ResultWaitCnt(%d)", __FUNCTION__, i4FrmId, i4ResultWaitCnt);
            i4ResultWaitCnt--;

            pAllResult = m_pResultPoolObj->getAllResultCur(i4FrmId);

            i4Validate = m_pResultPoolObj->isValidateCur(i4FrmId, E_ISP_RESULTTOMETA);
            if (i4Validate)
               break;
        }

        // ResultPool - CaptureStart ReqMagic is 2 3 4 5.., always update Result. If fail to get Current All Result, get last current All Result with ReqMagic 4.
        if(NULL == pAllResult)
        {
            // ResultPool - Get History
            MINT32 rHistoryReqMagic[HistorySize] = {0,0,0};
            m_pResultPoolObj->getHistory(rHistoryReqMagic);

            CAM_LOGW("[%s] History (Req0, Req1, Req2) = (#%d, #%d, #%d), Fail to get ResultCur with ReqMagic(%d), try getting ResultLastCur(%d)", __FUNCTION__,
                        rHistoryReqMagic[0], rHistoryReqMagic[1], rHistoryReqMagic[2], i4FrmId, rHistoryReqMagic[2]);
            i4Ret = -1;

            pAllResult = m_pResultPoolObj->getAllResultLastCur(rHistoryReqMagic[2]);//get req/stt:4/1

            // ResultPool - If fail to get last current All Result with last ReqMagic 4, get All Result with ReqMagic 2.
            if(pAllResult == NULL)
            {
                CAM_LOGW("Fail to get ResultLastCur, then get Result with ReqMagic(%d)", i4FrmId);
                pAllResult = m_pResultPoolObj->getAllResult(i4FrmId);//get req/stt:2/xx
                i4Ret = -2;
                if(pAllResult)
                    CAM_LOGW("[%s] (Req, Req, Stt) = (#%d, #%d, #%d)", __FUNCTION__, i4FrmId, pAllResult->rResultCfg.i4ReqMagic, pAllResult->rResultCfg.i4StatisticMagic);
                else
                    CAM_LOGE("[%s] Ret(%d) pISPResult is NULL", __FUNCTION__, i4Ret);
            }
            else
                CAM_LOGW("[%s] (Req, Req, Stt) = (#%d, #%d, #%d)", __FUNCTION__, i4FrmId, pAllResult->rResultCfg.i4ReqMagic, pAllResult->rResultCfg.i4StatisticMagic);
        }

        // ResultPool - std exif should be use capture start
        AllResult_T *pAllResultAtStart = m_pResultPoolObj->getAllResult(i4FrmId);

        if(NULL != pAllResultAtStart && pAllResult != NULL)
        {
            // protect vector before use vector
            std::lock_guard<std::mutex> Vec_lock(pAllResult->LockVecResult);

            CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_GET_P1, "[%s] (Req, ResultAtStartReq, ResultReq, Stt) = (#%d, #%d, #%d, #%d)", __FUNCTION__, i4FrmId, pAllResultAtStart->rResultCfg.i4ReqMagic, pAllResult->rResultCfg.i4ReqMagic, pAllResult->rResultCfg.i4StatisticMagic);

            MBOOL isIspStartEmpty = pAllResultAtStart->vecDbgIspInfo.empty();
            MBOOL isIspCurEmpty = pAllResult->vecDbgIspInfo.empty();

            if(!isIspStartEmpty && !isIspCurEmpty)
            {
                // get capture start AE setting to update EXIF info
                AAA_DEBUG_INFO2_T& rDbgISPInfoStart = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(pAllResultAtStart->vecDbgIspInfo.editArray());
                AAA_DEBUG_INFO2_T& rDbgISPInfoCur = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(pAllResult->vecDbgIspInfo.editArray());
                rDbgISPInfoCur.rISPDebugInfo = rDbgISPInfoStart.rISPDebugInfo;
            } else
            {
                CAM_LOGE("isIspStartEmpty(%d) isIspCurEmpty(%d)", isIspStartEmpty, isIspCurEmpty);
            }
        }
        else
        {
            CAM_LOGE("Fail get pResultAtStart (#%d) pAllResultAtStart/pAllResult:%p/%p", i4FrmId, pAllResultAtStart, pAllResult);
        }

    }
    else
    {
        CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_GET_P1, "[%s] got result (#%d)", __FUNCTION__, i4FrmId);
    }

    return i4Ret;
}

MINT32
HalIspImp::
getCurrResult(MUINT32 i4FrmId)
{
    // ResultPool - To update Vector info
    AllResult_T *pAllResult = m_pResultPoolObj->getAllResult(i4FrmId);
    if(pAllResult == NULL)
    {
        CAM_LOGE("[%s] pAllResult is NULL", __FUNCTION__);
        return MFALSE;
    }


    /*****************************
     *     Get ISP Result
     *****************************/
    std::lock_guard<std::mutex> autoLock(m_rResultMtx);
    if (m_u1ColorCorrectMode != MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX)
    {
        std::lock_guard<std::mutex> Vec_lock(pAllResult->LockVecResult);
        pAllResult->vecColorCorrectMat.resize(9);
        MFLOAT* pfColorCorrectMat = &(pAllResult->vecColorCorrectMat[0]);
        IspTuningMgr::getInstance().getColorCorrectionTransform(m_i4SensorDev,
        pfColorCorrectMat[0], pfColorCorrectMat[1], pfColorCorrectMat[2],
        pfColorCorrectMat[3], pfColorCorrectMat[4], pfColorCorrectMat[5],
        pfColorCorrectMat[6], pfColorCorrectMat[7], pfColorCorrectMat[8]
        );
    }

    ISPResultToMeta_T     rISPResult;
    MBOOL bRet = IspTuningMgr::getInstance().getCamInfo(m_i4SensorDev, rISPResult.rCamInfo);

    if (!bRet)
    {
        CAM_LOGE("Fail to get CamInfo");
    }else{
        LastInfo_T vLastInfo;
        // Backup caminfo
        ::memcpy(&vLastInfo.mBackupCamInfo, &rISPResult.rCamInfo, sizeof(NSIspTuning::RAWIspCamInfo));
        vLastInfo.mBackupCamInfo_copied = MTRUE;
        m_pResultPoolObj->updateLastInfo(vLastInfo);
        CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_GET_P1, "[%s] Backup caminfo,copied(%d)/mode(%d)/profile(%d)",__FUNCTION__, vLastInfo.mBackupCamInfo_copied,
            vLastInfo.mBackupCamInfo.rMapping_Info.eSensorMode,
            vLastInfo.mBackupCamInfo.rMapping_Info.eIspProfile);
    }
    IdxMgr::createInstance(static_cast<NSIspTuning::ESensorDev_T>(m_i4SensorDev))->setMappingInfo(static_cast<NSIspTuning::ESensorDev_T>(m_i4SensorDev), rISPResult.rCamInfo.rMapping_Info, i4FrmId);

    m_pResultPoolObj->updateResult(LOG_TAG, i4FrmId, E_ISP_RESULTTOMETA, &rISPResult);

    /*****************************
     *     Get ISP Exif Result
     *****************************/
    if (m_u1IsGetExif || m_bIsCapEnd)
    {
        // protect vector before use vector
        std::lock_guard<std::mutex> Vec_lock(pAllResult->LockVecResult);

        if(pAllResult->vecDbgIspInfo.size()==0)
            pAllResult->vecDbgIspInfo.resize(sizeof(AAA_DEBUG_INFO2_T));

        AAA_DEBUG_INFO2_T& rDbgIspInfo = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(pAllResult->vecDbgIspInfo.editArray());

        if(pAllResult->vecDbgIspInfo.size() != 0)
        {
            CAM_LOGD_IF(m_u4LogEn & HALISP_LOG_GET_P1, "[%s] vecDbgIspInfo - Size(%d) Addr(%p)", __FUNCTION__, (MINT32)pAllResult->vecDbgIspInfo.size(), &rDbgIspInfo);
            IspTuningMgr::getInstance().getDebugInfoP1(m_i4SensorDev, rDbgIspInfo.rISPDebugInfo, MFALSE);
        }
        else
            CAM_LOGE(, "[%s] vecDbgIspInfo - Size(%d) Addr(%p)", __FUNCTION__, (MINT32)pAllResult->vecDbgIspInfo.size(), &rDbgIspInfo);
    }

    /*****************************
     *     condition_variable notify_all()
     *****************************/
    m_rResultCond.notify_all();

    return MTRUE;
}

MVOID
HalIspImp::
resume(MINT32 MagicNum)
{
    // apply 3A module's config
    if (MagicNum > 0)
    {
        RequestSet_T rRequestSet;
        rRequestSet.vNumberSet.clear();
        rRequestSet.vNumberSet.push_back(MagicNum);
        m_pTuning->notifyRPGEnable(m_i4SensorDev, MTRUE);   // apply awb gain for init stat
        m_pTuning->validatePerFrameP1(m_i4SensorDev, rRequestSet);
    }
}

MINT32
HalIspImp::
getCurrLCSResult(ISP_LCS_OUT_INFO_T const &rLcsOutInfo)
{
    LCSOResultToMeta_T     rLCSResult;

    rLCSResult.rLcsOutInfo = rLcsOutInfo;

    m_pResultPoolObj->updateResult(LOG_TAG, rLCSResult.rLcsOutInfo.i4FrmId, E_LCSO_RESULTTOMETA, &rLCSResult);

    return MTRUE;
}


MINT32
HalIspImp::
InitP1Cb()
{
    CAM_LOGD_IF(1, "[%s] ++", __FUNCTION__);
    std::lock_guard<std::mutex> lock(m_Lock);

    //Driver P1 Callback for CCU
    mpHalISPP1DrvCbHub = new HalISPP1DrvCbHub(m_i4SensorDev, this);


    CAM_LOGD_IF(1, "[%s] --", __FUNCTION__);
    return MFALSE;
}


MINT32
HalIspImp::
UninitP1Cb()
{
    CAM_LOGD_IF(1, "[%s] ++", __FUNCTION__);
    std::lock_guard<std::mutex> lock(m_Lock);

    if(mpHalISPP1DrvCbHub != NULL)
    {
        delete mpHalISPP1DrvCbHub;
        mpHalISPP1DrvCbHub = NULL;
    }

    CAM_LOGD_IF(1, "[%s] --", __FUNCTION__);
    return MFALSE;
}


MINT32
HalIspImp::
ConfigP1Cb()
{
    CAM_LOGD_IF(1, "[%s] ++", __FUNCTION__);

    //Driver P1 Callback for CCU
    m_pCamIO->sendCommand(ENPipeCmd_SET_CQSTART_CBFP,
                         (MINTPTR)mpHalISPP1DrvCbHub->getCallbackAddr(EP1_CQ_DONE),
                         -1,-1);
    //Driver P1 Callback for W+T Switch
    m_pCamIO->sendCommand(ENPipeCmd_SET_WT_TUNING_CBFP,
                         (MINTPTR)mpHalISPP1DrvCbHub->getCallbackAddr(EW_T_SWITCH_TUNING),
                         -1,-1);
    //Driver P1 Callback for LCES
    m_pCamIO->sendCommand(ENPipeCmd_SET_LCS_CBFP,
                         (MINTPTR)mpHalISPP1DrvCbHub->getCallbackAddr(ELCES_TUNING_SET),
                         -1,-1);

    return MFALSE;
}

MBOOL
HalIspImp::
queryISPBufferInfo(Buffer_Info& bufferInfo)
{
    return m_pTuning->queryISPBufferInfo(m_i4SensorDev,bufferInfo);
}


