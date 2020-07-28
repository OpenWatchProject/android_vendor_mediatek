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
#include <property_utils.h>

#include <mtkcam/drv/IHalSensor.h>

#include <mutex>
#include <faces.h>

//tuning utils
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

/*******************************************************************************
* implementations
********************************************************************************/

HalIspImp::
HalIspImp(MINT32 const i4SensorIdx)
    : HalIspTemplate(i4SensorIdx)
    , m_pTuning(NULL)
    , m_CbLock()
    , m_i4CopyLscP1En(0)
    , mpHalISPP1DrvCbHub(NULL)
    , m_target_source(EFD_SOURCE_PASS2)
{
    getPropInt("vendor.debug.camera.copy.p1.lsc", &m_i4CopyLscP1En, 0);
    CAM_LOGD("[%s] sensorIdx(0x%04x) sensorDev(%d)", __FUNCTION__, i4SensorIdx, m_i4SensorDev);
}

HalIspImp*
HalIspImp::
createInstance(MINT32 const i4SensorIdx, const char* strUser)
{
    getPropInt("vendor.debug.halisp.log", &m_i4LogEn, 0);
    CAM_LOGD("[%s] sensorIdx(%d) %s", __FUNCTION__, i4SensorIdx, strUser);
    switch (i4SensorIdx) {
    case 0: {

            static HalIspImp _singleton(0);
            _singleton.init(strUser);
            return &_singleton;
        }
    case 1: {

            static HalIspImp _singleton(1);
            _singleton.init(strUser);
            return &_singleton;
        }
    case 2: {

            static HalIspImp _singleton(2);
            _singleton.init(strUser);
            return &_singleton;
        }
    case 3: {

            static HalIspImp _singleton(3);
            _singleton.init(strUser);
            return &_singleton;
        }
    default:

        CAM_LOGE("Unsupport sensor Index: %d\n", i4SensorIdx);
        return NULL;
    }
}

MINT32
HalIspImp::
sendIspCtrl(EISPCtrl_T eISPCtrl, MINTPTR iArg1, MINTPTR iArg2)
{
    switch (eISPCtrl) {
        // ----------------------------------ISP----------------------------------
        case EISPCtrl_GetIspGamma:

            IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, IspTuningMgr::E_ISPTUNING_GET_ISP_GAMMA, iArg1, iArg2);
            break;
        case EISPCtrl_SetFdSource:
             CAM_LOGD("SetFdSource from %d to %d", m_target_source, *(reinterpret_cast<MUINT32*>(arg1)));
             m_target_source = *(reinterpret_cast<MUINT32*>(arg1));
             break;
        case EISPCtrl_GetOBOffset: {

                MINT32 *OBOffset = reinterpret_cast<MINT32*>(iArg1);
                getOBCFromResultPool(&OBOffset);
            }
            break;
        case EISPCtrl_SetOperMode:

            MINT32 i4OperMode;
            i4OperMode = IspTuningMgr::getInstance().getOperMode(m_i4SensorDev);
            if(i4OperMode != EOperMode_Meta) {
                IspTuningMgr::getInstance().setOperMode(m_i4SensorDev, iArg1);
            }
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

            getLCSFromResultPool(*(ISP_LCS_OUT_INFO_T*)iArg1);
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
            return -ENOTSUP;
    }
    return 0;
}

