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
* @file Hal3AAdapter3.h
* @brief Declarations of Implementation of 3A Hal Class
*/

#ifndef __HAL_3A_ADAPTER_3_H__
#define __HAL_3A_ADAPTER_3_H__

#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/feature/hdrDetection/Defs.h>
#include <mtkcam/utils/hw/HwTransform.h>
#include "aaa_hal_if.h"
#include "aaa_utils.h"
#include <faces.h>

#include <string>
#include <map>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <list>
#include <ISync3A.h>
#include <IResultPool.h>

using namespace std;

namespace NS3Av3
{

class Hal3ACbSet : public IHal3ACb
{
public:
    Hal3ACbSet();
    virtual                 ~Hal3ACbSet();

    virtual void            doNotifyCb (
                               MINT32  _msgType,
                               MINTPTR _ext1,
                               MINTPTR _ext2,
                               MINTPTR _ext3
                            );

    virtual MINT32          addCallback(IHal3ACb* cb);
    virtual MINT32          removeCallback(IHal3ACb* cb);
private:
    std::list<IHal3ACb*>    m_CallBacks;
    std::mutex              m_Mutex;
};

struct StaticInfo_T
{
  MINT32 i4MaxRegionAe;
  MINT32 i4MaxRegionAwb;
  MINT32 i4MaxRegionAf;
  MUINT8 u1RollingShutterSkew;
};

class Hal3AAdapter3 : public IHal3A, public I3ACallBack
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //    Ctor/Dtor.
    Hal3AAdapter3(MINT32 const i4SensorIdx);
    virtual ~Hal3AAdapter3(){}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
     //
    /**
      * @brief Create instance of Hal3AAdapter3
      * @param [in] i4SensorIdx.
      */
    static Hal3AAdapter3* createInstance(MINT32 const i4SensorIdx, const char* strUser);
    /**
      * @brief destroy instance of Hal3AAdapter3
      */
    virtual MVOID       destroyInstance(const char* strUser);
    /**
     * @brief config 3A
     */
    virtual MINT32      config(const ConfigInfo_T& rConfigInfo);
    /**
     * @brief start 3A
     */
    virtual MINT32      start(MINT32 i4StartNum=0);
    /**
     * @brief stop 3A
     */
    virtual MINT32      stop();

    /**
     * @brief stop Stt
     */
    virtual MVOID      stopStt();

    /**
     * @brief pause 3A
     */
    virtual MVOID      pause();
    /**
     * @brief resume 3A
     */
    virtual MVOID      resume(MINT32 MagicNum = 0);

/********************************************************************************************************
****************************************** ISP_50_EN ****************************************************
********************************************************************************************************/

    /**
     * @brief Set list of controls in terms of metadata of capture request via IHal3A
     * @param [in] controls list of MetaSet_T
     */
    virtual MINT32      startCapture(const vector<MetaSet_T*>& requestQ, MINT32 i4StartNum = 0);

    // interfaces for pre-metadata processing
    /**
     * @brief Set list of controls in terms of metadata via IHal3A
     * @param [in] controls list of MetaSet_T
     */
    virtual MINT32      startRequestQ(const vector<MetaSet_T*>& requestQ);

    // interfaces for metadata processing
    /**
     * @brief Set list of controls in terms of metadata via IHal3A
     * @param [in] controls list of MetaSet_T
     */
    virtual MINT32      set(const vector<MetaSet_T*>& requestQ);

    /**
     * Difference from set : Due to full CCU feature, current request must be delivered before Vsync
     * Middleware will guarantee that the size of vector won't be modified during set function is working
     * @param [in] controls list of MetaSet_T
     */
    virtual MINT32      preset(const std::vector<MetaSet_T*>& requestQ);

//****************************************Difference Begin Here*****************************************

    /**
     * @brief Set list of controls in terms of metadata of capture request via IHal3A
     * @param [in] controls list of MetaSet_T
     */
    virtual MINT32      startCapture(const list<MetaSet_T>& controls, MINT32 i4StartNum=0, MINT32 i4RequestQSize = -1);

    // interfaces for pre-metadata processing
    /**
     * @brief Set list of controls in terms of metadata via IHal3A
     * @param [in] controls list of MetaSet_T
     */
    virtual MINT32      startRequestQ(const list<MetaSet_T>& requestQ, MINT32 i4RequestQSize = -1);

