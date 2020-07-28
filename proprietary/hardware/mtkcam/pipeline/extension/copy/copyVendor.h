/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_EXTENSION_COPYVENDORIMP_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_EXTENSION_COPYVENDORIMP_H_

#include <utils/RefBase.h>
#include <utils/Vector.h>
#include <utils/threads.h>
#include <future>
#include <vector>

#include "../BaseVendor.h"
#include "../utils/RequestFrame.h"
#include <mtkcam/pipeline/extension/Vendor.h>

using namespace android;

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace plugin {

class CopyVendorImp
    : public BaseVendor
    , public CopyVendor
{
public:
                                    CopyVendorImp(
                                        char const*  pcszName,
                                        MINT32 const i4OpenId,
                                        MINT64 const vendorMode
                                    );

    virtual                         ~CopyVendorImp();

public:     //// construct setting.
    virtual status_t                get(
                                        MINT32           openId,
                                        const InputInfo& in,
                                        FrameInfoSet&    out
                                    );
    virtual status_t                set(
                                        MINT32              openId,
                                        const InputSetting& in
                                    );

public:     //// for user.
    virtual status_t                queue(
                                        MINT32  const              openId,
                                        MUINT32 const              requestNo,
                                        wp<IVendor::IDataCallback> cb,
                                        BufferParam                bufParam,
                                        MetaParam                  metaParam
                                    );
    virtual status_t                beginFlush( MINT32 openId );
    virtual status_t                endFlush( MINT32 openId );
    virtual status_t                sendCommand(
                                        MINT32 cmd,
                                        MINT32 openId,
                                        MetaItem& meta,
                                        MINT32& arg1, MINT32& arg2,
                                        void*  arg3rg3
                                    );

public:     //// debug
    virtual status_t                dump( MINT32 openId );

public:
    class PluginThread : public Thread
    {
    public:
        PluginThread(CopyVendorImp* pImp)
            : mpImp(pImp)
        {}
        ~PluginThread() {}

        // good place to do one-time initializations
        status_t readyToRun() override;
        bool     threadLoop() override;

        void     requestExit() override;
    private:
        CopyVendorImp* mpImp;
    };

    class RequestSet : public RefBase
    {
    public:
        enum {
            STATE_IDLE,
            STATE_FRAME_READY, // all frame collected
            STATE_RUNNING,
        };
    public:
        RequestSet(int openId)
            : mOpenId(openId)
            , mState(STATE_IDLE)
            , mpImp(NULL)
        {}
        ~RequestSet();

        MVOID    setBelong(InputSetting setting) { mInput = setting; }
        MVOID    setImgMap(RequestFrame::StreamIdMap_Img& m)
                        { m_streamIdMap_img = m; }
        //
        MVOID    remove(MUINT32 requestNo);
        MBOOL    belong(MUINT32 requestNo);
        MBOOL    empty() { return mInput.vFrame.isEmpty(); }
        MINT32   getState();
        status_t add(
                    const MINT32 openId,
                    MUINT32 requestNo,
                    BufferParam bufParam,
                    MetaParam metaParam,
                    sp<IVendor::IDataCallback> cb
                );
        status_t execute();
        //
        status_t flush();
    private:
        status_t onExecute();

    private:
        MINT32          mOpenId;
        MINT32          mState;
        CopyVendorImp*  mpImp;
        //
        InputSetting    mInput;
        //
        std::vector<std::shared_ptr<RequestFrame>>  mRequestList;
        //
        RequestFrame::StreamIdMap_Img  m_streamIdMap_img;
        RequestFrame::StreamIdMap_Meta m_streamIdMap_meta;

        std::vector< std::future<MERROR> > mvFutures;
    };

protected:
    status_t                        onDequeRequest(sp<RequestSet>& pRequest);
    void                            waitPluginDrained();

protected:  ////
    MBOOL                           mIsAsync;

protected:  ////
    mutable Mutex                   mLock;
    MBOOL                           mFlush;
    Vector< sp<RequestSet> >        mRequestList;

protected:
    mutable Mutex                   mReadyLock;
    Condition                       mReadyCond;
    Vector< sp<RequestSet> >        mvReadyList;

protected:
    MBOOL                           mPluginExit;
    sp<PluginThread>                mPluginThread;

protected:
    MBOOL                           mPluginDrained;
    Condition                       mPluginDrainedCond;
};

} // namespace plugin
} // namespace NSCam

#endif // _MTK_HARDWARE_INCLUDE_MTKCAM_V3_EXTENSION_COPYVENDORIMP_H_
