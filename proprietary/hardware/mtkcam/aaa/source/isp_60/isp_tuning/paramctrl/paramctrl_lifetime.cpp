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
//#include <awb_param.h>
//#include <ae_param.h>
//#include <af_param.h>
//#include <flash_param.h>
#include <isp_tuning.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_mgr_config.h>
#include <isp_mgr_helper.h>
#include <gma_mgr.h>
#include <lce_mgr.h>
#include <dce_mgr.h>
#include <ispfeature.h>
#include <ccm_mgr.h>
#include <lsc/ILscMgr.h>
#include "paramctrl.h"
//#include <hwutils/CameraProfile.h>
#include <mtkcam/drv/IHalSensor.h>
#include <nvbuf_util.h>

#include <sys/mman.h>
#include <fcntl.h>

#include "private/PDTblGen.h"
#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuning;
using namespace NSCam;


struct Instance_T {
     Paramctrl* pParamctrl;
     int  InstanceDone;
     Instance_T(): pParamctrl(nullptr),InstanceDone(0){}
};
static std::array<Instance_T, SENSOR_IDX_MAX>  gMultiton;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IParamctrl*
IParamctrl::createInstance(MUINT32 const eSensorDev, MINT32 const i4SensorIdx)
{
    return Paramctrl::getInstance(eSensorDev, i4SensorIdx);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Paramctrl*
Paramctrl::
getInstance(MUINT32 const eSensorDev, MINT32 const i4SensorIdx)
{
    CAM_LOGD("%s(): eSensorDev = %d, i4SensorIdx = %d\n", __FUNCTION__, eSensorDev, i4SensorIdx);

	Instance_T & rSingleton = gMultiton[i4SensorIdx];
	if (rSingleton.InstanceDone == 0)
	{
		NVRAM_CAMERA_ISP_PARAM_STRUCT*  pNvram_Isp = MNULL;

		int err;
		err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_ISP, eSensorDev, (void*&)pNvram_Isp);
		if(err!=0)
		{
			CAM_LOGE("NvBufUtil getBufAndRead fail\n");
			return NULL;
		}

		//::IPDTblGen::getInstance()->start(eSensorDev, i4SensorIdx);

#if 1
		CAM_LOGD("sizeof(pNvram_Isp->SensorId) = %zu\n", sizeof(pNvram_Isp->SensorId));
		CAM_LOGD("sizeof(pNvram_Isp->ISPComm) = %zu\n", sizeof(pNvram_Isp->ISPComm));
		CAM_LOGD("sizeof(pNvram_Isp->ISPPca) = %zu\n", sizeof(pNvram_Isp->ISPColorTbl));
		CAM_LOGD("sizeof(pNvram_Isp->ISPRegs) = %zu\n", sizeof(pNvram_Isp->ISPRegs));
		//CAM_LOGD("sizeof(pNvram_Isp->ISPMulitCCM) = %d\n", sizeof(pNvram_Isp->ISPMulitCCM));
		CAM_LOGD("sizeof(pNvram_Isp) = %zu\n", sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT));
#endif


		auto u4SensorID = NS3Av3::mapSensorIdxToSensorId(i4SensorIdx);

		CAM_LOGD("u4SensorID = %d\n", u4SensorID);
		ESensorTG_T eSensorTG = ESensorTG_1; // just default setting, will be updated later

		rSingleton.pParamctrl  = new Paramctrl(eSensorDev, i4SensorIdx, u4SensorID, eSensorTG, pNvram_Isp);
		rSingleton.InstanceDone = 1;
	}
	else
	{
		CAM_LOGD("i4SensorIdx(%d) has already had Paramctrl\n", i4SensorIdx);        
	}
       if (rSingleton.pParamctrl)
                    return gMultiton[i4SensorIdx].pParamctrl; 
       else
            return NULL;

}

void
Paramctrl::
destroyInstance()
{
#if 0
    switch  ( m_eSensorDev )
    {
    case ESensorDev_Main:
        pParamctrl_Main=NULL;
        break;
    case ESensorDev_Sub:
        pParamctrl_Sub=NULL;
        break;
    case ESensorDev_MainSecond:
        pParamctrl_Main2=NULL;
        break;
    case ESensorDev_SubSecond:
        pParamctrl_Sub2=NULL;
        break;
    case ESensorDev_MainThird:
        pParamctrl_Main3=NULL;
        break;
    default:    //  Shouldn't happen.
        CAM_LOGE("Invalid sensor device: %d", m_eSensorDev);
    }

    delete this;
#endif
}