    // interfaces for metadata processing
    /**
     * @brief Set list of controls in terms of metadata via IHal3A
     * @param [in] controls list of MetaSet_T
     */
    virtual MINT32      set(const list<MetaSet_T>& controls, MINT32 i4RequestQSize = -1);

//****************************************Difference End Here*******************************************

    /**
     * @brief Set pass2 tuning in terms of metadata via IHal3A
     * @param [in] flowType 0 for processed raw, 1 for pure raw
     * @param [in] control MetaSet_T
     * @param [out] pRegBuf buffer address for register setting
     * @param [out] result IMetadata
     */
    virtual MINT32      setIsp(MINT32 flowType, const MetaSet_T& control, TuningParam* pTuningBuf, MetaSet_T* pResult);

    /**
     * @brief dump pass2 tuning in terms of metadata via IHal3A
     * @param [in] flowType 0 for processed raw, 1 for pure raw
     * @param [in] control MetaSet_T
     * @param [out] pRegBuf buffer address for register setting
     */
    virtual MINT32      dumpIsp(MINT32 flowType, const MetaSet_T& control, TuningParam* pTuningBuf, MetaSet_T* pResult);

    /**
     * @brief Get dynamic result with specified frame ID via IHal3A
     * @param [in] frmId specified frame ID (magic number)
     * @param [out] result in terms of metadata
     */
    //virtual MINT32      get(MUINT32 frmId, IMetadata&  result);
    virtual MINT32      get(MUINT32 frmId, MetaSet_T& result);
    virtual MINT32      getCur(MUINT32 frmId, MetaSet_T& result);

    /**
     * @brief Attach callback for notifying
     * @param [in] eId Notification message type
     * @param [in] pCb Notification callback function pointer
     */
    virtual MINT32      attachCb(IHal3ACb::ECb_T eId, IHal3ACb* pCb);

    /**
     * @brief Dettach callback
     * @param [in] eId Notification message type
     * @param [in] pCb Notification callback function pointer
     */
    virtual MINT32      detachCb(IHal3ACb::ECb_T eId, IHal3ACb* pCb);

    /**
     * @brief Get delay frames via Hal3AAdapter3
     * @param [out] delay_info in terms of metadata with MTK defined tags.
     */
    virtual MINT32      getDelay(IMetadata& delay_info) const;

    virtual MINT32      getDelay(MUINT32 tag) const;

    /**
     * @brief Get capacity of metadata list via IHal3A
     * @return
     * - MINT32 value of capacity.
     */
    virtual MINT32      getCapacity() const;

    virtual MINT32      send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR i4Arg1, MINTPTR i4Arg2);

    /**
     * @brief set sensor mode
     * @param [in] i4SensorMode
     */
    virtual MVOID       setSensorMode(MINT32 i4SensorMode);

    virtual MVOID       notifyP1Done(MINT32 i4MagicNum, MVOID* pvArg = 0);

    virtual MBOOL       notifyPwrOn();

    virtual MBOOL       notifyPwrOff();

    virtual MBOOL       checkCapFlash();

    virtual MVOID       setFDEnable(MBOOL fgEnable);

    virtual MBOOL       setFDInfo(MVOID* prFaces);

    virtual MBOOL       setFDInfoOnActiveArray(MVOID* prFaces);

    virtual MBOOL       setOTInfo(MVOID* prOT);

    /**
     * @brief notify CCU power on.
     * @return
     * - MBOOL value of TRUE/FALSE.
     */
    virtual MBOOL       notifyP1PwrOn();

    /**
     * @brief notify CCU power off.
     * @return
     * - MBOOL value of TRUE/FALSE.
     */
    virtual MBOOL       notifyP1PwrOff();

    virtual void        doNotifyCb (
                           MINT32  _msgType,
                           MINTPTR _ext1,
                           MINTPTR _ext2,
                           MINTPTR _ext3
                        );

    virtual void        doDataCb (
                           int32_t _msgType,
                           void*   _data,
                           uint32_t _size
                        );

    static MUINT32          mu4LogEn;
    static MUINT32          mu4DbgLogWEn;
    static MUINT32          mu4ManualMode;
    static MUINT32          mu4ManualModeP2;
    static MUINT32          mi4ForceDebugDump;
    static map<MINT32,NSCamHW::HwMatrix>       mMapMat;
    static std::mutex       mMapMatLock;

