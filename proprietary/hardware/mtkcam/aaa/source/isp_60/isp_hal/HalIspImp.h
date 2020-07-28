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

#include <HalISPCbHub.h>
//
#include <aaa_result.h>

namespace NS3Av3
{
using namespace NSIspTuning;

class HalIspImp : public IHalISP
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    /**
      * @brief Create instance of Hal3AAdapter3
      * @param [in] i4SensorIdx.
      */
    static HalIspImp* createInstance(MINT32 const i4SensorIdx, const char* strUser);

    /**
     * @brief destroy instance of IHal3A
     * @param [in] strUser user name
     */
    virtual MVOID       destroyInstance(const char* strUser);

    /**
     * @brief config ISP setting
     * @param [in] rConfigInfo ISP setting of ConfigInfo_T
     * @return
     * - MINT32 value of TRUE/FALSE.
     */
    virtual MINT32      config(const ConfigInfo_T& rConfigInfo);

    virtual MVOID       WT_Reconfig(MVOID* pInput);

    /**
     * @brief set sensor mode
     * @param [in] i4SensorMode
     */
    virtual MVOID       setSensorMode(MINT32 i4SensorMode);

    /**
     * @brief init ISP
     */
    virtual MBOOL       init(const char* strUser);

    /**
     * @brief uninit ISP
     */
    virtual MBOOL       uninit(const char* strUser);

    /**
     * @brief start ISP
     */
    virtual MBOOL       start();

      /**
     * @brief stop ISP
     */
    virtual MBOOL       stop();

    /**
     * @brief
     * @param [in]
     * @param [in]
     * @param [out]
     * @param [out]
     */
    virtual MBOOL       setP1Isp(const std::vector<MetaSet_T*>& requestQ, MBOOL const fgForce = 0);

    /**
     * @brief
     * @param [in]
     * @param [in]
     * @param [out]
     * @param [out]
     */
    virtual MBOOL       setP2Isp(MINT32 flowType, const MetaSet_T& control, TuningParam* pTuningBuf, MetaSet_T* pResult);
    virtual MBOOL       setFdGamma(MINT32 i4Index=-1, MINT32* pTonemapCurveRed=NULL, MUINT32 u4size=0);

    virtual MINT32      sendIspCtrl(EISPCtrl_T eISPCtrl, MINTPTR iArg1, MINTPTR iArg2);

    /**
     * @brief Attach callback for notifying
     * @param [in] eId Notification message type
     * @param [in] pCb Notification callback function pointer
     */
    virtual MINT32      attachCb(/*IHal3ACb::ECb_T eId, IHal3ACb* pCb*/);

    /**
     * @brief Dettach callback
     * @param [in] eId Notification message type
     * @param [in] pCb Notification callback function pointer
     */
    virtual MINT32      detachCb(/*IHal3ACb::ECb_T eId, IHal3ACb* pCb*/);

    virtual MVOID       setFDEnable(MBOOL fgEnable);

    virtual MBOOL       setFDInfo(MVOID* prFaces);

    virtual MBOOL       setOTInfo(MVOID* prOT);

    virtual MINT32      dumpIsp(MINT32 flowType, const MetaSet_T& control, TuningParam* pTuningBuf, MetaSet_T* pResult);

    virtual MINT32      get(MUINT32 frmId, MetaSet_T& result);

    virtual MINT32      getCur(MUINT32 frmId, MetaSet_T& result);

    /**
     * @brief resume 3A
     * @param [in] MagicNum which want to resume 3A
     */
    virtual MVOID       resume(MINT32 MagicNum = 0);

//LCS Part
    virtual MINT32      InitP1Cb();
    virtual MINT32      UninitP1Cb();
    virtual MINT32      ConfigP1Cb();

    virtual MBOOL       queryISPBufferInfo(Buffer_Info& bufferInfo);



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:  //    Ctor/Dtor.
    HalIspImp(MINT32 const i4SensorIdx);
    virtual ~HalIspImp(){}
protected: 
    MBOOL               setFDInfo(MVOID* prFaces, MVOID* prAFFaces);
    MINT32              updateTGInfo();
    MVOID               querySensorStaticInfo();
    MBOOL               validateP1(const ParamIspProfile_T& rParamIspProfile);
    MBOOL               setISPInfo(P2Param_T const &rNewP2Param, NSIspTuning::ISP_INFO_T &rIspInfo, MINT32 type);
    MBOOL               setP2Params(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2);
    MBOOL               generateP2(MINT32 flowType, const NSIspTuning::ISP_INFO_T& rIspInfo, void* pTuningBuf, ResultP2_T* pResultP2);
    MBOOL               getP2Result(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2);
    MINT32              convertP2ResultToMeta(const ResultP2_T& rResultP2, MetaSet_T* pResult) const;
    MBOOL               _readDump(TuningParam* pTuningBuf, const MetaSet_T& control, MetaSet_T* pResult, ISP_INFO_T* pIspInfo, MINT32 i4Format);
    IMetadata           getReprocStdExif(const MetaSet_T& control);
    unsigned int        queryTuningSize();
    MINT32              getResultCur(MINT32 i4FrmId);
    MINT32              getCurrResult(MUINT32 i4FrmId);
    MINT32              getCurrLCSResult(ISP_LCS_OUT_INFO_T const &rLcsOutInfo);

private:

    static MUINT32                                  m_u4LogEn;
    volatile std::atomic<int>                       m_Users;
    mutable std::mutex                              m_Lock;
    MINT32                                          m_i4SensorIdx;
    MUINT32                                         m_i4SensorDev;
    IspTuningMgr*                                   m_pTuning;
    NSCam::NSIoPipe::NSCamIOPipe::INormalPipe*      m_pCamIO;
    IResultPool*                                    m_pResultPoolObj;
    MINT32                                          m_i4SensorMode;
    MINT32                                          m_i4TgWidth;
    MINT32                                          m_i4TgHeight;
    MINT32                                          m_i4faceNum;
    MBOOL                                           m_bFaceDetectEnable;

    //Member
    NSIspTuning::EIspProfile_T                      m_eIspProfile;
    MINT32                                          m_i4Magic;
    MINT32                                          m_i4CopyLscP1En;
    std::mutex                                      m_P2Mtx;

    std::condition_variable                         m_rResultCond;
    std::mutex                                      m_rResultMtx;
    MUINT8                                          m_u1ColorCorrectMode;
    MUINT8                                          m_u1IsGetExif;
    MBOOL                                           m_bIsCapEnd;
    MBOOL                                           m_bDbgInfoEnable;

    MUINT32                                         m_i4SubsampleCount;
    // Pass1 HW setting callback
    HalISPP1DrvCbHub                               *mpHalISPP1DrvCbHub;
};

};

#endif //__HAL_ISP_IMP_H__

