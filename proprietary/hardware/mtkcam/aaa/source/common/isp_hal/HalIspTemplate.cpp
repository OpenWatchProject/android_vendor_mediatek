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
#define LOG_TAG "HalIspTemplate"

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>

//For Metadata
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include "aaa_utils.h"


#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include <property_utils.h>

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

#include "HalIspTemplate.h"

using namespace NS3Av3;
using namespace NSCam::TuningUtils;


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

#define HALISP_LOG_SET_P1       (1<<0)
#define HALISP_LOG_SET_P2       (2<<0)
#define HALISP_LOG_SETPARAM_P2  (3<<0)
#define HALISP_LOG_GET_P1       (4<<0)
#define HALISP_LOG_GET_P2       (5<<0)
#define HALISP_LOG_CONVERT_P2   (6<<0)

MINT32 HalIspTemplate::m_i4LogEn = 0;

static void
_dumpAAO(const char* filename, MUINT32 SensorDev, MINT32 MagicNumberRequest)
{
    FILE *fptr;

    std::shared_ptr<ISttBufQ> pSttBufQ;
    std::shared_ptr<ISttBufQ::DATA> pData;
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

    fptr = std::fopen(filename, "w");
    if (fptr == NULL) {

        CAM_LOGW("open file(%s) error!!", filename);
        goto lbExit;
    }

    N = pData->AAO.size();
    if (std::fwrite(pData->AAO.data(), sizeof(char), N, fptr) != N) {

        CAM_LOGW("file(%s) write error!!", filename);
        std::fclose(fptr);
        goto lbExit;
    }
    std::fclose(fptr);

lbExit:
    if(pData.get()) {

        pSttBufQ->enque_first(pData);
    }
}

static MBOOL _isFileExist(const char* file)
{
  struct stat buffer;
  return (stat (file, &buffer) == 0);
}

HalIspTemplate::
HalIspTemplate(MINT32 const i4SensorIdx)
    : m_pCamIO(NULL)
    , m_i4SensorIdx(i4SensorIdx)
    , m_i4SensorDev(0)
    , m_i4SensorMode(0)
    , m_i4TgWidth(1000)
    , m_i4TgHeight(1000)
    , m_Users(0)
    , m_Lock()
    , m_P2Mtx()
    , m_pResultPoolWrapper(NULL)
    , m_rResultMtx()
    , m_i4Magic(-1)
    , m_i4SubsampleCount(1)
    , m_u1ColorCorrectMode(MTK_COLOR_CORRECTION_MODE_FAST)
    , m_u1IsGetExif(0)
    , m_bIsCapEnd(0)
    , m_P1CamInfo()
{
    // query SensorDev from HalSensorList
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList) {

        CAM_LOGE("[%s] MAKE HalSensorList fail", __FUNCTION__);
    } else {

        m_i4SensorDev = pHalSensorList->querySensorDevIdx(i4SensorIdx);
    }

    CAM_LOGD("[%s] sensorIdx(0x%04x) sensorDev(%d)", __FUNCTION__, i4SensorIdx, m_i4SensorDev);

    m_pResultPoolWrapper = ResultPoolImpWrapper::getInstance(m_i4SensorDev);
    if(m_pResultPoolWrapper == NULL) {

        CAM_LOGE("ResultPool getInstance fail");
    }
}

MVOID
HalIspTemplate::
destroyInstance(const char* strUser)
{
    CAM_LOGD("[%s]+ sensorIdx(%d)  User(%s)", __FUNCTION__, m_i4SensorIdx, strUser);
    uninit(strUser);
    CAM_LOGD("[%s]- ", __FUNCTION__);
}

MINT32
HalIspTemplate::
config(const ConfigInfo_T& rConfigInfo)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    CAM_LOGD("[%s] config i4SubsampleCount(%d)", __FUNCTION__, rConfigInfo.i4SubsampleCount);

    if (m_i4SubsampleCount != rConfigInfo.i4SubsampleCount) {

        CAM_LOGD("[%s] m_i4SubsampleCount(%d, %d)", __FUNCTION__, m_i4SubsampleCount, rConfigInfo.i4SubsampleCount);
        m_i4SubsampleCount = rConfigInfo.i4SubsampleCount;

        configTuning(rConfigInfo.i4SubsampleCount);
    }


    // NormalIOPipe create instance
    if (m_pCamIO == NULL) {

        m_pCamIO = (INormalPipe*)INormalPipeUtils::get()->createDefaultNormalPipe(m_i4SensorIdx, LOG_TAG);
        if (m_pCamIO == NULL) {

            CAM_LOGE("Fail to create NormalPipe");
            return MFALSE;
        }
    }

    updateTGInfo(); // Get TG Width and Height

    P1Param_T rNewConfigParam;

    // Get Bin info
    m_pCamIO->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_BIN_INFO,
                        (MINTPTR)&rNewConfigParam.u4ISPRawWidth, (MINTPTR)&rNewConfigParam.u4ISPRawHeight, 0);
    // Get RawAB/C Info
    m_pCamIO->sendCommand(NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_CUR_RAW,
                        (MINTPTR)&rNewConfigParam.ERawPath, 0, 0);

    // P1 YUV Port Info
    rNewConfigParam.u4P1DirectYUV_Port = rConfigInfo.u4P1DirectYUV_Port;

    // AE, AWB info
    AllResult_T *pAllResult = m_pResultPoolWrapper->getResultByReqNum(ConfigMagic, __FUNCTION__);
    rNewConfigParam.pAEResultInfo  = pAllResult->ModuleResult[E_AE_RESULTINFO]->read();
    rNewConfigParam.pAWBResultInfo = pAllResult->ModuleResult[E_AWB_RESULTINFO4ISP]->read();

    // Set Config to ISP

    setConfigParams(rNewConfigParam, m_P1CamInfo);

    m_pResultPoolWrapper->returnResult(pAllResult, __FUNCTION__);

    RequestSet_T rRequestSet;
    rRequestSet.vNumberSet.clear();
    for (MINT32 i4Num = 0; i4Num < m_i4SubsampleCount; i4Num++) {

        rRequestSet.vNumberSet.push_back(ConfigMagic);
    }

    validateP1(rRequestSet, m_P1CamInfo);

    CAM_LOGD("[%s]- After Bin Raw Size(%dx%d)", __FUNCTION__, rNewConfigParam.u4ISPRawWidth, rNewConfigParam.u4ISPRawHeight);
    return MTRUE;
}


MVOID
HalIspTemplate::
WT_Reconfig(MVOID* pInput)
{
//Chooo WT
#if 0
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

    const AEResultInfo_T  *pAEResultInfo = (AEResultInfo_T*)m_pResultPoolWrapper->getResult(u4MagicNum, E_AE_RESULTINFO, __FUNCTION__);
    if(pAEResultInfo){
        CAM_LOGD_IF(1, "[%s] u4DGNGain(%d)", __FUNCTION__, pAEResultInfo->AEPerframeInfo.rAEISPInfo.u4P1DGNGain);
        m_pTuning->getInstance().setAEInfo2ISP(m_i4SensorDev, pAEResultInfo->AEPerframeInfo.rAEISPInfo);
    }
    else{
        CAM_LOGD("[%s] pAEResultInfo(%p)", __FUNCTION__, pAEResultInfo);
    }

    const AWBResultInfo_T *pAWBResultInfo = (AWBResultInfo_T*)m_pResultPoolWrapper->getResult(u4MagicNum, E_AWB_RESULTINFO4ISP, __FUNCTION__);
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

    validateP1(rRequestSet, MTRUE);
#endif
}

MVOID
HalIspTemplate::
setSensorMode(MINT32 i4SensorMode)
{
    CAM_LOGD("[%s] mode(%d)", __FUNCTION__, i4SensorMode);
    m_i4SensorMode = i4SensorMode;
}

