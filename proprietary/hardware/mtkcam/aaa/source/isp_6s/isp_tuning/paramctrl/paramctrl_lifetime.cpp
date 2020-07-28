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

#include "property_utils.h"
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

using namespace NSIspTuning;
using namespace NSCam;

static Paramctrl* pParamctrl_Main = MNULL;
static Paramctrl* pParamctrl_Sub = MNULL;
static Paramctrl* pParamctrl_Main2 = MNULL;
static Paramctrl* pParamctrl_Sub2 = MNULL;
static Paramctrl* pParamctrl_Main3 = MNULL;

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
    Paramctrl** ppParamctrl = NULL;
    SensorStaticInfo rSensorStaticInfo;
    IHalSensorList*const pIHalSensorList = MAKE_HalSensorList();

    NVRAM_CAMERA_ISP_PARAM_STRUCT*  pNvram_Isp = MNULL;

    int err;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_ISP, eSensorDev, (void*&)pNvram_Isp);
    if(err!=0)
    {
        CAM_LOGE("NvBufUtil getBufAndRead fail\n");
        goto lbExit;
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

    switch  ( eSensorDev )
    {
    case ESensorDev_Main:
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
        ppParamctrl=&pParamctrl_Main;
        break;
    case ESensorDev_Sub:
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
        ppParamctrl=&pParamctrl_Sub;
        break;
    case ESensorDev_MainSecond:
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
        ppParamctrl=&pParamctrl_Main2;
        break;
    case ESensorDev_SubSecond:
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB_2, &rSensorStaticInfo);
        ppParamctrl=&pParamctrl_Sub2;
        break;
    case ESensorDev_MainThird:
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_3, &rSensorStaticInfo);
        ppParamctrl=&pParamctrl_Main3;
        break;
    default:    //  Shouldn't happen.
        CAM_LOGE("Invalid sensor device: %d", eSensorDev);
        goto lbExit;
    }

    u4SensorID = rSensorStaticInfo.sensorDevID;

    CAM_LOGD("u4SensorID = %d\n", u4SensorID);

    if(*ppParamctrl==NULL)
    {
        CAM_LOGD("%s():life createInstance, eSensorDev = %d, i4SensorIdx = %d\n", __FUNCTION__, eSensorDev, i4SensorIdx);
        (*ppParamctrl) = new Paramctrl(
        	eSensorDev, i4SensorIdx, u4SensorID, pNvram_Isp
        );
    }

lbExit:
    //NvramDrvMgr::getInstance().uninit();

    //if  ( pIHalSensor )
    //    pIHalSensor->destroyInstance("paramctrl_lifetime");
    if(ppParamctrl)
        return (*ppParamctrl);
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
    ESensorDev_T const eSensorDev,
    MINT32 const i4SensorIdx,
    MUINT32 const u4SensorID,
    NVRAM_CAMERA_ISP_PARAM_STRUCT*const pNvram_Isp
)
    : IParamctrl(eSensorDev)
    , m_u4ParamChangeCount(0)
    , m_fgDynamicTuning(MTRUE)
    , m_fgDynamicCCM(MTRUE)
    , m_fgDynamicBypass(MFALSE)
    , m_fgDynamicShading(MTRUE)
    , m_RRZInfo()
    , m_eSensorDev(eSensorDev)
    , m_eOperMode(EOperMode_Normal)
    , m_u4P1DirectYUV_PortInfo(0)
    , m_rIspExifDebugInfo()
    , m_pIspTuningCustom(IspTuningCustom::createInstance(eSensorDev, u4SensorID))
    , m_pLCESBuffer(NULL)
    , m_pLCESHOBuffer(NULL)
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
    , m_i4SensorIdx(i4SensorIdx)
    , m_pAaaTimer(MNULL)
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
{
    m_pGmaMgr = GmaMgr::createInstance(eSensorDev, &(m_rIspParam.ISPToneMap.GMA[0]) );

    m_pLceMgr = LceMgr::createInstance(eSensorDev, &(m_rIspParam.ISPToneMap.LCE[0]) );

    m_pDceMgr = DceMgr::createInstance(eSensorDev, &(m_rIspParam.ISPToneMap.DCE[0]) );

    m_pCcuIsp= NSCcuIf::ICcuCtrlLtm::getInstance(i4SensorIdx, eSensorDev);

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
    int err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_FEATURE, m_eSensorDev, (void*&)m_pFeatureParam);
	if (err!=0) {
		CAM_LOGE("NvBufUtil getBufAndRead fail");
	}

    // init the module list for scalabilty design
    initModuleList();

}


