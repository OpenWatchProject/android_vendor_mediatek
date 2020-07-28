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
#define LOG_TAG "paramctrl_lifetime"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <isp_tuning.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_idx.h>
#include <isp_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_mgr_helper.h>
#include <pca_mgr.h>
#include <gma_mgr.h>
#include <lce_mgr.h>
#include <ispfeature.h>
#include <ccm_mgr.h>
#include <lsc/ILscMgr.h>
#include "paramctrl.h"
//#include <hwutils/CameraProfile.h>
#include <mtkcam/drv/IHalSensor.h>
#include <nvbuf_util.h>

#include <sys/mman.h>
#include <fcntl.h>

using namespace NSIspTuning;
using namespace NSIspTuningv3;
using namespace NSCam;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IParamctrl*
IParamctrl::createInstance(ESensorDev_T const eSensorDev, MINT32 const i4SensorIdx)
{
    return Paramctrl::getInstance(eSensorDev, i4SensorIdx);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Paramctrl*
Paramctrl::
getInstance(ESensorDev_T const eSensorDev, MINT32 const i4SensorIdx)
{
    CAM_LOGD("%s(): eSensorDev = %d, i4SensorIdx = %d\n", __FUNCTION__, eSensorDev, i4SensorIdx);
    Paramctrl* pParamctrl = MNULL;
    SensorStaticInfo rSensorStaticInfo;
    IHalSensorList*const pIHalSensorList = MAKE_HalSensorList();

    //IHalSensor* pIHalSensor = pIHalSensorList->createSensor("paramctrl_lifetime", i4SensorIdx);
    NVRAM_CAMERA_ISP_PARAM_STRUCT*  pNvram_Isp = MNULL;


    int err;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_ISP, eSensorDev, (void*&)pNvram_Isp);
    if(err!=0)
    {
        CAM_LOGE("NvBufUtil getBufAndRead fail\n");
        goto lbExit;
    }

#if 1
    CAM_LOGD("sizeof(pNvram_Isp->SensorId) = %zu\n", sizeof(pNvram_Isp->SensorId));
    CAM_LOGD("sizeof(pNvram_Isp->ISPComm) = %zu\n", sizeof(pNvram_Isp->ISPComm));
    CAM_LOGD("sizeof(pNvram_Isp->ISPPca) = %zu\n", sizeof(pNvram_Isp->ISPColorTbl));
    CAM_LOGD("sizeof(pNvram_Isp->ISPRegs) = %zu\n", sizeof(pNvram_Isp->ISPRegs));
    //CAM_LOGD("sizeof(pNvram_Isp->ISPMulitCCM) = %d\n", sizeof(pNvram_Isp->ISPMulitCCM));
    CAM_LOGD("sizeof(pNvram_Isp) = %zu\n", sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT));
#endif


   /*
    // ISP NVRAM
    if (MERR_OK != NvramDrvMgr::getInstance().init(eSensorDev))
    {
        goto lbExit;
    }

    NvramDrvMgr::getInstance().getRefBuf(pNvram_Isp);

    if (! pNvram_Isp)
    {
        CAM_LOGE("[createInstance] (pNvram_Isp) = (%p)", pNvram_Isp);
        goto lbExit;
    }

#if 1
    CAM_LOGD("sizeof(pNvram_Isp->Version) = %d\n", sizeof(pNvram_Isp->Version));
    CAM_LOGD("sizeof(pNvram_Isp->SensorId) = %d\n", sizeof(pNvram_Isp->SensorId));
    CAM_LOGD("sizeof(pNvram_Isp->ISPComm) = %d\n", sizeof(pNvram_Isp->ISPComm));
    CAM_LOGD("sizeof(pNvram_Isp->ISPPca) = %d\n", sizeof(pNvram_Isp->ISPPca));
    CAM_LOGD("sizeof(pNvram_Isp->ISPRegs) = %d\n", sizeof(pNvram_Isp->ISPRegs));
    CAM_LOGD("sizeof(pNvram_Isp->ISPMfbMixer) = %d\n", sizeof(pNvram_Isp->ISPMfbMixer));
    CAM_LOGD("sizeof(pNvram_Isp->ISPMulitCCM) = %d\n", sizeof(pNvram_Isp->ISPMulitCCM));
    CAM_LOGD("sizeof(pNvram_Isp) = %d\n", sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT));
#endif
*/

    //  Sensor info
    //SensorDynamicInfo rSensorDynamicInfo;
    MUINT32 u4SensorID;
    ESensorTG_T eSensorTG;

    switch  ( eSensorDev )
    {
    case ESensorDev_Main:
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
        //pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_MAIN, &rSensorDynamicInfo);
        break;
    case ESensorDev_Sub:
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
        //pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_SUB, &rSensorDynamicInfo);
        break;
    case ESensorDev_MainSecond:
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
        //pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorDynamicInfo);
        break;
    case ESensorDev_SubSecond:
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB_2, &rSensorStaticInfo);
        //pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorDynamicInfo);
        break;
    default:    //  Shouldn't happen.
        CAM_LOGE("Invalid sensor device: %d", eSensorDev);
        goto lbExit;
    }

    u4SensorID = rSensorStaticInfo.sensorDevID;

    CAM_LOGD("u4SensorID = %d\n", u4SensorID);

    eSensorTG = ESensorTG_1; // just default setting, will be updated later

    //  Here, pIspParam must be legal.
    pParamctrl = new Paramctrl(
        eSensorDev, i4SensorIdx, u4SensorID, eSensorTG, pNvram_Isp
    );