MBOOL
HalIspTemplate::
init(const char* strUser)
{
    CAM_LOGD("[%s] m_Users: %d, SensorDev %d, index %d \n", __FUNCTION__, std::atomic_load((&m_Users)), m_i4SensorDev, m_i4SensorIdx);

    // check user count
    std::lock_guard<std::mutex> lock(m_Lock);

    if (m_Users > 0) {

        CAM_LOGD("[%s] %d has created \n", __FUNCTION__, std::atomic_load((&m_Users)));
        MINT32 ret __unused = std::atomic_fetch_add((&m_Users), 1);
        return MTRUE;
    }

    initTuning();

    querySensorStaticInfo(m_P1CamInfo);

    setZoomRatio(m_P1CamInfo, 100);

    MINT32 ret __unused = std::atomic_fetch_add((&m_Users), 1);
    return MTRUE;
}

MBOOL
HalIspTemplate::
uninit(const char* strUser)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    // If no more users, return directly and do nothing.
    if (m_Users <= 0) {

        return MTRUE;
    }
    CAM_LOGD("[%s] m_Users: %d \n", __FUNCTION__, std::atomic_load((&m_Users)));

    // More than one user, so decrease one User.
    MINT32 ret __unused = std::atomic_fetch_sub((&m_Users), 1);

    // There is no more User after decrease one User
    if (m_Users == 0) {

        uninitTuning();

        UninitP1Cb();

        CAM_LOGD("[%s] done\n", __FUNCTION__);
    } else { // There are still some users.

        CAM_LOGD("[%s] Still %d users \n", __FUNCTION__, std::atomic_load((&m_Users)));
    }
    return MTRUE;
}

MBOOL
HalIspTemplate::
start()
{
    CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1, "[%s] +", __FUNCTION__);

    IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_NOTIFY_START, 0, 0);

    CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1, "[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
HalIspTemplate::
stop()
{
    CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1, "[%s] +", __FUNCTION__);

    IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_NOTIFY_STOP, 0, 0);
    // NormalIOPipe destroy instance
    if (m_pCamIO != NULL) {

        m_pCamIO->destroyInstance(LOG_TAG);
        m_pCamIO = NULL;
    }

    CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1, "[%s] -", __FUNCTION__);
    return MTRUE;
}

MVOID
HalIspTemplate::
resume(MINT32 MagicNum)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    // apply 3A module's config
    if (MagicNum > 0) {

        // apply awb gain for init stat
        notifyRPGEnable(m_P1CamInfo, MTRUE);

        RequestSet_T rRequestSet;
        rRequestSet.vNumberSet.clear();
        rRequestSet.vNumberSet.push_back(MagicNum);
        validateP1(rRequestSet, m_P1CamInfo);
    }
}


