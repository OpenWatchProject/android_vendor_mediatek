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
* @file IHalISP.h
* @brief Declarations of Abstraction of ISP Hal Class and Top Data Structures
*/

#ifndef __IHAL_ISP_V3_H__
#define __IHAL_ISP_V3_H__

#include <mtkcam/def/common.h>
#include <vector>

//Warning For Build Pass MetaSet_T
#include "IHal3A.h"

#include "aaa_hal_common.h"

#include <mtkcam/drv/iopipe/CamIO/Cam_Notify.h>

using namespace std;

namespace NS3Av3
{

enum E_P1YUV_PORT_T
{
    E_YUVO_R1 = 0x1,
    E_CRZO_R1 = 0x10,
    E_CRZO_R2 = 0x100
} ;

/**
 *  @EISPCtrl_GetMaxRrzRatio: Since RRZ ratio may cause some ISP module
 *    registers overflow, caller has to check the maximum RRZ downscale ratio.
 *    @iArg1: address of an integer to represent denominator (1/iArg1).
 *    @iArg2: non-use, keeps it to 0.
 *    @return: 0 for ok, if this control doesn't support, returns -ENOTSUP and
 *             it means there's no limitation of RRZ ratio.
 */
enum EISPCtrl_T
{
    EISPCtrl_Begin = 0,
    //ISP

