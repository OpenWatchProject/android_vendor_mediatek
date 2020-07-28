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
#ifndef _PARAMCTRL_H_
#define _PARAMCTRL_H_

#include <drv/tuning_mgr.h>
#include <mtkcam/drv/def/ispio_sw_scenario.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <ggm_mgr.h>
#include <gma_mgr.h>
#include <lce_mgr.h>
#include <dce_mgr.h>
#include <idx_cache.h>
#include "paramctrl_if.h"
#include <aaa_timer.h>
#include <lsc/ILscMgr.h>
#include <mutex>
#include <IResultPool.h>
#include <ResultPool4LSCConfig.h>
#include <mtkcam/utils/TuningUtils/FileReadRule.h>

#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>

#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>

#include "ccu_ext_interface/ccu_ext_interface.h"
#include "iccu_ctrl_ltm.h"
#include "iccu_mgr.h"
#include <isp_config/ae_config.h>
#include <isp_config/isp_af_config.h>
#include <isp_config/isp_pdo_config.h>
#include <isp_config/isp_flicker_config.h>

using namespace NSCam;

namespace NSIspTuning
{
class CcmMgr;
class GmaMgr;
class LceMgr;

class Paramctrl : public IParamctrl
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    //  Copy constructor is disallowed.
    Paramctrl(Paramctrl const&);
    //  Copy-assignment operator is disallowed.
    Paramctrl& operator=(Paramctrl const&);

protected:
	Paramctrl(MUINT32 const eSensorDev,
			  MINT32 const i4SensorIdx,
			  MUINT32 const u4SensorID,
			  ESensorTG_T const eSensorTG,
			  NVRAM_CAMERA_ISP_PARAM_STRUCT*const pNvram_Isp);
    ~Paramctrl();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Instance
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static Paramctrl* getInstance(MUINT32 const eSensorDev, MINT32 const i4SensorIdx);
    virtual MVOID destroyInstance();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Parameters Change.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////

