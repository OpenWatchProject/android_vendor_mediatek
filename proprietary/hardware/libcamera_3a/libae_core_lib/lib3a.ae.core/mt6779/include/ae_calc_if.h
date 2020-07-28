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

/**
 * @file ae_algo_if.h
 * @brief Interface to AE algorithm library
 */

#ifndef _AE_CALC_IF_H_
#define _AE_CALC_IF_H_

#include "aaa_algo_option.h"
#define AE_ALGO_IF_REVISION   7471001 

namespace NS3A
{
/**
 * @brief Interface to AE algorithm library
 */
class IAeCalc {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //    Ctor/Dtor.
    IAeCalc() {}
    virtual ~IAeCalc() {}

private: // disable copy constructor and copy assignment operator
    IAeCalc(const IAeCalc&);
    IAeCalc& operator=(const IAeCalc&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief create instance
     * @param [in] eAAAOpt 3A option
     * @param [in] i4SensorDev input sensor device for algorithm init; please refer to isp_tuning.h
     */
    template <EAAAOpt_T const eAAAOpt>
    static  IAeCalc* createInstance(MINT32 const i4SensorDev);
    /**
     * @brief destroy instance
     */
    virtual MVOID   destroyInstance() = 0;

    /**
     * @brief AE init function
     * @param [in] a_pAEInitParam AE init input parameters; please refer to ae_param.h
     * @param [out] a_pAEOutput AE algorithm output; please refer to ae_param.h
     * @param [out] a_pAEConfig AE statistics config parameter; please refer to ae_param.h
     */
    virtual MRESULT initAE(AE_INITIAL_INPUT_T *a_pAEInitParam, AE_STAT_PARAM_T *a_pAEConfig) = 0;

    /**
     * @brief update AE parameter information
     * @param [in] a_pAEInitParam AE input parameters; please refer to ae_param.h
     */
    virtual MRESULT updateAEParam(AE_INITIAL_INPUT_T *a_pAEUpdateParam) = 0;

    virtual MRESULT updateAEAlgoInfo(AE_INFO_T *a_pAEAlgoInfo) = 0;      

    virtual MRESULT setAECalc(AE_Calc_Setting_T *a_pAECalcSetting)= 0; 
    
    /**
     * @brief output the AE statistic window config for AE algorithm
     * @param [in] a_eZoomWindow update AE algorithm calculate window information; please refer to Ae_param.h
     * @param [out] a_pAEHistConfig AE statistics config parameter; please refer to ae_param.h
     */
    virtual MRESULT modifyHistogramWinConfig(EZOOM_WINDOW_T a_eZoomWindow, AE_STAT_PARAM_T *a_pAEHistConfig, MUINT32 u4PixelWidth, MUINT32 u4PixelHeight,MUINT32 m_i4AETgValidBlockWidth,MUINT32 m_i4AETgValidBlockHeight) = 0;    

    /**
     * @brief modify the sensor shutter and sensor gain to meet the sensor driver request
     * @param [in] bCaptureTable search table by preview or capture table
     * @param [in] rInputData input sensor shutter, sensor gain, isp gain and ISO speed; please refer to Ae_param.h
     * @param [out] rOutputData output sensor shutter, sensor gain, isp gain and ISO speed; please refer to Ae_param.h
    */
    virtual MRESULT switchSensorExposureGain(MBOOL bCaptureTable, AE_EXP_GAIN_MODIFY_T &rInputData, AE_EXP_GAIN_MODIFY_T &rOutputData) = 0;

    /**
     * @brief output the AE capture parameter information by the difference EV compensation
     * @param [out] aeoutput AE capture information ouput; please refer to ae_param.h
     * @param [in] iDiffEV difference EV compensation value
     */
    virtual MRESULT switchCaptureState(strAERealSetting *in, strAERealSetting *out, MINT32 iDiffEV) = 0;
  //virtual MRESULT switchCaptureDiffEVState(strAERealSetting *out, MUINT32 u4CaptureIdxF, UINT32 u4AEFinerEVIdxBase, MINT32 iDiffEV) = 0;    

    virtual MRESULT switchAFAssistState(strAERealSetting *in, strAERealSetting *out) = 0;

    /**
     * @brief output HDR LE, ME, SE exposure settings transformed by LE and hdrRatio
     * @param [out] LE, ME, SE exposure settings are stored in out->HdrEvSetting
     * @param [in] please fill in->EvSetting
     * @param [hdrRatio] please fill hdrRatio (100: 1x)
     */
    virtual MRESULT transHDREVSetting(strAERealSetting *in, MINT32 hdrRatio) = 0;

    /**
     * @brief get AE senstivity delta value information
     * @param [in] u4NextSenstivity the brightness different ratio
     */
    virtual MRESULT getSenstivityDeltaIndex(MUINT32 u4NextSenstivity) = 0;
    
    /**
     * @brief update AE pline for shutter/iso priority
     * @param [in] original priority info
     * @param [in] sensor device info (within nvram)
     * @param [in] device gain list
     * @param [out] new AE pline
     */
    virtual MRESULT updateAEPriorityPlineInfo(strAETable *pOut, CCU_AE_PRIORITY_INFO *pIn, CCU_AE_NVRAM_T *pNvram, strAEPLineGainList *pGainList) = 0;

    /**
     * @brief update finer AE pline between two points
     * @param [in] original AE real setting
     * @param [in] AE pline table
     * @param [in] device gain list
     * @param [out] new AE real setting
     */
    virtual MRESULT recalcAEExpInfo_v4p0(AE_INFO_T *pAEAlgoInfo, MUINT32 *pLog2x1000, strAERealSetting *pOut, strAERealSetting *pIn, strAETable *pNewPline, strAETable *pOriPline, strAEPLineGainList *pGainList) = 0;


    /**
     * @brief update finer AE pline between two points
     * @param [in] original AE real setting
     * @param [in] AE pline table
     * @param [in] device gain list
     * @param [out] new AE real setting
     */
    virtual MRESULT updateAEExpInfo_v4p0(strAERealSetting *pOut, strAERealSetting *pIn, strAETable *pNewPline, strAETable *pOriPline, strAEPLineGainList *pGainList) = 0;

    /**
     * @brief update AE pline pointer
     * @param [in] AE tuning file
     */
    virtual MRESULT updateAEPlineInfo_v4p0(strAETable *pCurrentAEPlineTable, strAETable *pCurrentCaptureAEPlineTable,strFinerEvPline *m_pCurrentTableF,strFinerEvPline *m_pCurrentCaptureTableF,strAEPLineGainList *AEGainList,MUINT32 &u4FinerEVIndex) = 0;

    /**
     * @brief get the AE current table max ISO
     * @param [out] u4MaxISO output max ISO of AE Pline table
     * @param [out] u4MaxShutter output max shutter of AE Pline table
     */
    virtual MRESULT getAEMaxISO(MUINT32 m_u4IndexMax, MUINT32 &u4MaxShutter, MUINT32 &u4MaxISO) = 0;
    /**
     * @update AE Light Sensor Init Index
     */
    virtual MRESULT updateAELSInitIndex(MINT32 i4LsBv, MINT32 i4AeBv, MUINT32 u4IndexMax, MUINT32 u4IndexMin,
        AE_LIGHTSENSOR_OUTPUT_T *prOutput) = 0;
    /**
     * @update AE exposure setting when ev comp
     */    
    virtual MRESULT recalcAEExpInfoEVcomp_v4p0(strAERealSetting *pOut, MUINT32 u4CurIdxF, strFinerEvPline *pCurPline, CCU_LIB3A_AE_EVCOMP_T a_ePreEVIndex, CCU_LIB3A_AE_EVCOMP_T a_eCurEVIndex,MINT32 m_u4AEFinerEVIdxBase,MINT32 m_u4IndexFMax,MINT32 m_u4IndexFMin)=0;

};

}; // namespace NS3A

#endif