MBOOL
HalIspTemplate::
setP1Isp(const vector<MetaSet_T*>& requestQ, MBOOL const fgForce/*MINT32 const i4SensorDev, RequestSet_T const RequestSet, MetaSet_T& control, MBOOL const fgForce, MINT32 i4SubsampleIdex*/)
{
    // check user count
    std::lock_guard<std::mutex> lock(m_Lock);

    AAA_TRACE_D("setP1ISP");

    MetaSet_T* it = requestQ[0];
    MINT32 i4FrmId = it->MagicNum;
    if (i4FrmId <= 0) {

        QUERY_ENTRY_SINGLE(it->halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, i4FrmId);
    }
    MINT32 i4FrmId4SMVR = 0;
    m_i4Magic = i4FrmId;

    CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1, "[%s] + i4FrmId(%d) fgForce(%d)", __FUNCTION__, i4FrmId, fgForce);

    /*****************************
     *     Special flow - InitReq
     *     Warning : MW set one request only, unlike set three request to HAL3A
     *               So ISP_profile will wrong, this issue need to fix from MW
     *****************************/
    if(fgForce) {

        MetaSet_T* it = requestQ[0];
        RequestSet_T rRequestSet;
        rRequestSet.vNumberSet.clear();
        rRequestSet.vNumberSet.push_back(i4FrmId);

        setIspProfile(m_P1CamInfo, NSIspTuning::EIspProfile_Preview);
        validateP1(rRequestSet, m_P1CamInfo);
        getCurrResult(i4FrmId);
        AAA_TRACE_END_D;
        return MTRUE;
    }

    /*****************************
     *     Parse ISP Param
     *****************************/

    RequestSet_T rRequestSet;
    rRequestSet.vNumberSet.clear();
    P1Param_T rNewP1Param;
    const IMetadata& _appmeta = it->appMeta;
    const IMetadata& _halmeta = it->halMeta;
    MINT32 i4DisableP1=0;

    MSize targetSize(0,0);
    QUERY_ENTRY_SINGLE(_halmeta, MTK_3A_ISP_MDP_TARGET_SIZE, targetSize);
    QUERY_ENTRY_SINGLE(_halmeta, MTK_P1NODE_RAW_TYPE, rNewP1Param.bIMGO_RawType);
    QUERY_ENTRY_SINGLE(_halmeta, MTK_PIPELINE_REQUEST_NUMBER, rNewP1Param.i4RequestNumber);
    CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P2, "[%s] R(%d), MTK_3A_ISP_MDP_TARGET_SIZE(%d, %d)", __FUNCTION__, rNewP1Param.i4RequestNumber, targetSize.w, targetSize.h);

    for (MINT32 i = 0; i < requestQ.size(); i++) {

        MetaSet_T* it = requestQ[i];
        i4FrmId = it->MagicNum;
        if (i4FrmId <= 0)   QUERY_ENTRY_SINGLE(it->halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, i4FrmId);
        m_i4Magic = i4FrmId;

        CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1, "[%s] i4FrmId(%d)", __FUNCTION__, i4FrmId);
        rRequestSet.vNumberSet.push_back(std::max(0, i4FrmId));

        //re-init params
        rNewP1Param.u1CapIntent = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
        rNewP1Param.u1ColorCorrectMode = MTK_COLOR_CORRECTION_MODE_FAST;
        rNewP1Param.u1IspProfile = 0xFF;

        const IMetadata& _appmeta = it->appMeta;
        const IMetadata& _halmeta = it->halMeta;

        QUERY_ENTRY_SINGLE(_halmeta, MTK_3A_ISP_PROFILE, rNewP1Param.u1IspProfile);
        QUERY_ENTRY_SINGLE(_halmeta, MTK_HAL_REQUEST_PASS1_DISABLE, i4DisableP1); // Dual PDAF support for EngMode
        if (!QUERY_ENTRY_SINGLE(_halmeta, MTK_HAL_REQUEST_REQUIRE_EXIF, m_u1IsGetExif))
            m_u1IsGetExif = 0;

        for (MINT32 j = 0; j < _appmeta.count(); j++) {

            IMetadata::IEntry entry = _appmeta.entryAt(j);
            mtk_camera_metadata_tag_t tag = (mtk_camera_metadata_tag_t)entry.tag();

            // convert metadata tag into ISP settings.
            switch (tag) {

            case MTK_CONTROL_MODE:  // dynamic
                rNewP1Param.i1ControlMode = (MINT8)entry.itemAt(0, Type2Type< MUINT8 >());
                break;
            case MTK_CONTROL_CAPTURE_INTENT:
                rNewP1Param.u1CapIntent = entry.itemAt(0, Type2Type< MUINT8 >());
                break;
            // ISP
            case MTK_CONTROL_EFFECT_MODE:
                rNewP1Param.i1EffectMode = (MINT8)entry.itemAt(0, Type2Type< MUINT8 >());
                break;
            case MTK_CONTROL_SCENE_MODE:
                rNewP1Param.i1SceneMode = (MINT8)entry.itemAt(0, Type2Type< MUINT8 >());
                break;
            case MTK_EDGE_MODE:
                rNewP1Param.i1EdgeMode = (MINT8)entry.itemAt(0, Type2Type< MUINT8 >());
                break;
            case MTK_NOISE_REDUCTION_MODE:
                rNewP1Param.i1NRMode = (MINT8)entry.itemAt(0, Type2Type< MUINT8 >());
                break;
            // Color correction
            case MTK_COLOR_CORRECTION_MODE:
                m_u1ColorCorrectMode = entry.itemAt(0, Type2Type<MUINT8>());
                rNewP1Param.i1ColorCorrectMode = (MINT8)m_u1ColorCorrectMode;
                break;
            case MTK_COLOR_CORRECTION_TRANSFORM:
                rNewP1Param.fgColorCorrectModeIsSet = MTRUE;
                for (MINT32 k = 0; k < 9; k++) {

                    MRational rMat = entry.itemAt(k, Type2Type<MRational>());
                    rNewP1Param.fColorCorrectMat[k] = (0.0f != rMat.denominator) ? (MFLOAT)rMat.numerator / rMat.denominator : 0.0f;
                }
                break;
            }
        }
    }

    /*****************************
     *     Convert frame id to magic number
     *****************************/
    if(m_i4SubsampleCount>1) {

        AllResult_T *pAllResult = m_pResultPoolWrapper->getResultByReqNum(i4FrmId, __FUNCTION__);
        i4FrmId4SMVR = pAllResult->rOld3AInfo.i4ConvertMagic[0];
        m_pResultPoolWrapper->returnResult(pAllResult, __FUNCTION__);
    } else {

        i4FrmId4SMVR = i4FrmId;
    }

    /*****************************
     *     Set AWBInfo, AEInfo, and 3A HAL info to P1 Param
     *****************************/
    AllResult_T *pAllResult = m_pResultPoolWrapper->getResultByReqNum(i4FrmId4SMVR, __FUNCTION__);
    rNewP1Param.pAEResultInfo  = pAllResult->ModuleResult[E_AE_RESULTINFO]->read();
    rNewP1Param.pAWBResultInfo = pAllResult->ModuleResult[E_AWB_RESULTINFO4ISP]->read();
    rNewP1Param.pHALResult     = pAllResult->ModuleResult[E_HAL_RESULTTOMETA]->read(); //

    if(rNewP1Param.pHALResult) {

        rRequestSet.fgKeep = ((HALResultToMeta_T*)rNewP1Param.pHALResult)->fgKeep; // this logic is fixed to update Keep flag within paramctrl
        m_bIsCapEnd = ((HALResultToMeta_T*)rNewP1Param.pHALResult)->fgKeep;

        CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1, "[%s] pHALResult->i4ZoomRatio(%d), pHALResult->fgFdEnable(%d)", __FUNCTION__,
            ((HALResultToMeta_T*)rNewP1Param.pHALResult)->i4ZoomRatio,
            ((HALResultToMeta_T*)rNewP1Param.pHALResult)->fgFdEnable);
        rNewP1Param.fgFdEnable = ((HALResultToMeta_T*)rNewP1Param.pHALResult)->fgFdEnable;
        rNewP1Param.i4ZoomRatio = ((HALResultToMeta_T*)rNewP1Param.pHALResult)->i4ZoomRatio;
    }
    m_pResultPoolWrapper->returnResult(pAllResult, __FUNCTION__);
    CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1, "[%s] rRequestSet.fgKeep(%d)", __FUNCTION__, rRequestSet.fgKeep);

    /*****************************
     *     Set All P1 params to ISP
     *****************************/
    setP1Params(rNewP1Param, m_P1CamInfo);

    /*****************************
     *     ISP Validate
     *****************************/
    AAA_TRACE_D("P1_VLD");
    AAA_TRACE_ISP(P1_VLD);

    rRequestSet.fgDisableP1 = i4DisableP1;

    if (rRequestSet.vNumberSet[0] > 0) {

        CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1, "[%s] validateP1 rRequestSet.vNumberSet[0](%d)", __FUNCTION__, rRequestSet.vNumberSet[0]);
        validateP1(rRequestSet, m_P1CamInfo);
    }

    AAA_TRACE_END_ISP;
    AAA_TRACE_END_D;

    /*****************************
     *     Get ISP Result
     *****************************/
    std::vector<MINT32> rNumberSet = rRequestSet.vNumberSet;
    std::vector<MINT32>::iterator it4Magic;
    MINT32 i4MagicNum = 0;
    for (it4Magic = rNumberSet.begin(); it4Magic != rNumberSet.end(); it4Magic++) {

        i4MagicNum = (*it4Magic);
        getCurrResult(i4MagicNum);
    }
    CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1, "[%s] - i4FrmId(%d) i4FrmId4SMVR(%d)", __FUNCTION__, i4FrmId, i4FrmId4SMVR);

    AAA_TRACE_END_D;
    return MTRUE;
}

