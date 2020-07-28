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
* @file IHal3A.h
* @brief Declarations of Abstraction of 3A Hal Class and Top Data Structures
*/

#ifndef __IHAL_3A_V3_H__
#define __IHAL_3A_V3_H__

#include <utils/List.h>
#include <vector>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/module/module.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include "aaa_hal_common.h"
#include "IHal3ACb.h"

namespace NS3Av3
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct MetaSet_T
{
    MINT32 MagicNum;
    std::vector<MINT32> vMagicNumSet;
    MUINT8 Dummy;
    MINT32 PreSetKey;
    MINT32 FrameNum;
    NSCam::IMetadata    appMeta;
    NSCam::IMetadata    halMeta;

    MetaSet_T(): MagicNum(-1), Dummy(0), PreSetKey(-1), FrameNum(-1){}
    MetaSet_T(MINT32 _MagicNum, MUINT8 _Dummy, MINT32 _PreSetKey, MINT32 _FrameNum, NSCam::IMetadata _appMeta, NSCam::IMetadata _halMeta)
    : MagicNum(_MagicNum)
    , Dummy(_Dummy)
    , PreSetKey(_PreSetKey)
    , FrameNum(_FrameNum)
    , appMeta(_appMeta)
    , halMeta(_halMeta)
    {}
    MetaSet_T(NSCam::IMetadata appMeta, NSCam::IMetadata halMeta): MetaSet_T(-1, 0, -1, -1, appMeta, halMeta){}
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**
 * @brief Interface of 3A Hal Class
 */
class IHal3A {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //    Ctor/Dtor.
                        IHal3A(){}
    virtual             ~IHal3A(){}

private: // disable copy constructor and copy assignment operator
                        IHal3A(const IHal3A&);
    IHal3A&             operator=(const IHal3A&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    enum E_VER
    {
        E_Camera_1 = 0,
        E_Camera_3 = 1
    };

    /**
     * @brief destroy instance of IHal3A
     * @param [in] strUser user name
     */
    virtual MVOID       destroyInstance(const char* /*strUser*/) {}
    /**
     * @brief config 3A setting
     * @param [in] rConfigInfo 3A setting of ConfigInfo_T
     * @return
     * - MINT32 value of TRUE/FALSE.
     */
    virtual MINT32      config(const ConfigInfo_T& rConfigInfo) = 0;
    /**
     * @brief config 3A with default setting
     * @param [in] SubsampleCount 3A setting of SubsampleCount
     * @return
     * - MINT32 value of TRUE/FALSE.
     */
    virtual MINT32      config(MINT32 i4SubsampleCount = 0)
                        {
                            ConfigInfo_T info;
                            info.i4SubsampleCount = i4SubsampleCount;
                            return config(info);
                        };
    /**
     * @brief start 3A
     * @param [in] i4StartNum not to use
     * @return
     * - MINT32 value of FALSE.
     */
    virtual MINT32      start(MINT32 i4StartNum=0) = 0;
    /**
     * @brief stop 3A
     * @return
     * - MINT32 value of TRUE/FALSE.
     */
    virtual MINT32      stop() = 0;

    /**
     * @brief pre-stop statisticpipe
     */
    virtual MVOID       stopStt() = 0;

    /**
     * @brief pause 3A
     */
    virtual MVOID      pause() = 0;
    /**
     * @brief resume 3A
     * @param [in] MagicNum which want to resume 3A
     */
    virtual MVOID      resume(MINT32 MagicNum = 0) = 0;

/****************************************************************
 *************************CAM3_3A_ISP_50_EN***********************
 ***************************************************************/

    /**
     * @brief Set controls via IHal3A. This will speed-up displaying in contrast to set()
     * @param [in] requestQ is controls in terms of vector of pointer to MetaSet_T
     * @return
     * - MINT32 value of FALSE.
     */
    virtual MINT32      startRequestQ(const std::vector<MetaSet_T*>& requestQ) = 0;

    /**
     * @brief Set list of controls via IHal3A
     * @param [in] requestQ is controls in terms of vector of pointer to MetaSet_T
     * @param [in] i4StartNum not to use
     * @return
     * - MINT32 value of Capture type.
     */
    virtual MINT32      startCapture(const std::vector<MetaSet_T*>& requestQ, MINT32 i4StartNum = 0) = 0;

    /**
     * @brief Differences from ISP4.x set : No more Android List and Size requisition
     * @brief Middleware will guarantee that the size of vector won't be modified within set function scope
     * @param [in] requestQ is in terms of vector of pointer to MetaSet_T
     * @return
     * - MINT32 value of FALSE.
     */
    virtual MINT32      set(const std::vector<MetaSet_T*>& requestQ) = 0;

    /**
     * @brief Difference from set : Due to full CCU feature, current request must be delivered before Vsync
     * @brief Middleware will guarantee that the size of vector won't be modified within set function scope
     * @param [in] requestQ is in terms of vector of pointer to MetaSet_T
     * @return
     * - MINT32 value of FALSE.
     */
    virtual MINT32      preset(const std::vector<MetaSet_T*>& requestQ) = 0;

//*****************************************Previously Compatible**********************************************

    /**
     * @brief Older version
     * @param [in] requestQ is controls list of MetaSet_T
     * @param [in] i4RequestQSize is size of controls list
     * @return
     * - MINT32 value of FALSE.
     */
    virtual MINT32      startRequestQ(const android::List<MetaSet_T>& requestQ, MINT32 i4RequestQSize = -1) = 0;

    /**
     * @brief Older version
     * @param [in] controls is controls list of MetaSet_T
     * @param [in] i4StartNum not to use
     * @param [in] i4RequestQSize is size of controls list
     * @return
     * - MINT32 value of FALSE.
     */
    virtual MINT32      startCapture(const android::List<MetaSet_T>& controls, MINT32 i4StartNum=0, MINT32 i4RequestQSize = -1) = 0;

    /**
     * @brief Older version
     * @param [in] controls is controls list of MetaSet_T, size of controls list
     * @param [in] i4RequestQSize is size of controls list
     * @return
     * - MINT32 value of FALSE.
     */
    virtual MINT32      set(const android::List<MetaSet_T>& controls, MINT32 i4RequestQSize = -1) = 0;

//****************************************Difference END here**************************************************

    /**
     * @brief Set pass2 tuning via IHal3A
     * @param [in] flowType 0 for processed raw, 1 for pure raw
     * @param [in] control is controls list of MetaSet_T
     * @param [out] pRegBuf buffer address for register setting
     * @param [out] pResult is result of IMetadata
     * @return
     * - MINT32 value of FALSE/(-1).
     */
    virtual MINT32      setIsp(MINT32 flowType, const MetaSet_T& control, TuningParam* pTuningBuf, MetaSet_T* pResult) = 0;

    /**
     * @brief Get dynamic result with specified frame ID via IHal3A
     * @param [in] frmId specified frame ID (magic number)
     * @param [out] result in terms of metadata
     * @return
     * - MINT32 value of MTRUE/(-1).
     */
    //virtual MINT32      get(MUINT32 frmId, IMetadata&  result) = 0;
    virtual MINT32      get(MUINT32 frmId, MetaSet_T& result) = 0;

    /**
     * @brief Get High quality capture result with specified frame ID via IHal3A
     * @param [in] frmId specified frame ID (magic number)
     * @param [out] result in terms of metadata
     * @return
     * - MINT32 value of FALSE.
     */
    virtual MINT32      getCur(MUINT32 frmId, MetaSet_T& result) = 0;

    /**
     * @brief Attach callback for notifying
     * @param [in] eId Notification message type
     * @param [in] pCb Notification callback function pointer
     * @return
     * - MINT32 value of call back size.
     */
    virtual MINT32      attachCb(IHal3ACb::ECb_T eId, IHal3ACb* pCb) = 0;

    /**
     * @brief Detach callback
     * @param [in] eId Notification message type
     * @param [in] pCb Notification callback function pointer
     * @return
     * - MINT32 value of call back size.
     */
    virtual MINT32      detachCb(IHal3ACb::ECb_T eId, IHal3ACb* pCb) = 0;


    /**
     * @brief Get delay frames via IHal3A
     * @param [out] delay_info in terms of metadata with MTK defined tags.
     * @return
     * - MINT32 value of MFALSE.
     */
    virtual MINT32      getDelay(NSCam::IMetadata& delay_info) const= 0;

    /**
     * @brief Get delay frames via IHal3A
     * @param [in] tag belongs to control+dynamic
     * @return
     * - MINT32 delay frame.
     */
    virtual MINT32      getDelay(MUINT32 tag) const = 0;

    /**
     * @brief Get capacity of metadata list via IHal3A
     * @return
     * - MINT32 value of capacity.
     */
    virtual MINT32      getCapacity() const = 0;

    /**
     * @brief Backdoor function to control manager
     * @param [in] e3ACtrl is 3A controls event
     * @param [in] i4Arg1 is one parameter
     * @param [in] i4Arg2 is second parameter
     * @return
     * - MINT32 value of TRUE/FALSE.
     */
    virtual MINT32      send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR i4Arg1, MINTPTR i4Arg2) = 0;

    /**
     * @brief set sensor mode
     * @param [in] i4SensorMode is sensor mode
     */
    virtual MVOID       setSensorMode(MINT32 i4SensorMode) = 0;

    /**
     * @brief Flash control flow and dump pass1 register
     * @param [in] i4MagicNum is request number
     * @param [in] pvArg is not to use
     */
#if (CAM3_3A_IP_BASE)
    virtual MVOID       notifyP1Done(MINT32 i4MagicNum, MVOID* pvArg = 0) = 0;
#else
    virtual MVOID       notifyP1Done(MUINT32 u4MagicNum, MVOID* pvArg = 0) = 0;
#endif

    /**
     * @brief notify sensor power on
     * @return
     * - MINT32 value of TRUE.
     */
    virtual MBOOL       notifyPwrOn() = 0;

    /**
     * @brief notify sensor power off
     * @return
     * - MINT32 value of TRUE.
     */
    virtual MBOOL       notifyPwrOff() = 0;

    /**
     * @brief notify CCU power .
     * @return
     * - MBOOL value of TRUE/FALSE.
     */
    virtual MBOOL       notifyP1PwrOn() = 0;

    /**
     * @brief notify CCU power off.
     * @return
     * - MBOOL value of TRUE/FALSE.
     */
    virtual MBOOL       notifyP1PwrOff() = 0;

    /**
     * @brief check whether flash on while capture
     * @return
     * - MBOOL value of IsHighQualityCaptureOn.
    */
    virtual MBOOL       checkCapFlash() = 0;

    /**
     * @brief set face detection
     * @param [in] fgEnable is on/off
    */
    virtual MVOID       setFDEnable(MBOOL fgEnable) = 0;

    /**
     * @brief set face info to manager
     * @param [in] prFaces is face info
     * @return
     * - MBOOL value of MTRUE.
    */
    virtual MBOOL       setFDInfo(MVOID* prFaces) = 0;

    virtual MBOOL       setFDInfoOnActiveArray(MVOID* prFaces) = 0;

    /**
     * @brief set object tracking info to manager
     * @param [in] prOT is object tracking info
     * @return
     * - MBOOL value of MTRUE.
    */
    virtual MBOOL       setOTInfo(MVOID* prOT) = 0;


   /**
     * @brief dump pass2 tuning in terms of metadata via IHal3A
     * @param [in] flowType 0 for processed raw, 1 for pure raw
     * @param [in] control MetaSet_T
     * @param [out] pRegBuf buffer address for register setting
     * @return
     * - MBOOL value of MTRUE.
     */
    virtual MINT32      dumpIsp(MINT32 flowType, const MetaSet_T& control, TuningParam* pTuningBuf, MetaSet_T* pResult) = 0;
};

}; // namespace NS3Av3


/**
 * @brief The definition of the maker of IHal3A instance.
 * @param [in] i4SensorIdx sensor index
 * @param [in] strUser user name
 */
typedef NS3Av3::IHal3A* (*Hal3A_FACTORY_T)(MINT32 const i4SensorIdx, const char* strUser);
#define MAKE_Hal3A(...) \
    MAKE_MTKCAM_MODULE(MTKCAM_MODULE_ID_AAA_HAL_3A, Hal3A_FACTORY_T, __VA_ARGS__)


#endif