    EISPCtrl_GetIspGamma      = 0x0001,
    EISPCtrl_ValidatePass1,
    EISPCtrl_SetIspProfile,
    EISPCtrl_GetOBOffset,
    EISPCtrl_GetRwbInfo,
    EISPCtrl_SetOperMode,
    EISPCtrl_GetOperMode,
    EISPCtrl_GetMfbSize,
    EISPCtrl_SetLcsoParam,
    EISPCtrl_GetLtmCurve,
    EISPCtrl_GetP2TuningInfo,
    EISPCtrl_NotifyCCU,
    EISPCtrl_WTSwitch,
    EISPCtrl_GetLCEGain,
    EISPCtrl_GetAINRParam,
    EISPCtrl_SetFdSource,
    EISPCtrl_GetMaxRrzRatio,
    EISPCtrl_Num
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/**
 * @brief Interface of ISP Hal Class
 */

class IHalISP {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //    Ctor/Dtor.
                        IHalISP(){}
    virtual             ~IHalISP(){}

private: // disable copy constructor and copy assignment operator
                        IHalISP(const IHalISP&);
    IHalISP&             operator=(const IHalISP&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    /**
     * @brief destroy instance of IHal3A
     * @param [in] strUser user name
     * @return
     * - NULL
     */
    virtual MVOID       destroyInstance(const char* strUser) { (void)strUser; }

    /**
     * @brief config ISP setting
     * @param [in] rConfigInfo ISP setting of ConfigInfo_T
     * @return
     * - MINT32 value of TRUE/FALSE.
     */
    virtual MINT32      config(const ConfigInfo_T& rConfigInfo) = 0;

    virtual MINT32      config(MINT32 i4SubsampleCount = 1)
                        {
                            ConfigInfo_T info;
                            info.i4SubsampleCount = i4SubsampleCount;
                            return config(info);
                        };

    /**
     * @brief set sensor mode
     * @param [in] i4SensorMode
     * @return
     * - NULL
     */
    virtual MVOID       setSensorMode(MINT32 i4SensorMode) = 0;

    /**
     * @brief init ISP
     * @param [in] strUser: user name in char
     * @return
     * - MBOOL value of TRUE/FALSE.
     */
    virtual MBOOL       init(const char* strUser) = 0;

    /**
     * @brief uninit ISP
     * @param [in] strUser: user name in char
     * @return
     * - MBOOL value of TRUE/FALSE.
     */
    virtual MBOOL       uninit(const char* strUser) = 0;

     /**
     * @brief start ISP
     * @return
     * - MBOOL value of TRUE/FALSE.
     */
    virtual MBOOL       start() = 0;

      /**
     * @brief stop ISP
     * @return
     * - MBOOL value of TRUE/FALSE.
     */
    virtual MBOOL       stop() = 0;

    /**
     * @brief resume 3A
     * @param [in] MagicNum which want to resume 3A
     * @return
     * - NULL
     */
    virtual MVOID      resume(MINT32 MagicNum = 0) = 0;

    /**
     * @brief set p1 isp tuning
     * @param [in] requestQ: p1 request metadata
     * @param [in] fgForce: dummy request
     * @return
     * - MBOOL value of TRUE/FALSE.
     */
    virtual MBOOL       setP1Isp(const vector<MetaSet_T*>& requestQ, MBOOL const fgForce = 0) = 0;

    /**
     * @brief get ISP setting
     * @param [in] frmId: want to get the result of setting with frame id
     * @param [out] result: output metadata for p1
     * @return
     * - MINT32 value of TRUE/FALSE.
     */
    virtual MINT32      get(MUINT32 frmId, MetaSet_T& result) = 0;

    /**
     * @brief get current ISP setting when capture
     * @param [in] frmId: want to get the result of setting with frame id
     * @param [out] result: output metadata for p1
     * @return
     * - MINT32 value of TRUE/FALSE.
     */
    virtual MINT32      getCur(MUINT32 frmId, MetaSet_T& result) = 0;

    /**
     * @brief set p2 isp tuning
     * @param [in] flowType: 0 -> not High Quality Capture, 1 -> is Hight Quality Capture
     * @param [in] control: p2 request metadata
     * @param [out] pTuningBuf: tuning buffer for pass-2 setting
     * @param [out] pResult: output metadata
     * @return
     * - MBOOL value of TRUE/FALSE.
     */
    virtual MBOOL       setP2Isp(MINT32 flowType, const MetaSet_T& control, TuningParam* pTuningBuf, MetaSet_T* pResult) = 0;

    /**
     * @brief set FD enable to isp
     * @param [in] fgEnable: status of on/off
     * @return
     * - NULL
     */
    virtual MVOID       setFDEnable(MBOOL fgEnable) = 0;

    /**
     * @brief Set FD info to isp
     * @param [in] prFaces: MtkCameraFaceMetadata for setting isp
     * @return
     * - MBOOL value of TRUE/FALSE.
     */
    virtual MBOOL       setFDInfo(MVOID* prFaces) = 0;

    /**
     * @brief
     * @param [in] index for check
     * @param [in] curve for gamma
     * @param [in] size of curve
     * - MBOOL value of TRUE/FALSE.
     */
    virtual MBOOL       setFdGamma(MINT32 i4Index=-1, MINT32* pTonemapCurveRed=NULL, MUINT32 u4size=0) = 0;

    /**
     * @brief This function is deprecated after isp60
     * @return
     * - MBOOL value of TRUE/FALSE.
     */
    virtual MBOOL       setOTInfo(MVOID* prOT) = 0;

    /**
     * @brief standard api to Send isp control
     * @param [in] eISPCtrl: control code
     * @param [in] iArg1: input1
     * @param [in] iArg2: input2
     * @return 0 for ok, otherwise checks POSIX error code. Basically, if the
     *         control is not support, returns -ENOTSUP.
     */
    virtual MINT32      sendIspCtrl(EISPCtrl_T eISPCtrl, MINTPTR iArg1, MINTPTR iArg2) = 0;

    //For p1 driver's callback
    virtual MINT32      InitP1Cb() = 0;
    virtual MINT32      UninitP1Cb() = 0;
    virtual MINT32      ConfigP1Cb() =0;

    /**
     * @brief Attach callback for notifying
     * @param [in] eId Notification message type
     * @param [in] pCb Notification callback function pointer
     * @return
     * - MINT32 value of TRUE/FALSE.
     */
    virtual MINT32      attachCb(/*IHal3ACb::ECb_T eId, IHal3ACb* pCb*/) = 0;

    /**
     * @brief Dettach callback
     * @param [in] eId Notification message type
     * @param [in] pCb Notification callback function pointer
     * @return
     * - MINT32 value of TRUE/FALSE.
     */
    virtual MINT32      detachCb(/*IHal3ACb::ECb_T eId, IHal3ACb* pCb*/) = 0;

    /**
     * @brief dump isp info with following info
     * @param [in] flowType: 0 -> not High Quality Capture, 1 -> is Hight Quality Capture
     * @param [in] control: p2 request metadata
     * @param [out] pTuningBuf: tuning buffer for pass-2 setting
     * @param [out] pResult: output metadata
     * @return
     * - MINT32 value of TRUE/FALSE.
     */
    virtual MINT32      dumpIsp(MINT32 flowType, const MetaSet_T& control, TuningParam* pTuningBuf, MetaSet_T* pResult) = 0;

    /**
     * @brief Query current isp buf information
     * @param [out] bufferInfo
     * @return
     * - MBOOL value of TRUE/FALSE.
     */
    virtual MBOOL       queryISPBufferInfo(Buffer_Info& bufferInfo) = 0;
};

}; // namespace NS3Av3


/**
 * @brief The definition of the maker of IHalISP instance.
 */
typedef NS3Av3::IHalISP* (*HalISP_FACTORY_T)(MINT32 const i4SensorIdx, const char* strUser);
#define MAKE_HalISP(...) \
    MAKE_MTKCAM_MODULE(MTKCAM_MODULE_ID_AAA_HAL_ISP, HalISP_FACTORY_T, __VA_ARGS__)


#endif