    template <class T>
    MBOOL checkParamChange(T const old_value, T const new_value)
    {
        if  ( old_value != new_value )
        {
            m_u4ParamChangeCount++;
            return  MTRUE;
        }
        return  MFALSE;
    }

protected:  ////
    inline MUINT32  getParamChangeCount() const { return m_u4ParamChangeCount; }
    inline MVOID    resetParamChangeCount() { m_u4ParamChangeCount = 0; }


// Data Members
private:
    //  It means that any params have changed if > 0.
    MUINT32         m_u4ParamChangeCount;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public: // Dynamic Tuning
    virtual MVOID   enableDynamicTuning(MBOOL const fgEnable);
    virtual MBOOL   isDynamicTuning() const { return m_fgDynamicTuning; }
    virtual MBOOL   enableDynamicCCM(MBOOL const fgEnable);
    virtual MBOOL   isDynamicCCM() const { return m_fgDynamicCCM; }
    virtual MBOOL   enableDynamicBypass(MBOOL const fgEnable) {m_fgDynamicBypass = fgEnable; return MTRUE; }
    virtual MBOOL   isDynamicBypass() const {return m_fgDynamicBypass; }
    virtual MBOOL   enableDynamicShading(MBOOL fgEnable);
    virtual MBOOL   isDynamicShading() const { return m_fgDynamicShading; }
    virtual MBOOL   notifyRPGEnable(MBOOL const fgEnable) {m_rIspCamInfo.fgRPGEnable = fgEnable; return MTRUE; }
    virtual MBOOL   notifyIMGOType(MBOOL fgEnable) {m_rIspCamInfo.i4RawType = (ERawType_T)fgEnable; return MTRUE; }
    virtual MBOOL   isRPGEnable() const {return m_rIspCamInfo.fgRPGEnable; }
    virtual MBOOL   forceValidate() { m_u4ParamChangeCount++; return MTRUE;}

private:
    MBOOL m_fgDynamicTuning; //  Enable dynamic tuning if true; otherwise false. It's true by default (normal mode).
    MBOOL m_fgDynamicCCM; //  Enable dynamic CCM if true; otherwise false. It's true by default (normal mode).
    MBOOL m_fgDynamicBypass;
    MBOOL m_fgDynamicShading;   // Enable dynamic shading if true; otherwise false. It's true by default.
    MBOOL m_fgEnableRPG;
    ISP_NVRAM_OBC_T m_backup_OBCInfo;
    ISP_NVRAM_RRZ_T m_RRZInfo;
    NSCam::TuningUtils::FILE_DUMP_NAMING_HINT m_hint_p2;
private:
    MINT32 m_i4RequestNum;
protected:  ////
    //  Enable Shading if true; otherwise false.
    //  It's true by default (Enable).
    //MBOOL           m_fgShadingNVRAMdataChange;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public: // Attributes
    virtual MERROR_ENUM  setIspProfile(EIspProfile_T const eIspProfile);
    virtual MERROR_ENUM  setRequestNumber(MINT32 const i4RequestNumber);
    virtual MERROR_ENUM  setSceneMode(EIndex_Scene_T const eScene);
    virtual MERROR_ENUM  setEffect(EIndex_Effect_T const eEffect);
    virtual MUINT32 getSensorDev() const { return m_eSensorDev; }
    virtual EOperMode_T getOperMode()   const { return m_eOperMode; }
    virtual MERROR_ENUM setOperMode(EOperMode_T const eOperMode);
    virtual MERROR_ENUM  setSensorMode(ESensorMode_T const eSensorMode);
    virtual MERROR_ENUM  setRawPath(MUINT32 const u4RawPath);
    virtual ESensorMode_T  getSensorMode()   const { return m_rIspCamInfo.rMapping_Info.eSensorMode; }
    virtual MERROR_ENUM setRawSize(MBOOL const fgBin, NSCam::MSize const RawSize);
    virtual MERROR_ENUM setZoomRatio(MINT32 const i4ZoomRatio_x100);
    virtual MERROR_ENUM setAWBInfo2ISP(AWB_ISP_INFO_T const &rAWBInfo);
    virtual MERROR_ENUM setIspAEPreGain2(MINT32 i4SensorIndex, AWB_GAIN_T& rNewIspAEPreGain2);
    virtual MERROR_ENUM setAEInfo2ISP(AE_ISP_INFO_T const &rAEInfo);
    virtual MERROR_ENUM setCCUInfo2ISP(ISP_CCU_RESULT_T const &rCCUInfo);
    virtual MERROR_ENUM setFlashInfo(MUINT32 const u4FlashMappingInfo);
    virtual MERROR_ENUM setIndex_Shading(MINT32 const i4IDX);
    virtual MERROR_ENUM getIndex_Shading(MVOID*const pCmdArg);
    virtual MERROR_ENUM setPureOBCInfo(const ISP_NVRAM_OBC_T *pOBCInfo);
    virtual MERROR_ENUM setRRZInfo(const ISP_NVRAM_RRZ_T *pRRZInfo);
    virtual MERROR_ENUM setDGNDebugInfo4CCU(MUINT32 const u4Rto, MUINT32 const u4P1DGNGain, NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo);
    virtual MERROR_ENUM setHLRDebugInfo4CCU(MBOOL Enable, ISP_NVRAM_HLR_T const HLR, NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo);
    virtual MERROR_ENUM setLTMnLTMSDebugInfo4CCU(MBOOL Enable, ISP_NVRAM_LTM_T const LTM_R1, ISP_NVRAM_LTM_T const LTM_R2, ISP_NVRAM_LTMS_T const LTMS, NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo);
    virtual MERROR_ENUM getPureOBCInfo(ISP_NVRAM_OBC_T *pOBCInfo);
    virtual MERROR_ENUM setTGInfo(MINT32 const i4TGInfo, MINT32 const i4Width, MINT32 const i4Height);
    virtual MERROR_ENUM setFDEnable(MBOOL const FD_enable);
    virtual MERROR_ENUM setFDInfo(CAMERA_TUNING_FD_INFO_T const FD_Info);
    virtual MBOOL getDefaultObc(MVOID*& pISP_NVRAM_Reg);
    virtual MBOOL sendIspTuningIOCtrl(MUINT32 ctrl, MINTPTR arg1, MINTPTR arg2);
    virtual MBOOL   getIspGamma(NS3Av3::S3ACtrl_GetIspGamma* pParams) const;
    virtual const RAWIspCamInfo& getCamInfo();
    virtual MVOID*  getDMGItable(MBOOL const fgRPGEnable);