lbExit:
    //NvramDrvMgr::getInstance().uninit();

    //if  ( pIHalSensor )
    //    pIHalSensor->destroyInstance("paramctrl_lifetime");

    return  pParamctrl;
}


void
Paramctrl::
destroyInstance()
{
    delete this;
}


Paramctrl::
Paramctrl(
    ESensorDev_T const eSensorDev,
    MINT32 const i4SensorIdx,
    MUINT32 const u4SensorID,
    ESensorTG_T const eSensorTG,
    NVRAM_CAMERA_ISP_PARAM_STRUCT*const pNvram_Isp
)
    : IParamctrl(eSensorDev)
    , m_u4ParamChangeCount(0)
    , m_fgDynamicTuning(MTRUE)
    , m_fgDynamicCCM(MTRUE)
    , m_fgDynamicBypass(MFALSE)
    , m_fgDynamicShading(MTRUE)
    , m_fgEnableRPG(MFALSE)
    , m_backup_OBCInfo()
    , m_RRZInfo()
    , m_eIdx_Effect(MTK_CONTROL_EFFECT_MODE_OFF)
    , m_eSensorDev(eSensorDev)
    , m_eOperMode(EOperMode_Normal)
    , m_rIspExifDebugInfo()
    , m_rIspCamInfo()
    , m_pIspTuningCustom(IspTuningCustom::createInstance(eSensorDev, u4SensorID))
    , m_pLCSBuffer(NULL)
    , m_rIspParam(*pNvram_Isp)
    , m_rIspComm(m_rIspParam.ISPComm)
    , m_IspNvramMgr(&m_rIspParam.ISPRegs)
    , m_pPcaMgr(PcaMgr::createInstance(eSensorDev, m_rIspParam.ISPColorTbl.ISPColor))
    , m_pCcmMgr(CcmMgr::createInstance(eSensorDev, m_rIspParam.ISPColorTbl.ISPMulitCCM, m_pIspTuningCustom))
    , m_pGgmMgr(GgmMgr::createInstance(eSensorDev))
    , m_pNvram_Shading(NULL)
    , m_pLscMgr(ILscMgr::createInstance(eSensorDev, i4SensorIdx))
    , m_pTuning(TuningMgr::getInstance(i4SensorIdx))
    , m_Lock()
    , m_bDebugEnable(MFALSE)
    , m_bProfileEnable(MFALSE)
    , m_eSensorTG(eSensorTG)
    , m_ePCAMode(EPCAMode_180BIN)
    , m_i4SensorIdx(i4SensorIdx)
    , m_i4CamMode(0) // normal mode
    , m_u4SwnrEncEnableIsoThreshold(0)
    , m_pAaaTimer(MNULL)
    , m_eColorCorrectionMode(MTK_COLOR_CORRECTION_MODE_FAST)
    , m_i4SubsampleCount(1)
    , m_IspInterpCtrl(1)
    , m_IspFdFeatureCtrl(1)
    , m_DualPD_PureRaw(MFALSE)
    , m_bFlkEnable(MTRUE)
    , m_pFeatureCtrl(static_cast<ISP_FEATURE_TOP_CTL*>(m_pIspTuningCustom->get_feature_control(m_eSensorDev)))
    , m_u4RawFmtType(SENSOR_RAW_Bayer)
    , m_u4PDFmtType(0)
    , m_bSmoothColor_FirstTimeBoot(MTRUE)
    , m_fVCoreThrottling(-1)
    , m_pVCoreThrottling(NULL)
{
    m_pGmaMgr = GmaMgr::createInstance(eSensorDev, m_rIspParam.ISPToneMap, &(m_rIspParam.ISPToneMap.rGmaParam[0]));
    m_pLceMgr = LceMgr::createInstance(eSensorDev, &(m_rIspParam.ISPToneMap.rLceParam));

    m_fVCoreThrottling = open("/proc/driver/thermal/clNR_status", O_RDWR);
    if(m_fVCoreThrottling < 0) {
        CAM_LOGE("open /proc/driver/thermal/clNR_status failed, disable NR cooler\n");
    } else {
        m_pVCoreThrottling = (char*)mmap(NULL, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, m_fVCoreThrottling, 0);
        if (m_pVCoreThrottling == (char*)MAP_FAILED) {
            CAM_LOGE("mmap failed, disable NR cooler\n");
            m_pVCoreThrottling = NULL;
        }
    }
    memset(&m_ISP_INT, 0, sizeof(ISP_INT_INFO_T));
    memset(&m_ISP_DYM, 0, sizeof(ISP_DYNAMIC_INFO_T));
    memset(&m_ISP_ALG_VER, 0, sizeof(ISP_SWALGO_VER_T));
    memset(&m_RRZInfo, 0, sizeof(ISP_NVRAM_RRZ_T));
    memset(&m_ParamIdx_P1, 0, sizeof(ISP_NVRAM_REG_INDEX_T));
    memset(&m_ParamIdx_P2, 0, sizeof(ISP_NVRAM_REG_INDEX_T));

    memset(&m_GmaExifInfo, 0, sizeof(NSIspExifDebug::IspExifDebugInfo_T::IspGmaInfo));
    memset(&m_LceExifInfo, 0, sizeof(NSIspExifDebug::IspExifDebugInfo_T::IspLceInfo));
    memset(&m_fd_tuning_info, 0, sizeof(CAMERA_TUNING_FD_INFO_T));
    memset(&m_SmoothColor_Prv, 0, sizeof(ISP_NVRAM_COLOR_T));
}