MINT32
HalIspImp::
InitP1Cb()
{
    CAM_LOGD_IF(1, "[%s] ++", __FUNCTION__);
    std::lock_guard<std::mutex> lock(m_CbLock);

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
    std::lock_guard<std::mutex> lock(m_CbLock);

    if(mpHalISPP1DrvCbHub != NULL) {

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
    m_pCamIO->sendCommand(ENPipeCmd_SET_TUNING_CBFP,
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
initTuning()
{
    // TuningMgr init
    if (m_pTuning == NULL) {

        AAA_TRACE_D("TUNING init");
        m_pTuning = &IspTuningMgr::getInstance();
        if (!m_pTuning->init(m_i4SensorDev, m_i4SensorIdx)) {
            CAM_LOGE("Fail to init IspTuningMgr (%d,%d)", m_i4SensorDev, m_i4SensorIdx);
            AEE_ASSERT_ISP_HAL("Fail to init IspTuningMgr");
            AAA_TRACE_END_D;
            return MFALSE;
        }
        AAA_TRACE_END_D;
    }
    return MTRUE;
}

MBOOL
HalIspImp::
configTuning(MINT32 i4SubsampleCount)
{
    // TuningMgr uninit
    if (m_pTuning) {

        CAM_LOGD("[%s] m_pTuning uninit +", __FUNCTION__);
        m_pTuning->uninit(m_i4SensorDev);
        m_pTuning = NULL;
        CAM_LOGD("[%s] m_pTuning uninit -", __FUNCTION__);
    }
    // TuningMgr init
    if (m_pTuning == NULL) {

        CAM_LOGD("[%s] m_pTuning init +", __FUNCTION__);
        AAA_TRACE_D("TUNING init");
        m_pTuning = &IspTuningMgr::getInstance();
        if (!m_pTuning->init(m_i4SensorDev, m_i4SensorIdx, i4SubsampleCount)) {

            CAM_LOGE("Fail to init IspTuningMgr (%d,%d)", m_i4SensorDev, m_i4SensorIdx);
            AEE_ASSERT_ISP_HAL("Fail to init IspTuningMgr");
            AAA_TRACE_END_D;
            return MFALSE;
        }
        CAM_LOGD("[%s] m_pTuning init -", __FUNCTION__);
        AAA_TRACE_END_D;
    }
    return MTRUE;
}

MBOOL
HalIspImp::
uninitTuning()
{
    // TuningMgr uninit
    if (m_pTuning) {
        m_pTuning->uninit(m_i4SensorDev);
        m_pTuning = NULL;
    }
    return MTRUE;
}

MBOOL
HalIspImp::
setConfigParams(P1Param_T const &rNewConfigParam, RAWIspCamInfo &rP1CamInfo)
{
    MBOOL bFrontalBin =
        (m_i4TgWidth == (MINT32)rNewConfigParam.u4ISPRawWidth &&
         m_i4TgHeight == (MINT32)rNewConfigParam.u4ISPRawHeight) ?
         MFALSE : MTRUE;

    setSensorMode(rP1CamInfo, m_i4SensorMode, bFrontalBin, rNewConfigParam.u4ISPRawWidth, rNewConfigParam.u4ISPRawHeight);

    m_pTuning->setRawPath(m_i4SensorDev, rNewConfigParam.ERawPath);
    m_pTuning->setP1DirectYUV_Port(m_i4SensorDev, rNewConfigParam.u4P1DirectYUV_Port);

    //Need change
    setIspProfile(rP1CamInfo, NSIspTuning::EIspProfile_Preview);

    notifyRPGEnable(rP1CamInfo, MTRUE);   // apply awb gain for init stat

    //Set 2A data to caminfo
    if(rNewConfigParam.pAEResultInfo) {

        CAM_LOGD_IF(1, "[%s] u4DGNGain(%d)", __FUNCTION__, ((AEResultInfo_T*)(rNewConfigParam.pAEResultInfo))->AEPerframeInfo.rAEISPInfo.u4P1DGNGain);
        setAEInfo2ISP(rP1CamInfo, ((AEResultInfo_T*)(rNewConfigParam.pAEResultInfo))->AEPerframeInfo.rAEISPInfo);
    } else {

        CAM_LOGD("[%s] pAEResultInfo(%p)", __FUNCTION__, rNewConfigParam.pAEResultInfo);
    }

    if(rNewConfigParam.pAWBResultInfo) {

        CAM_LOGD_IF(1, "[%s] rAwbGainNoPref(%d, %d, %d)", __FUNCTION__,
            ((AWBResultInfo_T*)(rNewConfigParam.pAWBResultInfo))->AWBInfo4ISP.rAwbGainNoPref.i4R,
            ((AWBResultInfo_T*)(rNewConfigParam.pAWBResultInfo))->AWBInfo4ISP.rAwbGainNoPref.i4G,
            ((AWBResultInfo_T*)(rNewConfigParam.pAWBResultInfo))->AWBInfo4ISP.rAwbGainNoPref.i4B);
        setAWBInfo2ISP(rP1CamInfo, ((AWBResultInfo_T*)(rNewConfigParam.pAWBResultInfo))->AWBInfo4ISP);
    } else {

        CAM_LOGD("[%s] pAWBResultInfo(%p)", __FUNCTION__, rNewConfigParam.pAWBResultInfo);
    }

    return MTRUE;
}

MBOOL
HalIspImp::
setP1Params(P1Param_T const &rNewP1Param, RAWIspCamInfo &rP1CamInfo)
{
    // Set APP data to ISP
    if(rNewP1Param.i1ControlMode           != -1) {

        CAM_LOGD("[%s] control(old, new)=(%d -> %d)", __FUNCTION__, rP1CamInfo.eControlMode, rNewP1Param.i1ControlMode);

        rP1CamInfo.eControlMode = static_cast<mtk_camera_metadata_enum_android_control_mode_t>(rNewP1Param.i1ControlMode);
    }
    if(rNewP1Param.i1EffectMode            != -1) {

        CAM_LOGD("[%s] effect(old, new)=(%d, %d)", __FUNCTION__, rP1CamInfo.eIdx_Effect, rNewP1Param.i1EffectMode);

        rP1CamInfo.eIdx_Effect = static_cast<EIndex_Effect_T>(rNewP1Param.i1EffectMode);
    }
    if(rNewP1Param.i1SceneMode             != -1) {

        CAM_LOGD("[%s] scene(old, new)=(%d, %d)", __FUNCTION__, rP1CamInfo.eIdx_Scene, rNewP1Param.i1SceneMode);

        rP1CamInfo.eIdx_Scene = static_cast<EIndex_Scene_T>(rNewP1Param.i1SceneMode);
    }
    if(rNewP1Param.i1EdgeMode              != -1) {

        CAM_LOGD("[%s] edge (old, new)=(%d, %d)", __FUNCTION__, rP1CamInfo.eEdgeMode, rNewP1Param.i1EdgeMode);

        rP1CamInfo.eEdgeMode = static_cast<mtk_camera_metadata_enum_android_edge_mode_t>(rNewP1Param.i1EdgeMode);
    }
    if(rNewP1Param.i1ColorCorrectMode      != -1) {

        CAM_LOGD("[%s] colorCorrectMode (old, new)=(%d, %d)", __FUNCTION__, rP1CamInfo.eColorCorrectionMode, rNewP1Param.i1ColorCorrectMode);

        rP1CamInfo.eColorCorrectionMode = static_cast<mtk_camera_metadata_enum_android_color_correction_mode_t>(rNewP1Param.i1ColorCorrectMode);
    }
    if(rNewP1Param.fgColorCorrectModeIsSet != MTRUE) {

        IspTuningMgr::getInstance().setColorCorrectionTransform(m_i4SensorDev,
                    rNewP1Param.fColorCorrectMat[0], rNewP1Param.fColorCorrectMat[1], rNewP1Param.fColorCorrectMat[2],
                    rNewP1Param.fColorCorrectMat[3], rNewP1Param.fColorCorrectMat[4], rNewP1Param.fColorCorrectMat[5],
                    rNewP1Param.fColorCorrectMat[6], rNewP1Param.fColorCorrectMat[7], rNewP1Param.fColorCorrectMat[8]);
    }

    NSIspTuning::EIspProfile_T eIspProfile = static_cast<NSIspTuning::EIspProfile_T>(rNewP1Param.u1IspProfile);
    // replace ISP profile
    if (rNewP1Param.u1IspProfile == 0xFF) {

        switch (rNewP1Param.u1CapIntent) {

            case MTK_CONTROL_CAPTURE_INTENT_PREVIEW:
            case MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG:
                eIspProfile = NSIspTuning::EIspProfile_Preview;
                break;

            case MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD:
            case MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT:
                eIspProfile = NSIspTuning::EIspProfile_Video;
                break;

            default:
                eIspProfile = NSIspTuning::EIspProfile_Preview;
                break;
        }
        CAM_LOGE("[%s] No ISP Profile: CaptureIntent: %d, Default Profile: %d", __FUNCTION__, rNewP1Param.u1CapIntent, eIspProfile);
    }
    setIspProfile(rP1CamInfo, eIspProfile);

    rP1CamInfo.i4RawType = (ERawType_T)rNewP1Param.bIMGO_RawType;
    IspTuningMgr::getInstance().setRequestNumber(m_i4SensorDev, rNewP1Param.i4RequestNumber);

    //Need perframe call??
    notifyRPGEnable(rP1CamInfo, MTRUE);

    //Clear dump buffer
    IspTuningBufCtrl::getInstance(m_i4SensorDev)->clearP1Buffer();

    //Set 2A data to caminfo
    if(rNewP1Param.pAEResultInfo) {

        CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1, "[%s] u4DGNGain(%d)", __FUNCTION__, ((AEResultInfo_T*)(rNewP1Param.pAEResultInfo))->AEPerframeInfo.rAEISPInfo.u4P1DGNGain);
        setAEInfo2ISP(rP1CamInfo, ((AEResultInfo_T*)(rNewP1Param.pAEResultInfo))->AEPerframeInfo.rAEISPInfo);
    } else {

        CAM_LOGD("[%s] pAEResultInfo(%p)", __FUNCTION__, rNewP1Param.pAEResultInfo);
    }

    if(rNewP1Param.pAWBResultInfo) {

        CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1, "[%s] rAwbGainNoPref(%d, %d, %d)", __FUNCTION__,
            ((AWBResultInfo_T*)(rNewP1Param.pAWBResultInfo))->AWBInfo4ISP.rAwbGainNoPref.i4R,
            ((AWBResultInfo_T*)(rNewP1Param.pAWBResultInfo))->AWBInfo4ISP.rAwbGainNoPref.i4G,
            ((AWBResultInfo_T*)(rNewP1Param.pAWBResultInfo))->AWBInfo4ISP.rAwbGainNoPref.i4B);
        setAWBInfo2ISP(rP1CamInfo, ((AWBResultInfo_T*)(rNewP1Param.pAWBResultInfo))->AWBInfo4ISP);
    } else {

        CAM_LOGD("[%s] pAWBResultInfo(%p)", __FUNCTION__, rNewP1Param.pAWBResultInfo);
    }

    setFDEnabletoISP(rP1CamInfo, rNewP1Param.fgFdEnable);

    setZoomRatio(rP1CamInfo, rNewP1Param.i4ZoomRatio);

    return MTRUE;
}


MBOOL
HalIspImp::
validateP1(RequestSet_T const RequestSet, RAWIspCamInfo &rP1CamInfo)
{
    m_pTuning->validatePerFrameP1(m_i4SensorDev, RequestSet, rP1CamInfo);

    return MTRUE;
}

MBOOL
HalIspImp::
setP1GetParams(P1GetParam_T const &rNewP1GetParam, RAWIspCamInfo &tempCamInfo, RAWIspCamInfo &rP1CamInfo)
{
    if(rNewP1GetParam.pISPResult) {

        tempCamInfo = ((ISPResultToMeta_T*)(rNewP1GetParam.pISPResult))->rCamInfo;

        //write back to caminfo
        setCCUInfo2ISP(rP1CamInfo, tempCamInfo.rCCU_Result);
    } else {

        CAM_LOGE("No ISP in Result Pool, FrmID: %d", rNewP1GetParam.frmId);
    }

    if(rNewP1GetParam.pLCSOResult) {

        tempCamInfo.rLCS_Info = ((LCSOResultToMeta_T*)(rNewP1GetParam.pLCSOResult))->rLcsOutInfo;
    } else {

        CAM_LOGE("No LCS in Result Pool, FrmID: %d", rNewP1GetParam.frmId);
    }

    return MTRUE;
}

MBOOL
HalIspImp::
setCCUFeedbackExif(P1GetParam_T const &rNewP1GetParam, RAWIspCamInfo &tempCamInfo)
{
    AAA_DEBUG_INFO2_T& rDbg3AInfo2 = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(rNewP1GetParam.pDbg3AInfo2);
    NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo = rDbg3AInfo2.rISPDebugInfo;

    if(rNewP1GetParam.pISPResult) {
        IspTuningMgr::getInstance().setHLRDebugInfo4CCU(m_i4SensorDev, tempCamInfo.rCCU_Result.HLR.HLR_Data , rDbgIspInfo);
            IspTuningMgr::getInstance().setLTMnLTMSDebugInfo4CCU(m_i4SensorDev, tempCamInfo.rCCU_Result.LTM.P1_LTM_Reg[0],
                                                                                tempCamInfo.rCCU_Result.LTM.P1_LTM_Reg[1],
                                                                                tempCamInfo.rCCU_Result.LTM.P1_LTMS_Reg,
                                                                                rDbgIspInfo);
    }

    if(rNewP1GetParam.pCCUResult) {

        IspTuningMgr::getInstance().setDGNDebugInfo4CCU(m_i4SensorDev,
            ((CCUResultInfo_T*)(rNewP1GetParam.pCCUResult))->u4Rto,
            ((CCUResultInfo_T*)(rNewP1GetParam.pCCUResult))->u4P1DGNGain, rDbgIspInfo);
    }

    return MTRUE;
}

MBOOL
HalIspImp::
setISPInfo(P2Param_T const &rNewP2Param, TuningParam* pTuningBuf, NSIspTuning::ISP_INFO_T &rIspInfo, MINT32 type)
{
    AAA_TRACE_HAL(setISPInfo);

    if(rNewP2Param.pNR3DBuf) {

        ::memcpy(&rIspInfo.rCamInfo.NR3D_Data, rNewP2Param.pNR3DBuf, sizeof(NSCam::NR3D::NR3DIspParam));
    }

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

        if (rNewP2Param.u1IspProfile != 255) {

            rIspInfo.rCamInfo.rMapping_Info.eIspProfile = static_cast<NSIspTuning::EIspProfile_T>(rNewP2Param.u1IspProfile);
        } else {

            switch (rNewP2Param.u1CapIntent) {

            case MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD:
            case MTK_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT:

                rIspInfo.rCamInfo.rMapping_Info.eIspProfile = NSIspTuning::EIspProfile_Video;
                break;
            case MTK_CONTROL_CAPTURE_INTENT_PREVIEW:
            case MTK_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG:

                rIspInfo.rCamInfo.rMapping_Info.eIspProfile = NSIspTuning::EIspProfile_Preview;
                break;
            case MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE:

                if(rIspInfo.rCamInfo.fgRPGEnable) {

                    rIspInfo.rCamInfo.rMapping_Info.eIspProfile = NSIspTuning::EIspProfile_Preview;
                } else {

                    rIspInfo.rCamInfo.rMapping_Info.eIspProfile = NSIspTuning::EIspProfile_Capture;
                }
                break;
            }
        }

        rIspInfo.rCamInfo.i4RawType = static_cast<ERawType_T>(rNewP2Param.i4RawType);

        if (((!rIspInfo.rCamInfo.fgRPGEnable) && (rNewP2Param.i4RawType == NSIspTuning::ERawType_Pure)) || m_i4CopyLscP1En == 1) {

            if (rNewP2Param.rpLscData) {

                rIspInfo.rLscData = std::vector<MUINT8>((rNewP2Param.rpLscData)->array(), (rNewP2Param.rpLscData)->array()+(rNewP2Param.rpLscData)->size());
            } else {

                CAM_LOGD("[%s] No shading entry in metadata\n", __FUNCTION__);
            }

            if (rNewP2Param.rpTsfData) {

                rIspInfo.rTsfData = std::vector<MUINT8>((rNewP2Param.rpTsfData)->array(), (rNewP2Param.rpTsfData)->array()+(rNewP2Param.rpTsfData)->size());
            } else {

                CAM_LOGD("[%s] No Tsfs output entry in metadata\n", __FUNCTION__);
            }
        }

        MINT32 _i4LSCDumpEn = 0;
        getPropInt("vendor.debug.lsc_mgr.log", &_i4LSCDumpEn, 0);
        if(_i4LSCDumpEn == 4095) {

            if (rNewP2Param.rpTsfDumpNo) {

                std::vector<MUINT8> rLscDumpNo = std::vector<MUINT8>((rNewP2Param.rpTsfDumpNo)->array(), (rNewP2Param.rpTsfDumpNo)->array()+(rNewP2Param.rpTsfDumpNo)->size());

                CAM_LOGD("[%s] Copy Tsf dump %s to hint\n", __FUNCTION__, (char *)&rLscDumpNo[0]);
                strncpy(rIspInfo.hint.additStr, (char *)&rLscDumpNo[0], rLscDumpNo.size());
            } else {

                CAM_LOGD("[%s] No Tsf dump no entry in metadata\n", __FUNCTION__);
            }
        }

        if(rIspInfo.rLscData.size()) {

            MINT32 mi4DumpLscP1En=0;
            MINT32 mi4DumpLscP1CapEn=0;
            getPropInt("vendor.debug.camera.dump.p1.lsc", &mi4DumpLscP1En, 0);
            getPropInt("vendor.debug.camera.dump.cap.lsc", &mi4DumpLscP1CapEn, 0);
            if(mi4DumpLscP1En || mi4DumpLscP1CapEn && rIspInfo.isCapture) {

                char filename[256];
                rIspInfo.hint.IspProfile = rIspInfo.rCamInfo.rMapping_Info.eIspProfile;
                genFileName_LSC(filename, sizeof(filename), &rIspInfo.hint);
                android::sp<IFileCache> fidLscDump;
                fidLscDump = IFileCache::open(filename);

                if (fidLscDump->write(&rIspInfo.rLscData[0], rIspInfo.rLscData.size()) != rIspInfo.rLscData.size()) {
                    CAM_LOGD("[%s] write error %s", __FUNCTION__, filename);
                }
            }
        }

        if(rIspInfo.rTsfData.size()) {

            MINT32 mi4DumpTsfP1En=0;
            MINT32 mi4DumpTsfP1CapEn=0;
            getPropInt("vendor.debug.camera.dump.p1.tsfo", &mi4DumpTsfP1En, 0);
            getPropInt("vendor.debug.camera.dump.cap.tsfo", &mi4DumpTsfP1CapEn, 0);
            if(mi4DumpTsfP1En || mi4DumpTsfP1CapEn && rIspInfo.isCapture) {

                char filename[256];
                rIspInfo.hint.IspProfile = rIspInfo.rCamInfo.rMapping_Info.eIspProfile;
                genFileName_TSF(filename, sizeof(filename), &rIspInfo.hint);
                android::sp<IFileCache> fidLscDump;
                fidLscDump = IFileCache::open(filename);

                if (fidLscDump->write(&rIspInfo.rTsfData[0], rIspInfo.rTsfData.size()) != rIspInfo.rTsfData.size()) {

                    CAM_LOGD("[%s] write error %s", __FUNCTION__, filename);
                }
            }
        }

        rIspInfo.i4UniqueKey = rNewP2Param.i4UniqueKey;
        rIspInfo.rCamInfo.targetSize = rNewP2Param.targetSize;

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

        if(rNewP2Param.i4P2InImgFmt == EYuv2Yuv) {

            MUINT32 ResizeYUV_W = rNewP2Param.ResizeYUV & 0x0000FFFF;
            MUINT32 ResizeYUV_H = rNewP2Param.ResizeYUV >> 16;

            if( ResizeYUV_W != 0 && ResizeYUV_H !=0) {

                if((ResizeYUV_W != rIspInfo.rCamInfo.rCropRzInfo.sRRZout.w) ||
                (ResizeYUV_H != rIspInfo.rCamInfo.rCropRzInfo.sRRZout.h)) {

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


        if (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_YUV_Reprocess) {

            rIspInfo.rCamInfo.rAEInfo.u4P2RealISOValue= rNewP2Param.i4ISO;
            rIspInfo.rCamInfo.eIdx_Scene = static_cast<NSIspTuning::EIndex_Scene_T>(0);  //MTK_CONTROL_SCENE_MODE_DISABLED
            rIspInfo.rCamInfo.rMapping_Info.eSensorMode = NSIspTuning::ESensorMode_Capture;
        }
    }

    // set ISP Profile to file naming hint
    rIspInfo.hint.IspProfile = rIspInfo.rCamInfo.rMapping_Info.eIspProfile;

    //force diable LCE if buffer is unavailable
    if(!rIspInfo.rCamInfo.bBypassLCE) {

        if (pTuningBuf->pLcsBuf == NULL) {

             CAM_LOGE("[%s] [-No Lcso Buffer ]", __FUNCTION__);
             rIspInfo.rCamInfo.bBypassLCE = MTRUE;
        }

        else if (pTuningBuf->pLceshoBuf == NULL) {

             CAM_LOGE("[%s] [-No Lcesho Buffer ]", __FUNCTION__);
        }
    }

    CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P2, "[%s]+ sensorDev(%d), key(%09d), #(%d), ispProfile(%d), rpg(%d), bBypassLCE(%d)",
                        __FUNCTION__, m_i4SensorDev, rIspInfo.i4UniqueKey, rIspInfo.rCamInfo.u4Id,
                        rIspInfo.rCamInfo.rMapping_Info.eIspProfile, rIspInfo.rCamInfo.fgRPGEnable, rIspInfo.rCamInfo.bBypassLCE);

    AAA_TRACE_END_HAL;

    return MTRUE;
}

MBOOL
HalIspImp::
restoreCaminfo(P2Param_T &rNewP2Param, RAWIspCamInfo &mBackupCamInfo)
{
    mBackupCamInfo.rCropRzInfo.sTGout.w = m_i4TgWidth;
    mBackupCamInfo.rCropRzInfo.sTGout.h = m_i4TgHeight;
    mBackupCamInfo.rMapping_Info.eSensorMode = static_cast<ESensorMode_T>(m_i4SensorMode);
    mBackupCamInfo.rMapping_Info.eIspProfile = static_cast<NSIspTuning::EIspProfile_T>(rNewP2Param.u1IspProfile);

    CAM_LOGD("[%s] Restore caminfo,mode(%d)/profile(%d)/FrmId(%d)/TG(%d,%d)",__FUNCTION__,
            mBackupCamInfo.rMapping_Info.eSensorMode,
            mBackupCamInfo.rMapping_Info.eIspProfile,
             rNewP2Param.i4MagicNum, m_i4TgWidth, m_i4TgHeight);

    return MTRUE;
}

MBOOL
HalIspImp::
setP2Params(P2Param_T const &rNewP2Param, RAWIspCamInfo &rCamInfo, ResultP2_T* pResultP2)
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


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  ISP End-User-Define Tuning Index.
    //  set user settings
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SETPARAM_P2,
        "[setIspUserIdx] (edge, hue, saturation, brightness, constrast) = (%d->%d, %d->%d, %d->%d, %d->%d, %d->%d)",
        rCamInfo.rIspUsrSelectLevel.eIdx_Edge, static_cast<EIndex_Isp_Edge_T>(rNewP2Param.i4halEdgeMode),
        rCamInfo.rIspUsrSelectLevel.eIdx_Hue, static_cast<EIndex_Isp_Hue_T>(rNewP2Param.i4HueMode),
        rCamInfo.rIspUsrSelectLevel.eIdx_Sat, static_cast<EIndex_Isp_Saturation_T>(rNewP2Param.i4SaturationMode),
        rCamInfo.rIspUsrSelectLevel.eIdx_Bright, static_cast<EIndex_Isp_Brightness_T>(rNewP2Param.i4BrightnessMode),
        rCamInfo.rIspUsrSelectLevel.eIdx_Contrast, static_cast<EIndex_Isp_Contrast_T>(rNewP2Param.i4ContrastMode)
        );

    rCamInfo.rIspUsrSelectLevel.eIdx_Edge     = static_cast<EIndex_Isp_Edge_T>(rNewP2Param.i4halEdgeMode);
    rCamInfo.rIspUsrSelectLevel.eIdx_Hue      = static_cast<EIndex_Isp_Hue_T>(rNewP2Param.i4HueMode);
    rCamInfo.rIspUsrSelectLevel.eIdx_Sat      = static_cast<EIndex_Isp_Saturation_T>(rNewP2Param.i4SaturationMode);
    rCamInfo.rIspUsrSelectLevel.eIdx_Bright   = static_cast<EIndex_Isp_Brightness_T>(rNewP2Param.i4BrightnessMode);
    rCamInfo.rIspUsrSelectLevel.eIdx_Contrast = static_cast<EIndex_Isp_Contrast_T>(rNewP2Param.i4ContrastMode);

    //IspTuningMgr::getInstance().setToneMapMode(m_i4SensorDev, rNewP2Param.u1TonemapMode);
    if (rNewP2Param.u1TonemapMode == MTK_TONEMAP_MODE_CONTRAST_CURVE) {

        MINT32 i = 0;
        std::vector<MFLOAT> vecIn, vecOut;
        MINT32 i4Cnt = rNewP2Param.u4TonemapCurveRedSize/ 2;
        vecIn.resize(i4Cnt);
        vecOut.resize(i4Cnt);
        MFLOAT* pArrayIn = &(vecIn[0]);
        MFLOAT* pArrayOut = &(vecOut[0]);
        const MFLOAT* pCurve = rNewP2Param.pTonemapCurveRed;

        for (i = i4Cnt; i != 0; i--) {

            MFLOAT x, y;
            x = *pCurve++;
            y = *pCurve++;
            *pArrayIn++ = x;
            *pArrayOut++ = y;
            pResultP2->vecTonemapCurveRed.push_back(x);
            pResultP2->vecTonemapCurveRed.push_back(y);
            CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SETPARAM_P2, "[Red]#%d(%f,%f)", rNewP2Param.i4MagicNum, x, y);
        }
        IspTuningMgr::getInstance().setTonemapCurve_Red(m_i4SensorDev, &(vecIn[0]), &(vecOut[0]), &i4Cnt);

        i4Cnt = rNewP2Param.u4TonemapCurveGreenSize/ 2;
        vecIn.resize(i4Cnt);
        vecOut.resize(i4Cnt);
        pArrayIn = &(vecIn[0]);
        pArrayOut = &(vecOut[0]);
        pCurve = rNewP2Param.pTonemapCurveGreen;

        for (i = i4Cnt; i != 0; i--) {

            MFLOAT x, y;
            x = *pCurve++;
            y = *pCurve++;
            *pArrayIn++ = x;
            *pArrayOut++ = y;
            pResultP2->vecTonemapCurveGreen.push_back(x);
            pResultP2->vecTonemapCurveGreen.push_back(y);
            CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SETPARAM_P2, "[Green]#%d(%f,%f)", rNewP2Param.i4MagicNum, x, y);
        }
        IspTuningMgr::getInstance().setTonemapCurve_Green(m_i4SensorDev, &(vecIn[0]), &(vecOut[0]), &i4Cnt);

        i4Cnt = rNewP2Param.u4TonemapCurveBlueSize/ 2;
        vecIn.resize(i4Cnt);
        vecOut.resize(i4Cnt);
        pArrayIn = &(vecIn[0]);
        pArrayOut = &(vecOut[0]);
        pCurve = rNewP2Param.pTonemapCurveBlue;

        for (i = i4Cnt; i != 0; i--) {

            MFLOAT x, y;
            x = *pCurve++;
            y = *pCurve++;
            *pArrayIn++ = x;
            *pArrayOut++ = y;
            pResultP2->vecTonemapCurveBlue.push_back(x);
            pResultP2->vecTonemapCurveBlue.push_back(y);
            CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SETPARAM_P2, "[Blue]#%d(%f,%f)", rNewP2Param.i4MagicNum, x, y);
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

    //update RegBuf
    void* pRegBuf = ((TuningParam*)pTuningBuf)->pRegBuf;
    CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SETPARAM_P2, "[%s] + flow(%d), buf(%p)", __FUNCTION__, flowType, pRegBuf);


    AAA_TRACE_ISP(P2_VLD);
    IspTuningMgr::getInstance().validatePerFrameP2(m_i4SensorDev, flowType, rIspInfo, pTuningBuf);
    AAA_TRACE_END_ISP;