    virtual EISO_T map_ISO_value2index(MUINT32 const u4Iso, EISO_GROUP_T u4ISO_Group) const;
    virtual MUINT32 map_ISO_index2value(EISO_T const eIdx, EISO_GROUP_T u4ISO_Group) const;
    virtual EZoom_T map_Zoom_value2index(MUINT32 const i4ZoomRatio_x100) const;
    virtual ELV_T map_LV_value2index(MINT32 const u4Lv) const;
    virtual ECT_T map_CT_value2index(MINT32 const u4Ct) const;


private:
    MUINT32    m_eSensorDev;
    EOperMode_T     m_eOperMode;


public:
    MUINT32 m_u4P1DirectYUV_PortInfo;

    virtual inline MBOOL    setP1DirectYUV_Port(MUINT32 const u4P1DirectYUV_Port) { m_u4P1DirectYUV_PortInfo = u4P1DirectYUV_Port; return MTRUE; }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:
    virtual MERROR_ENUM validatePerFrameP1(RequestSet_T const RequestSet, MBOOL bReCalc = MFALSE, MINT32 i4SubsampleIdex=0);
    virtual MERROR_ENUM validatePerFrameP2(MINT32 flowType, const ISP_INFO_T& rIspInfo, void* pTuningBuf);
    virtual MERROR_ENUM init(MINT32 const i4SubsampleCount=1);
    virtual MERROR_ENUM uninit();
    virtual MERROR_ENUM reinitP1TuningMgr(MINT32 const i4SubsampleCount=1);

protected:
    inline EIndex_Effect_T getEffect() const { return m_rIspCamInfo.eIdx_Effect; }
    //virtual MERROR_ENUM do_init();

private:

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////    Exif.
    virtual MERROR_ENUM getDebugInfoP1(
        NSIspExifDebug::IspExifDebugInfo_T& rDebugInfo,
        MBOOL const fgReadFromHW
    ) const;

    virtual MERROR_ENUM getDebugInfoP2(
        const NSIspTuning::ISP_INFO_T& _rIspInfo,
        NSIspExifDebug::IspExifDebugInfo_T& rIspExifDebugInfo,
        const void* pRegBuf,
        const void* pMfbBuf
    ) const;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:  //// Camera HAL 3.2
    virtual MERROR_ENUM getColorCorrectionTransform(MFLOAT& M11, MFLOAT& M12, MFLOAT& M13,
                                                    MFLOAT& M21, MFLOAT& M22, MFLOAT& M23,
                                                    MFLOAT& M31, MFLOAT& M32, MFLOAT& M33);
    virtual MERROR_ENUM setColorCorrectionTransform(MFLOAT& M11, MFLOAT& M12, MFLOAT& M13,
                                                    MFLOAT& M21, MFLOAT& M22, MFLOAT& M23,
                                                    MFLOAT& M31, MFLOAT& M32, MFLOAT& M33);
    virtual MERROR_ENUM setColorCorrectionMode(MINT32 i4ColorCorrectionMode);
    virtual MERROR_ENUM setEdgeMode(MINT32 i4EdgeMode);
    virtual MERROR_ENUM setNoiseReductionMode(MINT32 i4NRMode);
    virtual MERROR_ENUM setToneMapMode(MINT32 i4ToneMapMode);
    virtual MERROR_ENUM getTonemapCurve_Red(MFLOAT*& p_in_red, MFLOAT*& p_out_red, MINT32 *pCurvePointNum);
    virtual MERROR_ENUM getTonemapCurve_Green(MFLOAT*& p_in_green, MFLOAT*& p_out_green, MINT32 *pCurvePointNum);
    virtual MERROR_ENUM getTonemapCurve_Blue(MFLOAT*& p_in_blue, MFLOAT*& p_out_blue, MINT32 *pCurvePointNum);
    virtual MERROR_ENUM setTonemapCurve_Red(MFLOAT *p_in_red, MFLOAT *p_out_red, MINT32 *pCurvePointNum);
    virtual MERROR_ENUM setTonemapCurve_Green(MFLOAT *p_in_green, MFLOAT *p_out_green, MINT32 *pCurvePointNum);
    virtual MERROR_ENUM setTonemapCurve_Blue(MFLOAT *p_in_blue, MFLOAT *p_out_blue, MINT32 *pCurvePointNum);
    virtual MBOOL convertPtPairsToGMA(const MFLOAT* inPtPairs, MUINT32 u4NumOfPts, MINT32* outGMA);
    virtual MBOOL setFdGamma(MINT32 i4Index, MINT32* fdGammaCurve, MUINT32 u4size);
protected:  ////

