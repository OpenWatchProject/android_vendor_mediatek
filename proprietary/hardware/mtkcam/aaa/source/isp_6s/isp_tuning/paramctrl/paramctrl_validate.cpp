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
#define LOG_TAG "paramctrl_validate"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <mtkcam/utils/std/Log.h>
#include <aaa_error_code.h>
//#include <mtkcam/featureio/aaa_hal_if.h>
//#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <isp_tuning.h>
//#include <awb_param.h>
//#include <ae_param.h>
//#include <af_param.h>
//#include <flash_param.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_custom.h>
#include <isp_mgr.h>
#include "paramctrl.h"
//#include <hwutils/CameraProfile.h>

#include "property_utils.h"
#include <mtkcam/utils/sys/IFileCache.h>

using namespace NS3Av3;
using namespace NSIspTuning;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
validatePerFrameP1(RequestSet_T const RequestSet, RAWIspCamInfo &rP1CamInfo, MBOOL bReCalc, MINT32 i4SubsampleIdex)
{
    MERROR_ENUM err = MERR_UNKNOWN;

    MBOOL fgKeep = RequestSet.fgKeep;
    MINT32 i4FrameID = RequestSet.vNumberSet[0];
    CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_VALIDATE_P1, "[validatePerFrameP1] i4FrameID(%d), profile(%d), keep(%d)", i4FrameID, rP1CamInfo.rMapping_Info.eIspProfile, fgKeep);

    std::lock_guard<std::mutex> lock(m_Lock);

    //m_DualPD_PureRaw = RequestSet.fgDisableP1;

    rP1CamInfo.u4Id = i4FrameID;
    rP1CamInfo.fgNeedKeepP1 = fgKeep;

    rP1CamInfo.rMapping_Info.eApp = (EApp_T)m_pIspTuningCustom->map_AppName_to_MappingInfo();

    for(MINT32 group_index = 0; group_index< NVRAM_ISP_REGS_ISO_GROUP_NUM; group_index++){
        EISO_T eIdx_ISO = map_ISO_value2index(rP1CamInfo.rAEInfo.u4P1RealISOValue, (EISO_GROUP_T)group_index);
        rP1CamInfo.rMapping_Info.eISO_Idx[group_index] = eIdx_ISO;
        get_ISOIdx_neighbor(rP1CamInfo, rP1CamInfo.rMapping_Info.eISO_Idx[group_index], rP1CamInfo.rAEInfo.u4P1RealISOValue, (EISO_GROUP_T)group_index);
        CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_VALIDATE_P1,"[+rP1CamInfo.eIdx_ISO][%d](upper, lower)=(%d, %d)", group_index, rP1CamInfo.eIdx_ISO_U[group_index], rP1CamInfo.eIdx_ISO_L[group_index]);
    }

    EIspLV_T const eIdx_IspLv = map_LV_value2IspLV_index(rP1CamInfo.rAEInfo.i4RealLightValue_x10);
    rP1CamInfo.rMapping_Info.eIspLV_Idx = eIdx_IspLv;
    get_IspLVIdx_neighbor(rP1CamInfo, eIdx_IspLv);
    CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_VALIDATE_P1, "(rAEInfo.i4RealLightValue_x10, eIdx_IspLv)=(%d, %d)", rP1CamInfo.rAEInfo.i4RealLightValue_x10, eIdx_IspLv);

    ELV_T const eIdx_Lv = map_LV_value2index(rP1CamInfo.rAEInfo.i4RealLightValue_x10);
    rP1CamInfo.rMapping_Info.eLV_Idx = eIdx_Lv;
    get_LVIdx_neighbor(rP1CamInfo, eIdx_Lv);
    CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_VALIDATE_P1, "(rAEInfo.i4RealLightValue_x10, eIdx_Lv)=(%d, %d)", rP1CamInfo.rAEInfo.i4RealLightValue_x10, eIdx_Lv);

    ECT_T const eIdx_CT = map_CT_value2index(rP1CamInfo.rAWBInfo.i4CCT);
    rP1CamInfo.rMapping_Info.eCT_Idx = eIdx_CT;
    get_CTIdx_neighbor(rP1CamInfo, rP1CamInfo.rMapping_Info.eCT_Idx);

    rP1CamInfo.rMapping_Info.eZoom_Idx = map_Zoom_value2index(rP1CamInfo.i4ZoomRatio_x100);


    CAM_IDX_QRY_COMB rMapping_Info;
    IdxMgr::createInstance(static_cast<NSIspTuning::ESensorDev_T>(m_eSensorDev))->getMappingInfo(static_cast<NSIspTuning::ESensorDev_T>(m_eSensorDev), rMapping_Info, rP1CamInfo.u4Id);
    rP1CamInfo.rMapping_Info.eSize = rMapping_Info.eSize;
    rP1CamInfo.rMapping_Info.eFlash = rMapping_Info.eFlash;
    rP1CamInfo.rMapping_Info.eFaceDetection = rMapping_Info.eFaceDetection;
    IdxMgr::createInstance(static_cast<NSIspTuning::ESensorDev_T>(m_eSensorDev))->setMappingInfoByDim(static_cast<NSIspTuning::ESensorDev_T>(m_eSensorDev), rP1CamInfo.u4Id, EDim_ISO, (MVOID*)&rP1CamInfo.rMapping_Info.eISO_Idx);
    IdxMgr::createInstance(static_cast<NSIspTuning::ESensorDev_T>(m_eSensorDev))->setMappingInfoByDim(static_cast<NSIspTuning::ESensorDev_T>(m_eSensorDev), rP1CamInfo.u4Id, EDim_LV, (MVOID*)&rP1CamInfo.rMapping_Info.eLV_Idx);
    IdxMgr::createInstance(static_cast<NSIspTuning::ESensorDev_T>(m_eSensorDev))->setMappingInfoByDim(static_cast<NSIspTuning::ESensorDev_T>(m_eSensorDev), rP1CamInfo.u4Id, EDim_CT, (MVOID*)&rP1CamInfo.rMapping_Info.eCT_Idx);
    IdxMgr::createInstance(static_cast<NSIspTuning::ESensorDev_T>(m_eSensorDev))->setMappingInfoByDim(static_cast<NSIspTuning::ESensorDev_T>(m_eSensorDev), rP1CamInfo.u4Id, EDim_IspLV, (MVOID*)&rP1CamInfo.rMapping_Info.eIspLV_Idx);

    MINT32 cacheEnable;
    getPropInt("vendor.debug.cache.mode", &cacheEnable, 1);
    cacheEnable = cacheEnable && (rP1CamInfo.eControlMode != MTK_CONTROL_MODE_OFF);
    m_rIdxCache.setEnable(cacheEnable);
    m_rIdxCache.updateKey(rP1CamInfo.rMapping_Info, IDXCACHE_STAGE_P1);
    m_rIdxCache.setLogEnable();

#if 0
    if(getOperMode() == EOperMode_Meta) {
        err = MERR_OK;
        CAM_LOGD("[validatePerFrame] Meta Mode\n");
        goto lbExit;
    }
#endif

    if (m_bDebugEnable & PARAMCTL_LOG_VALIDATE_P1) {
        CAM_LOGD("%s()\n", __FUNCTION__);
        //IspDebug::getInstance().dumpIspDebugMessage();
    }

    CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_VALIDATE_P1, "[validatePerFrameP1](ParamChangeCount)=(%d)", getParamChangeCount());

    //  (1) reset to 0 since all params have been applied.
    resetParamChangeCount();

    //  (2) Do something.
    if (fgKeep)
    {
        err = MERR_OK;
        prepareHw_PerFrame_LSC(rP1CamInfo); // lsc pass1
        if (!applyToHw_PerFrame_P1(RequestSet, rP1CamInfo, bReCalc, i4SubsampleIdex))
        {
            err = MERR_SET_ISP_REG;
        }
    }
    else
    {
        err = do_validatePerFrameP1(RequestSet, rP1CamInfo, bReCalc, i4SubsampleIdex);
    }
    if (MERR_OK != err)
    {
        CAM_LOGE("[validatePerFrameP1]do_validatePerFrameP1 returns err(%d)", err);
        goto lbExit;
    }

    err = MERR_OK;