#if CAM3_LSC_FEATURE_EN
    ILscBuf* pLscBuf = NSIspTuning::ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->getP2Buf();
    if (pLscBuf) {

        ((TuningParam*)pTuningBuf)->pLsc2Buf = pLscBuf->getBuf();
    } else {

        ((TuningParam*)pTuningBuf)->pLsc2Buf = NULL;
    }
#endif

    if (!((dip_a_reg_t*)pRegBuf)->DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_LCE_D1_EN) {

        ((TuningParam*)pTuningBuf)->pLcsBuf = NULL;
        ((TuningParam*)pTuningBuf)->pLceshoBuf = NULL;
    }

    ((TuningParam*)pTuningBuf)->pBpc2Buf = IspTuningMgr::getInstance().getDMGItable(m_i4SensorDev, rIspInfo.rCamInfo.fgRPGEnable);

    IspTuningBufCtrl::getInstance(m_i4SensorDev)->updateHint((void*)(&rIspInfo.hint), rIspInfo.rCamInfo.u4Id);

    // debug info
    if (pResultP2) {

        CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SETPARAM_P2, "[%s] get debug info p2 #(%d)", __FUNCTION__, rIspInfo.rCamInfo.u4Id);
        if (0 == pResultP2->vecDbgIspP2.size()) {

            CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SETPARAM_P2, "[%s] Need to allocate P2 result", __FUNCTION__);
            pResultP2->vecDbgIspP2.resize(sizeof(AAA_DEBUG_INFO2_T));
        }

        AAA_DEBUG_INFO2_T& rDbg3AInfo2 = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(pResultP2->vecDbgIspP2.editArray());
        NSIspExifDebug::IspExifDebugInfo_T& rIspExifDebugInfo = rDbg3AInfo2.rISPDebugInfo;
        IspTuningMgr::getInstance().getDebugInfoP2(m_i4SensorDev, rIspInfo, rIspExifDebugInfo, pTuningBuf);

        MBOOL bDump = 0;
        getPropInt("vendor.debug.tuning.dump_capture", &bDump, 0);
        if (!rIspInfo.rCamInfo.fgRPGEnable && bDump) {

            char filename[512];
            sprintf(filename, "/sdcard/debug/p2dbg_dump_capture-%04d.bin", rIspInfo.i4UniqueKey);
            FILE* fp = fopen(filename, "wb");
            if (fp) {

                ::fwrite(rIspExifDebugInfo.P2RegInfo.regDataP2, sizeof(rIspExifDebugInfo.P2RegInfo.regDataP2), 1, fp);
            }
            if (fp) {
                fclose(fp);
            }
        }
    }
    //update mapping info
    CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SETPARAM_P2, "[%s] -", __FUNCTION__);

    return MTRUE;
}