Paramctrl::
Paramctrl(
    MUINT32 const eSensorDev,
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
    , m_eSensorDev(eSensorDev)
    , m_eOperMode(EOperMode_Normal)
    , m_u4P1DirectYUV_PortInfo(0)
    , m_rIspExifDebugInfo()
    , m_rIspCamInfo()
    , m_pIspTuningCustom(IspTuningCustom::createInstance(eSensorDev, u4SensorID))
    , m_pLCESBuffer(NULL)
    , m_pDCESBuffer(NULL)
    , m_rIspParam(*pNvram_Isp)
    , m_rIspComm(m_rIspParam.ISPComm)
    , m_pCcmMgr(CcmMgr::createInstance(eSensorDev, m_rIspParam.ISPColorTbl, m_pIspTuningCustom))
    , m_pGgmMgr(GgmMgr::createInstance(eSensorDev))
    , m_pNvram_Shading(NULL)
    , m_pLscMgr(ILscMgr::createInstance(eSensorDev, i4SensorIdx))
    , m_pTuning(TuningMgr::getInstance(i4SensorIdx))
    , m_Lock()
    , m_bDebugEnable(MFALSE)
    , m_bProfileEnable(MFALSE)
    , m_eSensorTG(eSensorTG)
    , m_i4SensorIdx(i4SensorIdx)
    , m_pAaaTimer(MNULL)
    , m_eColorCorrectionMode(MTK_COLOR_CORRECTION_MODE_FAST)
    , m_i4SubsampleCount(1)
    , m_IspInterpCtrl(1)
    , m_pFeatureCtrl(static_cast<ISP_FEATURE_TOP_CTL*>(m_pIspTuningCustom->get_feature_control(m_eSensorDev)))
    , m_u4RawFmtType(SENSOR_RAW_Bayer)
    , m_bSmoothColor_FirstTimeBoot(MTRUE)
    , m_bFirstPrepare(MTRUE)
    , m_i4PreISO_L(0)
    , m_i4PreRealISO(0)
    , m_i4PreZoomRatio_x100(0)
    , m_bPreRrzOn(MFALSE)
    , m_isoThresh(5)
    , m_bUseIsoThreshEnable(MFALSE)
    , m_i4PrePrevProfile(static_cast<MINT32>(EIspProfile_Preview))
    , m_fVCoreThrottling(-1)
    , m_pVCoreThrottling(NULL)
    , m_rIdxCache(eSensorDev)
    , m_pResultPoolObj(IResultPool::getInstance(eSensorDev))
    , m_ISO_Group_P1(EISO_GROUP_00)
    , m_ISO_Group_P2(EISO_GROUP_00)
    , m_ERawPath(NSCam::NSIoPipe::NSCamIOPipe::ENPipe_UNKNOWN)
    , m_i4RequestNum(0)
    , m_SW_YNR_LTM_Ver(0)
    , m_bSlave(MFALSE)
{
    m_pGmaMgr = GmaMgr::createInstance(eSensorDev, &(m_rIspParam.ISPToneMap.GMA[0]) );

    m_pLceMgr = LceMgr::createInstance(eSensorDev, &m_rIspParam.ISPToneMap);

    m_pDceMgr = DceMgr::createInstance(eSensorDev, &(m_rIspParam.ISPToneMap.DCE[0]) );

    m_pCcuIsp= NSCcuIf::ICcuCtrlLtm::getInstance(i4SensorIdx, (ESensorDev_T)eSensorDev);

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
    memset(&m_ISP_ALG_VER, 0, sizeof(ISP_SWALGO_VER_T));
    memset(&m_RRZInfo, 0, sizeof(ISP_NVRAM_RRZ_T));

    memset(&m_DebugCTL_Disable, 0, sizeof(ISP_DEBUG_CONTROL_T));

    memset(&m_GmaExifInfo, 0, sizeof(NSIspExifDebug::IspGmaInfo));
    memset(&m_LceExifInfo, 0, sizeof(NSIspExifDebug::IspLceInfo));
    memset(&m_DceExifInfo, 0, sizeof(NSIspExifDebug::IspDceInfo));
    memset(&m_SmoothColor_Prv, 0, sizeof(ISP_NVRAM_COLOR_T));
    memset(&m_sw_ynr, 0, sizeof(ISP_NVRAM_YNR_LCE_LINK_T));
    memset(&m_ynr_fd, 0, sizeof(ISP_NVRAM_YNR_FD_T));
    memset(&m_ISP_Sync_Info, 0, sizeof(DUAL_ISP_SYNC_INFO_T));

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
    ISP_MGR_YNR_T::getInstance(m_eSensorDev).createAlphaMap(ISP_MGR_YNR::EYNR_D1, 320, 320);

    setZoomRatio(100);

    //set lv value to custom instance
    m_pIspTuningCustom->set_nvram_value(&m_rIspParam);

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
reinitP1TuningMgr(MINT32 const i4SubsampleCount)
{
    MBOOL retUninit = MFALSE;
    MBOOL retInit   = MFALSE;

    //reinit TuningMgr
    if(m_pTuning)
    {
        int retUninit = m_pTuning->uninit(LOG_TAG);
        int retInit = m_pTuning->init(LOG_TAG, i4SubsampleCount);
    }

    CAM_LOGE_IF((retUninit==MFALSE || retInit==MFALSE), "[%s] MERR_BAD_ISP_DRV", __func__);

    return (retUninit && retInit)?MERR_OK:MERR_BAD_ISP_DRV;
}

MERROR_ENUM
Paramctrl::
uninit()
{
    CAM_LOGD("[+uninit]");

    //  Uninit ISP driver manager & tuning manager
    m_pTuning->uninit(LOG_TAG);
    ISP_MGR_YNR_T::getInstance(m_eSensorDev).releaseAlphaMap();

    //m_pLscMgr->uninit();

    if (m_bProfileEnable) {
        delete m_pAaaTimer;
        m_pAaaTimer = MNULL;
    }

    return  MERR_OK;
}

