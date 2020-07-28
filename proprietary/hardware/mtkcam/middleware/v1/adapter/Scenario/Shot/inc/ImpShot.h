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

#ifndef _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_INC_IMPSHOT_H_
#define _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_INC_IMPSHOT_H_

#include <mtkcam/utils/std/JobQueue.h>
#include <memory>

using NSCam::JobQueue;

namespace NSCam {
    class IMetadata;
}

namespace android {
namespace NSShot {
/******************************************************************************
 *
 ******************************************************************************/


/******************************************************************************
 *  Implement Shot Class
 ******************************************************************************/
class ICamBufHandler;
class ImpShot : virtual public RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Attributes.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Attributes.

    String8 const                   ms8ShotName;        //  shot name
    uint32_t const                  mu4ShotMode;        //  shot mode (defined in EShotMode)
    int32_t const                   mi4OpenId;          //  open id: 0/1/2
    NSCam::EPipelineMode            mPipelineMode;      //  pipeline mode (defined in EPipelineMode)

    sp<IShotCallback>               mpShotCallback;     //  pointer to IShotCallback.

    ShotParam                       mShotParam;         //  Shot parameters.
    JpegParam                       mJpegParam;         //  Jpeg parameters.

    int32_t                         mi4DataMsgSet;

    // vendorManager
    int64_t                         mPluginUser;

private:    //// forbidden modification to derived class
    int64_t                         mInstanceId;        // unique instance ID, a timestamp which is compatible for BackgroundService.
    int32_t                         mVendorManagerId;   // unique ID, default is 0. Caller can invoke
                                                        // ImpShot::genVendorManagerId to generate one.
    std::shared_ptr<
        NSCam::JobQueue<void()>
    >                               mMinorJobQueue;     // A shared minor JobQueue for all derived ImpShot classes.

    static std::atomic<int32_t>     sVendorManagerId;   // unique VendorManagerId

private:    //// implementations
    // Acquire the shared JobQueue instance. If not exists, create a new one.
    //  @note After this method has been invoked, ImpShot::mMinorJobQueue instance
    //        is supposed to have an instance.
    //        This method is thread-safe.
    void                            acquireJobQueue();


public:     ////                    Attributes.
    inline char const*              getShotName() const     { return ms8ShotName.string(); }
    inline uint32_t                 getShotMode() const     { return mu4ShotMode; }
    inline NSCam::EPipelineMode     getPipelineMode() const { return mPipelineMode; }
    inline int32_t                  getOpenId() const       { return mi4OpenId; }
    inline int64_t                  getInstanceId() const   { return mInstanceId; }

    // Customized Vendor Manager methods. If derived classes wants to use the customized
    // VendorManager, caller can invoke genVendorManagerId to get the unique vendorManager ID
    // between all ImpShot instances.
    inline int32_t                  getVendorManagerId() const { return mVendorManagerId; }
    inline void                     setVendorManagerId(int32_t id) { mVendorManagerId = id; }
    inline void                     genVendorManagerId()    { mVendorManagerId = ++sVendorManagerId; }

    //  Set Shot parameters.
    virtual bool                    setShotParam(void const* pParam, size_t const size);

    //  Set Jpeg parameters.
    virtual bool                    setJpegParam(void const* pParam, size_t const size);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
    virtual                         ~ImpShot();
                                    ImpShot(
                                        char const*const pszShotName,
                                        uint32_t const u4ShotMode,
                                        int32_t const i4OpenId
                                    );

public:     ////                    Operations.

    //  This function is invoked when this object is ready to destryoed in the
    //  destructor. All resources must be released before this returns.
    virtual void                    onDestroy()                             = 0;

    //  Set callbacks.
    virtual bool                    setCallback(sp<IShotCallback>& rpShotCallback);

    virtual bool                    sendCommand(
                                        uint32_t const  cmd,
                                        MUINTPTR const  arg1,
                                        uint32_t const  arg2,
                                        uint32_t const  arg3 = 0
                                    );

    virtual bool                    isDataMsgEnabled(MINT32 const i4MsgTypes);

    virtual void                    enableDataMsg(MINT32 const i4MsgTypes);

    virtual void                    disableDataMsg(MINT32 const i4MsgTypes);

    bool                            updatePostNRSetting(
                                        MINT32 nrtype,
                                        bool const isMfll,
                                        NSCam::IMetadata* pMeta
                                    );

    bool                            updatePostNRSetting(
                                        NRParam& param
                                    );

    bool                            insertRawPlugin(
                                        int64_t             pluginUser,
                                        uint64_t            pluginModeId,
                                        NSCam::IMetadata*   pAppMeta,
                                        NSCam::IMetadata*   pHalMeta,
                                        NSCam::MSize        rawSize,
                                        int                 rawStreamId
                                    );

    virtual bool                    setManualAEControl(
                                        NSCam::IMetadata* pAppSetting,
                                        MUINT32 u4ExpTime,
                                        MUINT32 u4ExpGain,
                                        MUINT32 u4IspGain = 1024
                                    );

    int                             getShotRawBitDepth();


    // Check if using BackgroundService.
    //  @return                     MTRUE for need BackgroundService, otherwise no.
    virtual MBOOL                   supportBackgroundService() const;

    // Send P2done callback. Notice, if the P2 done cannot be sent (by some condition,
    // e.g.: memory is not enough), this job, sending P2 done callback, will be moved
    // to ImpShot::mMinorJobQueue, wait until it's ready to send P2 done callback.
    //  @return                     MTRUE indicates sending p2done cb successfully.
    virtual MBOOL                   onP2done();

    // Wait P2done ready. If P2done costs some time to be finished, this method returns
    // MFALSE and move the job, sending P2 done callback, to ImpShot::mMinorJobQueue
    // to wait until it's ready to send P2 done cb.
    //  @return                     MTRUE wait OK, caller can invoke
    //                              IShotCallback::onCB_P2done. Otherwise, do not
    //                              invoke IShotCallback::onCB_P2done, it will be
    //                              sent when the condition come true, where the condition
    //                              may be "available memory usage enought". Maybe.
    virtual MBOOL                   waitP2DoneReady();
};


/******************************************************************************
 *
 ******************************************************************************/
}; // namespace NSShot
}; // namespace android
#endif  //  _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_INC_IMPSHOT_H_