static
inline MBOOL
_dumpDebugInfo(const char* filename, const IMetadata& metaExif)
{
    FILE *fid = std::fopen(filename, "w");
    if (fid != NULL) {

        IMetadata::Memory p3ADbg;
        if (IMetadata::getEntry<IMetadata::Memory>(&metaExif, MTK_3A_EXIF_DBGINFO_AAA_DATA, p3ADbg)) {

            CAM_LOGD("[%s] %s, 3A(%p, %d)", __FUNCTION__, filename, p3ADbg.array(), (MINT32)p3ADbg.size());
            MUINT8 hdr[6] = {0, 0, 0xFF, 0xE6, 0, 0};
            MUINT16 size = (MUINT16)(p3ADbg.size()+2);
            hdr[4] = (size >> 8); // big endian
            hdr[5] = size & 0xFF;
            std::fwrite(hdr, sizeof(MUINT8), 6, fid);
            std::fwrite(p3ADbg.array(), sizeof(uint8_t), p3ADbg.size(), fid);
        }

        IMetadata::Memory pIspDbg;
        if (IMetadata::getEntry<IMetadata::Memory>(&metaExif, MTK_3A_EXIF_DBGINFO_ISP_DATA, pIspDbg)) {

            CAM_LOGD("[%s] %s, ISP(%p, %d)", __FUNCTION__, filename, pIspDbg.array(), (MINT32)pIspDbg.size());
            MUINT8 hdr[4] = {0xFF, 0xE7, 0, 0};
            MUINT16 size = (MUINT16)(pIspDbg.size()+2);
            hdr[2] = (size >> 8);
            hdr[3] = size & 0xFF;
            std::fwrite(hdr, sizeof(MUINT8), 4, fid);
            std::fwrite(pIspDbg.array(), sizeof(uint8_t), pIspDbg.size(), fid);
        }
        std::fclose(fid);
        return MTRUE;
    } else {

        CAM_LOGW("IFileCache open fail <%s>", filename);
        return MFALSE;
    }
}


