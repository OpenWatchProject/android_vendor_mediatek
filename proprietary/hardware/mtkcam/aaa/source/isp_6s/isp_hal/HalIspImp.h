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

/**
* @file HalIspImp.h
* @brief Declarations of Implementation of ISP Hal Class
*/

#ifndef __HAL_ISP_IMP_H__
#define __HAL_ISP_IMP_H__

#include <mtkcam/aaa/IHalISP.h>
#include <mtkcam/utils/hw/HwTransform.h>

using namespace NSCam;
using namespace NSCamHW;

#include <isp_tuning/isp_tuning_mgr.h>
#include <private/IopipeUtils.h>

#include <IResultPool.h>
#include <ResultPool4LSCConfig.h>
#include <ResultPool4Module.h>

//Module
#include <mtkcam/aaa/IDngInfo.h>
#if CAM3_LSC_FEATURE_EN
#include <lsc/ILscMgr.h>
#endif

#include <HalIspTemplate.h>
#include <HalISPCbHub.h>
//
#include <aaa_result.h>

namespace NS3Av3
{
using namespace NSIspTuning;

class HalIspImp : public HalIspTemplate
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    /**
     * @brief Create instance of HalIspImp
     * @param [in] i4SensorIdx: sensor index
     * @param [in] strUser: user name
     * @return
     * - HalIspImp an instance of HalispImp
     */
    static HalIspImp* createInstance(MINT32 const i4SensorIdx, const char* strUser);

    /**
     * @brief set FD enable to isp
     * @param [in] fgEnable: status of on/off
     * @return
     * - NULL
     */

    /**
     * @brief
     * @param [in] index for check
     * @param [in] curve for gamma
     * @param [in] size of curve
     * - MBOOL value of TRUE/FALSE.
     */
    virtual MBOOL       setFdGamma(MINT32 i4Index=-1, MINT32* pTonemapCurveRed=NULL, MUINT32 u4size=0) {return MTRUE;}

    /**
     * @brief This function is deprecated after isp60
     * @return
     * - MBOOL value of TRUE/FALSE.
     */
    virtual MBOOL       setOTInfo(MVOID* prOT) {return MTRUE;}

    /**
     * @brief standard api to Send isp control
     * @param [in] eISPCtrl: control code
     * @param [in] iArg1: input1
     * @param [in] iArg2: input2
     * @return
     * - HalIspImp an instance of HalispImp
     */
    virtual MINT32      sendIspCtrl(EISPCtrl_T eISPCtrl, MINTPTR iArg1, MINTPTR iArg2);

    //For p1 driver's callback
    virtual MINT32      InitP1Cb();
    virtual MINT32      UninitP1Cb();
    virtual MINT32      ConfigP1Cb();

    /**
     * @brief Attach callback for notifying
     * @param [in] eId Notification message type
     * @param [in] pCb Notification callback function pointer
     * @return
     * - MINT32 value of TRUE/FALSE.
     */
    virtual MINT32      attachCb(/*IHal3ACb::ECb_T eId, IHal3ACb* pCb*/) {return MTRUE;}

    /**
     * @brief Dettach callback
     * @param [in] eId Notification message type
     * @param [in] pCb Notification callback function pointer
     * @return
     * - MINT32 value of TRUE/FALSE.
     */
    virtual MINT32      detachCb(/*IHal3ACb::ECb_T eId, IHal3ACb* pCb*/) {return MTRUE;}

    /**
     * @brief dump isp info by following info
     * @param [in] flowType: 0 -> not High Quality Capture, 1 -> is Hight Quality Capture
     * @param [in] control: p2 request metadata
     * @param [out] pTuningBuf: tuning buffer for pass-2 setting
     * @param [out] pResult: output metadata
     * @return
     * - MINT32 value of TRUE/FALSE.
     */
    virtual MINT32      dumpIsp(MINT32 flowType, const MetaSet_T& control, TuningParam* pTuningBuf, MetaSet_T* pResult);

    /**
     * @brief Query current isp buf information
     * @param [out] bufferInfo
     * @return
     * - MBOOL value of TRUE/FALSE.
     */
    virtual MBOOL       queryISPBufferInfo(Buffer_Info& bufferInfo);



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //    Ctor/Dtor.
    HalIspImp(MINT32 const i4SensorIdx);
    virtual ~HalIspImp(){}

