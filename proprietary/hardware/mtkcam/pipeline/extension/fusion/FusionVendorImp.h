/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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
#ifndef _MTK_HARDWARE_MTKCAM_PIPELINE_EXTENSION_FUSION_VENDOR_IMP_H_
#define _MTK_HARDWARE_MTKCAM_PIPELINE_EXTENSION_FUSION_VENDOR_IMP_H_

// MTKCAM
#include <mtkcam/pipeline/extension/Vendor.h>
#include "../utils/VendorUtils.h"

#include "../BaseVendor.h"
#include "../utils/ControllerContainer.h"
//
#include "controller/BaseFusionCtrler.h"

#include <utils/Thread.h>

// STL
#include <future>
#include <vector>
#include <deque>
#include <atomic>
#include <memory>
#include <functional>

using namespace android;


namespace NSCam
{
namespace plugin
{

// class FusionVendorImp.
class FusionVendorImp
    : public virtual FusionVendor // implement FusionVendor
    , public virtual BaseVendor // use the implementations from BaseVendor
{
        //
        // IVendor Interface
        //
    public:
        using BaseVendor::getName;
        using BaseVendor::getOpenId;
        using BaseVendor::getVendorMode;
        using BaseVendor::acquireWorkingBuffer;
        using BaseVendor::releaseWorkingBuffer;
        using BaseVendor::prepareOneWithoutPool;

        // **************** Construct setting. ****************
        status_t            get(
            MINT32           openId,
            const InputInfo& in,
            FrameInfoSet&    out
        ) override;

        status_t            set(
            MINT32              openId,
            const InputSetting& in
        ) override;


        // **************** For user ****************
        status_t            queue(
            MINT32  const              openId,
            MUINT32 const              requestNo,
            wp<IVendor::IDataCallback> cb,
            BufferParam                bufParam,
            MetaParam                  metaParam
        ) override;

        status_t            beginFlush(MINT32 openId) override;
        status_t            endFlush(MINT32 openId) override;
        status_t            sendCommand(
            MINT32 cmd,
            MINT32 openId,
            MetaItem& meta,
            MINT32& arg1, MINT32& arg2,
            void*  arg3
        ) override;

        // **************** debug ****************
        status_t            dump(MINT32 openId) override;



        //
        // Methods
        //
    protected:
        // Wait until all controllers has finished jobs.
        //  @param bCancel          Tells all controllers to cancel job.
        virtual void                drain(bool bCancel = true);

        // Controller utility
    public:
        // Controller container selector
        enum CTRLER
        {
            CTRLER_USED,
            CTRLER_UNUSED
        };
    protected:
        // Get the container's pointer by selector.
        //  @param u                Selection of the container.
        //  @return                 The pointer of the container. Exception returns NULL.
        ControllerContainer<BaseFusionCtrler>*   getCtrlerContainer(CTRLER u);

        // Get the first controller from the specified container. If the container
        // is empty, to create a controller and push back into container.
        //  @param u                Selection of the container.
        //  @param mode             current vendor mode.
        //  @return                 Controller instance.
        //  @note                   This function IS SUPPOSED TO always return an
        //                          instance. If returns NULL, it means an unexpected
        //                          error happened. Need to figure out why.
        std::shared_ptr<BaseFusionCtrler> getCtrler(CTRLER u, MINT32 mode);

        // Get the controller from the specified container by the request number.
        // If the request number is not belong to any one, returns NULL.
        //  @param requestNo        The request number to found.
        //  @param u                Selection of the container.
        //  @return                 Controller instance.
        std::shared_ptr<BaseFusionCtrler> getCtrler(MUINT32 requestNo, CTRLER u);

        // Take the controller from the beginning of the container. If the container
        // is empty, this method returns NULL.
        //  @param u                Selection of the container.
        //  @return                 Controller instance.
        std::shared_ptr<BaseFusionCtrler> takeCtrler(CTRLER u);

        // Push back a controller to the specified container.
        //  @param c                The controller.
        //  @param u                Selection of container to be added.
        void                        pushCtrler(
            const std::shared_ptr<BaseFusionCtrler>& c,
            CTRLER u
        );

        // Removes the controller from the container.
        //  @param c                The controller to be removed.
        //  @param u                Seleciton of the container.
        //  @return                 If removed successfully, return true.
        bool                        removeCtrler(
            std::shared_ptr<BaseFusionCtrler> c,
            CTRLER u
        );




        // BaseVendor implementation
    protected:
        using BaseVendor::setThreadShotsSize;
        using BaseVendor::asyncRun;
        using BaseVendor::syncAllThread;

        // Attributes
    protected:
        using BaseVendor::mMode;
        using BaseVendor::mOpenId;
        using BaseVendor::mName;

        // Fusion Controller containers
    public:
        // Controller Container(s)
        static
        ControllerContainer<BaseFusionCtrler> sUnusedCtrlers; // everyone may want push un-used controller
        ControllerContainer<BaseFusionCtrler> mUsedCtrlers;


        // Constructor & Destructor
    public:
        FusionVendorImp(
            char const*  pcszName,
            MINT32 const i4OpenId,
            MINT64 const vendorMode
        ) noexcept;

        // copy and move object is forbidden
        FusionVendorImp(const FusionVendorImp&) = delete;
        FusionVendorImp(FusionVendorImp&&) = delete;

        virtual ~FusionVendorImp();



        // **************** local usage ****************
    public:
        // static
        static wp<FusionVendor> mThis;
        static MINT32           mTotalRefCount;
        MINT32                  mRefCount = 0;
        std::mutex              mRequestLock;

        Mutex           mMyLock;        // Thread::mLock is private
        Condition       mMyCond;        // Thread::mThreadExitedCondition is private

    private:
        //
        MBOOL                   mTeleSet = false;
        MSize                   mRawSize[2];
        MSize                   mJpegYuvSize[2];

    private:
        MINT32                  mMain1OpenId = -1;
        MINT32                  mMain2OpenId = -1;

};

} // namespace plugin
} // namespace NSCam
#endif // _MTK_HARDWARE_MTKCAM_PIPELINE_EXTENSION_FUSION_VENDOR_IMP_H_