MBOOL
HalIspImp::
dumpP2DotTuning(ISP_INFO_T& rIspInfo, P2Param_T &rNewP2Param)
{
    MINT32 CaptureDump = 0;
    getPropInt("vendor.debug.camera.dump.p2.debuginfo", &CaptureDump, 0);
    MINT32 PreviewDump = 0;
    getPropInt("vendor.debug.camera.dump.isp.preview", &PreviewDump, 0);
    if ((CaptureDump && rIspInfo.isCapture) || PreviewDump || rNewP2Param.u1DumpExif) {

        char filename[512];
        if (rIspInfo.rCamInfo.u1P2TuningUpdate == 3 || rIspInfo.rCamInfo.u1P2TuningUpdate == 6) {
            genFileName_TUNING(filename, sizeof(filename), &rIspInfo.hint,"lpcnrout1");
        } else if (rIspInfo.rCamInfo.u1P2TuningUpdate == 4 || rIspInfo.rCamInfo.u1P2TuningUpdate == 7) {
            genFileName_TUNING(filename, sizeof(filename), &rIspInfo.hint, "lpcnrout2");
        } else {

            genFileName_TUNING(filename, sizeof(filename), &rIspInfo.hint);
        }
        _dumpDebugInfo(filename, rNewP2Param.rexifMeta);
    }

    return MTRUE;
}