Paramctrl::
~Paramctrl()
{
    if (m_pVCoreThrottling != NULL) munmap(m_pVCoreThrottling, PAGE_SIZE);
    if (m_fVCoreThrottling != -1) close(m_fVCoreThrottling);

    //uninit the module list for scalabilty design
    uninitModuleList();
}

MERROR_ENUM
Paramctrl::
init(MINT32 const i4SubsampleCount)
{
    MERROR_ENUM err = MERR_OK;

    getPropInt("vendor.debug.paramctrl.enable", &m_bDebugEnable, 0);

    getPropInt("vendor.profile.paramctrl.enable", &m_bProfileEnable, 0);


    //  (1) Force to assume all params have chagned and different.
    m_u4ParamChangeCount = 1;
    m_i4SubsampleCount = i4SubsampleCount;

    //  (2) Init ISP driver manager & tuning manager
    m_pTuning->init(LOG_TAG, i4SubsampleCount);
    ISP_MGR_YNR_T::getInstance(m_eSensorDev).createAlphaMap(ISP_MGR_YNR::EYNR_D1, 320, 320);

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

    CAM_LOGD("[%s] err(%X)", __FUNCTION__, err);
    return  err;
}

MERROR_ENUM
Paramctrl::
initModuleList()
{

#define MAKE_ISP_MODULE_LIST(DECISION_FUNC, SUB_SAMPLE, GET_FUNC) \
{ \
    std::bind(&Paramctrl::DECISION_FUNC, this, std::placeholders::_1), \
    SUB_SAMPLE, \
    std::bind(&Paramctrl::GET_FUNC   , this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) \
}

    unordered_map<const char *, MBOOL> availableList;
    m_pIspTuningCustom->get_feature_scalibity(availableList);

    for(auto eachISPModule : availableList)
    {
        CAM_LOGD("[%s] Add module (%s) to module list", __FUNCTION__, eachISPModule.first);
        moduleList[string(eachISPModule.first)].isAvaiable = eachISPModule.second;
    }

    //start of p1
    moduleList["OBC"].p1List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P1_OBC, ISP_MGR_OBC::EOBC_R1, getNvram_PerFrame_OBC));

    //TO-DO replace real dec for FRZ and RRZ
    moduleList["FRZ"].p1List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P1_OBC, ISP_MGR_FRZ::EFRZ_R1, getNvram_PerFrame_FRZ));
    moduleList["RRZ"].p1List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P1_OBC, ISP_MGR_RRZ::ERRZ_R1, getNvram_PerFrame_RRZ));

    moduleList["BPC"].p1List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P1_BPC, ISP_MGR_BPC::EBPC_R1, getNvram_PerFrame_BPC));
    moduleList["CT"].p1List.push_back( MAKE_ISP_MODULE_LIST(decision_PerFrame_P1_CT , ISP_MGR_BPC::EBPC_R1, getNvram_PerFrame_CT ));
    moduleList["PDC"].p1List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P1_PDC, ISP_MGR_BPC::EBPC_R1, getNvram_PerFrame_PDC));
    moduleList["DGN"].p1List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P1_DGN, ISP_MGR_DGN::EDGN_R1, getNvram_PerFrame_Dummy));
    moduleList["LSC"].p1List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P1_LSC, ISP_MGR_LSC::ELSC_R1, getNvram_PerFrame_Dummy));
    moduleList["WB"].p1List.push_back( MAKE_ISP_MODULE_LIST(decision_PerFrame_P1_WB , ISP_MGR_WB::EWB_R1  , getNvram_PerFrame_Dummy));
    moduleList["LTM"].p1List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P1_LTM, ISP_MGR_LTM::ELTM_R1, getNvram_PerFrame_Dummy));
    //HLR must be after LTM
    moduleList["HLR"].p1List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P1_HLR, ISP_MGR_HLR::EHLR_R1, getNvram_PerFrame_Dummy));

    //Direct YUV
    moduleList["SLK"].p1List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P1_SLK, ISP_MGR_SLK::ESLK_R1, getNvram_PerFrame_Dummy)); //must after LSC
    moduleList["DM"].p1List.push_back( MAKE_ISP_MODULE_LIST(decision_PerFrame_P1_DM,  ISP_MGR_DM::EDM_R1,   getNvram_PerFrame_DM));
    moduleList["FLC"].p1List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P1_FLC, ISP_MGR_FLC::EFLC_R1, getNvram_PerFrame_FLC));
    //not const
    //TO-DO make the interface as same as other p1 modules
    moduleList["CCM"].p1List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P1_CCM, ISP_MGR_CCM::ECCM_R1, getNvram_PerFrame_CCM_P1));
    //LCES must be after CCM
    moduleList["LCES"].p1List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P1_LCES,ISP_MGR_LCES::ELCES_R1, getNvram_PerFrame_LCES));
    moduleList["GGM"].p1List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P1_GGM, ISP_MGR_GGM::EGGM_R1, getNvram_PerFrame_GGM));
    moduleList["GGM"].p1List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P1_GGM, ISP_MGR_GGM::EGGM_R2, getNvram_PerFrame_GGM));
    moduleList["G2C"].p1List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P1_G2C, ISP_MGR_G2C::EG2C_R1, getNvram_PerFrame_Dummy));
    //end of p1


    //start of p2
    moduleList["OBC"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_OBC, ISP_MGR_OBC::EOBC_D1, getNvram_PerFrame_OBC));
    moduleList["BPC"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_BPC, ISP_MGR_BPC::EBPC_D1, getNvram_PerFrame_BPC));
    moduleList["CT"].p2List.push_back( MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_CT,  ISP_MGR_BPC::EBPC_D1, getNvram_PerFrame_CT));
    moduleList["PDC"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_PDC, ISP_MGR_BPC::EBPC_D1, getNvram_PerFrame_PDC));
    moduleList["DGN"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_DGN, ISP_MGR_DGN::EDGN_D1, getNvram_PerFrame_Dummy));
    moduleList["LSC"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_LSC, ISP_MGR_LSC::ELSC_D1, getNvram_PerFrame_Dummy));
    moduleList["WB"].p2List.push_back( MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_WB , ISP_MGR_WB::EWB_D1  , getNvram_PerFrame_Dummy));
    moduleList["LTM"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_LTM, ISP_MGR_LTM::ELTM_D1, getNvram_PerFrame_Dummy));
    //HLR must be after LTM
    moduleList["HLR"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_HLR, ISP_MGR_HLR::EHLR_D1, getNvram_PerFrame_Dummy));

    //ISP Pass2 After Raw
    //SLK must after LSC
    moduleList["SLK"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_SLK, ISP_MGR_SLK::ESLK_D1, getNvram_PerFrame_Dummy)); //must after LSC
    moduleList["DM"].p2List.push_back( MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_DM,  ISP_MGR_DM::EDM_D1,   getNvram_PerFrame_DM));
    moduleList["LDNR"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_LDNR,ISP_MGR_LDNR::ELDNR_D1,getNvram_PerFrame_LDNR));
    moduleList["CCM"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_CCM, ISP_MGR_CCM::ECCM_D1, getNvram_PerFrame_CCM_P2));
    moduleList["CCM"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_CCM, ISP_MGR_CCM::ECCM_D2, getNvram_PerFrame_CCM_P2));
    //GGM_D3 include GGM_D1, GGM_D3, IGGM_D1
    moduleList["GGM"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_GGM, ISP_MGR_GGM::EGGM_D3, getNvram_PerFrame_GGM));
    moduleList["GGM"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_GGM, ISP_MGR_GGM::EGGM_D2, getNvram_PerFrame_GGM));
    moduleList["LCE"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_LCE, ISP_MGR_LCE::ELCE_D1, getNvram_PerFrame_LCE));
    moduleList["G2CX"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_G2C,ISP_MGR_G2CX::EG2CX_D1,getNvram_PerFrame_G2CX));

    //YNR must be after GGM & LCE
    moduleList["YNR"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_YNR, ISP_MGR_YNR::EYNR_D1, getNvram_PerFrame_YNR));
    moduleList["YNR"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_YNR, ISP_MGR_YNR::EYNR_D1, getNvram_PerFrame_YNR_TBL));
    moduleList["YNR_FACE"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_YNR_LINK, ISP_MGR_YNR::EYNR_D1, getNvram_PerFrame_YNR_FACE));
    moduleList["YNR_LCE"].p2List.push_back( MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_YNR_LINK, ISP_MGR_YNR::EYNR_D1, getNvram_PerFrame_YNR_LCE));
    moduleList["DCES"].p2List.push_back( MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_DCES, ISP_MGR_DCES::EDCES_D1, getNvram_PerFrame_DCES));
    moduleList["DCE"].p2List.push_back( MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_DCE, ISP_MGR_DCE::EDCE_D1, getNvram_PerFrame_DCE));

    // FLC must be after DCE
    moduleList["FLC"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_FLC, ISP_MGR_FLC::EFLC_D1, getNvram_PerFrame_FLC));
    moduleList["EE"].p2List.push_back( MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_EE,  ISP_MGR_EE::EEE_D1,   getNvram_PerFrame_EE));
    moduleList["CNR"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_CNR, ISP_MGR_CNR::ECNR_D1, getNvram_PerFrame_CNR));
    moduleList["CCR"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_CCR, ISP_MGR_CNR::ECNR_D1, getNvram_PerFrame_CNR));
    moduleList["ABF"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_ABF, ISP_MGR_CNR::ECNR_D1, getNvram_PerFrame_ABF));
    moduleList["BOK"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_BOK, ISP_MGR_CNR::ECNR_D1, getNvram_PerFrame_Dummy));

    // NDG must be after YNR & CNR
    moduleList["NDG"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_NDG, ISP_MGR_NDG::ENDG_D1, getNvram_PerFrame_Dummy));
    moduleList["COLOR"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_COLOR, ISP_MGR_COLOR::ECOLOR_D1, getNvram_PerFrame_COLOR));

    //TO-DO replace real dec for AKS
    moduleList["AKS"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_COLOR, ISP_MGR_AKS::EAKS_D1, getNvram_PerFrame_AKS));

    //Feature
    moduleList["NR3D"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_NR3D, ISP_MGR_NR3D::ENR3D_D1, getNvram_PerFrame_NR3D));
    moduleList["MIX"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_MIX, ISP_MGR_MIX::EMIX_D3, getNvram_PerFrame_MIX));

    //Normal Pass2 Flow LPCNR always be off
    moduleList["LPCNR"].p2List.push_back(MAKE_ISP_MODULE_LIST(decision_PerFrame_P2_LPCNR, ISP_MGR_LPCNR::ELPCNR_D1, getNvram_PerFrame_Dummy));
    //end of p2

    return  MERR_OK;
}

MERROR_ENUM
Paramctrl::
uninit()
{
    CAM_LOGD("[%s]", __FUNCTION__);

    //  Uninit ISP driver manager & tuning manager
    m_pTuning->uninit(LOG_TAG);
    ISP_MGR_YNR_T::getInstance(m_eSensorDev).releaseAlphaMap();

    if (m_bProfileEnable) {
        delete m_pAaaTimer;
        m_pAaaTimer = MNULL;
    }

    return  MERR_OK;
}


MERROR_ENUM
Paramctrl::
uninitModuleList()
{
    for(auto eachIspModule: moduleList)
    {
        eachIspModule.second.p1List.clear();
        eachIspModule.second.p2List.clear();
    }

    //clear module list
    moduleList.clear();

    return  MERR_OK;
}
