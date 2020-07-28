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

#ifndef __HAL_ISP_TEMPLATE_H__
#define __HAL_ISP_TEMPLATE_H__

#include <mtkcam/aaa/IHalISP.h>
#include <mtkcam/utils/hw/HwTransform.h>

using namespace NSCam;
using namespace NSCamHW;

#include <isp_tuning/isp_tuning_mgr.h>
#include <private/IopipeUtils.h>

#include <IResultPool.h>
#include <ResultPool4LSCConfig.h>
#include <ResultPool4Module.h>
#include <ResultPoolImpWrapper.h>

//Module
#include <mtkcam/aaa/IDngInfo.h>
#if CAM3_LSC_FEATURE_EN
#include <lsc/ILscMgr.h>
#endif

#include <HalISPCbHub.h>
#include <IHalIspProtocol.h>
//
#include <aaa_result.h>

namespace NS3Av3
{
using namespace NSIspTuning;

class HalIspTemplate : public IHalISP, public IHalIspProtocol
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

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

    /**
     * @brief config ISP setting when W+T callback is triggered
     * @param [in] TUNINGCB_INPUT_INFO *apConfigInfo
     * @return
     * - NULL
     */
    virtual MVOID       WT_Reconfig(MVOID* pInput);

    /**
     * @brief set sensor mode
     * @param [in] i4SensorMode
     * @return
     * - NULL
     */
    virtual MVOID       setSensorMode(MINT32 i4SensorMode);

    /**
     * @brief init ISP
     * @param [in] strUser: user name in char
     * @return
     * - MBOOL value of TRUE/FALSE.
     */
    virtual MBOOL       init(const char* strUser);

    /**
     * @brief uninit ISP
     * @param [in] strUser: user name in char
     * @return
     * - MBOOL value of TRUE/FALSE.
     */
    virtual MBOOL       uninit(const char* strUser);

    /**
     * @brief start ISP
     * @return
     * - MBOOL value of TRUE/FALSE.
     */
    virtual MBOOL       start();

    /**
     * @brief stop ISP
     * @return
     * - MBOOL value of TRUE/FALSE.
     */
    virtual MBOOL       stop();

    /**
     * @brief resume 3A
     * @param [in] MagicNum which want to resume 3A
     * @return
     * - NULL
     */
    virtual MVOID       resume(MINT32 MagicNum = 0);

    /**
     * @brief set p1 isp tuning
     * @param [in] requestQ: p1 request metadata
     * @param [in] fgForce: dummy request
     * @return
     * - MBOOL value of TRUE/FALSE.
     */
    virtual MBOOL       setP1Isp(const std::vector<MetaSet_T*>& requestQ, MBOOL const fgForce = 0);

    /**
     * @brief get ISP setting
     * @param [in] frmId: want to get the result of setting with frame id
     * @param [out] result: output metadata for p1
     * @return
     * - MINT32 value of TRUE/FALSE.
     */
    virtual MINT32      get(MUINT32 frmId, MetaSet_T& result);

    /**
     * @brief get current ISP setting when capture
     * @param [in] frmId: want to get the result of setting with frame id
     * @param [out] result: output metadata for p1
     * @return
     * - MINT32 value of TRUE/FALSE.
     */
    virtual MINT32      getCur(MUINT32 frmId, MetaSet_T& result);

    /**
     * @brief set p2 isp tuning
     * @param [in] flowType: 0 -> not High Quality Capture, 1 -> is Hight Quality Capture
     * @param [in] control: p2 request metadata
     * @param [out] pTuningBuf: tuning buffer for pass-2 setting
     * @param [out] pResult: output metadata
     * @return
     * - MBOOL value of TRUE/FALSE.
     */
    virtual MBOOL       setP2Isp(MINT32 flowType, const MetaSet_T& control, TuningParam* pTuningBuf, MetaSet_T* pResult);

    virtual MVOID       setFDEnable(MBOOL fgEnable) ;

    virtual MBOOL       setFDInfo(MVOID* prFaces) ;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //    Ctor/Dtor.
    HalIspTemplate(MINT32 const i4SensorIdx);
    virtual ~HalIspTemplate(){}

    //shared member function to child
    MINT32              getCurrResult(MUINT32 i4FrmId);
    MINT32              getResultCur(MINT32 i4FrmId);
    IMetadata           getReprocStdExif(const MetaSet_T& control);
    MINT32              convertP2ResultToMeta(const ResultP2_T& rResultP2, MetaSet_T* pResult) const;
    MINT32              getOBCFromResultPool(MINT32 **OBOffset);
    MINT32              getLCSFromResultPool(ISP_LCS_OUT_INFO_T const &rLcsOutInfo);

    //shared members to child
    static MINT32                                   m_i4LogEn;
    NSCam::NSIoPipe::NSCamIOPipe::INormalPipe*      m_pCamIO;
    MINT32                                          m_i4SensorIdx;
    MUINT32                                         m_i4SensorDev;
    MINT32                                          m_i4SensorMode;

    MINT32                                          m_i4TgWidth;
    MINT32                                          m_i4TgHeight;

private:
    //member function
    MINT32              updateTGInfo();
    MVOID               querySensorStaticInfo(RAWIspCamInfo &rP1CamInfo);
    MBOOL               getP2Result(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2);
    MBOOL               dumpP2AAO(ISP_INFO_T& rIspInfo, P2Param_T &rNewP2Param);

    //members
    volatile std::atomic<int>                       m_Users;
    std::mutex                                      m_Lock;
    std::mutex                                      m_P2Mtx;

    ResultPoolImpWrapper*                           m_pResultPoolWrapper;
    std::condition_variable                         m_rResultCond;
    std::mutex                                      m_rResultMtx;

    MINT32                                          m_i4Magic;
    MUINT32                                         m_i4SubsampleCount;

    MUINT8                                          m_u1ColorCorrectMode;
    MUINT8                                          m_u1IsGetExif;
    MBOOL                                           m_bIsCapEnd;
    // backup & restore caminfo
    std::mutex                                      m_LockLastInfo;
    NSIspTuning::RAWIspCamInfo                      m_BackupCamInfo;
    MBOOL                                           m_BackupCamInfo_copied;
    NSIspTuning::RAWIspCamInfo                      m_P1CamInfo;
};

};

#endif //__HAL_ISP_IMP_H__