MBOOL
HalIspImp::
_readDumpP2Buf(TuningParam* pTuningBuf, const MetaSet_T& control, MetaSet_T* pResult, NSCam::TuningUtils::FileReadRule &rule,
               ISP_INFO_T* pIspInfo, NSCam::TuningUtils::ETuningFeedbackStage_T stage)
{
    MBOOL ret;
    if (rule.isREADEnable("ISPHAL")) {
        switch(stage) {
            case E_P2_Before_SetParam:

                _readDump(pTuningBuf, control, pResult, pIspInfo, (MINT32)E_Lsc_Output);
                ret = MTRUE;
                break;
            case E_P2_The_End:

                _readDump(pTuningBuf, control, pResult, pIspInfo, (MINT32)E_LPCNR_Output);
                _readDump(pTuningBuf, control, pResult, pIspInfo, (MINT32)E_Tuning_Output);
                _readDump(pTuningBuf, control, pResult, pIspInfo, (MINT32)E_MFB_Output);
                _readDump(pTuningBuf, control, pResult, pIspInfo, (MINT32)E_LTM_CURVE_Output);
                _readDump(pTuningBuf, control, pResult, pIspInfo, (MINT32)E_YNR_MAP_Output);
                ret = MTRUE;
                break;
            default:

                CAM_LOGD("[%s] undefine dump stage", __FUNCTION__);
                ret = MFALSE;
                break;
        }
    } // enable dump
    return ret;
}