MINT32
HalIspTemplate::
get(MUINT32 frmId, MetaSet_T& result)
{
    AAA_TRACE_D("HalISPGet");
    AAA_TRACE_HAL(HalISPGet);
    CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_GET_P1, "[%s] sensorDev(%d), sensorIdx(%d) R(%d)", __FUNCTION__, m_i4SensorDev, m_i4SensorIdx, frmId);

    /*****************************
     *     To get ISP Result Pointer
     *****************************/
    MINT32 i4Ret = MTRUE;
    // Vector pointer
    AllResult_T *pAllResult = m_pResultPoolWrapper->getResultByReqNum(frmId, __FUNCTION__);
    if (pAllResult == NULL) {
        m_pResultPoolWrapper->returnResult(pAllResult, __FUNCTION__);
        AAA_TRACE_END_HAL;
        AAA_TRACE_END_D;
        return MFALSE;
    }

    P1GetParam_T rNewP1GetParam;
    NSIspTuning::RAWIspCamInfo tempCamInfo;

    // Get result pointer if validateP1 true. Otherwise false
    rNewP1GetParam.pISPResult =  pAllResult->ModuleResult[E_ISP_RESULTTOMETA]->read(); //(ISPResultToMeta_T*)
    rNewP1GetParam.pLCSOResult = pAllResult->ModuleResult[E_LCSO_RESULTTOMETA]->read(); //(LCSOResultToMeta_T*)
    rNewP1GetParam.pCCUResult =  pAllResult->ModuleResult[E_CCU_RESULTINFO4OVERWRITE]->read(); //(CCUResultInfo_T*)

    if(rNewP1GetParam.pISPResult == NULL) {

        MY_LOGW("[%s] Not find result to conver metadata(#%d)", __FUNCTION__, frmId);
        AAA_TRACE_END_HAL;
        AAA_TRACE_END_D;
        //Special flow - InitReq, so not to return -1
        //return (-1);
    }

    //set frame id for debug purpose
    rNewP1GetParam.frmId;

    //set write back params to isp mgr and caminfo
    setP1GetParams(rNewP1GetParam, tempCamInfo, m_P1CamInfo);

    //ISP part
    if(rNewP1GetParam.pISPResult) {

        UPDATE_MEMORY(result.halMeta, MTK_PROCESSOR_CAMINFO, tempCamInfo);
        UPDATE_ENTRY_SINGLE(result.halMeta, MTK_FEATURE_FACE_APPLIED_GAMMA, tempCamInfo.rFdInfo.FaceGGM_Idx);

        // color correction matrix
        if (pAllResult->vecColorCorrectMat.size()) {

            const MFLOAT* pfMat = &(pAllResult->vecColorCorrectMat[0]);
            IMetadata::IEntry entry(MTK_COLOR_CORRECTION_TRANSFORM);
            for (MINT32 k = 0; k < 9; k++) {

                MRational rMat;
                MFLOAT fVal = *pfMat++;
                rMat.numerator = fVal*512;
                rMat.denominator = 512;
                entry.push_back(rMat, Type2Type<MRational>());
                CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_GET_P1, "[%s] Mat[%d] = (%3.6f, %d)", __FUNCTION__, k, fVal, rMat.numerator);
            }
            result.appMeta.update(MTK_COLOR_CORRECTION_TRANSFORM, entry);
        }

    }

    // Update to resultPool buffer
    if(rNewP1GetParam.pCCUResult) {

        // Need to over-write CCU AEInfo data to CamInfo
        // Update to resultPool buffer
        const AEResultInfo_T  *pAEResultInfo = (AEResultInfo_T*)(pAllResult->ModuleResult[E_AE_RESULTINFO]->read());
        if(pAEResultInfo) {

            ISPResultToMeta_T     rISPResult;
            if (rNewP1GetParam.pISPResult) {

                rISPResult.rCamInfo = ((ISPResultToMeta_T*)rNewP1GetParam.pISPResult)->rCamInfo;
            }
            ::memcpy(&rISPResult.rCamInfo.rAEInfo, &pAEResultInfo->AEPerframeInfo.rAEISPInfo, sizeof(AE_ISP_INFO_T));
            pAllResult->ModuleResult[E_ISP_RESULTTOMETA]->write(&rISPResult);
        } else {

            MY_LOGE("[%s] R(%d) pAEResultInfo is NULL", __FUNCTION__, frmId);
        }
    }
    m_pResultPoolWrapper->returnResult(pAllResult, __FUNCTION__);

    /*****************************
     *     Convert ISP Result to Metadata
     *****************************/
    if(pAllResult->vecExifInfo.size() && pAllResult->vecDbgIspInfo.size() > 0) {

        //query exif data
        IMetadata metaExif;
        QUERY_ENTRY_SINGLE(result.halMeta, MTK_3A_EXIF_METADATA, metaExif);

        //setup exif input
        rNewP1GetParam.pDbg3AInfo2 = pAllResult->vecDbgIspInfo.editArray();

        // Need to over-write CCU data to ISP EXIF
        setCCUFeedbackExif(rNewP1GetParam, tempCamInfo);

        // debug info
        IMetadata::Memory dbgIspP1;
        dbgIspP1.appendVector(pAllResult->vecDbgIspInfo);
        UPDATE_ENTRY_SINGLE(metaExif, MTK_3A_EXIF_DBGINFO_ISP_DATA, dbgIspP1);
        UPDATE_ENTRY_SINGLE(result.halMeta, MTK_3A_EXIF_METADATA, metaExif);
    }

    CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_GET_P1, "[%s] - pAllResult:%p", __FUNCTION__, pAllResult);
    AAA_TRACE_END_HAL;
    AAA_TRACE_END_D;
    return i4Ret;
}

MINT32
HalIspTemplate::
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
    if(1 == i4Ret) {

        i4Validate = m_pResultPoolWrapper->getAllHQCResultWithType(frmId, E_ISP_RESULTTOMETA, &pAllResult, __FUNCTION__);
    }

    /*****************************
     *     get special result
     *****************************/
    if(-1 == i4Ret) {

        i4Validate = m_pResultPoolWrapper->getOldestAllResultWithType(E_ISP_RESULTTOMETA, &pAllResult, __FUNCTION__);
    }
    if(-2 == i4Ret) {

        i4Validate = m_pResultPoolWrapper->getAllResultWithType(frmId, E_ISP_RESULTTOMETA, &pAllResult, __FUNCTION__);
    }

    /*****************************
     *     convert result to metadata and update metedata to MW
     *****************************/

    const ISPResultToMeta_T* pISPResult = NULL;
    if(pAllResult->ModuleResult[E_ISP_RESULTTOMETA]->isValidate()) {

        pISPResult = ( (ISPResultToMeta_T*)(pAllResult->ModuleResult[E_ISP_RESULTTOMETA]->read()) );
    }

    const LCSOResultToMeta_T* pLCSOResult = (LCSOResultToMeta_T*)(pAllResult->ModuleResult[E_LCSO_RESULTTOMETA]->read());
    if(i4Validate == MTRUE && pISPResult != NULL) {

        NSIspTuning::RAWIspCamInfo tempCamInfo = pISPResult->rCamInfo;

        if(pLCSOResult) {

            tempCamInfo.rLCS_Info = pLCSOResult->rLcsOutInfo;
        } else{

            CAM_LOGE("No LCS in Result Pool, FrmID: %d", frmId);
        }

        UPDATE_MEMORY(result.halMeta, MTK_PROCESSOR_CAMINFO, tempCamInfo);
        UPDATE_ENTRY_SINGLE(result.halMeta, MTK_FEATURE_FACE_APPLIED_GAMMA, tempCamInfo.rFdInfo.FaceGGM_Idx);

        // color correction matrix
        if (pAllResult->vecColorCorrectMat.size()) {

            const MFLOAT* pfMat = &(pAllResult->vecColorCorrectMat[0]);
            IMetadata::IEntry entry(MTK_COLOR_CORRECTION_TRANSFORM);

            for (MINT32 k = 0; k < 9; k++) {

                MRational rMat;
                MFLOAT fVal = *pfMat++;
                rMat.numerator = fVal*512;
                rMat.denominator = 512;
                entry.push_back(rMat, Type2Type<MRational>());
                CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_GET_P1, "[%s] Mat[%d] = (%3.6f, %d)", __FUNCTION__, k, fVal, rMat.numerator);
            }
            result.appMeta.update(MTK_COLOR_CORRECTION_TRANSFORM, entry);
        }
    }
    m_pResultPoolWrapper->returnResult(pAllResult, __FUNCTION__);
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

    CAM_LOGD("[%s] - Validate:%d", __FUNCTION__, i4Validate);
    AAA_TRACE_END_HAL;
    return 0;
}