    //MERROR_ENUM         saveDebugInfo();

    mutable NSIspExifDebug::IspExifDebugInfo_T    m_rIspExifDebugInfo;
    mutable NSIspExifDebug::IspGmaInfo m_GmaExifInfo;
    mutable NSIspExifDebug::IspLceInfo m_LceExifInfo;
    mutable NSIspExifDebug::IspDceInfo m_DceExifInfo;
    ISP_NVRAM_YNR_LCE_LINK_T m_sw_ynr;
    ISP_NVRAM_YNR_FD_T       m_ynr_fd;
    MSize m_FaceIsize;
    MUINT16 m_SW_YNR_LTM_Ver;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation: Per-Frame
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  // Invoked only by validatePerFrame()
    MERROR_ENUM do_validatePerFrameP1(RequestSet_T const RequestSet, MBOOL bReCalc, MINT32 i4SubsampleIdex);
    MERROR_ENUM do_validatePerFrameP2(MINT32 flowType, const ISP_INFO_T& rIspInfo, void* pRegBuf, void* pMfbBuf);

protected:  ////    All
    MBOOL   prepareHw_PerFrame_P1();
    MBOOL   prepareHw_PerFrame_Partial();
    MBOOL   prepareHw_PerFrame_P2(const ISP_INFO_T& rIspInfo);
    MBOOL   prepareHw_PerFrame_Partial(const ISP_INFO_T& rIspInfo);
    MBOOL   applyToHw_PerFrame_P1(RequestSet_T const RequestSet, MBOOL bReCalc, MINT32 i4SubsampleIdex);
    MBOOL   applyToHw_PerFrame_P2(MINT32 flowType, const ISP_INFO_T& rIspInfo, void* pRegBuf);
    MBOOL   applyToHw_PerFrame_MFB(MINT32 flowType, const ISP_INFO_T& rIspInfo, void* pMfbBuf);
    MVOID   ispP2HWConstraintSet(dip_x_reg_t* pReg);
    MVOID   ispP2IdentitySet(dip_x_reg_t* pReg, MBOOL bReset);

protected:
    MBOOL check_ISO_env_info();
    MBOOL check_CT_env_info();
    MVOID setLCESOBuffer(void* lcesBuffer);
    MVOID setDCESOBuffer(void* dcesBuffer);
    MVOID setDualSyncInfo(void* pDualSynInfo);
    MVOID getDualSyncInfo(void* pDualSynInfo);
    MVOID clearDualSyncInfo();
    MVOID get_ISOIdx_neighbor(RAWIspCamInfo& rIspCamInfo, EISO_T const IsoIdx, MUINT32 const u4Iso, EISO_GROUP_T u4ISO_Group);
    MVOID get_CTIdx_neighbor(RAWIspCamInfo& rIspCamInfo, ECT_T const CTIdx, MUINT32 const u4CT);
    MVOID get_LVIdx_neighbor(RAWIspCamInfo& rIspCamInfo, ELV_T const LVIdx, MINT32 const i4LVx10);
    MBOOL isIspInterpolation(const RAWIspCamInfo& rIspCamInfo);
    MBOOL getP1ExifSizeInfo (MUINT32& u4RegCount, MUINT32& u4StartRegCount) const;
    MBOOL getP2ExifSizeInfo (MUINT32& u4RegCount, MUINT32& u4StartRegCount)const;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////    Module.