lbExit:

    return  err;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
validatePerFrameP2(MINT32 flowType, const ISP_INFO_T& rIspInfo, void* pTuningBuf)
{
    MERROR_ENUM err = MERR_UNKNOWN;

    CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_VALIDATE_P2, "[validatePerFrameP2] i4FrameID = %d, profile = %d", rIspInfo.rCamInfo.u4Id, rIspInfo.rCamInfo.rMapping_Info.eIspProfile);

    //std::lock_guard<std::mutex> lock(m_Lock);

#if 0
    if(getOperMode() == EOperMode_Meta) {
        err = MERR_OK;
        CAM_LOGD("[validatePerFrame] Meta Mode\n");
        goto lbExit;
    }
#endif

    void* pRegBuf = ((TuningParam*)pTuningBuf)->pRegBuf;
    void* pMfbBuf = ((TuningParam*)pTuningBuf)->pMfbBuf;
    void* pLcsBuf = ((TuningParam*)pTuningBuf)->pLcsBuf;
    void* pMssBuf = (((TuningParam*)pTuningBuf)->pMssBuf;
    void* pLceshoBuf = ((TuningParam*)pTuningBuf)->pLceshoBuf;

    MUINT32 DcsFrmId = ((TuningParam*)pTuningBuf)->i4DcsMagicNo;
    void* pDcsBuf = ((TuningParam*)pTuningBuf)->pDcsBuf;

    if(( rIspInfo.rCamInfo.u4Id - DcsFrmId ) > 2){
        pDcsBuf = NULL;
        CAM_LOGE("DCESO too old, use default setting, u4id(%d) DcsFrmId(%d)", rIspInfo.rCamInfo.u4Id, DcsFrmId);
    }

    //  (2) Do something.

    setLCESOBuffer(pLcsBuf, pLceshoBuf);
    setDCESOBuffer(pDcsBuf);

    err = (do_validatePerFrameP2(flowType, rIspInfo, pRegBuf, pMfbBuf, pMssBuf));

    MBOOL bDumpface = MFALSE;
    MINT32 captureDump;
    getPropInt("vendor.isp.faceinfo.dump", &captureDump, 0);
    MINT32 previewDump;
    getPropInt("vendor.isp.faceinfo.dump.preview", &previewDump, 0);

    if ((!rIspInfo.rCamInfo.fgRPGEnable && captureDump) || previewDump)
        bDumpface = MTRUE;
    if (bDumpface){
        char filename[512];
        genFileName_face_info(filename, sizeof(filename), &m_hint_p2);
        FILE *fidFaceDump;
        fidFaceDump = std::fopen(filename,"w");
        CAM_LOGW("[%s] %s", __FUNCTION__, filename);
        if (std::fwrite(&rIspInfo.rCamInfo.rFdInfo, sizeof(CAMERA_TUNING_FD_INFO_T), 1, fidFaceDump) != 1)
            CAM_LOGW("[%s] fail dump to %s", __FUNCTION__, filename);
        std::fclose(fidFaceDump);

    }

    MBOOL bForceSkin = MFALSE;
    getPropInt("vendor.isp.ynr_skin_link_force", &bForceSkin, 0); // 0: enable, 1: disable

    MBOOL skin_link_top_control = ISP_MGR_YNR_T::getInstance(m_eSensorDev).isSkinEnable(ISP_MGR_YNR::EYNR_D1);
    MBOOL bSkin_link = ((reinterpret_cast<dip_a_reg_t*>(pRegBuf)->YNR_D1A_YNR_SKIN_CON.Bits.YNR_SKIN_LINK) || (bForceSkin && skin_link_top_control));
    reinterpret_cast<dip_a_reg_t*>(pRegBuf)->YNR_D1A_YNR_SKIN_CON.Bits.YNR_SKIN_LINK = bSkin_link;

    if (bSkin_link){
            //get map if skin link is set enable
            ((TuningParam*)pTuningBuf)->pFaceAlphaBuf = ISP_MGR_YNR_T::getInstance(m_eSensorDev).getAlphaMap(ISP_MGR_YNR::EYNR_D1, rIspInfo.rCamInfo.u4Id);
            CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_VALIDATE_P2,"[validatePerFrameP2] alphamap(%x) skin_link(%d)"
                ,((TuningParam*)pTuningBuf)->pFaceAlphaBuf
                , bSkin_link);
     }
     else {
         ((TuningParam*)pTuningBuf)->pFaceAlphaBuf = NULL;
         CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_VALIDATE_P2,"[validatePerFrameP2] skin_link(%d)"
         ,((dip_a_reg_t*)pRegBuf)->YNR_D1A_YNR_SKIN_CON.Bits.YNR_SKIN_LINK);
     }

    ((TuningParam*)pTuningBuf)->bDCES_Enalbe = reinterpret_cast<dip_a_reg_t*>(pRegBuf)->DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_DCES_D1_EN;
    ((TuningParam*)pTuningBuf)->bYNR_LCES_In = reinterpret_cast<dip_a_reg_t*>(pRegBuf)->YNR_D1A_YNR_CON1.Bits.YNR_LCE_LINK;

    if  (MERR_OK != err)
    {
        CAM_LOGE("[validatePerFrameP2]do_validatePerFrameP2 returns err(%d)", err);
        goto lbExit;
    }

    err = MERR_OK;

lbExit:

    return  err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