MBOOL
HalIspTemplate::
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
    getPropInt("vendor.debug.hal3av3.p2", &i4P2En, defaultValue);

    MINT32 i4Ret = -1;

    if (i4P2En == 0 || pTuningBuf == NULL) {

        CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P2, "[%s] IT: flowType(%d), pTuningBuf(%p)", __FUNCTION__, flowType, pTuningBuf);
    } else {

        AAA_TRACE_HAL(P2ParsingMetadata);
        ResultP2_T rResultP2;
        P2Param_T rNewP2Param;
        NSIspTuning::ISP_INFO_T rIspInfo;
        IMetadata::Memory pCaminfoMeta;
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
        if (!control.halMeta.entryFor(MTK_3A_REPEAT_RESULT).isEmpty()) {

            QUERY_ENTRY_SINGLE(control.halMeta, MTK_3A_REPEAT_RESULT,           rNewP2Param.u1RepeatResult);
        }
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_ISP_P2_IN_IMG_FMT,              rNewP2Param.i4P2InImgFmt);
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_ISP_P2_TUNING_UPDATE_MODE,      rNewP2Param.u1P2TuningUpdate);
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_ISP_P2_IN_IMG_RES_REVISED,      rNewP2Param.ResizeYUV);

        CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P2, "[%s] u1RepeatResult(%d): i4MagicNum(%d)", __FUNCTION__, rNewP2Param.u1RepeatResult, rNewP2Param.i4MagicNum);

        QUERY_ENTRY_SINGLE(control.halMeta, MTK_3A_PGN_ENABLE,                  rNewP2Param.u1PGN);
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_3A_ISP_PROFILE,                 rNewP2Param.u1IspProfile);
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_P1NODE_RAW_TYPE,                rNewP2Param.i4RawType);
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_HAL_REQUEST_BRIGHTNESS_MODE,    rNewP2Param.i4BrightnessMode);
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_HAL_REQUEST_CONTRAST_MODE,      rNewP2Param.i4ContrastMode);
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_HAL_REQUEST_HUE_MODE,           rNewP2Param.i4HueMode);
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_HAL_REQUEST_SATURATION_MODE,    rNewP2Param.i4SaturationMode);
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_HAL_REQUEST_EDGE_MODE,          rNewP2Param.i4halEdgeMode);

        QUERY_ENTRY_SINGLE(control.appMeta, MTK_CONTROL_CAPTURE_INTENT,         rNewP2Param.u1CapIntent);
        QUERY_ENTRY_SINGLE(control.appMeta, MTK_TONEMAP_MODE,                   rNewP2Param.u1TonemapMode);
        QUERY_ENTRY_SINGLE(control.appMeta, MTK_EDGE_MODE,                      rNewP2Param.u1appEdgeMode);
        QUERY_ENTRY_SINGLE(control.appMeta, MTK_NOISE_REDUCTION_MODE,           rNewP2Param.u1NrMode);
        QUERY_ENTRY_SINGLE(control.appMeta, MTK_SENSOR_SENSITIVITY,             rNewP2Param.i4ISO);

        QUERY_ENTRY_SINGLE(control.halMeta, MTK_3A_ISP_MDP_TARGET_SIZE, rNewP2Param.targetSize);
        CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P2, "[%s] MTK_3A_ISP_MDP_TARGET_SIZE(%d, %d)", __FUNCTION__, rNewP2Param.targetSize.w, rNewP2Param.targetSize.h);

        NSCam::IMetadata::Memory rpdbgIsp;
        if (QUERY_ENTRY_SINGLE(control.halMeta, MTK_3A_EXIF_METADATA, rNewP2Param.rexifMeta)) {

            QUERY_ENTRY_SINGLE<NSCam::IMetadata::Memory>(rNewP2Param.rexifMeta, MTK_3A_EXIF_DBGINFO_ISP_DATA, rpdbgIsp);
            rNewP2Param.rpdbgIsp = &rpdbgIsp;
        }

        P2Info_T rP2Info;
        MINT32 isLmvValid = 0;
        if(!QUERY_ENTRY_SINGLE(control.halMeta, MTK_LMV_VALIDITY, isLmvValid)) {

            CAM_LOGD("[%s] MTK_LMV_VALIDITY query fail",__FUNCTION__);
        }
        if(isLmvValid) {
            MBOOL isGMVValid=MFALSE;
            MINT32 GMV_X=0, GMV_Y=0, GMVCONF_X=0, GMVCONF_Y=0, GMVMAX=0;
            isGMVValid |= QUERY_ENTRY_SINGLE_BY_IDX(control.halMeta, MTK_EIS_REGION, GMV_X, EIS_REGION_INDEX_GMVX);
            isGMVValid |= QUERY_ENTRY_SINGLE_BY_IDX(control.halMeta, MTK_EIS_REGION, GMV_Y, EIS_REGION_INDEX_GMVY);
            isGMVValid |= QUERY_ENTRY_SINGLE_BY_IDX(control.halMeta, MTK_EIS_REGION, GMVCONF_X, EIS_REGION_INDEX_CONFX);
            isGMVValid |= QUERY_ENTRY_SINGLE_BY_IDX(control.halMeta, MTK_EIS_REGION, GMVCONF_Y, EIS_REGION_INDEX_CONFY);
            isGMVValid |= QUERY_ENTRY_SINGLE_BY_IDX(control.halMeta, MTK_EIS_REGION, GMVMAX, EIS_REGION_INDEX_MAX_GMV);
            CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P2, "[%s] isGMVValid(%d) GMV = (%d, %d, %d, %d, %d) ",
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


        if (rNewP2Param.rpSclCropRect) {

            // crop info for AE
            rNewP2Param.rScaleCropRect.i4Xoffset = rNewP2Param.rpSclCropRect->p.x;
            rNewP2Param.rScaleCropRect.i4Yoffset = rNewP2Param.rpSclCropRect->p.y;
            rNewP2Param.rScaleCropRect.i4Xwidth  = rNewP2Param.rpSclCropRect->s.w;
            rNewP2Param.rScaleCropRect.i4Yheight = rNewP2Param.rpSclCropRect->s.h;

            rP2Info.rSclCropRectl.p.x = rNewP2Param.rpSclCropRect->p.x;
            rP2Info.rSclCropRectl.p.y = rNewP2Param.rpSclCropRect->p.y;
            rP2Info.rSclCropRectl.s.w = rNewP2Param.rpSclCropRect->s.w;
            rP2Info.rSclCropRectl.s.h = rNewP2Param.rpSclCropRect->s.h;
            CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P2, "[%s] Pass2 info AE SCL CROP(%d,%d,%d,%d)",
                __FUNCTION__, rNewP2Param.rpSclCropRect->p.x, rNewP2Param.rpSclCropRect->p.y, rNewP2Param.rpSclCropRect->s.w, rNewP2Param.rpSclCropRect->s.h);
        } else {

            CAM_LOGW("[%s] HalMeta not send preview crop region", __FUNCTION__);
        }
        m_pResultPoolWrapper->updateP2Info(rP2Info);
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
        if (entryRed.tag() != IMetadata::IEntry::BAD_TAG) {

            rNewP2Param.u4TonemapCurveRedSize = entryRed.count();
            rNewP2Param.pTonemapCurveRed = (const MFLOAT*)entryRed.data();
        }

        IMetadata::IEntry entryGreen = control.appMeta.entryFor(MTK_TONEMAP_CURVE_GREEN);
        if (entryGreen.tag() != IMetadata::IEntry::BAD_TAG) {

            rNewP2Param.u4TonemapCurveGreenSize = entryGreen.count();
            rNewP2Param.pTonemapCurveGreen = (const MFLOAT*)entryGreen.data();
        }

        IMetadata::IEntry entryBlue = control.appMeta.entryFor(MTK_TONEMAP_CURVE_BLUE);
        if (entryBlue.tag() != IMetadata::IEntry::BAD_TAG) {

            rNewP2Param.u4TonemapCurveBlueSize = entryBlue.count();
            rNewP2Param.pTonemapCurveBlue = (const MFLOAT*)entryBlue.data();
        }
        AAA_TRACE_END_HAL;

        CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P2,"[%s] FrameId(%d), HueMode(%d),BrightnessMode(%d), ContrastMode(%d), SaturationMode(%d), i4EdgeMode(%d)"
                , __FUNCTION__, rNewP2Param.i4MagicNum, rNewP2Param.i4HueMode, rNewP2Param.i4BrightnessMode, rNewP2Param.i4ContrastMode, rNewP2Param.i4SaturationMode, rNewP2Param.i4halEdgeMode);

        CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P2, "[%s] MTK_TONEMAP_MODE(%d), MagicNum(%d) Rsize(%d) Gsize(%d) Bsize(%d)",
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

        //update backup caminfo in result pool, and copy them to pCaminfoBuf
        {
            std::lock_guard<std::mutex> lock(m_LockLastInfo);
            // Restore caminfo
            if (pCaminfoBuf == NULL && m_BackupCamInfo_copied) {

                //update backup caminfo in result pool by current status
                restoreCaminfo(rNewP2Param, m_BackupCamInfo);

                //copy caminfo from result pool
                pCaminfoBuf = &m_BackupCamInfo;
            }
        }

        if (pCaminfoBuf) {

            AAA_TRACE_HAL(CopyCaminfo);

            ::memcpy(&rIspInfo.rCamInfo, pCaminfoBuf, sizeof(NSIspTuning::RAWIspCamInfo));

            //NR3D Params
            IMetadata::Memory pNR3DMeta;
            if (IMetadata::getEntry<IMetadata::Memory>(&control.halMeta, MTK_3A_ISP_NR3D_HW_PARAMS, pNR3DMeta)) {

                rNewP2Param.pNR3DBuf = (const NSCam::NR3D::NR3DIspParam *)(pNR3DMeta.array());
            }

            //Feature Control Overwrite
            QUERY_ENTRY_SINGLE(control.halMeta, MTK_3A_ISP_DISABLE_NR, rNewP2Param.bBypassNR);
            QUERY_ENTRY_SINGLE(control.halMeta, MTK_3A_ISP_BYPASS_LCE, rNewP2Param.bBypassLCE);

            AAA_TRACE_END_HAL;

            //update isp info by p2 param
            setISPInfo(rNewP2Param, pTuningBuf, rIspInfo, 0);

            //Feedback LSC tuning data via dump file
            _readDumpP2Buf(pTuningBuf, control, pResult, rule, &rIspInfo, E_P2_Before_SetParam);

            MINT32 i4ManualMode = 0;
#if HAL3A_TEST_OVERRIDE
            getPropInt("vendor.debug.hal3av3.testp2", &i4ManualMode, 0);
            _test_p2(i4ManualMode, mParams, rNewP2Param);
#endif

            //update p2 result by p2 param
            setP2Params(rNewP2Param, m_P1CamInfo, &rResultP2);

            CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P2, "[%s]+ sensorDev(%d), key(%09d), flow(%d), pTuningBuf(%p)",
                __FUNCTION__, m_i4SensorDev, rNewP2Param.i4UniqueKey, flowType, pTuningBuf);

            // check if need to update exif info
            if (rNewP2Param.u1Exif) {

                if(rNewP2Param.rpdbgIsp) {

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

                if (pResult) {

                    UPDATE_ENTRY_SINGLE(pResult->halMeta, MTK_3A_EXIF_METADATA, rNewP2Param.rexifMeta);
                }

                //dump P2 .tuning data
                dumpP2DotTuning(rIspInfo, rNewP2Param);

                //dump AAO buf
                dumpP2AAO(rIspInfo, rNewP2Param);
            } else {

                // generate P2 tuning only
                generateP2(flowType, rIspInfo, pTuningBuf, NULL);
            }

            CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P2, "[%s]- OK(%p, %p)", __FUNCTION__, pTuningBuf->pRegBuf, pTuningBuf->pLsc2Buf);
            i4Ret = 0;
        } else if(rNewP2Param.u1IspProfile == EIspProfile_YUV_Reprocess) {

            setISPInfo(rNewP2Param, pTuningBuf, rIspInfo, 1);

            if (pResult) {

                UPDATE_ENTRY_SINGLE(pResult->appMeta, MTK_EDGE_MODE, static_cast<MUINT8>(rIspInfo.rCamInfo.eEdgeMode));
                UPDATE_ENTRY_SINGLE(pResult->appMeta, MTK_NOISE_REDUCTION_MODE, static_cast<MUINT8>(rIspInfo.rCamInfo.eNRMode));
                UPDATE_ENTRY_SINGLE(pResult->halMeta, MTK_3A_EXIF_METADATA, getReprocStdExif(control));
            }

            generateP2(flowType, rIspInfo, pTuningBuf, NULL);

            i4Ret = 0;
        } else {

            CAM_LOGE("[%s] NG (no caminfo)", __FUNCTION__);
            i4Ret = -1;
        }

        AAA_TRACE_HAL(getP2Result);
        getP2Result(rNewP2Param, &rResultP2);
        AAA_TRACE_END_HAL;

        AAA_TRACE_HAL(Convert2Meta);
        convertP2ResultToMeta(rResultP2, pResult);
        AAA_TRACE_END_HAL;

        //Feedback LPCNR, tuning, MFB, LTM curve, and YNR map tuning data via dump file
        AAA_TRACE_HAL(ReadDump);
        _readDumpP2Buf(pTuningBuf, control, pResult, rule, &rIspInfo, E_P2_The_End);
        AAA_TRACE_END_HAL;

        _dumpP2Buf(pTuningBuf, rule, rIspInfo);
    }

    AAA_TRACE_END_D;
    return i4Ret;
}