    MUINT16 getISPIndex(IDXCACHE_VALTYPE eValType, const RAWIspCamInfo& rIspCamInfo, MUINT32 module, IDXCACHE_STAGE eStage, IDXCACHE_QRYMODE eMode = IDXCACHE_QRYMODE_NORMAL);
    MBOOL   prepareHw_PerFrame_AF(MUINT32 MagicNum, AFResultConfig_T* pReconfigAF = NULL);
    MBOOL   prepareHw_PerFrame_PDO(MUINT32 MagicNum, isp_pdo_cfg_t* pReconfigPDO = NULL);
    MBOOL   prepareHw_PerFrame_AE(MUINT32 MagicNum, AEResultConfig_T* pWT_ReConfigAE = NULL);
    MBOOL   prepareHw_PerFrame_AWB();
    MBOOL   prepareHw_PerFrame_Flicker(MUINT32 MagicNum, FLKResultConfig_T* pWT_ReConfigFLK = NULL);

//Shading
    MBOOL   prepareHw_PerFrame_LSC(MUINT8 SubModuleIndex);

//ADB Debug Collection
    MBOOL   debugCTL_Collecttion_PerFrame();
//ISP Pass1 Raw
    MBOOL   decision_PerFrame_P1_OBC();
    MBOOL   decision_PerFrame_P1_LSC();
    MBOOL   decision_PerFrame_P1_BPC();
    MBOOL   decision_PerFrame_P1_CT();
    MBOOL   decision_PerFrame_P1_PDC();
    MBOOL   decision_PerFrame_P1_DGN();
    MBOOL   decision_PerFrame_P1_WB();
    MBOOL   decision_PerFrame_P1_LCES();
    MBOOL   decision_PerFrame_P1_HLR();
    MBOOL   decision_PerFrame_P1_LTM();
//ISP Pass1 Direct YUV
    MBOOL   decision_PerFrame_P1_SLK();
    MBOOL   decision_PerFrame_P1_DM();
    MBOOL   decision_PerFrame_P1_FLC();
    MBOOL   decision_PerFrame_P1_CCM();
    MBOOL   decision_PerFrame_P1_GGM();
    MBOOL   decision_PerFrame_P1_G2C();
    MBOOL   decision_PerFrame_P1_YNRS();
//ISP Pass2 Raw
    MBOOL   decision_PerFrame_P2_OBC(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_LSC(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_BPC(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_CT(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_PDC(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_DGN(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_WB(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_HLR(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_LTM(const RAWIspCamInfo& rIspCamInfo);
//ISP Pass2 After Raw
    MBOOL   decision_PerFrame_P2_SLK(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_DM(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_LDNR(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_FLC(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_CCM(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_GGM(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_G2C(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_NDG(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_YNR(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_YNR_LINK(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_LCE(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_DCES(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_DCE(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_EE(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_CNR(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_CCR(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_ABF(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_BOK(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_COLOR(const RAWIspCamInfo& rIspCamInfo);
//feature
    MBOOL   decision_PerFrame_P2_NR3D(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_MFB(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_MIX(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   decision_PerFrame_P2_LPCNR(const RAWIspCamInfo& rIspCamInfo);
//Get Index
    MVOID   ISOThresholdCheck(const ISP_INFO_T& rIspInfo, MBOOL &b1Update);
    MBOOL   get_P1_AllIndex(const RAWIspCamInfo& rIspCamInfo);
    MBOOL   get_P2_AllIndex(const RAWIspCamInfo& rIspCamInfo);

    MBOOL   getNvram_PerFrame_OBC(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_BPC(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_CT(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_PDC(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_DM(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_LDNR(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_FLC(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_LCES(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_GGM(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_G2CX(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo);
    MBOOL   getNvram_PerFrame_CNR(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_CCR(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_ABF(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_EE(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_COLOR(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_YNRS(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_YNR(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_YNR_TBL(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_YNR_FACE(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_MFB(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_MIX(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_NR3D(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_LPCNR(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_LCE(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_DCES(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_DCE(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_CCM_P1(MUINT8 SubModuleIndex, RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);
    MBOOL   getNvram_PerFrame_CCM_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage);

    MBOOL   prepareHw_PerFrame_LSC_2(MUINT8 SubModuleIndex, const ISP_INFO_T& rIspCamInfo);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ISP End-User-Define Tuning Index.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MERROR_ENUM setIspUserIdx_Edge(EIndex_Isp_Edge_T const eIndex);
    virtual MERROR_ENUM setIspUserIdx_Hue(EIndex_Isp_Hue_T const eIndex);
    virtual MERROR_ENUM setIspUserIdx_Sat(EIndex_Isp_Saturation_T const eIndex);
    virtual MERROR_ENUM setIspUserIdx_Bright(EIndex_Isp_Brightness_T const eIndex);
    virtual MERROR_ENUM setIspUserIdx_Contrast(EIndex_Isp_Contrast_T const eIndex);

protected:

    inline
    IspUsrSelectLevel_T const&
    getIspUsrSelectLevel() const
    {
        return m_rIspCamInfo.rIspUsrSelectLevel;
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    RAWIspCamInfo       m_rIspCamInfo;
    //RAWIspCamInfo       m_rIspCamInfoP2; //used to maintain 1. mapping info 2. aeinfo 3. iso neignbor 4. lv neighbor
    IspTuningCustom*    m_pIspTuningCustom;
    IdxCache            m_rIdxCache;
private:
    IImageBuffer* m_pLCESBuffer;
    IImageBuffer* m_pDCESBuffer;
private:
    MBOOL m_bSlave;
    DUAL_ISP_SYNC_INFO_T m_ISP_Sync_Info;

//..............................................................................
//  ISP Tuning Parameters.
//..............................................................................
protected:  ////    ISP Tuning Parameters.

    //  Reference to a given buffer.
    NVRAM_CAMERA_ISP_PARAM_STRUCT&  m_rIspParam;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////    ISP Common Parameters.

    //  Reference to m_rIspParam.ISPComm
    ISP_NVRAM_COMMON_STRUCT&        m_rIspComm;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////    ISP ISO Partition.

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
private:  ////    ISP Register Parameters.
    ISP_DEBUG_CONTROL_T m_DebugCTL_Disable;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:
    // CCM Manager
    CcmMgr* m_pCcmMgr;

    // GGM Manager
    GgmMgr* m_pGgmMgr;

    // GMA Manager
    GmaMgr* m_pGmaMgr;

    // LCE Manager
    LceMgr* m_pLceMgr;

    // DCE Manager
    DceMgr* m_pDceMgr;

    // CCU
    NSCcuIf::ICcuCtrlLtm* m_pCcuIsp;

//..............................................................................
//  ISP Shading Parameters.
//..............................................................................
protected:  ////    ISP Shading Manager.

    //  LSC Manager.
    NVRAM_CAMERA_SHADING_STRUCT     *m_pNvram_Shading;
    NSIspTuning::ILscMgr            *m_pLscMgr;
    TuningMgr                       *m_pTuning;

    mutable std::mutex  m_Lock;
    MBOOL m_bDebugEnable;
    MBOOL m_bProfileEnable;
    ESensorTG_T m_eSensorTG;
    MINT32 m_i4SensorIdx;
    NS3Av3::AaaTimer* m_pAaaTimer;
    mtk_camera_metadata_enum_android_color_correction_mode_t m_eColorCorrectionMode;
    MINT32  m_i4SubsampleCount;
    MINT32 m_IspInterpCtrl;
    ISP_SWALGO_VER_T m_ISP_ALG_VER;
    ISP_FEATURE_TOP_CTL* m_pFeatureCtrl;
    MUINT32 m_u4RawFmtType;
    ISP_NVRAM_COLOR_T m_SmoothColor_Prv;
    MBOOL m_bSmoothColor_FirstTimeBoot;
    MBOOL   m_bFirstPrepare;
    MINT32  m_i4PreISO_L;
    MINT32  m_i4PreRealISO;
    MINT32  m_i4PreZoomRatio_x100;
    MBOOL   m_bPreRrzOn;
    MINT32  m_isoThresh;
    MBOOL   m_bUseIsoThreshEnable;
    MINT32  m_i4PrePrevProfile;
    LSCConfigResult_T m_pISPLscResult;
    IResultPool* m_pResultPoolObj;
    EISO_GROUP_T m_ISO_Group_P1;
    EISO_GROUP_T m_ISO_Group_P2;
    NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM m_ERawPath;


//..............................................................................
//  used for v-core throttling
//..............................................................................
private:
    int m_fVCoreThrottling;
    char *m_pVCoreThrottling;
};


};  //  namespace NSIspTuningv3
#endif // _PARAMCTRL_H_