protected:
    MBOOL                   init(const char* strUser);
    MBOOL                   uninit(const char* strUser);
    MUINT8                  parseMeta(const vector<MetaSet_T*>& requestQ);
//#ifdef CAM3_3A_ISP50_EN
    MINT32                  setNormal(const vector<MetaSet_T*>& requestQ);
//#else
    MINT32                  setNormal(const list<MetaSet_T>& requestQ, MINT32 i4RequestQSize = -1);
//#endif
    MBOOL                   doInit();
    MBOOL                   doUninit();
    MBOOL                   doInitMetaResultQ();
    MINT32                  buildSceneModeOverride();
    MINT32                  setupStaticInfo();

    MINT32                  convertP2ResultToMeta(const ResultP2_T& rResultP2, MetaSet_T* pResult) const;

    //IMetadata               getReprocStdExif(const MetaSet_T& control);
    //unsigned int            queryTuningSize();
    //MBOOL                   setP2Params(P2Param_T &rNewP2Param, const MetaSet_T& control, ResultP2_T* pResultP2, NSIspTuning::ISP_INFO_T& rReprocInput);
    //MBOOL                   _readDump(TuningParam* pTuningBuf, const MetaSet_T& control, MetaSet_T* pResult, ISP_INFO_T* pIspInfo, MINT32 i4Format);
    MBOOL                   getStaticMeta();
    MVOID                   parseConfig(const ConfigInfo_T& rConfigInfo);


    typedef list<MetaSet_T> MetaSetList_T;
    struct Mode3A_T
    {
        MUINT8 u1AeMode;
        MUINT8 u1AwbMode;
        MUINT8 u1AfMode;
    };

    struct UpdateMetaResult_T
    {
        mtk_camera_metadata_tag_t tag;
        IMetadata::IEntry entry;
    };

    Hal3AIf*                mpHal3aObj;
    IResultPool*            mpResultPoolObj;
    MINT32                  mi4FrmId;
    MINT32                  mi4SensorIdx;
    MUINT32                 mu4SensorDev;
    MINT32                  mi4SensorMode;
    MUINT32                 mu4Counter;
    std::mutex              mLock;
    std::mutex              mLockIsp;
    std::mutex              mLockResult;
    std::mutex              mutexParseMeta;

    MUINT8                  mu1CapIntent;
    MUINT8                  mu1Precature;
    MUINT8                  mu1Start;
    MUINT8                  mu1FdEnable;
    MUINT8                  mu1RepeatTag;
    MUINT8                  mu1HdrEnable;

    Param_T                 mParams;
    AF_Param_T              mAfParams;
    IMetadata               mMetaStaticInfo;
    std::vector<UpdateMetaResult_T> mUpdateMetaResult;
    MUINT32                 mu4MetaResultQueueCapacity;
    MRect                   mActiveArraySize;

#if CAM3_STEREO_FEATURE_EN
    ISync3AMgr::Stereo_Param_T mStereoParam;
#endif

protected:

    Hal3ACbSet              m_CbSet[IHal3ACb::eID_MSGTYPE_NUM];
    StaticInfo_T            m_rStaticInfo;
    ConfigInfo_T            m_rConfigInfo;
    MINT32                  m_i4Presetkey;
    std::map<MUINT8, Mode3A_T> m_ScnModeOvrd;
    std::unordered_map<std::string, MINT32> m_Users;

    // for FD
    MRect                   mPrvCropRegion;
    MtkCameraFaceMetadata   mAFFaceMeta;
    MtkCameraFace           mFace[15];
    MtkFaceInfo             mPosInfo[15];

    // for App meta
    MRect                   mAppCropRegion;

    // for OT
    MtkCameraFaceMetadata   mAFOTMeta;
    MtkCameraFace           mOT[15];
    // for repeat result
    MetaSet_T               mMetaSet;
    MUINT32                 mi4PreTgWidth;
    MUINT32                 mi4PreTgHeight;
    MUINT32                 mi4AppfgCrop;
    std::mutex              mLockConfigSend3ACtrl;
    MUINT32                 mSubSampleCount4Clear;
    MUINT8                  mu1Override3A;
    MUINT8                  mu1AfTrigSMVR;
    MUINT8                  mu1IspProfile;
    MBOOL                   mbEnableOverride;
    MINT32                  m_i4IsByPassCal;
};

};

#endif //__HAL_3A_ADAPTER_3_H__