check_ISO_env_info()
{
    int temp_count;

    for(int j=0; j<NVRAM_ISP_REGS_ISO_GROUP_NUM; j++){

        if( m_rIspParam.ISPRegs.Iso_Env[j].u2Length > NVRAM_ISP_REGS_ISO_SUPPORT_NUM){
            CAM_LOGE("CUSTOM_ISO_ENV_LENGTH_ERROR");
            return MFALSE;
        }
        else{

            temp_count = 0;

            for(int i=0; i<(m_rIspParam.ISPRegs.Iso_Env[j].u2Length-1); i++)
            {
                if( (m_rIspParam.ISPRegs.Iso_Env[j].IDX_Partition[i] < m_rIspParam.ISPRegs.Iso_Env[j].IDX_Partition[i+1])&&
                    (m_rIspParam.ISPRegs.Iso_Env[j].IDX_Partition[i] > 0))
                {
                    temp_count++;
                }
            }
            if ( temp_count != (m_rIspParam.ISPRegs.Iso_Env[j].u2Length-1) ){
                CAM_LOGE("CUSTOM_ISO_ENV_SETTING_ERROR");
                return MFALSE;
            }
        }
    }
    return MTRUE;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
check_CT_env_info()
{
    int temp_count = 0;
    MINT32 COLOR_CT_Env[5] ={
        2000, 3500, 5000, 6500, 8000
    };
    for(int i=0; i<(ISP_NVRAM_COLOR_CT_NUM-1); i++)
    {
        if( (COLOR_CT_Env[i] < COLOR_CT_Env[i+1])&& (COLOR_CT_Env[i] > 0))
        {
            temp_count++;
        }
    }
    if ( temp_count != (ISP_NVRAM_COLOR_CT_NUM-1) ){
        CAM_LOGE("CUSTOM_ISO_ENV_SETTING_ERROR");
        return MFALSE;
    }
    return MTRUE;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
Paramctrl::getDefaultObc(MVOID*& pISP_NVRAM_Reg)
{
    pISP_NVRAM_Reg = &m_rIspParam.ISPRegs;

    return MTRUE;
}

MUINT16
Paramctrl::
getISPIndex(IDXCACHE_VALTYPE eValType, const RAWIspCamInfo& rIspCamInfo, MUINT32 module, IDXCACHE_STAGE eStage, IDXCACHE_QRYMODE eMode)
{
    CAM_IDX_QRY_COMB qry = rIspCamInfo.rMapping_Info;

    switch(eValType)
    {
        case IDXCACHE_VALTYPE_UPPERISO:
        {
            ::memcpy(&(qry.eISO_Idx), &(rIspCamInfo.eIdx_ISO_U), sizeof(EISO_T)*NVRAM_ISP_REGS_ISO_GROUP_NUM);
            break;
        }
        case IDXCACHE_VALTYPE_LOWERISO:
        {
            ::memcpy(&(qry.eISO_Idx), &(rIspCamInfo.eIdx_ISO_L), sizeof(EISO_T)*NVRAM_ISP_REGS_ISO_GROUP_NUM);
            break;
        }
        case IDXCACHE_VALTYPE_UPPERLV:
        {
            qry.eLV_Idx =           (ELV_T)rIspCamInfo.eIdx_LV_U;
            break;
        }
        case IDXCACHE_VALTYPE_LOWERLV:
        {
            qry.eLV_Idx =           (ELV_T)rIspCamInfo.eIdx_LV_L;
            break;
        }
        case IDXCACHE_VALTYPE_LOWERLV_LOWERCT:
        {
            qry.eLV_Idx =           (ELV_T)rIspCamInfo.eIdx_LV_L;
            qry.eCT_Idx =           (ECT_T)rIspCamInfo.eIdx_CT_L;
            break;
        }
        case IDXCACHE_VALTYPE_LOWERLV_UPPERCT:
        {
            qry.eLV_Idx =           (ELV_T)rIspCamInfo.eIdx_LV_L;
            qry.eCT_Idx =           (ECT_T)rIspCamInfo.eIdx_CT_U;
            break;
        }
        case IDXCACHE_VALTYPE_UPPERLV_LOWERCT:
        {
            qry.eLV_Idx =           (ELV_T)rIspCamInfo.eIdx_LV_U;
            qry.eCT_Idx =           (ECT_T)rIspCamInfo.eIdx_CT_L;
            break;
        }
        case IDXCACHE_VALTYPE_UPPERLV_UPPERCT:
        {
            qry.eLV_Idx =           (ELV_T)rIspCamInfo.eIdx_LV_U;
            qry.eCT_Idx =           (ECT_T)rIspCamInfo.eIdx_CT_U;
            break;
        }
        default:
            break;
    }

    MUINT16 index = m_rIdxCache.query((EModule_T)module, eStage, eValType, qry, __FUNCTION__, eMode);

    return index;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
do_validatePerFrameP1(RequestSet_T const RequestSet, RAWIspCamInfo& rIspCamInfo, MBOOL bReCalc, MINT32 i4SubsampleIdex)
{
    MERROR_ENUM err = MERR_OK;
    MBOOL prepare_rdy;
    MINT32 time_ms;

    CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_VALIDATE_P1, "[do_validatePerFrameP1]");

    if (m_pVCoreThrottling!=NULL && *m_pVCoreThrottling) {
        CAM_LOGW("VCore Thermal Throttling!!! Disable some ISP H/W");
    }

    getPropInt("vendor.isp.interp.ctrl", &m_IspInterpCtrl, 1); //default: enable

    //CPTLog(Event_Pipe_3A_ISP_VALIDATE_PERFRAME_DYNAMIC_TUNING, CPTFlagStart); // Profiling Start.
    if (m_bProfileEnable)
        m_pAaaTimer->start("do_validatePerFrameP1: nvram index", m_eSensorDev, m_bProfileEnable);

    //  (1) dynamic tuning
    if (isDynamicTuning())
    {
        // Refine camera info.
        m_pIspTuningCustom->refine_CamInfo(rIspCamInfo);

        rIspCamInfo.fgHDR = m_pIspTuningCustom->is_HDRTuning(rIspCamInfo);
    }

    if (m_bProfileEnable)
        time_ms = m_pAaaTimer->End();

    if (m_bProfileEnable)
        m_pAaaTimer->start("do_validatePerFrameP1: prepareHw_PerFrame_P1()", m_eSensorDev, m_bProfileEnable);
    //  (2) Apply Per-Frame Parameters.

    debugCTL_Collecttion_PerFrame();

    (isDynamicBypass() == MTRUE) ? (prepare_rdy = prepareHw_PerFrame_Partial(rIspCamInfo))
                                 : (prepare_rdy = prepareHw_PerFrame_P1(rIspCamInfo));
    if (m_bProfileEnable)
        time_ms = m_pAaaTimer->End();

    if (m_bProfileEnable)
        m_pAaaTimer->start("do_validatePerFrameP1: applyToHw_PerFrame_P1()", m_eSensorDev, m_bProfileEnable);

    if(!prepare_rdy || ! applyToHw_PerFrame_P1(RequestSet, rIspCamInfo, bReCalc, i4SubsampleIdex))  //  Apply the ispmgr's buffer to H/W.
    {
        err = MERR_SET_ISP_REG;
        goto lbExit;
    }

#if 0
    if  (
            ! prepareHw_PerFrame_All()          //  Prepare param members to the ispmgr's buffer.
        ||  ! applyToHw_PerFrame_P1()          //  Apply the ispmgr's buffer to H/W.
        )
    {
        err = MERR_SET_ISP_REG;
        goto lbExit;
    }
#endif

lbExit:
    if  ( MERR_OK != err )
    {
        CAM_LOGE("[-do_validatePerFrameP1]err(%X)", err);
    }

    if (m_bProfileEnable)
        time_ms = m_pAaaTimer->End();

    return  err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID*
Paramctrl::getDMGItable(MBOOL const fgRPGEnable){

    MINT32 captureDump;
    getPropInt("vendor.isp.bpc_tbl.dump", &captureDump, 0);
    MINT32 previewDump;
    getPropInt("vendor.isp.bpc_tbl.dump.preview", &previewDump, 0);
    MBOOL bDumpTbl = MFALSE;
    if ((!fgRPGEnable && captureDump) || previewDump)
        bDumpTbl = MTRUE;

    if(bDumpTbl){
        char filename[512];
        genFileName_BPC_TBL(filename, sizeof(filename), &m_hint_p2);
        IImageBuffer* pBPCTBL = (IImageBuffer*)ISP_MGR_BPC_T::getInstance(m_eSensorDev).getDMGItable();

        if (pBPCTBL != NULL){
            pBPCTBL->saveToFile(filename);
        }
        else{
            FILE* pFileTemp = fopen(filename, "w");
            if(pFileTemp) fclose(pFileTemp);
        }
    }

    if(ISP_MGR_BPC_T::getInstance(m_eSensorDev).isPDCEnable(ISP_MGR_BPC::EBPC_D1))
        return ISP_MGR_BPC_T::getInstance(m_eSensorDev).getDMGItable();
    else
        return NULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
do_validatePerFrameP2(MINT32 flowType, const ISP_INFO_T& _rIspInfo, void* pRegBuf, void* pMfbBuf, void* pMssBuf)
{
    MERROR_ENUM err = MERR_OK;
    MBOOL prepare_rdy = MTRUE;

    CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_VALIDATE_P2, "[do_validatePerFrameP2] updateMode(%d)", _rIspInfo.rCamInfo.u1P2TuningUpdate);

    ISP_INFO_T rIspInfo_temp = _rIspInfo;

    //YUV Reprocess
    // Should use YUV Reprocess Scenario as input to query ISO Group Index
    for(MINT32 group_index = 0; group_index< NVRAM_ISP_REGS_ISO_GROUP_NUM; group_index++){
         rIspInfo_temp.rCamInfo.rMapping_Info.eISO_Idx[group_index] = map_ISO_value2index(rIspInfo_temp.rCamInfo.rAEInfo.u4P2RealISOValue, (EISO_GROUP_T)group_index);
         get_ISOIdx_neighbor(rIspInfo_temp.rCamInfo, rIspInfo_temp.rCamInfo.rMapping_Info.eISO_Idx[group_index], rIspInfo_temp.rCamInfo.rAEInfo.u4P2RealISOValue, (EISO_GROUP_T)group_index);
    }
    rIspInfo_temp.rCamInfo.rMapping_Info.eCT_Idx = map_CT_value2index(rIspInfo_temp.rCamInfo.rAWBInfo.i4CCT);
    get_CTIdx_neighbor(rIspInfo_temp.rCamInfo, rIspInfo_temp.rCamInfo.rMapping_Info.eCT_Idx);
    rIspInfo_temp.rCamInfo.rMapping_Info.eSize = (ESize_T)m_pIspTuningCustom->map_Target_Size_to_MappingInfo(_rIspInfo.rCamInfo.targetSize.w, _rIspInfo.rCamInfo.targetSize.h, "do_validatePerFrameP2");
    rIspInfo_temp.rCamInfo.fgHDR = m_pIspTuningCustom->is_HDRTuning(rIspInfo_temp.rCamInfo);

    m_rIdxCache.updateKey(rIspInfo_temp.rCamInfo.rMapping_Info, IDXCACHE_STAGE_P2);

    const ISP_INFO_T &rIspInfo = rIspInfo_temp;

    //  (1) dynamic tuning
    if (isDynamicTuning())
    {
        const RAWIspCamInfo& rIspCamInfo = rIspInfo.rCamInfo;

        CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_VALIDATE_P2, "[do_validatePerFrameP2()] isDynamicBypass() = %d\n", isDynamicBypass());

        debugCTL_Collecttion_PerFrame();

        (isDynamicBypass() == MTRUE) ? (prepare_rdy = prepareHw_PerFrame_Partial(rIspInfo))
                                     : (prepare_rdy = prepareHw_PerFrame_P2(rIspInfo));
    }

    if(rIspInfo.rCamInfo.u1P2TuningUpdate == ELPCNR_8Bit_Pass1
       || rIspInfo.rCamInfo.u1P2TuningUpdate == ELPCNR_10Bit_Pass1){
/* HHHHHH fix
        //LPCNR Pass1
        ISP_MGR_LPCNR_T::getInstance(m_eSensorDev).setEnable(ISP_MGR_LPCNR::ELPCNR_D1, MTRUE);
        getNvram_PerFrame_LPCNR(ISP_MGR_LPCNR::ELPCNR_D1, rIspInfo.rCamInfo, IDXCACHE_STAGE_P2);

        dip_a_reg_t* pReg = reinterpret_cast<dip_a_reg_t*>(pRegBuf);
        ::memset((void*)pReg, 0, sizeof(dip_a_reg_t));
        ISP_MGR_LPCNR_T::getInstance(m_eSensorDev).apply_P2(ISP_MGR_LPCNR::ELPCNR_D1, rIspInfo.rCamInfo, pReg);
        pReg->LPCNR_D1A_LPCNR_TOP.Bits.LPCNR_PASS_MODE = 0;


        if(rIspInfo.rCamInfo.u1P2TuningUpdate == ELPCNR_8Bit_Pass1)
            pReg->LPCNR_D1A_LPCNR_TOP.Bits.LPCNR_LBIT_MODE = 1;
        else
            pReg->LPCNR_D1A_LPCNR_TOP.Bits.LPCNR_LBIT_MODE = 0;
*/
    }
    else if(rIspInfo.rCamInfo.u1P2TuningUpdate == ELPCNR_8Bit_Pass2
            || rIspInfo.rCamInfo.u1P2TuningUpdate == ELPCNR_10Bit_Pass2){
/* HHH fix
        //LPCNR Pass2
        dip_a_reg_t* pReg = reinterpret_cast<dip_a_reg_t*>(pRegBuf);
        pReg->LPCNR_D1A_LPCNR_TOP.Bits.LPCNR_PASS_MODE = 1;

        if(rIspInfo.rCamInfo.u1P2TuningUpdate == ELPCNR_8Bit_Pass2)
            pReg->LPCNR_D1A_LPCNR_TOP.Bits.LPCNR_LBIT_MODE = 1;
        else
            pReg->LPCNR_D1A_LPCNR_TOP.Bits.LPCNR_LBIT_MODE = 0;
*/
    }
    else if(rIspInfo.rCamInfo.u1P2TuningUpdate == EIdenditySetting){
        //P2 Identity Setting
        dip_a_reg_t* pReg = reinterpret_cast<dip_a_reg_t*>(pRegBuf);

        ispP2IdentitySet(pReg);
    }
    else if(rIspInfo.rCamInfo.u1P2TuningUpdate != EAllKeep){

        if(pRegBuf){
            if( (!prepare_rdy) ||
                (! applyToHw_PerFrame_P2(flowType, rIspInfo, pRegBuf)))//  Apply the ispmgr's buffer to H/W.
            {
                err = MERR_SET_ISP_REG;
                goto lbExit;
            }
        }
    }
    else{
        CAM_LOGD("Fix tuning buf u1P2TuningUpdate=2, and skip apply tuning P2");
    }

    if(pMfbBuf){
        decision_PerFrame_P2_MFB(rIspInfo.rCamInfo);
		decision_PerFrame_P2_MSF(rIspInfo.rCamInfo);
		decision_PerFrame_P2_MSS(rIspInfo.rCamInfo);
        getNvram_PerFrame_MFB(ISP_MGR_MFB::EMFB_D1, rIspInfo.rCamInfo, IDXCACHE_STAGE_P2);
		getNvram_PerFrame_MSF(ISP_MGR_MSF::EMSF_D1, rIspInfo.rCamInfo, IDXCACHE_STAGE_P2);
		getNvram_PerFrame_OMC(ISP_MGR_OMC::EOMC_D1, rIspInfo.rCamInfo, IDXCACHE_STAGE_P2);
        applyToHw_PerFrame_MFB(flowType, rIspInfo, pMfbBuf);
        applyToHw_PerFrame_MSS(flowType, rIspInfo, pMssBuf);

		MINT32 CaptureDump = property_get_int32("vendor.debug.camera.dump.p2.debuginfo.hh", 0);
		MINT32 PreviewDump = property_get_int32("vendor.debug.camera.dump.isp.preview.hh", 0);

		if ((CaptureDump && rIspInfo.isCapture) || PreviewDump)
		{

			char strTuningFile[512] = {'\0'};
			sprintf(strTuningFile, "/data/vendor/camera_dump/%09d-%04d-%04d-%d-%d.mfbreg", rIspInfo.hint.UniqueKey, rIspInfo.hint.RequestNo, rIspInfo.hint.FrameNo, m_eSensorDev, rIspInfo.rCamInfo.rMapping_Info.eIspProfile);

			FILE* fidTuning = fopen(strTuningFile, "wb");
			if (fidTuning)
			{
				fwrite((MUINT32*)mMfbBuffer, sizeof(mfb_reg_t), 1, fidTuning);
				fclose(fidTuning);
			}

			char _strTuningFile[512] = {'\0'};
			sprintf(_strTuningFile, "/data/vendor/camera_dump/%09d-%04d-%04d-%d-%d.mssreg", rIspInfo.hint.UniqueKey, rIspInfo.hint.RequestNo, rIspInfo.hint.FrameNo, m_eSensorDev, rIspInfo.rCamInfo.rMapping_Info.eIspProfile);

			FILE* _fidTuning = fopen(_strTuningFile, "wb");
			if (_fidTuning)
			{
				fwrite((MUINT32*)mMssBuffer, sizeof(mss_reg_t), 1, _fidTuning);
				fclose(_fidTuning);
			}

	        MUINT16 idxL_TBL = 0;
			ISP_NVRAM_MSF_TBL_T msf_tbl;

			switch(rIspInfo.rCamInfo.rMapping_Info.eIspProfile)
			{
				case EIspProfile_MFNR_MSF_0:
				{
					msf_tbl = {
						0x0000,0x0100,0x0200,0x0300,0x0400,0x0500,0x0600,0x0700,0x0800,0x0900,0x0a00,0x0b00,0x0c00,0x0d00,0x0e00,0x0f00,0x1000,0x1100,0x1200,0x1300,0x1400,0x1500,0x1600,0x1700,0x1800,0x1900,0x1a00,0x1b00,0x1c00,0x1d00,0x1e00,0x1f00,0x2000,0x2100,0x2200,0x2300,0x2400,0x2500,0x2600,0x2700,0x2800,0x2900,0x2a00,0x2b00,0x2c00,0x2d00,0x2e00,0x2f00,0x3000,0x3100,0x3200,0x3300,0x3400,0x3500,0x3600,0x3700,0x3800,0x3900,0x3a00,0x3b00,0x3c00,0x3d00,0x3e00,0x3f00,0x4000,0x4100,0x4200,0x4300,0x4400,0x4500,0x4600,0x4700,0x4800,0x4900,0x4a00,0x4b00,0x4c00,0x4d00,0x4e00,0x4f00,0x5000,0x5100,0x5200,0x5300,0x5400,0x5500,0x5600,0x5700,0x5800,0x5900,0x5a00,0x5b00,0x5c00,0x5d00,0x5e00,0x5f00,0x6000,0x6100,0x6200,0x6300,0x6400,0x6500,0x6600,0x6700,0x6800,0x6900,0x6a00,0x6b00,0x6c00,0x6d00,0x6e00,0x6f00,0x7000,0x7100,0x7200,0x7300,0x7400,0x7500,0x7600,0x7700,0x7800,0x7900,0x7a00,0x7b00,0x7c00,0x7d00,0x7e00,0x7f00,0x8000,0x8100,0x8200,0x8300,0x8400,0x8500,0x8600,0x8700,0x8800,0x8900,0x8a00,0x8b00,0x8c00,0x8d00,0x8e00,0x8f00,0x9000,0x9100,0x9200,0x9300,0x9400,0x9500,0x9600,0x9700,0x9800,0x9900,0x9a00,0x9b00,0x9c00,0x9d00,0x9e00,0x9f00,0xa000,0xa100,0xa200,0xa300,0xa400,0xa500,0xa600,0xa700,0xa800,0xa900,0xaa00,0xab00,0xac00,0xad00,0xae00,0xaf00,0xb000,0xb100,0xb200,0xb300,0xb400,0xb500,0xb600,0xb700,0xb800,0xb901,0xba01,0xbb01,0xbc01,0xbd01,0xbe01,0xbf01,0xc002,0xc102,0xc202,0xc302,0xc402,0xc503,0xc603,0xc703,0xc804,0xc904,0xca04,0xcb05,0xcc05,0xcd06,0xce06,0xcf07,0xd007,0xd108,0xd209,0xd30a,0xd40b,0xd50b,0xd60c,0xd70e,0xd80f,0xd910,0xda11,0xdb13,0xdc14,0xdd16,0xde18,0xdf1a,0xe01c,0xe11e,0xe220,0xe323,0xe426,0xe528,0xe62c,0xe72f,0xe833,0xe937,0xea3b,0xeb3f,0xec44,0xed49,0xee4e,0xef54,0xf05a,0xf161,0xf268,0xf370,0xf478,0xf581,0xf68a,0xf794,0xf89e,0xf9aa,0xfab6,0xfbc2,0xfcd0,0xfddf,0xfeee,0xffff
				    };
					break;
				}
				case EIspProfile_MFNR_MSF_1:
				{
					msf_tbl = {
						0x0000,0x0100,0x0200,0x0300,0x0400,0x0500,0x0600,0x0700,0x0800,0x0900,0x0a00,0x0b00,0x0c00,0x0d00,0x0e00,0x0f00,0x1000,0x1100,0x1200,0x1300,0x1400,0x1500,0x1600,0x1700,0x1800,0x1900,0x1a00,0x1b00,0x1c00,0x1d00,0x1e00,0x1f00,0x2000,0x2100,0x2200,0x2300,0x2400,0x2500,0x2600,0x2700,0x2800,0x2900,0x2a00,0x2b00,0x2c00,0x2d00,0x2e00,0x2f00,0x3000,0x3100,0x3200,0x3300,0x3400,0x3500,0x3600,0x3700,0x3800,0x3900,0x3a00,0x3b00,0x3c00,0x3d00,0x3e00,0x3f00,0x4000,0x4100,0x4200,0x4300,0x4400,0x4500,0x4600,0x4700,0x4800,0x4900,0x4a00,0x4b00,0x4c00,0x4d00,0x4e00,0x4f00,0x5000,0x5100,0x5200,0x5300,0x5400,0x5500,0x5600,0x5700,0x5800,0x5900,0x5a00,0x5b00,0x5c00,0x5d00,0x5e00,0x5f00,0x6000,0x6100,0x6200,0x6300,0x6400,0x6500,0x6600,0x6700,0x6800,0x6900,0x6a00,0x6b00,0x6c00,0x6d00,0x6e00,0x6f00,0x7000,0x7100,0x7200,0x7300,0x7400,0x7500,0x7600,0x7700,0x7800,0x7900,0x7a00,0x7b00,0x7c00,0x7d00,0x7e00,0x7f00,0x8000,0x8100,0x8200,0x8300,0x8400,0x8500,0x8600,0x8700,0x8800,0x8900,0x8a00,0x8b00,0x8c00,0x8d00,0x8e00,0x8f00,0x9000,0x9100,0x9200,0x9300,0x9400,0x9500,0x9600,0x9700,0x9800,0x9900,0x9a00,0x9b00,0x9c00,0x9d00,0x9e00,0x9f00,0xa000,0xa100,0xa200,0xa300,0xa400,0xa500,0xa600,0xa701,0xa801,0xa901,0xaa01,0xab01,0xac01,0xad01,0xae01,0xaf01,0xb002,0xb102,0xb202,0xb302,0xb402,0xb502,0xb603,0xb703,0xb803,0xb903,0xba04,0xbb04,0xbc04,0xbd05,0xbe05,0xbf05,0xc006,0xc106,0xc207,0xc307,0xc408,0xc508,0xc609,0xc70a,0xc80a,0xc90b,0xca0c,0xcb0d,0xcc0e,0xcd0e,0xce0f,0xcf10,0xd012,0xd113,0xd214,0xd315,0xd417,0xd518,0xd61a,0xd71b,0xd81d,0xd91f,0xda21,0xdb23,0xdc25,0xdd27,0xde2a,0xdf2c,0xe02f,0xe132,0xe235,0xe338,0xe43b,0xe53f,0xe642,0xe746,0xe84a,0xe94e,0xea53,0xeb58,0xec5d,0xed62,0xee67,0xef6d,0xf073,0xf17a,0xf281,0xf388,0xf48f,0xf597,0xf69f,0xf7a8,0xf8b1,0xf9bb,0xfac5,0xfbcf,0xfcda,0xfde6,0xfef2,0xffff
				    };
					break;
				}
				case EIspProfile_MFNR_MSF_2:
				{
					msf_tbl = {
						0x0000,0x0100,0x0200,0x0300,0x0400,0x0500,0x0600,0x0700,0x0800,0x0900,0x0a00,0x0b00,0x0c00,0x0d00,0x0e00,0x0f00,0x1000,0x1100,0x1200,0x1300,0x1400,0x1500,0x1600,0x1700,0x1800,0x1900,0x1a00,0x1b00,0x1c00,0x1d00,0x1e00,0x1f00,0x2000,0x2100,0x2200,0x2300,0x2400,0x2500,0x2600,0x2700,0x2800,0x2900,0x2a00,0x2b00,0x2c00,0x2d00,0x2e00,0x2f00,0x3000,0x3100,0x3200,0x3300,0x3400,0x3500,0x3600,0x3700,0x3800,0x3900,0x3a00,0x3b00,0x3c00,0x3d00,0x3e00,0x3f00,0x4000,0x4100,0x4200,0x4300,0x4400,0x4500,0x4600,0x4700,0x4800,0x4900,0x4a00,0x4b00,0x4c00,0x4d00,0x4e00,0x4f00,0x5000,0x5100,0x5200,0x5300,0x5400,0x5500,0x5600,0x5700,0x5800,0x5900,0x5a00,0x5b00,0x5c00,0x5d00,0x5e00,0x5f00,0x6000,0x6100,0x6200,0x6300,0x6400,0x6500,0x6600,0x6700,0x6800,0x6900,0x6a00,0x6b00,0x6c00,0x6d00,0x6e00,0x6f00,0x7000,0x7100,0x7200,0x7300,0x7400,0x7500,0x7600,0x7700,0x7800,0x7900,0x7a00,0x7b00,0x7c00,0x7d00,0x7e00,0x7f00,0x8000,0x8100,0x8200,0x8300,0x8400,0x8500,0x8600,0x8700,0x8800,0x8900,0x8a01,0x8b01,0x8c01,0x8d01,0x8e01,0x8f01,0x9001,0x9101,0x9201,0x9301,0x9401,0x9502,0x9602,0x9702,0x9802,0x9902,0x9a02,0x9b02,0x9c03,0x9d03,0x9e03,0x9f03,0xa003,0xa104,0xa204,0xa304,0xa404,0xa505,0xa605,0xa705,0xa805,0xa906,0xaa06,0xab06,0xac07,0xad07,0xae08,0xaf08,0xb009,0xb109,0xb20a,0xb30a,0xb40b,0xb50b,0xb60c,0xb70c,0xb80d,0xb90e,0xba0e,0xbb0f,0xbc10,0xbd11,0xbe12,0xbf12,0xc013,0xc114,0xc215,0xc316,0xc417,0xc518,0xc61a,0xc71b,0xc81c,0xc91d,0xca1f,0xcb20,0xcc22,0xcd23,0xce25,0xcf27,0xd028,0xd12a,0xd22c,0xd32e,0xd430,0xd532,0xd634,0xd736,0xd839,0xd93b,0xda3e,0xdb40,0xdc43,0xdd46,0xde49,0xdf4c,0xe04f,0xe152,0xe256,0xe359,0xe45d,0xe560,0xe664,0xe768,0xe86c,0xe971,0xea75,0xeb7a,0xec7f,0xed83,0xee89,0xef8e,0xf093,0xf199,0xf29f,0xf3a5,0xf4ab,0xf5b1,0xf6b8,0xf7bf,0xf8c6,0xf9cd,0xfad5,0xfbdd,0xfce5,0xfded,0xfef6,0xffff
				    };

					break;
				}
				case EIspProfile_MFNR_MSF_3:
				{
					msf_tbl = {
						0x0000,0x0100,0x0200,0x0300,0x0400,0x0500,0x0600,0x0700,0x0800,0x0900,0x0a00,0x0b00,0x0c00,0x0d00,0x0e00,0x0f00,0x1000,0x1100,0x1200,0x1300,0x1400,0x1500,0x1600,0x1700,0x1800,0x1900,0x1a00,0x1b00,0x1c00,0x1d00,0x1e00,0x1f00,0x2000,0x2100,0x2200,0x2300,0x2400,0x2500,0x2600,0x2700,0x2800,0x2900,0x2a00,0x2b00,0x2c00,0x2d00,0x2e00,0x2f00,0x3000,0x3100,0x3200,0x3300,0x3400,0x3500,0x3600,0x3700,0x3800,0x3900,0x3a00,0x3b00,0x3c00,0x3d00,0x3e00,0x3f00,0x4000,0x4100,0x4200,0x4300,0x4400,0x4500,0x4600,0x4700,0x4800,0x4900,0x4a00,0x4b00,0x4c00,0x4d00,0x4e00,0x4f00,0x5000,0x5100,0x5200,0x5300,0x5400,0x5501,0x5601,0x5701,0x5801,0x5901,0x5a01,0x5b01,0x5c01,0x5d01,0x5e01,0x5f01,0x6001,0x6102,0x6202,0x6302,0x6402,0x6502,0x6602,0x6702,0x6802,0x6903,0x6a03,0x6b03,0x6c03,0x6d03,0x6e03,0x6f03,0x7004,0x7104,0x7204,0x7304,0x7404,0x7505,0x7605,0x7705,0x7805,0x7906,0x7a06,0x7b06,0x7c06,0x7d07,0x7e07,0x7f07,0x8008,0x8108,0x8208,0x8309,0x8409,0x8509,0x860a,0x870a,0x880b,0x890b,0x8a0b,0x8b0c,0x8c0c,0x8d0d,0x8e0d,0x8f0e,0x900e,0x910f,0x920f,0x9310,0x9410,0x9511,0x9611,0x9712,0x9813,0x9913,0x9a14,0x9b15,0x9c15,0x9d16,0x9e17,0x9f18,0xa018,0xa119,0xa21a,0xa31b,0xa41c,0xa51c,0xa61d,0xa71e,0xa81f,0xa920,0xaa21,0xab22,0xac23,0xad24,0xae25,0xaf26,0xb027,0xb129,0xb22a,0xb32b,0xb42c,0xb52d,0xb62f,0xb730,0xb831,0xb933,0xba34,0xbb36,0xbc37,0xbd39,0xbe3a,0xbf3c,0xc03d,0xc13f,0xc240,0xc342,0xc444,0xc546,0xc647,0xc749,0xc84b,0xc94d,0xca4f,0xcb51,0xcc53,0xcd55,0xce57,0xcf59,0xd05c,0xd15e,0xd260,0xd362,0xd465,0xd567,0xd66a,0xd76c,0xd86f,0xd971,0xda74,0xdb77,0xdc79,0xdd7c,0xde7f,0xdf82,0xe085,0xe188,0xe28b,0xe38e,0xe491,0xe594,0xe698,0xe79b,0xe89e,0xe9a2,0xeaa5,0xeba9,0xecad,0xedb0,0xeeb4,0xefb8,0xf0bc,0xf1c0,0xf2c4,0xf3c8,0xf4cc,0xf5d0,0xf6d5,0xf7d9,0xf8dd,0xf9e2,0xfae6,0xfbeb,0xfcf0,0xfdf5,0xfefa,0xffff
				    };

					break;
				}
				case EIspProfile_MFNR_MSF_4:
				{
					msf_tbl = {
						0x0000,0x0100,0x0200,0x0300,0x0400,0x0500,0x0600,0x0700,0x0800,0x0900,0x0a00,0x0b00,0x0c00,0x0d00,0x0e00,0x0f00,0x1001,0x1101,0x1201,0x1301,0x1401,0x1501,0x1601,0x1702,0x1802,0x1902,0x1a02,0x1b02,0x1c03,0x1d03,0x1e03,0x1f03,0x2004,0x2104,0x2204,0x2304,0x2405,0x2505,0x2605,0x2705,0x2806,0x2906,0x2a06,0x2b07,0x2c07,0x2d07,0x2e08,0x2f08,0x3009,0x3109,0x3209,0x330a,0x340a,0x350b,0x360b,0x370b,0x380c,0x390c,0x3a0d,0x3b0d,0x3c0e,0x3d0e,0x3e0f,0x3f0f,0x4010,0x4110,0x4211,0x4311,0x4412,0x4512,0x4613,0x4713,0x4814,0x4914,0x4a15,0x4b16,0x4c16,0x4d17,0x4e17,0x4f18,0x5019,0x5119,0x521a,0x531b,0x541b,0x551c,0x561d,0x571d,0x581e,0x591f,0x5a1f,0x5b20,0x5c21,0x5d21,0x5e22,0x5f23,0x6024,0x6124,0x6225,0x6326,0x6427,0x6528,0x6628,0x6729,0x682a,0x692b,0x6a2c,0x6b2c,0x6c2d,0x6d2e,0x6e2f,0x6f30,0x7031,0x7132,0x7232,0x7333,0x7434,0x7535,0x7636,0x7737,0x7838,0x7939,0x7a3a,0x7b3b,0x7c3c,0x7d3d,0x7e3e,0x7f3f,0x8040,0x8141,0x8242,0x8343,0x8444,0x8545,0x8646,0x8747,0x8848,0x8949,0x8a4a,0x8b4b,0x8c4c,0x8d4d,0x8e4f,0x8f50,0x9051,0x9152,0x9253,0x9354,0x9455,0x9557,0x9658,0x9759,0x985a,0x995b,0x9a5d,0x9b5e,0x9c5f,0x9d60,0x9e61,0x9f63,0xa064,0xa165,0xa266,0xa368,0xa469,0xa56a,0xa66c,0xa76d,0xa86e,0xa970,0xaa71,0xab72,0xac74,0xad75,0xae76,0xaf78,0xb079,0xb17a,0xb27c,0xb37d,0xb47f,0xb580,0xb681,0xb783,0xb884,0xb986,0xba87,0xbb89,0xbc8a,0xbd8c,0xbe8d,0xbf8f,0xc090,0xc192,0xc293,0xc395,0xc496,0xc598,0xc699,0xc79b,0xc89c,0xc99e,0xcaa0,0xcba1,0xcca3,0xcda4,0xcea6,0xcfa8,0xd0a9,0xd1ab,0xd2ac,0xd3ae,0xd4b0,0xd5b1,0xd6b3,0xd7b5,0xd8b6,0xd9b8,0xdaba,0xdbbc,0xdcbd,0xddbf,0xdec1,0xdfc3,0xe0c4,0xe1c6,0xe2c8,0xe3ca,0xe4cb,0xe5cd,0xe6cf,0xe7d1,0xe8d3,0xe9d4,0xead6,0xebd8,0xecda,0xeddc,0xeede,0xefe0,0xf0e1,0xf1e3,0xf2e5,0xf3e7,0xf4e9,0xf5eb,0xf6ed,0xf7ef,0xf8f1,0xf9f3,0xfaf5,0xfbf7,0xfcf9,0xfdfb,0xfefd,0xffff
				    };

					break;
				}
				case EIspProfile_MFNR_MSF_5:
				{
					msf_tbl = {
						0x0000,0x0100,0x0200,0x0300,0x0400,0x0500,0x0600,0x0700,0x0800,0x0900,0x0a00,0x0b00,0x0c00,0x0d00,0x0e00,0x0f00,0x1001,0x1101,0x1201,0x1301,0x1401,0x1501,0x1601,0x1702,0x1802,0x1902,0x1a02,0x1b02,0x1c03,0x1d03,0x1e03,0x1f03,0x2004,0x2104,0x2204,0x2304,0x2405,0x2505,0x2605,0x2705,0x2806,0x2906,0x2a06,0x2b07,0x2c07,0x2d07,0x2e08,0x2f08,0x3009,0x3109,0x3209,0x330a,0x340a,0x350b,0x360b,0x370b,0x380c,0x390c,0x3a0d,0x3b0d,0x3c0e,0x3d0e,0x3e0f,0x3f0f,0x4010,0x4110,0x4211,0x4311,0x4412,0x4512,0x4613,0x4713,0x4814,0x4914,0x4a15,0x4b16,0x4c16,0x4d17,0x4e17,0x4f18,0x5019,0x5119,0x521a,0x531b,0x541b,0x551c,0x561d,0x571d,0x581e,0x591f,0x5a1f,0x5b20,0x5c21,0x5d21,0x5e22,0x5f23,0x6024,0x6124,0x6225,0x6326,0x6427,0x6528,0x6628,0x6729,0x682a,0x692b,0x6a2c,0x6b2c,0x6c2d,0x6d2e,0x6e2f,0x6f30,0x7031,0x7132,0x7232,0x7333,0x7434,0x7535,0x7636,0x7737,0x7838,0x7939,0x7a3a,0x7b3b,0x7c3c,0x7d3d,0x7e3e,0x7f3f,0x8040,0x8141,0x8242,0x8343,0x8444,0x8545,0x8646,0x8747,0x8848,0x8949,0x8a4a,0x8b4b,0x8c4c,0x8d4d,0x8e4f,0x8f50,0x9051,0x9152,0x9253,0x9354,0x9455,0x9557,0x9658,0x9759,0x985a,0x995b,0x9a5d,0x9b5e,0x9c5f,0x9d60,0x9e61,0x9f63,0xa064,0xa165,0xa266,0xa368,0xa469,0xa56a,0xa66c,0xa76d,0xa86e,0xa970,0xaa71,0xab72,0xac74,0xad75,0xae76,0xaf78,0xb079,0xb17a,0xb27c,0xb37d,0xb47f,0xb580,0xb681,0xb783,0xb884,0xb986,0xba87,0xbb89,0xbc8a,0xbd8c,0xbe8d,0xbf8f,0xc090,0xc192,0xc293,0xc395,0xc496,0xc598,0xc699,0xc79b,0xc89c,0xc99e,0xcaa0,0xcba1,0xcca3,0xcda4,0xcea6,0xcfa8,0xd0a9,0xd1ab,0xd2ac,0xd3ae,0xd4b0,0xd5b1,0xd6b3,0xd7b5,0xd8b6,0xd9b8,0xdaba,0xdbbc,0xdcbd,0xddbf,0xdec1,0xdfc3,0xe0c4,0xe1c6,0xe2c8,0xe3ca,0xe4cb,0xe5cd,0xe6cf,0xe7d1,0xe8d3,0xe9d4,0xead6,0xebd8,0xecda,0xeddc,0xeede,0xefe0,0xf0e1,0xf1e3,0xf2e5,0xf3e7,0xf4e9,0xf5eb,0xf6ed,0xf7ef,0xf8f1,0xf9f3,0xfaf5,0xfbf7,0xfcf9,0xfdfb,0xfefd,0xffff
				    };
					break;
				}
				default:
					break;
			}

			char __strTuningFile[512] = {'\0'};
			sprintf(__strTuningFile, "/data/vendor/camera_dump/%09d-%04d-%04d-%d-%d.msftbl", rIspInfo.hint.UniqueKey, rIspInfo.hint.RequestNo, rIspInfo.hint.FrameNo, m_eSensorDev, rIspInfo.rCamInfo.rMapping_Info.eIspProfile);

			FILE* __fidTuning = fopen(__strTuningFile, "wb");
			if (__fidTuning)
			{
				fwrite((MUINT32*)msf_tbl.set, sizeof(ISP_NVRAM_MSF_TBL_T), 1, __fidTuning);
				fclose(__fidTuning);
			}

		}
    }

    IdxMgr::createInstance(static_cast<ESensorDev_T>(m_eSensorDev))->setMappingInfo(static_cast<ESensorDev_T>(m_eSensorDev), rIspInfo.rCamInfo.rMapping_Info, rIspInfo.rCamInfo.u4Id);

lbExit:
    if  ( MERR_OK != err )
    {
        CAM_LOGE("[-do_validatePerFrameP2]err(%X)", err);
    }

    return  err;
}


MVOID
Paramctrl::
ISOThresholdCheck(const ISP_INFO_T& rIspInfo, MBOOL &b1Update)
{
    MBOOL bUseIsoThreshEnable;
    getPropInt("vendor.debug.isp.isoThresh.enable", &bUseIsoThreshEnable, 1);
    getPropInt("vendor.debug.isp.isoThresh", &m_isoThresh, 5);

    MINT32 i4RealISO = static_cast<MINT32>(rIspInfo.rCamInfo.rAEInfo.u4P2RealISOValue);
    MINT32 i4ISO_L = static_cast<MINT32>(rIspInfo.rCamInfo.eIdx_ISO_L[0]);
    MINT32 i4ZoomRatio_x100 = static_cast<MINT32>(rIspInfo.rCamInfo.i4ZoomRatio_x100);
    MBOOL  bRrzOn = rIspInfo.rCamInfo.rCropRzInfo.fgRRZOnOff;
    MBOOL  bQuery = !(m_rIdxCache.isSameKey(IDXCACHE_STAGE_P2));
    memcpy(&m_hint_p2, &rIspInfo.hint, sizeof(NSCam::TuningUtils::FILE_DUMP_NAMING_HINT));

    if(bUseIsoThreshEnable)
    {
        CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_VALIDATE_P2, "[%s] m_bFirstPrepare(%d), i4ISO_L/m_i4PreISO_L(%d/%d), u4RealISO/m_i4PreRealISO(%d/%d), isoThresh(%d), m_i4PreRealISO*isoThresh/100(%d), i4ZoomRatio_x100/(%d)m_i4PreZoomRatio_x100(%d), eControlMode(%d)",
            __FUNCTION__, m_bFirstPrepare, i4ISO_L, m_i4PreISO_L, i4RealISO, m_i4PreRealISO, m_isoThresh, m_i4PreRealISO*m_isoThresh/100, i4ZoomRatio_x100, m_i4PreZoomRatio_x100, rIspInfo.rCamInfo.eControlMode);
        if(m_bFirstPrepare
            || i4ISO_L != m_i4PreISO_L
            || (abs(i4RealISO - m_i4PreRealISO) > m_i4PreRealISO*m_isoThresh/100) \
            || (rIspInfo.rCamInfo.rMapping_Info.eIspProfile != EIspProfile_Preview && rIspInfo.rCamInfo.rMapping_Info.eIspProfile != EIspProfile_Video)
            || i4ZoomRatio_x100 != m_i4PreZoomRatio_x100
            || rIspInfo.rCamInfo.rMapping_Info.eIspProfile != m_i4PrePrevProfile
            || bRrzOn != m_bPreRrzOn
            || bQuery
            || rIspInfo.rCamInfo.eControlMode == MTK_CONTROL_MODE_OFF)
        {
            CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_VALIDATE_P2, "[%s] Use new HW reg(%d)", __FUNCTION__, rIspInfo.rCamInfo.u4Id);
            b1Update = MTRUE;
            m_i4PreISO_L = i4ISO_L;
            m_i4PreRealISO = i4RealISO;
            m_bFirstPrepare = MFALSE;
            m_i4PreZoomRatio_x100 = i4ZoomRatio_x100;
            m_i4PrePrevProfile = static_cast<MINT32>(rIspInfo.rCamInfo.rMapping_Info.eIspProfile);
            m_bPreRrzOn = bRrzOn;
        }
        else
        {
            CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_VALIDATE_P2, "[%s] Use previous HW reg(%d)", __FUNCTION__, rIspInfo.rCamInfo.u4Id);
        }
    }
    else
    {
        b1Update = MTRUE;
    }
    m_bUseIsoThreshEnable = !b1Update;

}