Paramctrl::
~Paramctrl()
{
    if (m_pVCoreThrottling != NULL) munmap(m_pVCoreThrottling, PAGE_SIZE);
    if (m_fVCoreThrottling != -1) close(m_fVCoreThrottling);
}

MERROR_ENUM
Paramctrl::
init(MINT32 const i4SubsampleCount)
{
    MERROR_ENUM err = MERR_OK;

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.paramctrl.enable", value, "0");
    m_bDebugEnable = atoi(value);

    property_get("vendor.profile.paramctrl.enable", value, "0");
    m_bProfileEnable = atoi(value);

    //  (1) Force to assume all params have chagned and different.
    m_u4ParamChangeCount = 1;
    m_i4SubsampleCount = i4SubsampleCount;

    //  (2) Init ISP driver manager & tuning manager
    m_pTuning->init(LOG_TAG, i4SubsampleCount);

    m_pLscMgr->init();

    //  (3) validateFrameless() is invoked
    //err = validateFrameless();
    setZoomRatio(100);
    //check custom iso setting
    if (!check_ISO_env_info())
    {
        err = MERR_CUSTOM_ISO_ENV_ERR;
        goto lbExit;
    }

    if (!check_CT_env_info())
    {
        err = MERR_CUSTOM_CT_ENV_ERR;
        goto lbExit;
    }


    //  (4) however, is it needed to invoke validatePerFrame() in init()?
    //  or just invoke it only when a frame comes.
    //err = validatePerFrame(MTRUE);

    if (m_bProfileEnable) {
        m_pAaaTimer = new AaaTimer;
    }

lbExit:
    if  ( MERR_OK != err )
    {
        uninit();
    }

    CAM_LOGD("[-Paramctrl::init]err(%X)", err);
    return  err;
}


MERROR_ENUM
Paramctrl::
uninit()
{
    CAM_LOGD("[+uninit]");

    //  Uninit ISP driver manager & tuning manager
    m_pTuning->uninit(LOG_TAG);

    ISP_MGR_BNR2_T::getInstance(m_eSensorDev).unlockBPCIBuf();//unlock bpci table
    m_pLscMgr->uninit();

    if (m_bProfileEnable) {
        delete m_pAaaTimer;
        m_pAaaTimer = MNULL;
    }

    return  MERR_OK;
}

