
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
* @file IHalIspProtocol.h
* @brief Declarations of Abstraction of ISP Hal inter-protocol Class and Top Data Structures
*/

#ifndef __IHAL_ISP_PROTOCOL_V3_H__
#define __IHAL_ISP_PROTOCOL_V3_H__

#include <aaa_hal_common.h>
#include <private/aaa_hal_private.h>

#include <isp_tuning_cam_info.h>
#include <aaa_result.h>
#include <IHal3A.h>
#include <mtkcam/utils/TuningUtils/FileReadRule.h>

using namespace std;

namespace NS3Av3
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/**
 * @brief Interface of ISP Hal Class
 */

class IHalIspProtocol {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //    Ctor/Dtor.
                        IHalIspProtocol(){}
    virtual             ~IHalIspProtocol(){}

    //inter-protocol body
    virtual MBOOL       initTuning() = 0;
    virtual MBOOL       configTuning(MINT32 i4SubsampleCount) = 0;
    virtual MBOOL       uninitTuning() = 0;
    virtual MBOOL       setConfigParams(P1Param_T const &rNewConfigParam, RAWIspCamInfo &rP1CamInfo) = 0;
    virtual MBOOL       setP1Params(P1Param_T const &rNewP1Param, RAWIspCamInfo &rP1CamInfo) = 0;
    virtual MBOOL       validateP1(RequestSet_T const RequestSet, RAWIspCamInfo &rP1CamInfo) = 0;
    virtual MVOID       notifyRPGEnable(RAWIspCamInfo &rP1CamInfo, MBOOL const fgEnable) = 0;
    virtual MVOID       setIspProfile(RAWIspCamInfo &rP1CamInfo, EIspProfile_T const eIspProfile) = 0;
    virtual MVOID       setZoomRatio(RAWIspCamInfo &rP1CamInfo, MINT32 const i4ZoomRatio_x100) = 0;
    virtual MBOOL       setP1GetParams(P1GetParam_T const &rNewP1GetParam, RAWIspCamInfo &tempCamInfo, RAWIspCamInfo &rP1CamInfo) = 0;
    virtual MBOOL       setCCUFeedbackExif(P1GetParam_T const &rNewP1GetParam, RAWIspCamInfo &tempCamInfo) = 0;
    virtual MBOOL       setISPInfo(P2Param_T const &rNewP2Param, TuningParam* pTuningBuf, NSIspTuning::ISP_INFO_T &rIspInfo, MINT32 type) = 0;
    virtual MUINT32     queryTuningSize() = 0;
    virtual MBOOL       restoreCaminfo(P2Param_T &rNewP2Param, RAWIspCamInfo &mBackupCamInfo) = 0;
    virtual MBOOL       setP2Params(P2Param_T const &rNewP2Param, RAWIspCamInfo &rCamInfo, ResultP2_T* pResultP2) = 0;
    virtual MBOOL       generateP2(MINT32 flowType, const NSIspTuning::ISP_INFO_T& rIspInfo, void* pTuningBuf, ResultP2_T* pResultP2) = 0;
    virtual MBOOL       dumpP2DotTuning(ISP_INFO_T& rIspInfo, P2Param_T &rNewP2Param) = 0;
    virtual MBOOL       _readDumpP2Buf(TuningParam* pTuningBuf, const MetaSet_T& control, MetaSet_T* pResult, NSCam::TuningUtils::FileReadRule &rule,
                                       ISP_INFO_T* pIspInfo, NSCam::TuningUtils::ETuningFeedbackStage_T stage) = 0;
    virtual MBOOL       _dumpP2Buf(TuningParam* pTuningBuf, NSCam::TuningUtils::FileReadRule &rule, ISP_INFO_T& rIspInfo) = 0;

    virtual MVOID       setFDEnabletoISP(RAWIspCamInfo &rP1CamInfo, MBOOL const FD_enable) = 0 ;
    virtual MVOID       setFDInfotoISP(RAWIspCamInfo &rP1CamInfo, MVOID* prFaces) = 0;
    virtual MVOID       setSensorMode(RAWIspCamInfo &rP1CamInfo, MINT32 const i4SensorMode, MBOOL const bFrontalBin, MUINT32 const u4RawWidth, MUINT32 const u4RawHeight) = 0;
    virtual MVOID       setTGInfo(RAWIspCamInfo &rP1CamInfo, MINT32 const i4TGInfo, MINT32 const i4Width, MINT32 const i4Height) = 0;

};

}; // namespace NS3Av3

#endif


