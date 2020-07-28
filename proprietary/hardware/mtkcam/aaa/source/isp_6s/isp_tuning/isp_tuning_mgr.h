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

/**
 * @file isp_tuning_mgr.h
 * @brief ISP tuning manager
 */

#ifndef _ISP_TUNING_MGR_H_
#define _ISP_TUNING_MGR_H_

#include <Local.h>
#include <isp_tuning_cam_info.h>
#include <private/aaa_hal_private.h>
#include <tuning/ae_flow_custom.h>
#include <tuning/awb_flow_custom.h>

#include <stdio.h>
#include <stdarg.h>

using namespace NS3Av3;

namespace NSIspTuning
{

class IParamctrl;

/*******************************************************************************
*
*******************************************************************************/
class IspTuningMgr
{
    #define PARAMCTRL_SENSORDEV_INSTANCE(api,...)\
    if (i4SensorDev & ESensorDev_Main) {\
        if (m_pParamctrl_Main) {\
            ret = m_pParamctrl_Main->api(__VA_ARGS__);\
        }\
        else {\
            CAM_LOGE("m_pParamctrl_Main is NULL");\
            return MFALSE;\
        }\
    }\
    if (i4SensorDev & ESensorDev_Sub) {\
        if (m_pParamctrl_Sub) {\
            ret = m_pParamctrl_Sub->api(__VA_ARGS__);\
        }\
        else {\
            CAM_LOGE("m_pParamctrl_Sub is NULL");\
            return MFALSE;\
        }\
    }\
    if (i4SensorDev & ESensorDev_MainSecond) {\
        if (m_pParamctrl_Main2) {\
            ret = m_pParamctrl_Main2->api(__VA_ARGS__);\
        }\
        else {\
            CAM_LOGE("m_pParamctrl_Main2 is NULL");\
            return MFALSE;\
        }\
    }\
    if (i4SensorDev & ESensorDev_SubSecond) {\
        if (m_pParamctrl_Sub2) {\
            ret = m_pParamctrl_Sub2->api(__VA_ARGS__);\
        }\
        else {\
            CAM_LOGE("m_pParamctrl_Sub2 is NULL");\
            return MFALSE;\
        }\
    }\
    if (i4SensorDev & ESensorDev_MainThird) {\
        if (m_pParamctrl_Main3) {\
            ret = m_pParamctrl_Main3->api(__VA_ARGS__);\
        }\
        else {\
            CAM_LOGE("m_pParamctrl_Main3 is NULL");\
            return MFALSE;\
        }\
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    //  Copy constructor is disallowed.
    IspTuningMgr(IspTuningMgr const&);
    //  Copy-assignment operator is disallowed.
    IspTuningMgr& operator=(IspTuningMgr const&);

public:  ////
    IspTuningMgr();
    ~IspTuningMgr();

    /**
     * @brief get instance
     */
    static IspTuningMgr& getInstance();

    /**
     * @brief init function
     * @param [in] i4SensorDev sensor device
     */
    MBOOL init(MINT32 const i4SensorDev, MINT32 const i4SensorIdx, MINT32 const i4SubsampleCount = 1);

    /**
     * @brief uninit function
     * @param [in] i4SensorDev sensor device
     */
    MBOOL uninit(MINT32 const i4SensorDev);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public: // Attributes

    /**
     * @brief set ISP profile
     * @param [in] i4SensorDev sensor device
     * @param [in] i4IspProfile ISP profile
     */

    MBOOL setRequestNumber(MINT32 const i4SensorDev, MINT32 const i4RequestNumber);

    /**
     * @brief get ISP profile
     * @param [in] i4SensorDev sensor device
     * @return ISP profile
     */
    MINT32 getIspProfile(MINT32 const /*i4SensorDev*/) {return m_i4IspProfile;}

    /**
     * @brief set operation mode
     * @param [in] i4SensorDev sensor device
     * @param [in] i4OperMode operation mode
     */
    MBOOL setOperMode(MINT32 const i4SensorDev, MINT32 const i4OperMode);

    /**
     * @brief get operation mode
     * @param [in] i4SensorDev sensor device
     * @return operation mode
     */
    MINT32 getOperMode(MINT32 const i4SensorDev);

    /**
     * @brief set dynamic bypass
     * @param [in] i4SensorDev sensor device
     * @param [in] fgEnable TURE: enable dynamic bypass, FALSE: disable dynamic bypass
     */
    MBOOL setDynamicBypass(MINT32 const i4SensorDev, MBOOL const fgEnable);

    /**
     * @brief set dynamic CCM
     * @param [in] i4SensorDev sensor device
     * @param [in] bdynamic_ccm TURE: enable dynamic CCM, FALSE: disable dynamic CCM
     */
    MBOOL setDynamicCCM(MINT32 const i4SensorDev, MBOOL bdynamic_ccm);

    /**
     * @brief get dynamic bypass status
     * @param [in] i4SensorDev sensor device
     * @return dynamic bypass status
     */
    MINT32 getDynamicBypass(MINT32 const i4SensorDev);

    /**
     * @brief get dynamic CCM status
     * @param [in] i4SensorDev sensor device
     * @return dynamic CCM status
     */
    MINT32 getDynamicCCM(MINT32 const i4SensorDev);

    /**
      * @brief set sensor mode
      * @param [in] i4SensorDev sensor device
      * @param [in] i4SensorMode sensor mode
      */
    MBOOL setRawPath(MINT32 const i4SensorDev, MUINT32 const u4RawPath);

    /**
     * @brief enable dynamic tuning
     * @param [in] i4SensorDev sensor device
     * @param [in] fgEnable TURE: enable dynamic tuning, FALSE: disable dynamic tuning
     */
    MBOOL enableDynamicTuning(MINT32 const i4SensorDev, MBOOL const fgEnable);

    /**
     * @brief enable dynamic shading
     * @param [in] fgEnable TURE: enable dynamic shading, FALSE: disable dynamic shading
     */
    MBOOL enableDynamicShading(MINT32 const i4SensorDev, MBOOL const fgEnable);

    /**
     * @brief set shading index
     * @param [in] i4SensorDev sensor device
     * @param [in] i4IDX shading index
     */
    MBOOL setIndex_Shading(MINT32 const i4SensorDev, MINT32 const i4IDX);

    /**
     * @brief get shading index
     * @param [in] i4SensorDev sensor device
     * @param [in] pCmdArg pointer to command argument
     */
    MBOOL getIndex_Shading(MINT32 const i4SensorDev, MVOID*const pCmdArg);

    MBOOL setDGNDebugInfo4CCU(MINT32 const i4SensorDev, MUINT32 const u4Rto, MUINT32 const u4P1DGNGain, NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo);
    MBOOL setHLRDebugInfo4CCU(MINT32 const i4SensorDev, ISP_NVRAM_HLR_T const HLR, NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo);
    MBOOL setLTMnLTMSDebugInfo4CCU(MINT32 const i4SensorDev, ISP_NVRAM_LTM_T const LTM_R1, ISP_NVRAM_LTM_T const LTM_R2, ISP_NVRAM_LTMS_T const LTMS, NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo);

public: // Operations.

    /**
     * @brief validate frame-based ISP tuning parameter
     * @param [in] i4SensorDev sensor device
     * @param [in] i4FrameID frame ID
     * @param [in] fgForce TRUE: force validation
     */
    MBOOL validatePerFrameP1(MINT32 const i4SensorDev, RequestSet_T const RequestSet, RAWIspCamInfo &rP1CamInfo, MBOOL bReCalc = MFALSE, MINT32 i4SubsampleIdex = 0);

    /**
     * @brief validate frame-based ISP tuning parameter
     * @param [in] flowType 0 processed raw, 1 for ip pure raw
     * @param [in] rIspCamInfo
     * @param [out] pRegBuf: register buffer
     */
    MBOOL validatePerFrameP2(MINT32 const i4SensorDev, MINT32 flowType, const ISP_INFO_T& rIspInfo, void* pTuningBuf);

    /**
     * @brief get ISP debug info
     * @param [in] i4SensorDev sensor device
     * @param [out] rIspExifDebugInfo ISP debug info
     */
    MBOOL getDebugInfoP1(MINT32 const i4SensorDev, const RAWIspCamInfo &rP1CamInfo, NSIspExifDebug::IspExifDebugInfo_T& rDebugInfo, MBOOL const fgReadFromHW) const;

    /**
     * @brief get ISP debug info
     * @param [in] i4SensorDev sensor device
     * @param [out] rIspExifDebugInfo ISP debug info
     */
    MBOOL getDebugInfoP2(MINT32 const i4SensorDev, const NSIspTuning::ISP_INFO_T& rIspInfo, NSIspExifDebug::IspExifDebugInfo_T& rIspExifDebugInfo, const void* pTuningBuf);

    /**
     * @brief notify RRZ enable info
     * @param [in] i4SensorDev sensor device
     * @param [in] rIspAWBGain RRZ AWB gain
     */

    MVOID* getDMGItable(MINT32 const i4SensorDev, MBOOL const fgRPGEnable);

    /**
     * @brief force ISP validation
     * @param [in] i4SensorDev sensor device
     */
    MBOOL forceValidate(MINT32 const i4SensorDev);

 /**
 * @brief get ISP index set
 * @param [in] ISP profile
 * @param [in] sensor mode
 * @param [in] ISO
 * @return index set pointer
 */
    MBOOL getDefaultObc(MINT32 const i4SensorDev, MVOID*& pISP_NVRAM_Reg);
    /**
     * @brief get color correction transform
     * @param [out] rCCM CCM
     */
    MBOOL getColorCorrectionTransform(MINT32 const i4SensorDev,
                                      MFLOAT& M11, MFLOAT& M12, MFLOAT& M13,
                                      MFLOAT& M21, MFLOAT& M22, MFLOAT& M23,
                                      MFLOAT& M31, MFLOAT& M32, MFLOAT& M33);

    /**
     * @brief set color correction transform
     * @param [in] rCCM CCM
     */
    MBOOL setColorCorrectionTransform(MINT32 const i4SensorDev,
                                      MFLOAT M11, MFLOAT M12, MFLOAT M13,
                                      MFLOAT M21, MFLOAT M22, MFLOAT M23,
                                      MFLOAT M31, MFLOAT M32, MFLOAT M33);

    /**
     * @brief set color correction mode
     * @param [in] i4ColorCorrectionMode color correction mode
     */
    MBOOL setColorCorrectionMode(MINT32 const i4SensorDev, MINT32 i4ColorCorrectionMode);

    /**
     * @brief get tone mapping curve (R channel)
     * @param [in] p_in_red control point input
     * @param [in] p_out_red control point output
     * @param [in] pCurvePointNum curve point number
     */
    MBOOL getTonemapCurve_Red(MINT32 const i4SensorDev, MFLOAT*& p_in_red, MFLOAT*& p_out_red, MINT32 *pCurvePointNum);

     /**
     * @brief get tone mapping curve (G channel)
     * @param [in] p_in_green control point input
     * @param [in] p_out_green control point output
     * @param [in] pCurvePointNum curve point number
     */
    MBOOL getTonemapCurve_Green(MINT32 const i4SensorDev, MFLOAT*& p_in_green, MFLOAT*& p_out_green, MINT32 *pCurvePointNum);

    /**
     * @brief get tone mapping curve (B channel)
     * @param [in] p_in_blue control point input
     * @param [in] p_out_blue control point output
     * @param [in] pCurvePointNum curve point number
     */
    MBOOL getTonemapCurve_Blue(MINT32 const i4SensorDev, MFLOAT*& p_in_blue, MFLOAT*& p_out_blue, MINT32 *pCurvePointNum);

    /**
     * @brief set tone mapping curve (R channel)
     * @param [in] p_in_red control point input
     * @param [in] p_out_red control point output
     * @param [in] pCurvePointNum curve point number
     */
    MBOOL setTonemapCurve_Red(MINT32 const i4SensorDev, MFLOAT *p_in_red, MFLOAT *p_out_red, MINT32 *pCurvePointNum);

     /**
     * @brief get tone mapping curve (G channel)
     * @param [in] p_in_green control point input
     * @param [in] p_out_green control point output
     * @param [in] pCurvePointNum curve point number
     */
    MBOOL setTonemapCurve_Green(MINT32 const i4SensorDev, MFLOAT *p_in_green, MFLOAT *p_out_green, MINT32 *pCurvePointNum);

    /**
     * @brief get tone mapping curve (B channel)
     * @param [in] p_in_blue control point input
     * @param [in] p_out_blue control point output
     * @param [in] pCurvePointNum curve point number
     */
    MBOOL setTonemapCurve_Blue(MINT32 const i4SensorDev, MFLOAT *p_in_blue, MFLOAT *p_out_blue, MINT32 *pCurvePointNum);

    MBOOL convertPtPairsToGMA(MINT32 const i4SensorDev, const MFLOAT* inPtPairs, MUINT32 u4NumOfPts, MINT32* outGMA);

    MUINT32 getZoomIdx(MINT32 const i4SensorDev, MUINT32 const i4ZoomRatio_x100);

    MBOOL queryISPBufferInfo(MINT32 const m_i4SensorDev, Buffer_Info& bufferInfo);

    MBOOL setP1DirectYUV_Port(MINT32 const i4SensorDev, MUINT32 const u4P1DirectYUV_Port);


    enum E_ISPTUNING_CTRL
    {
        E_ISPTUNING_CTRL_BEGIN         = 0x0000,
        //basic flow control
        E_ISPTUNING_NOTIFY_START       = 0x0001,
        E_ISPTUNING_NOTIFY_STOP        = 0x0002,
        E_ISPTUNING_NOTIFY_SENSOR_TYPE = 0x0003,
        E_ISPTUNING_NOTIFY_CCU_START   = 0x0004,

        //Gamma
        E_ISPTUNING_GET_ISP_GAMMA = 0x3000,

        //LCE
        E_ISPTUNING_GET_LCE_GAIN       = 0x4001,

        //Shading
        E_ISPTUNING_UPDATE_LSC_INFO    = 0x5000,

        //OBC Gain
        E_ISPTUNING_UPDATE_OBC_INFO    = 0x6000,
        //
        E_ISPTUNING_GET_MFB_SIZE       = 0x7001,
        E_ISPTUNING_GET_LTM_CURVE_SIZE = 0x7002,
        E_ISPTUNING_GET_P2_BUFFER_SIZE = 0x7003,
        //AINR
        E_ISPTUNING_GetAINRParam       = 0x8001,
        E_ISPTUNING_CTRL_END
    };

    MBOOL sendIspTuningIOCtrl(MINT32 const i4SensorDev, E_ISPTUNING_CTRL const ctrl, MINTPTR arg1, MINTPTR arg2);

    static const char *getIspProfileName(EIspProfile_T IspProfile);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    IParamctrl* m_pParamctrl_Main;
    IParamctrl* m_pParamctrl_Sub;
    IParamctrl* m_pParamctrl_Main2;
    IParamctrl* m_pParamctrl_Sub2;
    IParamctrl* m_pParamctrl_Main3;
    MINT32      m_i4SensorDev;
    MBOOL       m_bDebugEnable;
    MINT32      m_i4IspProfile;
    MINT32      m_i4SensorIdx;
};

};  //  namespace NSIspTuningv3
#endif // _ISP_TUNING_MGR_H_

