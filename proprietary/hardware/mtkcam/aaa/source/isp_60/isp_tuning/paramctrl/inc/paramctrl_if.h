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
#ifndef _PARAMCTRL_IF_H_
#define _PARAMCTRL_IF_H_

//#include <awb_param.h>
//#include <af_param.h>
//#include <flash_param.h>
//#include <ae_param.h>
#include <Local.h>
#include <private/aaa_hal_private.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>

namespace NSIspTuning
{

class IParamctrl
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    //  Copy constructor is disallowed.
    IParamctrl(IParamctrl const&);
    //  Copy-assignment operator is disallowed.
    IParamctrl& operator=(IParamctrl const&);

protected:
    IParamctrl(MUINT32 const /*eSensorDev*/) {}
    virtual ~IParamctrl() {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Instance
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static IParamctrl* createInstance(MUINT32 const eSensorDev, MINT32 const i4SensorIdx);
    virtual MVOID destroyInstance() = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public: // Dynamic Tuning
    virtual MVOID       enableDynamicTuning(MBOOL const fgEnable) = 0;
    virtual MBOOL       isDynamicTuning() const = 0;

    virtual MBOOL       enableDynamicCCM(MBOOL const fgEnable) = 0;
    virtual MBOOL       isDynamicCCM() const = 0;

	virtual MBOOL   	enableDynamicBypass(MBOOL const fgEnable) = 0;
	virtual MBOOL   	isDynamicBypass() const = 0;

    virtual MBOOL   	enableDynamicShading(MBOOL fgEnable) = 0;
	virtual MBOOL   	isDynamicShading() const = 0;

    virtual MBOOL   	notifyRPGEnable(MBOOL const fgEnable) = 0;
    virtual MBOOL   	notifyIMGOType(MBOOL fgEnable) = 0;
    virtual MBOOL   	isRPGEnable() const = 0;
    virtual MBOOL       forceValidate() = 0;
    virtual inline MBOOL        setP1DirectYUV_Port(MUINT32 const u4P1DirectYUV_Port) = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     // Attributes
    virtual MERROR_ENUM  setIspProfile(EIspProfile_T const eIspProfile) = 0;
    virtual MERROR_ENUM  setRequestNumber(MINT32 const i4RequestNumber) = 0;
    virtual MERROR_ENUM  setSceneMode(EIndex_Scene_T const eScene) = 0;
    virtual MERROR_ENUM  setEffect(EIndex_Effect_T const eEffect) = 0;
    virtual MUINT32 getSensorDev() const = 0;
    virtual EOperMode_T  getOperMode()   const = 0;
    virtual MBOOL        getIspGamma(NS3Av3::S3ACtrl_GetIspGamma* pParams) const = 0;
    virtual MERROR_ENUM  setSensorMode(ESensorMode_T const eSensorMode) = 0;
    virtual MERROR_ENUM  setRawPath(MUINT32 const u4RawPath) = 0;
    virtual ESensorMode_T  getSensorMode()   const = 0;
    virtual MERROR_ENUM setRawSize(MBOOL const fgBin, NSCam::MSize const RawSize) = 0;
    virtual MERROR_ENUM  setOperMode(EOperMode_T const eOperMode) = 0;
    virtual MERROR_ENUM setZoomRatio(MINT32 const i4ZoomRatio_x100) = 0;
    virtual MERROR_ENUM setAWBInfo2ISP(AWB_ISP_INFO_T const &rAWBInfo) = 0;
    virtual MERROR_ENUM setIspAEPreGain2(MINT32 i4SensorIndex, AWB_GAIN_T& rNewIspAEPreGain2) = 0;
    virtual MERROR_ENUM setAEInfo2ISP(AE_ISP_INFO_T const &rAEInfo) = 0;
    virtual MERROR_ENUM setCCUInfo2ISP(ISP_CCU_RESULT_T const &rCCUInfo) = 0;
    virtual MERROR_ENUM setFlashInfo(MUINT32 const u4FlashMappingInfo) = 0;
    virtual const RAWIspCamInfo& getCamInfo() = 0;
    //virtual MERROR_ENUM setCCT(MINT32 const i4CCT) = 0;
    //virtual MERROR_ENUM setCCTIndex_CCM (
    //            MINT32 const i4CCT,
    //            MINT32 const i4FluorescentIndex
    //        ) = 0;
    //virtual MERROR_ENUM setCCTIndex_Shading(MINT32 const i4CCT) = 0;
    virtual MERROR_ENUM setIndex_Shading(MINT32 const i4IDX) = 0;
    virtual MERROR_ENUM getIndex_Shading(MVOID*const pCmdArg) = 0;
    virtual MERROR_ENUM setPureOBCInfo(const ISP_NVRAM_OBC_T *pOBCInfo) = 0;
    virtual MERROR_ENUM setRRZInfo(const ISP_NVRAM_RRZ_T *pRRZInfo) = 0;
    virtual MERROR_ENUM setDGNDebugInfo4CCU(MUINT32 const u4Rto, MUINT32 const u4P1DGNGain, NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo) = 0;
    virtual MERROR_ENUM setHLRDebugInfo4CCU(MBOOL Enable, ISP_NVRAM_HLR_T const HLR, NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo) = 0;
    virtual MERROR_ENUM setLTMnLTMSDebugInfo4CCU(MBOOL Enable, ISP_NVRAM_LTM_T const LTM_R1, ISP_NVRAM_LTM_T const LTM_R2, ISP_NVRAM_LTMS_T const LTMS, NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo) = 0;
    virtual MERROR_ENUM getPureOBCInfo(ISP_NVRAM_OBC_T *pOBCInfo) = 0;
    virtual MERROR_ENUM setTGInfo(MINT32 const i4TGInfo, MINT32 const i4Width, MINT32 const i4Height) = 0;
    virtual MERROR_ENUM setFDEnable(MBOOL const FD_enable) = 0;
    virtual MERROR_ENUM setFDInfo(CAMERA_TUNING_FD_INFO_T const FD_Info) = 0;
    virtual MVOID*      getDMGItable(MBOOL const fgRPGEnable) = 0;
    //
    //virtual MERROR_ENUM setSceneLightValue(MINT32 const i4SceneLV_x10) = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public: // ISP End-User-Defined Tuning Index.
    virtual MERROR_ENUM setIspUserIdx_Edge(EIndex_Isp_Edge_T const eIndex) = 0;
    virtual MERROR_ENUM setIspUserIdx_Hue(EIndex_Isp_Hue_T const eIndex) = 0;
    virtual MERROR_ENUM setIspUserIdx_Sat(EIndex_Isp_Saturation_T const eIndex) = 0;
    virtual MERROR_ENUM setIspUserIdx_Bright(EIndex_Isp_Brightness_T const eIndex) = 0;
    virtual MERROR_ENUM setIspUserIdx_Contrast(EIndex_Isp_Contrast_T const eIndex) = 0;
public:     ////    Exif.
    virtual MERROR_ENUM getDebugInfoP1(
        NSIspExifDebug::IspExifDebugInfo_T& rDebugInfo,
        MBOOL const fgReadFromHW
    ) const = 0;

    virtual MERROR_ENUM getDebugInfoP2(
        const NSIspTuning::ISP_INFO_T& _rIspInfo,
        NSIspExifDebug::IspExifDebugInfo_T& rIspExifDebugInfo,
        const void* pRegBuf,
        const void* pMfbBuf
    ) const = 0;

public:     ////    Operations.
    //virtual MERROR_ENUM construct() = 0;
    virtual MERROR_ENUM init(MINT32 const i4SubsampleCount=1) = 0;
    virtual MERROR_ENUM uninit() = 0;
    virtual MERROR_ENUM reinitP1TuningMgr(MINT32 const i4SubsampleCount=1) = 0;
    virtual MERROR_ENUM validatePerFrameP1(NS3Av3::RequestSet_T const RequestSet, MBOOL bReCalc = MFALSE, MINT32 i4SubsampleIdex=0) = 0;
    virtual MERROR_ENUM validatePerFrameP2(MINT32 flowType, const ISP_INFO_T& rIspInfo, void* pTuningBuf) = 0;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    virtual MBOOL sendIspTuningIOCtrl(MUINT32 ctrl, MINTPTR arg1, MINTPTR arg2)=0;
    virtual MBOOL getDefaultObc(MVOID*& pISP_NVRAM_Reg)=0;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:  //// Camera HAL 3.2
    virtual MERROR_ENUM getColorCorrectionTransform(MFLOAT& M11, MFLOAT& M12, MFLOAT& M13,
                                                    MFLOAT& M21, MFLOAT& M22, MFLOAT& M23,
                                                    MFLOAT& M31, MFLOAT& M32, MFLOAT& M33) = 0;
    virtual MERROR_ENUM setColorCorrectionTransform(MFLOAT& M11, MFLOAT& M12, MFLOAT& M13,
                                                    MFLOAT& M21, MFLOAT& M22, MFLOAT& M23,
                                                    MFLOAT& M31, MFLOAT& M32, MFLOAT& M33) = 0;
    virtual MERROR_ENUM setColorCorrectionMode(MINT32 i4ColorCorrectionMode) = 0;
    virtual MERROR_ENUM setEdgeMode(MINT32 i4EdgeMode) = 0;
    virtual MERROR_ENUM setNoiseReductionMode(MINT32 i4NRMode) = 0;
    virtual MERROR_ENUM setToneMapMode(MINT32 i4ToneMapMode) = 0;
    virtual MERROR_ENUM getTonemapCurve_Red(MFLOAT*& p_in_red, MFLOAT*& p_out_red, MINT32 *pCurvePointNum) = 0;
    virtual MERROR_ENUM getTonemapCurve_Green(MFLOAT*& p_in_green, MFLOAT*& p_out_green, MINT32 *pCurvePointNum) = 0;
    virtual MERROR_ENUM getTonemapCurve_Blue(MFLOAT*& p_in_blue, MFLOAT*& p_out_blue, MINT32 *pCurvePointNum) = 0;
    virtual MERROR_ENUM setTonemapCurve_Red(MFLOAT *p_in_red, MFLOAT *p_out_red, MINT32 *pCurvePointNum) = 0;
    virtual MERROR_ENUM setTonemapCurve_Green(MFLOAT *p_in_green, MFLOAT *p_out_green, MINT32 *pCurvePointNum) = 0;
    virtual MERROR_ENUM setTonemapCurve_Blue(MFLOAT *p_in_blue, MFLOAT *p_out_blue, MINT32 *pCurvePointNum) = 0;
    virtual MBOOL setFdGamma(MINT32 i4Index, MINT32* pTonemapCurveRed, MUINT32 u4size) = 0;
    virtual MBOOL convertPtPairsToGMA(const MFLOAT* inPtPairs, MUINT32 u4NumOfPts, MINT32* outGMA) = 0;

    virtual EZoom_T map_Zoom_value2index(MUINT32 const i4ZoomRatio_x100) const = 0;
};


};  //  namespace NSIspTuningv3
#endif // _PARAMCTRL_IF_H_