MBOOL
HalIspImp::
_dumpP2Buf(TuningParam* pTuningBuf, FileReadRule &rule, ISP_INFO_T& rIspInfo)
{
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
         rule.isDumpEnable("ISPHAL")) {

           MUINT32 u4RegCnt_start = 0; //start address count
           MUINT32 u4RegCnt = 0; // reg count in exif

           sendIspCtrl(EISPCtrl_GetP2TuningInfo, (MINTPTR)&u4RegCnt, (MINTPTR)&u4RegCnt_start);

           if (pTuningBuf->pRegBuf != NULL) {

                const char *ispProfileName = IspTuningMgr::getIspProfileName(rIspInfo.rCamInfo.rMapping_Info.eIspProfile);
                char strTuningFile[512] = {'\0'};
                sprintf(strTuningFile, "/data/vendor/camera_dump/%09d-%04d-%04d-%d-%s.p2buf", rIspInfo.hint.UniqueKey, rIspInfo.hint.RequestNo, rIspInfo.hint.FrameNo, m_i4SensorDev, ispProfileName);
                if (rIspInfo.rCamInfo.u1P2TuningUpdate == 3 || rIspInfo.rCamInfo.u1P2TuningUpdate == 6) {

                    sprintf(strTuningFile, "/data/vendor/camera_dump/%09d-%04d-%04d-%d-%s-%s.p2buf"
                        , rIspInfo.hint.UniqueKey, rIspInfo.hint.RequestNo, rIspInfo.hint.FrameNo, m_i4SensorDev, "lpcnrout1", ispProfileName);
                } else if (rIspInfo.rCamInfo.u1P2TuningUpdate == 4 || rIspInfo.rCamInfo.u1P2TuningUpdate == 7) {

                    sprintf(strTuningFile, "/data/vendor/camera_dump/%09d-%04d-%04d-%d-%s-%s.p2buf"
                        , rIspInfo.hint.UniqueKey, rIspInfo.hint.RequestNo, rIspInfo.hint.FrameNo, m_i4SensorDev, "lpcnrout2", ispProfileName);
                } else {
                    sprintf(strTuningFile, "/data/vendor/camera_dump/%09d-%04d-%04d-%d-%s.p2buf"
                        , rIspInfo.hint.UniqueKey, rIspInfo.hint.RequestNo, rIspInfo.hint.FrameNo, m_i4SensorDev, ispProfileName);
                }

                FILE* fidTuning = fopen(strTuningFile, "wb");
                if (fidTuning) {

                     CAM_LOGD("[p2buf] %s pRegBuf size(%d)",__FUNCTION__, strTuningFile, u4RegCnt*4);
                     fwrite((MUINT32*)pTuningBuf->pRegBuf+u4RegCnt_start, u4RegCnt*4, 1, fidTuning);
                     fclose(fidTuning);
                }
           } else {
                return MFALSE;
           }
    }

    return MTRUE;
}

MUINT32
HalIspImp::
queryTuningSize()
{
    return sizeof(dip_a_reg_t);
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
    setISPInfo(rNewP2Param, pTuningBuf, rIspInfo, 0);

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
}

MBOOL
HalIspImp::
queryISPBufferInfo(Buffer_Info& bufferInfo)
{
    return m_pTuning->queryISPBufferInfo(m_i4SensorDev,bufferInfo);
}

