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

#include <cutils/properties.h>
#include <mtkcam/utils/sys/IFileCache.h>

using namespace NS3Av3;
using namespace NSIspTuning;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
validatePerFrameP1(RequestSet_T const RequestSet, MBOOL bReCalc, MINT32 i4SubsampleIdex)
{
    MERROR_ENUM err = MERR_UNKNOWN;

    MBOOL fgKeep = RequestSet.fgKeep;
    MINT32 i4FrameID = RequestSet.vNumberSet[0];
    CAM_LOGD_IF(m_bDebugEnable, "[validatePerFrameP1] i4FrameID(%d), keep(%d)", i4FrameID, fgKeep);

    std::lock_guard<std::mutex> lock(m_Lock);

    //m_DualPD_PureRaw = RequestSet.fgDisableP1;

    m_rIspCamInfo.u4Id = i4FrameID;
    m_rIspCamInfo.fgNeedKeepP1 = fgKeep;


    CAM_IDX_QRY_COMB rMapping_Info;
    IdxMgr::createInstance(static_cast<NSIspTuning::ESensorDev_T>(m_eSensorDev))->getMappingInfo(static_cast<NSIspTuning::ESensorDev_T>(m_eSensorDev), rMapping_Info, m_rIspCamInfo.u4Id);
    m_rIspCamInfo.rMapping_Info.eSize = rMapping_Info.eSize;
    m_rIspCamInfo.rMapping_Info.eFlash = rMapping_Info.eFlash;
    m_rIspCamInfo.rMapping_Info.eFaceDetection = rMapping_Info.eFaceDetection;

    IdxMgr::createInstance(static_cast<NSIspTuning::ESensorDev_T>(m_eSensorDev))->setMappingInfoByDim(static_cast<NSIspTuning::ESensorDev_T>(m_eSensorDev), m_rIspCamInfo.u4Id, EDim_ISO, (MVOID*)&m_rIspCamInfo.rMapping_Info.eISO_Idx);
    IdxMgr::createInstance(static_cast<NSIspTuning::ESensorDev_T>(m_eSensorDev))->setMappingInfoByDim(static_cast<NSIspTuning::ESensorDev_T>(m_eSensorDev), m_rIspCamInfo.u4Id, EDim_LV, (MVOID*)&m_rIspCamInfo.rMapping_Info.eLV_Idx);
    IdxMgr::createInstance(static_cast<NSIspTuning::ESensorDev_T>(m_eSensorDev))->setMappingInfoByDim(static_cast<NSIspTuning::ESensorDev_T>(m_eSensorDev), m_rIspCamInfo.u4Id, EDim_CT, (MVOID*)&m_rIspCamInfo.rMapping_Info.eCT_Idx);

    MINT32 cacheEnable = property_get_int32("vendor.debug.cache.mode", 1);
    cacheEnable = cacheEnable && (m_rIspCamInfo.eControlMode != MTK_CONTROL_MODE_OFF);
    m_rIdxCache.setEnable(cacheEnable);
    m_rIdxCache.updateKey(m_rIspCamInfo.rMapping_Info, IDXCACHE_STAGE_P1);
    m_rIdxCache.setLogEnable();

#if 0
    if(getOperMode() == EOperMode_Meta) {
        err = MERR_OK;
        CAM_LOGD("[validatePerFrame] Meta Mode\n");
        goto lbExit;
    }
#endif

    if (m_bDebugEnable) {
        CAM_LOGD("%s()\n", __FUNCTION__);
        //IspDebug::getInstance().dumpIspDebugMessage();
    }

    CAM_LOGD_IF(m_bDebugEnable, "[validatePerFrameP1](ParamChangeCount)=(%d)", getParamChangeCount());

    //  (1) reset to 0 since all params have been applied.
    resetParamChangeCount();

    //  (2) Do something.
    if (fgKeep)
    {
        err = MERR_OK;
        prepareHw_PerFrame_LSC(0); // lsc pass1
        if (!applyToHw_PerFrame_P1(RequestSet, bReCalc, i4SubsampleIdex))
        {
            err = MERR_SET_ISP_REG;
        }
    }
    else
    {
        err = do_validatePerFrameP1(RequestSet, bReCalc, i4SubsampleIdex);
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

    CAM_LOGD_IF(m_bDebugEnable, "[validatePerFrameP2] i4FrameID = %d", rIspInfo.rCamInfo.u4Id);

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

    MUINT32 DcsFrmId = ((TuningParam*)pTuningBuf)->i4DcsMagicNo;
    void* pDcsBuf = ((TuningParam*)pTuningBuf)->pDcsBuf;
    if(( rIspInfo.rCamInfo.u4Id - DcsFrmId ) > 2){
        pDcsBuf = NULL;
        ALOGD("DCESO too old, use default setting, u4id(%d) DcsFrmId(%d)", rIspInfo.rCamInfo.u4Id, DcsFrmId);
    }

    MBOOL bSlave = ((TuningParam*)pTuningBuf)->bSlave;
    void* pDualSynInfo = ((TuningParam*)pTuningBuf)->pDualSynInfo;

    //  (2) Do something.

    setLCESOBuffer(pLcsBuf);
    setDCESOBuffer(pDcsBuf);

    if(pDualSynInfo){
        //slave get info
        if(bSlave) setDualSyncInfo(pDualSynInfo);
        else clearDualSyncInfo();
    }

    err = (do_validatePerFrameP2(flowType, rIspInfo, pRegBuf, pMfbBuf));

    if(pDualSynInfo){
        //master set info
        if(!bSlave) getDualSyncInfo(pDualSynInfo);
        else clearDualSyncInfo();
    }

    MBOOL bDumpface = MFALSE;
    MINT32 captureDump = property_get_int32("vendor.isp.faceinfo.dump", 0);
    MINT32 previewDump = property_get_int32("vendor.isp.faceinfo.dump.preview", 0);

    if ((!rIspInfo.rCamInfo.fgRPGEnable && captureDump && (rIspInfo.rCamInfo.bEnableNDD || (rIspInfo.rCamInfo.i4P2InImgFmt != EYuv2Yuv)))
        || previewDump)
        bDumpface = MTRUE;
    if (bDumpface){
        char filename[512];
        genFileName_face_info(filename, sizeof(filename), &m_hint_p2);
        android::sp<IFileCache> fidFaceDump;
        fidFaceDump = IFileCache::open(filename);
        CAM_LOGW("[%s] %s", __FUNCTION__, filename);
        if (fidFaceDump->write(&rIspInfo.rCamInfo.rFdInfo, sizeof(CAMERA_TUNING_FD_INFO_T)) != sizeof(CAMERA_TUNING_FD_INFO_T))
            CAM_LOGW("[%s] fail dump to %s", __FUNCTION__, filename);

    }

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.ynr_skin_link_force", value, "0"); // 0: enable, 1: disable
    MBOOL bForceSkin = atoi(value);

    MBOOL skin_link_top_control = ISP_MGR_YNR_T::getInstance(m_eSensorDev).isSkinEnable(ISP_MGR_YNR::EYNR_D1);
    MBOOL bSkin_link = ((reinterpret_cast<dip_x_reg_t*>(pRegBuf)->YNR_D1A_YNR_SKIN_CON.Bits.YNR_SKIN_LINK) || (bForceSkin && skin_link_top_control));
    reinterpret_cast<dip_x_reg_t*>(pRegBuf)->YNR_D1A_YNR_SKIN_CON.Bits.YNR_SKIN_LINK = bSkin_link;

    if (bSkin_link){
            //get map if skin link is set enable
            ((TuningParam*)pTuningBuf)->pFaceAlphaBuf = ISP_MGR_YNR_T::getInstance(m_eSensorDev).getAlphaMap(ISP_MGR_YNR::EYNR_D1, rIspInfo.rCamInfo.u4Id);
            CAM_LOGD_IF(m_bDebugEnable,"[validatePerFrameP2] alphamap(%x) skin_link(%d)"
                ,((TuningParam*)pTuningBuf)->pFaceAlphaBuf
                , bSkin_link);
     }
     else {
         ((TuningParam*)pTuningBuf)->pFaceAlphaBuf = NULL;
         CAM_LOGD_IF(m_bDebugEnable,"[validatePerFrameP2] skin_link(%d)"
         ,((dip_x_reg_t*)pRegBuf)->YNR_D1A_YNR_SKIN_CON.Bits.YNR_SKIN_LINK);
     }

    if(rIspInfo.rCamInfo.u1P2TuningUpdate == EDSDN_setting){
        ((TuningParam*)pTuningBuf)->bDCES_Enalbe = MFALSE;
        reinterpret_cast<dip_x_reg_t*>(pRegBuf)->DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_DCES_D1_EN = MFALSE;
    } else
        ((TuningParam*)pTuningBuf)->bDCES_Enalbe = reinterpret_cast<dip_x_reg_t*>(pRegBuf)->DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_DCES_D1_EN;
    ((TuningParam*)pTuningBuf)->bYNR_LCES_In = reinterpret_cast<dip_x_reg_t*>(pRegBuf)->YNR_D1A_YNR_CON1.Bits.YNR_LCE_LINK;

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

    MUINT16 index = m_rIdxCache.query((EModule_T)module, eStage, eValType, qry, eMode);

    return index;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
do_validatePerFrameP1(RequestSet_T const RequestSet, MBOOL bReCalc, MINT32 i4SubsampleIdex)
{
    MERROR_ENUM err = MERR_OK;
    MBOOL prepare_rdy;
    MINT32 time_ms;

    CAM_LOGD_IF(m_bDebugEnable, "[do_validatePerFrameP1]");

    if (m_pVCoreThrottling!=NULL && *m_pVCoreThrottling) {
        CAM_LOGW("VCore Thermal Throttling!!! Disable some ISP H/W");
    }

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.interp.ctrl", value, "1"); //default: enable
    m_IspInterpCtrl = atoi(value);

    //CPTLog(Event_Pipe_3A_ISP_VALIDATE_PERFRAME_DYNAMIC_TUNING, CPTFlagStart); // Profiling Start.
    if (m_bProfileEnable)
        m_pAaaTimer->start("do_validatePerFrameP1: nvram index", m_eSensorDev, m_bProfileEnable);

    //  (1) dynamic tuning
    if (isDynamicTuning())
    {
        // Refine camera info.
        m_pIspTuningCustom->refine_CamInfo(m_rIspCamInfo);

        m_rIspCamInfo.fgHDR = m_pIspTuningCustom->is_HDRTuning(m_rIspCamInfo);
    }

    if (m_bProfileEnable)
        time_ms = m_pAaaTimer->End();

    if (m_bProfileEnable)
        m_pAaaTimer->start("do_validatePerFrameP1: prepareHw_PerFrame_P1()", m_eSensorDev, m_bProfileEnable);
    //  (2) Apply Per-Frame Parameters.

    debugCTL_Collecttion_PerFrame();

    (isDynamicBypass() == MTRUE) ? (prepare_rdy = prepareHw_PerFrame_Partial())
                                 : (prepare_rdy = prepareHw_PerFrame_P1());
    if (m_bProfileEnable)
        time_ms = m_pAaaTimer->End();

    if (m_bProfileEnable)
        m_pAaaTimer->start("do_validatePerFrameP1: applyToHw_PerFrame_P1()", m_eSensorDev, m_bProfileEnable);

    if(!prepare_rdy || ! applyToHw_PerFrame_P1(RequestSet, bReCalc, i4SubsampleIdex))  //  Apply the ispmgr's buffer to H/W.
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

    MINT32 captureDump = property_get_int32("vendor.isp.bpc_tbl.dump", 0);
    MINT32 previewDump = property_get_int32("vendor.isp.bpc_tbl.dump.preview", 0);
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
do_validatePerFrameP2(MINT32 flowType, const ISP_INFO_T& _rIspInfo, void* pRegBuf, void* pMfbBuf)
{
    MERROR_ENUM err = MERR_OK;
    MBOOL prepare_rdy = MTRUE;

    CAM_LOGD_IF(m_bDebugEnable, "[do_validatePerFrameP2]");

    ISP_INFO_T rIspInfo_temp = _rIspInfo;

    //YUV Reprocess
    // Should use YUV Reprocess Scenario as input to query ISO Group Index
    for(MINT32 group_index = 0; group_index< NVRAM_ISP_REGS_ISO_GROUP_NUM; group_index++){
         rIspInfo_temp.rCamInfo.rMapping_Info.eISO_Idx[group_index] = map_ISO_value2index(rIspInfo_temp.rCamInfo.rAEInfo.u4P2RealISOValue, (EISO_GROUP_T)group_index);
         get_ISOIdx_neighbor(rIspInfo_temp.rCamInfo, rIspInfo_temp.rCamInfo.rMapping_Info.eISO_Idx[group_index], rIspInfo_temp.rCamInfo.rAEInfo.u4P2RealISOValue, (EISO_GROUP_T)group_index);
    }
    rIspInfo_temp.rCamInfo.rMapping_Info.eCT_Idx = map_CT_value2index(rIspInfo_temp.rCamInfo.rAWBInfo.i4CCT);
    get_CTIdx_neighbor(rIspInfo_temp.rCamInfo, rIspInfo_temp.rCamInfo.rMapping_Info.eCT_Idx, rIspInfo_temp.rCamInfo.rAWBInfo.i4CCT);
    rIspInfo_temp.rCamInfo.rMapping_Info.eSize = (ESize_T)m_pIspTuningCustom->map_Target_Size_to_MappingInfo(_rIspInfo.rCamInfo.targetSize.w, _rIspInfo.rCamInfo.targetSize.h, "do_validatePerFrameP2");
    rIspInfo_temp.rCamInfo.fgHDR = m_pIspTuningCustom->is_HDRTuning(rIspInfo_temp.rCamInfo);

    m_rIdxCache.updateKey(rIspInfo_temp.rCamInfo.rMapping_Info, IDXCACHE_STAGE_P2);

    const ISP_INFO_T &rIspInfo = rIspInfo_temp;

    //  (1) dynamic tuning
    if (isDynamicTuning())
    {
        const RAWIspCamInfo& rIspCamInfo = rIspInfo.rCamInfo;

        CAM_LOGD_IF(m_bDebugEnable, "[do_validatePerFrameP2()] isDynamicBypass() = %d\n", isDynamicBypass());

        debugCTL_Collecttion_PerFrame();

        (isDynamicBypass() == MTRUE) ? (prepare_rdy = prepareHw_PerFrame_Partial(rIspInfo))
                                     : (prepare_rdy = prepareHw_PerFrame_P2(rIspInfo));
    }

    if(rIspInfo.rCamInfo.u1P2TuningUpdate == ELPCNR_8Bit_Pass1
       || rIspInfo.rCamInfo.u1P2TuningUpdate == ELPCNR_10Bit_Pass1){
        //LPCNR Pass1
        ISP_MGR_LPCNR_T::getInstance(m_eSensorDev).setEnable(ISP_MGR_LPCNR::ELPCNR_D1, MTRUE);
        getNvram_PerFrame_LPCNR(ISP_MGR_LPCNR::ELPCNR_D1, rIspInfo.rCamInfo, IDXCACHE_STAGE_P2);

        dip_x_reg_t* pReg = reinterpret_cast<dip_x_reg_t*>(pRegBuf);
        ::memset((void*)pReg, 0, sizeof(dip_x_reg_t));
        ISP_MGR_LPCNR_T::getInstance(m_eSensorDev).apply_P2(ISP_MGR_LPCNR::ELPCNR_D1, rIspInfo.rCamInfo, pReg);
        pReg->LPCNR_D1A_LPCNR_TOP.Bits.LPCNR_PASS_MODE = 0;


        if(rIspInfo.rCamInfo.u1P2TuningUpdate == ELPCNR_8Bit_Pass1)
            pReg->LPCNR_D1A_LPCNR_TOP.Bits.LPCNR_LBIT_MODE = 1;
        else
            pReg->LPCNR_D1A_LPCNR_TOP.Bits.LPCNR_LBIT_MODE = 0;
    }
    else if(rIspInfo.rCamInfo.u1P2TuningUpdate == ELPCNR_8Bit_Pass2
            || rIspInfo.rCamInfo.u1P2TuningUpdate == ELPCNR_10Bit_Pass2){
        //LPCNR Pass2
        dip_x_reg_t* pReg = reinterpret_cast<dip_x_reg_t*>(pRegBuf);
        pReg->LPCNR_D1A_LPCNR_TOP.Bits.LPCNR_PASS_MODE = 1;

        if(rIspInfo.rCamInfo.u1P2TuningUpdate == ELPCNR_8Bit_Pass2)
            pReg->LPCNR_D1A_LPCNR_TOP.Bits.LPCNR_LBIT_MODE = 1;
        else
            pReg->LPCNR_D1A_LPCNR_TOP.Bits.LPCNR_LBIT_MODE = 0;
    }
    else if(rIspInfo.rCamInfo.u1P2TuningUpdate == EIdenditySetting){
        //P2 Identity Setting
        dip_x_reg_t* pReg = reinterpret_cast<dip_x_reg_t*>(pRegBuf);

        ispP2IdentitySet(pReg, MTRUE);
    }
    else if(rIspInfo.rCamInfo.u1P2TuningUpdate == EDSDN_setting){
        //P2 Identity Setting
        if(pRegBuf){
            if( (!prepare_rdy) ||
                (! applyToHw_PerFrame_P2(flowType, rIspInfo, pRegBuf)))//  Apply the ispmgr's buffer to H/W.
            {
                err = MERR_SET_ISP_REG;
                goto lbExit;
            }
        }

        dip_x_reg_t* pReg = reinterpret_cast<dip_x_reg_t*>(pRegBuf);

        ispP2IdentitySet(pReg, MFALSE);
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
        getNvram_PerFrame_MFB(ISP_MGR_MFB::EMFB_D1, rIspInfo.rCamInfo, IDXCACHE_STAGE_P2);
        applyToHw_PerFrame_MFB(flowType, rIspInfo, pMfbBuf);
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
    MBOOL bUseIsoThreshEnable = ::property_get_int32("vendor.debug.isp.isoThresh.enable", 1);
    m_isoThresh = ::property_get_int32("vendor.debug.isp.isoThresh", 5);

    MINT32 i4RealISO = static_cast<MINT32>(rIspInfo.rCamInfo.rAEInfo.u4P2RealISOValue);
    MINT32 i4ISO_L = static_cast<MINT32>(rIspInfo.rCamInfo.eIdx_ISO_L[0]);
    MINT32 i4ZoomRatio_x100 = static_cast<MINT32>(rIspInfo.rCamInfo.i4ZoomRatio_x100);
    MBOOL  bRrzOn = rIspInfo.rCamInfo.rCropRzInfo.fgRRZOnOff;
    MBOOL  bQuery = !(m_rIdxCache.isSameKey(IDXCACHE_STAGE_P2));
    memcpy(&m_hint_p2, &rIspInfo.hint, sizeof(NSCam::TuningUtils::FILE_DUMP_NAMING_HINT));

    if(bUseIsoThreshEnable)
    {
        CAM_LOGD_IF(m_bDebugEnable, "[%s] m_bFirstPrepare(%d), i4ISO_L/m_i4PreISO_L(%d/%d), u4RealISO/m_i4PreRealISO(%d/%d), isoThresh(%d), m_i4PreRealISO*isoThresh/100(%d), i4ZoomRatio_x100/(%d)m_i4PreZoomRatio_x100(%d), eControlMode(%d)",
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
            CAM_LOGD_IF(m_bDebugEnable, "[%s] Use new HW reg(%d)", __FUNCTION__, rIspInfo.rCamInfo.u4Id);
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
            CAM_LOGD_IF(m_bDebugEnable, "[%s] Use previous HW reg(%d)", __FUNCTION__, rIspInfo.rCamInfo.u4Id);
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
    char value[PROPERTY_VALUE_MAX] = {'\0'};

    property_get("vendor.isp.obc_r1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bOBC_R1 = atoi(value);

    property_get("vendor.isp.obc_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bOBC_D1 = atoi(value);

    property_get("vendor.isp.mobc_r2.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bMOBC_R2 = atoi(value);

    property_get("vendor.isp.bpc_r1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bBPC_BPC_R1 = atoi(value);

    property_get("vendor.isp.ct_r1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bBPC_CT_R1 = atoi(value);

    property_get("vendor.isp.pdc_r1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bBPC_PDC_R1 = atoi(value);

    property_get("vendor.isp.bpc_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bBPC_BPC_D1 = atoi(value);

    property_get("vendor.isp.ct_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bBPC_CT_D1 = atoi(value);

    property_get("vendor.isp.pdc_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bBPC_PDC_D1 = atoi(value);

    property_get("vendor.isp.dgn_r1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bDGN_R1 = atoi(value);

    property_get("vendor.isp.dgn_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bDGN_D1 = atoi(value);

    property_get("vendor.isp.lsc_r1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bLSC_R1 = atoi(value);

    property_get("vendor.isp.lsc_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bLSC_D1 = atoi(value);

    property_get("vendor.isp.wb_r1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bWB_R1 = atoi(value);

    property_get("vendor.isp.wb_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bWB_D1 = atoi(value);

    property_get("vendor.isp.hlr_r1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bHLR_R1 = atoi(value);

    property_get("vendor.isp.hlr_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bHLR_D1 = atoi(value);

    property_get("vendor.isp.ltm_r1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bLTM_R1 = atoi(value);

    property_get("vendor.isp.ltm_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bLTM_D1 = atoi(value);

    property_get("vendor.isp.slk_r1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bSLK_R1 = atoi(value);

    property_get("vendor.isp.slk_r2.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bSLK_R2 = atoi(value);

    property_get("vendor.isp.slk_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bSLK_D1 = atoi(value);

    property_get("vendor.isp.slk_d2.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bSLK_D2 = atoi(value);

    property_get("vendor.isp.slk_d3.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bSLK_D3 = atoi(value);

    property_get("vendor.isp.slk_d4.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bSLK_D4 = atoi(value);

    property_get("vendor.isp.slk_d5.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bSLK_D5 = atoi(value);

    property_get("vendor.isp.slk_d6.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bSLK_D6 = atoi(value);

    property_get("vendor.isp.dm_r1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bDM_R1 = atoi(value);

    property_get("vendor.isp.dm_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bDM_D1 = atoi(value);

    property_get("vendor.isp.flc_r1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bFLC_R1 = atoi(value);

    property_get("vendor.isp.flc_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bFLC_D1 = atoi(value);

    property_get("vendor.isp.ccm_r1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bCCM_R1 = atoi(value);

    property_get("vendor.isp.ccm_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bCCM_D1 = atoi(value);

    property_get("vendor.isp.ccm_d2.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bCCM_D2 = atoi(value);

    property_get("vendor.isp.ccm_d3.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bCCM_D3 = atoi(value);

    property_get("vendor.isp.ggm_r1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bGGM_R1 = atoi(value);

    property_get("vendor.isp.ggm_r2.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bGGM_R2 = atoi(value);

    property_get("vendor.isp.ggm_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bGGM_D1 = atoi(value);

    property_get("vendor.isp.ggm_d2.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bGGM_D2 = atoi(value);

    property_get("vendor.isp.ggm_d3.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bGGM_D3 = atoi(value);

    property_get("vendor.isp.iggm_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bIGGM_D1 = atoi(value);

    property_get("vendor.isp.g2c_r1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bG2C_R1 = atoi(value);

    property_get("vendor.isp.g2c_r2.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bG2C_R2 = atoi(value);

    property_get("vendor.isp.g2c_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bG2C_D1 = atoi(value);

    property_get("vendor.isp.g2cx_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bG2CX_D1 = atoi(value);

    property_get("vendor.isp.c2g_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bC2G_D1 = atoi(value);

    property_get("vendor.isp.ynrs_r1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bYNRS_R1 = atoi(value);

    property_get("vendor.isp.ynr_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bYNR_D1 = atoi(value);

    property_get("vendor.isp.ynr_d1_link.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bYNR_D1_LINK = atoi(value);

    property_get("vendor.isp.ynr_d1_map.disable", value, "1"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bYNR_D1_MAP = atoi(value);

    property_get("vendor.isp.ldnr_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bLDNR_D1 = atoi(value);

    property_get("vendor.isp.ndg_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bNDG_D1 = atoi(value);

    property_get("vendor.isp.ndg_d2.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bNDG_D2 = atoi(value);

    property_get("vendor.isp.lces_r1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bLCES_R1 = atoi(value);

    property_get("vendor.isp.lce_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bLCE_D1 = atoi(value);

    property_get("vendor.isp.dces_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bDCES_D1 = atoi(value);

    property_get("vendor.isp.dce_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bDCE_D1 = atoi(value);

    property_get("vendor.isp.ee_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bEE_D1 = atoi(value);

    property_get("vendor.isp.cnr_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bCNR_CNR_D1 = atoi(value);

    property_get("vendor.isp.ccr_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bCNR_CCR_D1 = atoi(value);

    property_get("vendor.isp.abf_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bCNR_ABF_D1 = atoi(value);

    property_get("vendor.isp.bok_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bCNR_BOK_D1 = atoi(value);

    property_get("vendor.isp.color_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bCOLOR_D1 = atoi(value);

    property_get("vendor.isp.mix_d3.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bMIX_D3 = atoi(value);

    property_get("vendor.isp.mfb_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bMFB_D1 = atoi(value);

    property_get("vendor.isp.nr3d_d1.disable", value, "0"); // 0: enable, 1: disable
    m_DebugCTL_Disable.bNR3D_D1 = atoi(value);


    return MTRUE;
}