MBOOL
Paramctrl::
debugCTL_Collecttion_PerFrame()
{

    getPropInt("vendor.isp.obc_r1.disable", &m_DebugCTL_Disable.bOBC_R1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.obc_d1.disable", &m_DebugCTL_Disable.bOBC_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.mobc_r2.disable", &m_DebugCTL_Disable.bMOBC_R2, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.bpc_r1.disable", &m_DebugCTL_Disable.bBPC_BPC_R1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.ct_r1.disable", &m_DebugCTL_Disable.bBPC_CT_R1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.pdc_r1.disable", &m_DebugCTL_Disable.bBPC_PDC_R1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.bpc_d1.disable", &m_DebugCTL_Disable.bBPC_BPC_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.ct_d1.disable", &m_DebugCTL_Disable.bBPC_CT_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.pdc_d1.disable", &m_DebugCTL_Disable.bBPC_PDC_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.dgn_r1.disable", &m_DebugCTL_Disable.bDGN_R1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.dgn_d1.disable", &m_DebugCTL_Disable.bDGN_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.lsc_r1.disable", &m_DebugCTL_Disable.bLSC_R1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.lsc_d1.disable", &m_DebugCTL_Disable.bLSC_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.wb_r1.disable", &m_DebugCTL_Disable.bWB_R1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.wb_d1.disable", &m_DebugCTL_Disable.bWB_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.hlr_r1.disable", &m_DebugCTL_Disable.bHLR_R1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.hlr_d1.disable", &m_DebugCTL_Disable.bHLR_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.ltm_r1.disable", &m_DebugCTL_Disable.bLTM_R1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.ltm_d1.disable", &m_DebugCTL_Disable.bLTM_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.slk_r1.disable", &m_DebugCTL_Disable.bSLK_R1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.slk_r2.disable", &m_DebugCTL_Disable.bSLK_R2, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.slk_d1.disable", &m_DebugCTL_Disable.bSLK_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.slk_d2.disable", &m_DebugCTL_Disable.bSLK_D2, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.slk_d3.disable", &m_DebugCTL_Disable.bSLK_D3, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.slk_d4.disable", &m_DebugCTL_Disable.bSLK_D4, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.slk_d5.disable", &m_DebugCTL_Disable.bSLK_D5, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.slk_d6.disable", &m_DebugCTL_Disable.bSLK_D6, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.dm_r1.disable", &m_DebugCTL_Disable.bDM_R1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.dm_d1.disable", &m_DebugCTL_Disable.bDM_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.flc_r1.disable", &m_DebugCTL_Disable.bFLC_R1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.flc_d1.disable", &m_DebugCTL_Disable.bFLC_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.ccm_r1.disable", &m_DebugCTL_Disable.bCCM_R1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.ccm_d1.disable", &m_DebugCTL_Disable.bCCM_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.ccm_d2.disable", &m_DebugCTL_Disable.bCCM_D2, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.ccm_d3.disable", &m_DebugCTL_Disable.bCCM_D3, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.ggm_r1.disable", &m_DebugCTL_Disable.bGGM_R1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.ggm_r2.disable", &m_DebugCTL_Disable.bGGM_R2, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.ggm_d1.disable", &m_DebugCTL_Disable.bGGM_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.ggm_d2.disable", &m_DebugCTL_Disable.bGGM_D2, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.ggm_d3.disable", &m_DebugCTL_Disable.bGGM_D3, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.iggm_d1.disable", &m_DebugCTL_Disable.bIGGM_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.g2c_r1.disable", &m_DebugCTL_Disable.bG2C_R1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.g2c_r2.disable", &m_DebugCTL_Disable.bG2C_R2, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.g2c_d1.disable", &m_DebugCTL_Disable.bG2C_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.g2cx_d1.disable", &m_DebugCTL_Disable.bG2CX_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.c2g_d1.disable", &m_DebugCTL_Disable.bC2G_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.ynrs_r1.disable", &m_DebugCTL_Disable.bYNRS_R1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.ynr_d1.disable", &m_DebugCTL_Disable.bYNR_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.ynr_d1_link.disable", &m_DebugCTL_Disable.bYNR_D1_LINK, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.ynr_d1_map.disable", &m_DebugCTL_Disable.bYNR_D1_MAP, 1); // 0: enable, 1: disable

    getPropInt("vendor.isp.ldnr_d1.disable", &m_DebugCTL_Disable.bLDNR_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.ndg_d1.disable", &m_DebugCTL_Disable.bNDG_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.ndg_d2.disable", &m_DebugCTL_Disable.bNDG_D2, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.lces_r1.disable", &m_DebugCTL_Disable.bLCES_R1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.lce_d1.disable", &m_DebugCTL_Disable.bLCE_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.dces_d1.disable", &m_DebugCTL_Disable.bDCES_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.dce_d1.disable", &m_DebugCTL_Disable.bDCE_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.ee_d1.disable", &m_DebugCTL_Disable.bEE_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.cnr_d1.disable", &m_DebugCTL_Disable.bCNR_CNR_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.ccr_d1.disable", &m_DebugCTL_Disable.bCNR_CCR_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.abf_d1.disable", &m_DebugCTL_Disable.bCNR_ABF_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.bok_d1.disable", &m_DebugCTL_Disable.bCNR_BOK_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.color_d1.disable", &m_DebugCTL_Disable.bCOLOR_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.mix_d3.disable", &m_DebugCTL_Disable.bMIX_D3, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.mfb_d1.disable", &m_DebugCTL_Disable.bMFB_D1, 0); // 0: enable, 1: disable

    getPropInt("vendor.isp.nr3d_d1.disable", &m_DebugCTL_Disable.bNR3D_D1, 0); // 0: enable, 1: disable


    return MTRUE;
}