MINT32
HalIspTemplate::
updateTGInfo()
{
    //Before wait for VSirq of IspDrv, we need to query IHalsensor for the current TG info
    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList) {

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

    setTGInfo(m_P1CamInfo, i4TgInfo, m_i4TgWidth, m_i4TgHeight);

    return MTRUE;
}

MVOID
HalIspTemplate::
querySensorStaticInfo(RAWIspCamInfo &rP1CamInfo)
{
    //Before phone boot up (before opening camera), we can query IHalsensor for the sensor static info (EX: MONO or Bayer)
    SensorStaticInfo sensorStaticInfo;
    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList) {

        CAM_LOGE("MAKE_HalSensorList() == NULL");
        return;
    }
    pHalSensorList->querySensorStaticInfo(m_i4SensorDev,&sensorStaticInfo);

    MUINT32 u4RawFmtType = sensorStaticInfo.rawFmtType; // SENSOR_RAW_MONO or SENSOR_RAW_Bayer

    // 3A/ISP mgr can query sensor static information here
    IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_NOTIFY_SENSOR_TYPE, u4RawFmtType, 0);

}

MBOOL
HalIspTemplate::
getP2Result(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2)
{
    if (rNewP2Param.u1TonemapMode != MTK_TONEMAP_MODE_CONTRAST_CURVE) {

        // Tonemap
        pResultP2->vecTonemapCurveRed.clear();
        pResultP2->vecTonemapCurveGreen.clear();
        pResultP2->vecTonemapCurveBlue.clear();

        MINT32 i = 0;
        MFLOAT *pIn, *pOut;
        MINT32 i4NumPt;
        IspTuningMgr::getInstance().getTonemapCurve_Blue(m_i4SensorDev, pIn, pOut, &i4NumPt);
        for (i = 0; i < i4NumPt; i++) {

            CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_GET_P2, "[%s][Blue](%f,%f)", __FUNCTION__, *pIn, *pOut);
            pResultP2->vecTonemapCurveBlue.push_back(*pIn++);
            pResultP2->vecTonemapCurveBlue.push_back(*pOut++);
        }
        IspTuningMgr::getInstance().getTonemapCurve_Green(m_i4SensorDev, pIn, pOut, &i4NumPt);
        for (i = 0; i < i4NumPt; i++) {

            CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_GET_P2, "[%s][Green](%f,%f)", __FUNCTION__, *pIn, *pOut);
            pResultP2->vecTonemapCurveGreen.push_back(*pIn++);
            pResultP2->vecTonemapCurveGreen.push_back(*pOut++);
        }
        IspTuningMgr::getInstance().getTonemapCurve_Red(m_i4SensorDev, pIn, pOut, &i4NumPt);
        for (i = 0; i < i4NumPt; i++) {

            CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_GET_P2, "[%s][Red](%f,%f)", __FUNCTION__, *pIn, *pOut);
            pResultP2->vecTonemapCurveRed.push_back(*pIn++);
            pResultP2->vecTonemapCurveRed.push_back(*pOut++);
        }
        CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_GET_P2,"[%s] rsize(%d) gsize(%d) bsize(%d)"
            ,__FUNCTION__, (MINT32)pResultP2->vecTonemapCurveRed.size(), (MINT32)pResultP2->vecTonemapCurveGreen.size(), (MINT32)pResultP2->vecTonemapCurveBlue.size());

    }
    return MTRUE;
}