MBOOL
HalIspImp::
_readDump(TuningParam* pTuningBuf, const MetaSet_T& control, MetaSet_T* pResult, ISP_INFO_T* pIspInfo, MINT32 i4Format)
{
#if 1
    FileReadRule rule;

    if (!rule.isREADEnable("ISPHAL") && pIspInfo == NULL) {

        return MFALSE;
    }
    if ((pIspInfo->rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_Capture) ||
        (pIspInfo->rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_Capture_DCE) ||
        (pIspInfo->rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_Capture_DSDN) ||
        (pIspInfo->rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_After_Blend) ||
        (pIspInfo->rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_MFB) ||
        (pIspInfo->rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_Single) ||
        (pIspInfo->rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_Before_Blend) ||
        (pIspInfo->rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_AINR_Main) ||
        (pIspInfo->rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_AINR_MainYUV) ||
        (pIspInfo->rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_AINR_Single)) {

        MINT32 i4ReqNo = 0;
        QUERY_ENTRY_SINGLE(control.halMeta, MTK_PIPELINE_REQUEST_NUMBER, i4ReqNo);
        if (QUERY_ENTRY_SINGLE(control.halMeta, MTK_HAL_REQUEST_INDEX_BSS, i4ReqNo)) {

            CAM_LOGD("[%s] MTK_HAL_REQUEST_INDEX_BSS(%d)",__FUNCTION__, i4ReqNo);
        }
        char strDump[512] = {'\0'};
        std::string strispProfileName_prefix = "EIspProfile_";
        std::string strispProfileName_prefix_postfix = IspTuningMgr::getIspProfileName(pIspInfo->rCamInfo.rMapping_Info.eIspProfile);
        std::string strispProfileName = strispProfileName_prefix + strispProfileName_prefix_postfix;
        MUINT32 u4P2TuningBufferSize = queryTuningSize();

        CAM_LOGD("[%s] i4ReqNo(%d) ispProfileName(%s) format(%d)",__FUNCTION__, i4ReqNo, strispProfileName.c_str(), i4Format);

        if ((i4Format == E_Tuning_Output) || (i4Format == E_LPCNR_Output)) {

            MUINT32 u4RegCnt_start = 0; //start address count
            MUINT32 u4RegCnt = 0; // reg count in exif
            MBOOL bIsLpcnr1 = (pIspInfo->rCamInfo.u1P2TuningUpdate == ELPCNR_8Bit_Pass1 || pIspInfo->rCamInfo.u1P2TuningUpdate == ELPCNR_10Bit_Pass1);
            MBOOL bIsLpcnr2 = (pIspInfo->rCamInfo.u1P2TuningUpdate == ELPCNR_8Bit_Pass2 || pIspInfo->rCamInfo.u1P2TuningUpdate == ELPCNR_10Bit_Pass2);

            sendIspCtrl(EISPCtrl_GetP2TuningInfo, (MINTPTR)&u4RegCnt, (MINTPTR)&u4RegCnt_start);

            if (i4Format == E_Tuning_Output) {
                if (!(bIsLpcnr1 || bIsLpcnr2)) {
                    rule.getFile_P2TUNING(i4ReqNo, strispProfileName.c_str(), strDump, 512, "ISPHAL");
                }
            } else if (i4Format == E_LPCNR_Output) {

                if (bIsLpcnr1) {
                    rule.getFile_LPCNR_TUNING(i4ReqNo, strispProfileName.c_str(), strDump, 512, "lpcnrout1");
                }
                if (bIsLpcnr2) {

                    rule.getFile_LPCNR_TUNING(i4ReqNo, strispProfileName.c_str(), strDump, 512, "lpcnrout2");
                }
            }

            FILE* fidTuning = fopen(strDump, "rb");
            if (fidTuning) {
                if (u4P2TuningBufferSize >= (u4RegCnt_start+u4RegCnt)*4) {
                    CAM_LOGD("[%s] i4Format(%d) %s pRegBuf size(%d), u4RegCnt(%d), u4RegCnt_start(%d)",__FUNCTION__, i4Format, strDump, u4P2TuningBufferSize, u4RegCnt, u4RegCnt_start);
                    fread((MUINT32*)pTuningBuf->pRegBuf+u4RegCnt_start, u4RegCnt*4, 1, fidTuning);
                } else {

                    CAM_LOGD("[%s] i4Format(%d) u4RegCnt_end*4(%d) > P2 Buffer Size(%d) ",
                        __FUNCTION__, i4Format, strDump, ((u4RegCnt_start+u4RegCnt))*4, u4P2TuningBufferSize);
                }
                fclose(fidTuning);
            }
        }

        if (i4Format == E_Lsc_Output) {

            rule.getFile_LSC(i4ReqNo, strispProfileName.c_str(), strDump, 512, "ISPHAL");
            FILE* fidLscRead = fopen(strDump, "rb");
            if (fidLscRead) {

                CAM_LOGD("[%s] %s ILscTbl::lscdata",__FUNCTION__, strDump, pIspInfo->rLscData.size());
                fread(pIspInfo->rLscData.data(), pIspInfo->rLscData.size(), 1, fidLscRead);
                fclose(fidLscRead);
            }
        }
        if (i4Format == E_MFB_Output && (pIspInfo->rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_MFB)) {

            MINT32 i4MfbSize = 0;
            sendIspCtrl(EISPCtrl_GetMfbSize, (MINTPTR)&i4MfbSize, (MINTPTR)NULL);
            if (i4MfbSize == 0) {

                return MTRUE;
            }
            rule.getFile_MFB(i4ReqNo, strispProfileName.c_str(), strDump, 512);
            FILE* fidTuning = fopen(strDump, "rb");
            if (fidTuning) {

                CAM_LOGD("[%s] %s pMfbBuf size(%d)",__FUNCTION__, strDump, i4MfbSize);
                fread(pTuningBuf->pMfbBuf, i4MfbSize, 1, fidTuning);
                fclose(fidTuning);
            }
        }

        if (i4Format == E_LTM_CURVE_Output) {

            MUINT32 u4RegCnt_start = 0; //start address count
            MUINT32 u4RegCnt = 0; // reg count in exif

            sendIspCtrl(EISPCtrl_GetLtmCurve, (MINTPTR)&u4RegCnt, (MINTPTR)&u4RegCnt_start);
            rule.getFile_LTMCURVE(i4ReqNo, strispProfileName.c_str(), strDump, 512, "ISPHAL");

            FILE* fidTuning = fopen(strDump, "rb");
            if (fidTuning) {

                if (u4P2TuningBufferSize >= (u4RegCnt_start+u4RegCnt)*4) {

                    CAM_LOGD("[%s] %s pRegBuf size(%d), LtmCurveCnt(%d), u4LtmCurveCnt_start(%d)",__FUNCTION__, strDump, u4P2TuningBufferSize, u4RegCnt, u4RegCnt_start);
                    fread((MUINT32*)pTuningBuf->pRegBuf+u4RegCnt_start, u4RegCnt*4, 1, fidTuning);
                } else {

                    CAM_LOGD("[%s] LTM_end(%d) > P2 Buffer Size(%d) ",
                        __FUNCTION__, strDump, ((u4RegCnt_start+u4RegCnt))*4, u4P2TuningBufferSize);
                }
                fclose(fidTuning);
            }

        }

        if (i4Format == E_YNR_MAP_Output) {

            if (pTuningBuf->pFaceAlphaBuf != NULL){

                rule.getFile_YNR_MAP(i4ReqNo, strispProfileName.c_str(), (IImageBuffer*)pTuningBuf->pFaceAlphaBuf, "ISPHAL", m_i4SensorIdx);
                CAM_LOGD("[readdump] facemap read done");
            }
        }

    }
#endif
    return MTRUE;
}