    MBOOL               initTuning();
    MBOOL               configTuning(MINT32 i4SubsampleCount);
    MBOOL               uninitTuning();
    MBOOL               setConfigParams(P1Param_T const &rNewConfigParam, RAWIspCamInfo &rP1CamInfo);
    MBOOL               setP1Params(P1Param_T const &rNewP1Param, RAWIspCamInfo &rP1CamInfo);
    MBOOL               validateP1(RequestSet_T const RequestSet, RAWIspCamInfo &rP1CamInfo);
    MBOOL               setP1GetParams(P1GetParam_T const &rNewP1GetParam, RAWIspCamInfo &tempCamInfo, RAWIspCamInfo &rP1CamInfo);
    MBOOL               setCCUFeedbackExif(P1GetParam_T const &rNewP1GetParam, RAWIspCamInfo &tempCamInfo);
    MBOOL               setISPInfo(P2Param_T const &rNewP2Param, TuningParam* pTuningBuf, NSIspTuning::ISP_INFO_T &rIspInfo, MINT32 type);
    MBOOL               restoreCaminfo(P2Param_T &rNewP2Param, RAWIspCamInfo &mBackupCamInfo);
    MBOOL               setP2Params(P2Param_T const &rNewP2Param, RAWIspCamInfo &rCamInfo, ResultP2_T* pResultP2);
    MBOOL               generateP2(MINT32 flowType, const NSIspTuning::ISP_INFO_T& rIspInfo, void* pTuningBuf, ResultP2_T* pResultP2);
    MBOOL               dumpP2DotTuning(ISP_INFO_T& rIspInfo, P2Param_T &rNewP2Param);
    MBOOL               _readDumpP2Buf(TuningParam* pTuningBuf, const MetaSet_T& control, MetaSet_T* pResult, NSCam::TuningUtils::FileReadRule &rule,
                                       ISP_INFO_T* pIspInfo, NSCam::TuningUtils::ETuningFeedbackStage_T stage);
    MBOOL               _dumpP2Buf(TuningParam* pTuningBuf, NSCam::TuningUtils::FileReadRule &rule, ISP_INFO_T& rIspInfo);
    MUINT32             queryTuningSize();

    MVOID               setFDEnabletoISP(RAWIspCamInfo &rP1CamInfo, MBOOL const FD_enable);
    MVOID               setFDInfotoISP(RAWIspCamInfo &rP1CamInfo, MVOID* prFaces);
    MVOID               setSensorMode(RAWIspCamInfo &rP1CamInfo, MINT32 const i4SensorMode, MBOOL const bFrontalBin, MUINT32 const u4RawWidth, MUINT32 const u4RawHeight);
    MVOID               setTGInfo(RAWIspCamInfo &rP1CamInfo, MINT32 const i4TGInfo, MINT32 const i4Width, MINT32 const i4Height);

private:
    IspTuningMgr*                                   m_pTuning;
    std::mutex                                      m_CbLock;

    //Member
    MINT32                                          m_i4CopyLscP1En;
    std::mutex                                      m_P2Mtx;
    MUINT32                                         m_target_source;

    // Pass1 HW setting callback
    HalISPP1DrvCbHub                               *mpHalISPP1DrvCbHub;

    // for tuning feedback
    MBOOL               _readDump(TuningParam* pTuningBuf, const MetaSet_T& control, MetaSet_T* pResult, ISP_INFO_T* pIspInfo, MINT32 i4Format);


    //Set P1 attributes
    MVOID notifyRPGEnable(RAWIspCamInfo &rP1CamInfo, MBOOL const fgEnable);
    MVOID setIspProfile(RAWIspCamInfo &rP1CamInfo, EIspProfile_T const eIspProfile);
    MVOID setZoomRatio(RAWIspCamInfo &rP1CamInfo, MINT32 const i4ZoomRatio_x100) ;
    MVOID setAEInfo2ISP(RAWIspCamInfo &rP1CamInfo, AE_ISP_INFO_T const &rAEInfo);
    MVOID setAWBInfo2ISP(RAWIspCamInfo &rP1CamInfo, AWB_ISP_INFO_T const &rAWBInfo);
    MVOID setCCUInfo2ISP(RAWIspCamInfo &rP1CamInfo, ISP_CCU_RESULT_T const &rCCUInfo);

};

};

#endif //__HAL_ISP_IMP_H__