MINT32
HalIspTemplate::
getCurrResult(MUINT32 i4FrmId)
{
    // ResultPool - To update Vector info
    AllResult_T *pAllResult = m_pResultPoolWrapper->getResultByReqNum(i4FrmId, __FUNCTION__);
    if(pAllResult == NULL) {
        m_pResultPoolWrapper->returnResult(pAllResult, __FUNCTION__);
        CAM_LOGE("[%s] pAllResult is NULL", __FUNCTION__);
        return MFALSE;
    }


    /*****************************
     *     Get ISP Result
     *****************************/
    if (m_u1ColorCorrectMode != MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX) {
        pAllResult->vecColorCorrectMat.resize(9);
        MFLOAT* pfColorCorrectMat = &(pAllResult->vecColorCorrectMat[0]);
        IspTuningMgr::getInstance().getColorCorrectionTransform(m_i4SensorDev,
        pfColorCorrectMat[0], pfColorCorrectMat[1], pfColorCorrectMat[2],
        pfColorCorrectMat[3], pfColorCorrectMat[4], pfColorCorrectMat[5],
        pfColorCorrectMat[6], pfColorCorrectMat[7], pfColorCorrectMat[8]
        );
    }

    ISPResultToMeta_T     rISPResult;
    rISPResult.rCamInfo = m_P1CamInfo;

    std::lock_guard<std::mutex> lock(m_LockLastInfo);
    // Backup caminfo
    ::memcpy(&m_BackupCamInfo, &rISPResult.rCamInfo, sizeof(NSIspTuning::RAWIspCamInfo));
    m_BackupCamInfo_copied = MTRUE;

    CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_GET_P1, "[%s] Backup caminfo,copied(%d)/mode(%d)/profile(%d)",__FUNCTION__, m_BackupCamInfo_copied,
            m_BackupCamInfo.rMapping_Info.eSensorMode,
            m_BackupCamInfo.rMapping_Info.eIspProfile);

    IdxMgr::createInstance(static_cast<NSIspTuning::ESensorDev_T>(m_i4SensorDev))->setMappingInfo(static_cast<NSIspTuning::ESensorDev_T>(m_i4SensorDev), rISPResult.rCamInfo.rMapping_Info, i4FrmId);

    pAllResult->ModuleResult[E_ISP_RESULTTOMETA]->write(&rISPResult);

    /*****************************
     *     Get ISP Exif Result
     *****************************/
    if (m_u1IsGetExif || m_bIsCapEnd) {
        if(pAllResult->vecDbgIspInfo.size()==0) {

            pAllResult->vecDbgIspInfo.resize(sizeof(AAA_DEBUG_INFO2_T));
        }

        AAA_DEBUG_INFO2_T& rDbgIspInfo = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(pAllResult->vecDbgIspInfo.editArray());

        if(pAllResult->vecDbgIspInfo.size() != 0) {

            CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_GET_P1, "[%s] vecDbgIspInfo - Size(%d) Addr(%p)", __FUNCTION__, (MINT32)pAllResult->vecDbgIspInfo.size(), &rDbgIspInfo);
            IspTuningMgr::getInstance().getDebugInfoP1(m_i4SensorDev, m_P1CamInfo, rDbgIspInfo.rISPDebugInfo, MFALSE);
        } else {
            CAM_LOGE(, "[%s] vecDbgIspInfo - Size(%d) Addr(%p)", __FUNCTION__, (MINT32)pAllResult->vecDbgIspInfo.size(), &rDbgIspInfo);
        }
    }
    m_pResultPoolWrapper->returnResult(pAllResult, __FUNCTION__);

    /*****************************
     *     condition_variable notify_all()
     *****************************/
    m_rResultCond.notify_all();

    return MTRUE;
}

MINT32
HalIspTemplate::
getResultCur(MINT32 i4FrmId)
{
    MINT32 i4ResultWaitCnt = 3;

    MINT32 i4Ret = 1;

    // ResultPool - get Current All Result with SttMagic
    // 1. Use SttMagic need to judge isValidate.
    // 2. If result validate, can get result address.

    AllResult_T *pAllResult;
    MINT32 i4Validate = m_pResultPoolWrapper->getAllHQCResultWithType(i4FrmId, E_ISP_RESULTTOMETA, &pAllResult, __FUNCTION__);

    //isp result not ready, block p1 setisp result
    if(i4Validate == 0) {
        std::unique_lock<std::mutex> autoLock(m_rResultMtx);
        // ResultPool - Wait to get Current All Result with SttMagic
        while (i4ResultWaitCnt) {
            m_pResultPoolWrapper->returnResult(pAllResult, __FUNCTION__);
            CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_GET_P1, "[%s] wait result #(%d) i4ResultWaitCnt(%d)", __FUNCTION__, i4FrmId, i4ResultWaitCnt);
            m_rResultCond.wait_for(autoLock, std::chrono::nanoseconds(1000000000));
            CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_GET_P1, "[%s] wait result done #(%d), i4ResultWaitCnt(%d)", __FUNCTION__, i4FrmId, i4ResultWaitCnt);
            i4ResultWaitCnt--;

            //continue polling
            i4Validate = m_pResultPoolWrapper->getAllHQCResultWithType(i4FrmId, E_ISP_RESULTTOMETA, &pAllResult, __FUNCTION__);

            if (i4Validate)
               break;
        }

        // ResultPool - CaptureStart ReqMagic is 2 3 4 5.., always update Result. If fail to get Current All Result, get last current All Result with ReqMagic 4.
        if(NULL == pAllResult) {
            m_pResultPoolWrapper->returnResult(pAllResult, __FUNCTION__);

            i4Validate = m_pResultPoolWrapper->getOldestAllResultWithType(E_ISP_RESULTTOMETA, &pAllResult, __FUNCTION__);
            i4Ret = -1;

            // ResultPool - If fail to get last current All Result with last ReqMagic 4, get All Result with ReqMagic 2.
            if(pAllResult == NULL) {
                m_pResultPoolWrapper->returnResult(pAllResult, __FUNCTION__);

                CAM_LOGW("Fail to get ResultLastCur, then get Result with ReqMagic(%d)", i4FrmId);
                i4Validate = m_pResultPoolWrapper->getAllResultWithType(i4FrmId, E_ISP_RESULTTOMETA, &pAllResult, __FUNCTION__);//get req/stt:2/xx
                i4Ret = -2;

                if(pAllResult) {
                    CAM_LOGW("[%s] (Req, Req, Stt) = (#%d, #%d, #%d)", __FUNCTION__, i4FrmId, pAllResult->rRequestInfo.i4ReqMagic, pAllResult->rRequestInfo.i4StatisticMagic);
                } else {
                    CAM_LOGE("[%s] Ret(%d) pISPResult is NULL", __FUNCTION__, i4Ret);
                }
            }
            else
                CAM_LOGW("[%s] (Req, Req, Stt) = (#%d, #%d, #%d)", __FUNCTION__, i4FrmId, pAllResult->rRequestInfo.i4ReqMagic, pAllResult->rRequestInfo.i4StatisticMagic);
        }

        // ResultPool - std exif should be use capture start
        AllResult_T *pAllResultAtStart = m_pResultPoolWrapper->getResultByReqNum(i4FrmId, __FUNCTION__);

        if(NULL != pAllResultAtStart && pAllResult != NULL) {
            CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_GET_P1, "[%s] (Req, ResultAtStartReq, ResultReq, Stt) = (#%d, #%d, #%d, #%d)", __FUNCTION__, i4FrmId, pAllResultAtStart->rRequestInfo.i4ReqMagic, pAllResult->rRequestInfo.i4ReqMagic, pAllResult->rRequestInfo.i4StatisticMagic);

            MBOOL isIspStartEmpty = pAllResultAtStart->vecDbgIspInfo.empty();
            MBOOL isIspCurEmpty = pAllResult->vecDbgIspInfo.empty();

            if(!isIspStartEmpty && !isIspCurEmpty) {

                // get capture start AE setting to update EXIF info
                AAA_DEBUG_INFO2_T& rDbgISPInfoStart = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(pAllResultAtStart->vecDbgIspInfo.editArray());
                AAA_DEBUG_INFO2_T& rDbgISPInfoCur = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(pAllResult->vecDbgIspInfo.editArray());
                rDbgISPInfoCur.rISPDebugInfo = rDbgISPInfoStart.rISPDebugInfo;
            } else {

                CAM_LOGE("isIspStartEmpty(%d) isIspCurEmpty(%d)", isIspStartEmpty, isIspCurEmpty);
            }
        } else {

            CAM_LOGE("Fail get pResultAtStart (#%d) pAllResultAtStart/pAllResult:%p/%p", i4FrmId, pAllResultAtStart, pAllResult);
        }
        m_pResultPoolWrapper->returnResult(pAllResultAtStart, __FUNCTION__);

    } else {

        CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_GET_P1, "[%s] got result (#%d)", __FUNCTION__, i4FrmId);
    }
    m_pResultPoolWrapper->returnResult(pAllResult, __FUNCTION__);

    return i4Ret;
}

IMetadata
HalIspTemplate::
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
    switch (u1AWBMode) {

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
    switch (u1SceneMode) {

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
    switch (u1SceneMode) {

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
    if (u1FlashState == MTK_FLASH_STATE_FIRED) {

        u4FlashLightTimeus = 30000;
    }

    //AE_EXP_BIAS

    IMetadata mMetaStaticInfo = m_pResultPoolWrapper->getMetaStaticInfo();
    QUERY_ENTRY_SINGLE(control.appMeta, MTK_CONTROL_AE_EXPOSURE_COMPENSATION, u4AEComp );
    // AE Comp Step
    MFLOAT fExpCompStep= 0.0f;
    MRational rStep;
    if (QUERY_ENTRY_SINGLE(mMetaStaticInfo, MTK_CONTROL_AE_COMPENSATION_STEP, rStep)) {

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

MINT32
HalIspTemplate::
convertP2ResultToMeta(const ResultP2_T& rResultP2, MetaSet_T* pResult) const
{
    MBOOL fgLog = (HALISP_LOG_CONVERT_P2 & m_i4LogEn);
    MINT32 i4Size = 0;
    if (pResult != NULL) {

        // tonemap
        i4Size = rResultP2.vecTonemapCurveBlue.size();
        if (i4Size) {

            const MFLOAT* pCurve = &(rResultP2.vecTonemapCurveBlue[0]);
            UPDATE_ENTRY_ARRAY(pResult->appMeta, MTK_TONEMAP_CURVE_BLUE, pCurve, i4Size);
            CAM_LOGD_IF(fgLog, "[%s] B size(%d), P0(%f,%f), P_end(%f,%f)", __FUNCTION__, i4Size, pCurve[0], pCurve[1], pCurve[i4Size-2], pCurve[i4Size-1]);
        }
        i4Size = rResultP2.vecTonemapCurveGreen.size();
        if (i4Size) {

            const MFLOAT* pCurve = &(rResultP2.vecTonemapCurveGreen[0]);
            UPDATE_ENTRY_ARRAY(pResult->appMeta, MTK_TONEMAP_CURVE_GREEN, pCurve, i4Size);
            CAM_LOGD_IF(fgLog, "[%s] G size(%d), P0(%f,%f), P_end(%f,%f)", __FUNCTION__, i4Size, pCurve[0], pCurve[1], pCurve[i4Size-2], pCurve[i4Size-1]);
        }
        i4Size = rResultP2.vecTonemapCurveRed.size();
        if (i4Size) {

            const MFLOAT* pCurve = &(rResultP2.vecTonemapCurveRed[0]);
            UPDATE_ENTRY_ARRAY(pResult->appMeta, MTK_TONEMAP_CURVE_RED, pCurve, i4Size);
            CAM_LOGD_IF(fgLog, "[%s] R size(%d), P0(%f,%f), P_end(%f,%f)", __FUNCTION__, i4Size, pCurve[0], pCurve[1], pCurve[i4Size-2], pCurve[i4Size-1]);
        }
    }
    return 0;
}

MINT32
HalIspTemplate::
getOBCFromResultPool(MINT32 **OBOffset)
{
    ISPResultToMeta_T *pISPResult = (ISPResultToMeta_T*)m_pResultPoolWrapper->getResult(m_i4Magic, E_ISP_RESULTTOMETA, __FUNCTION__);
    CAM_LOGD("[%s], GetOBOffset ISPResult(%p) at MagicNum(%d)", __FUNCTION__, pISPResult, m_i4Magic);
    if (!pISPResult) {

        m_pResultPoolWrapper->getPreviousResultWithType(E_ISP_RESULTTOMETA, &pISPResult);
    }

    (*OBOffset)[0] =  pISPResult->rCamInfo.rOBC_OFST[0];
    (*OBOffset)[1] =  pISPResult->rCamInfo.rOBC_OFST[1];
    (*OBOffset)[2] =  pISPResult->rCamInfo.rOBC_OFST[2];
    (*OBOffset)[3] =  pISPResult->rCamInfo.rOBC_OFST[3];

    return MTRUE;
}

MINT32
HalIspTemplate::
getLCSFromResultPool(ISP_LCS_OUT_INFO_T const &rLcsOutInfo)
{
    LCSOResultToMeta_T     rLCSResult;

    rLCSResult.rLcsOutInfo = rLcsOutInfo;

    m_pResultPoolWrapper->updateResult(LOG_TAG, rLCSResult.rLcsOutInfo.i4FrmId, E_LCSO_RESULTTOMETA, &rLCSResult);

    return MTRUE;
}

MBOOL
HalIspTemplate::
dumpP2AAO(ISP_INFO_T& rIspInfo, P2Param_T &rNewP2Param)
{
    MINT32 dumpAAO        = 0;
    getPropInt("vendor.debug.camera.AAO.dump", &dumpAAO, 0);
    MINT32 dumpAAOPreview = 0;
    getPropInt("vendor.debug.camera.AAO.dump.preview", &dumpAAOPreview, 0);

    if ((dumpAAO && rIspInfo.isCapture) || dumpAAOPreview) {

        char filename[512];
        char temp[512];
        genFileName_HW_AAO(filename, sizeof(filename), &rIspInfo.hint);

        sprintf(temp, "/sdcard/camera_dump/captue_end_aao_%d.hw_aao", rNewP2Param.i4MagicNum);
        if(_isFileExist(temp)) {

            int result;
            result= rename(temp, filename);
            if ( result == 0 ) {

                CAM_LOGD("Renamed success: %s", filename);
            } else {

                CAM_LOGD("Renamed fail: %s", temp);
            }
        } else {

            _dumpAAO(filename, m_i4SensorDev, rNewP2Param.i4MagicNum);
        }
    }

    return MTRUE;
}

MVOID
HalIspTemplate::
setFDEnable(MBOOL fgEnable)
{
    CAM_LOGD_IF(m_i4LogEn, "[%s] fgEnable(%d)", __FUNCTION__, fgEnable);
    setFDEnabletoISP(m_P1CamInfo, fgEnable);
}

MBOOL
HalIspTemplate::
setFDInfo(MVOID* prFaces)
{
    CAM_LOGD_IF(m_i4LogEn, "[%s] m_bFaceDetectEnable(%d)", __FUNCTION__, m_P1CamInfo.fgFDEnable);
    setFDInfotoISP(m_P1CamInfo, prFaces);
    return MTRUE;
}




