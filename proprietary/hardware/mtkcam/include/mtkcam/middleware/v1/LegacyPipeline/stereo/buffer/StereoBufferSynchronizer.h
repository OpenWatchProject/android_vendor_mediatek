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

#ifndef _MTK_HARDWARE_MTKCAM_V1_STEREO_BUFFER_SYNCHRONIZER_H_
#define _MTK_HARDWARE_MTKCAM_V1_STEREO_BUFFER_SYNCHRONIZER_H_

namespace NSCam {
namespace v1 {
namespace NSLegacyPipeline{

class StereoSelector;



class IPreviewBufferUser
    : public virtual android::RefBase
{
    public:
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  IPreviewBufferUser Interface.
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        virtual MERROR                          onPreviewBufferReady() = 0;
};

class StereoBufferSynchronizer
    : public virtual android::RefBase
{
public:

    struct BUFFER_SET{
        StreamId_T                          streamId      = 0;
        MINT32                              reqNo         = 0;
        sp<IImageBufferHeap>                heap          = NULL;
        Vector<ISelector::MetaItemSet>      metadata;
        MBOOL                               isReturned          = MFALSE;
        MBOOL                               isAFStable          = MFALSE;
        MBOOL                               isTolerance         = MFALSE;
        MBOOL                               isPairCheckFailed   = MFALSE;
    };

    struct DEBUG_MODE{
        enum {
            NONE = 0,
            SKIP_TIMESTAMP_CHECK,
            SKIP_METADATA_CHECK
        };
    };

    enum SYNC_MODE{
        NOT_SET = 0,
        UNSYNC,
        SYNC
    };

    static android::sp<StereoBufferSynchronizer>
                                        createInstance( char const* szCallerName, MINT32 delayCount);

    // virtual                             ~StereoBufferSynchronizer() {};

    /**
    *    add streams to syncronizer which enable the sync process of this stream buffer
    *    MUST add streams before enque/deque
    */
    virtual     MERROR                  addStream(
                                            StreamId_T                      streamId,
                                            wp<StereoSelector>              rpSelector
                                        ) = 0;

    virtual     MERROR                  removeStream(
                                            StreamId_T                      streamId
                                        ) = 0;

    /**
    *    enqueBuffer from selector to pending pool.
    */
    virtual     MERROR                  enqueBuffer(
                                            MINT32                          rRequestNo,
                                            StreamId_T                      streamId,
                                            Vector<ISelector::MetaItemSet>  vResultMeta,
                                            android::sp<IImageBufferHeap>   rpHeap
                                        ) = 0;
    /**
    *    dequeBuffer from Capture pool to selector.
    *    The buffer dequed is syncronized.
    */
    virtual     MERROR                  dequeBufferCapture(
                                            MINT32&                         rRequestNo,
                                            StreamId_T                      streamId,
                                            Vector<ISelector::MetaItemSet>& rMeta,
                                            android::sp<IImageBufferHeap>&  rpHeap,
                                            MBOOL                           bNeedLockBuffer
                                        ) = 0;

    /**
    *    dequeBuffer from available pool to selector.
    *    The buffer dequed is syncronized.
    */
    virtual     MERROR                  dequeBuffer(
                                            MINT32&                         rRequestNo,
                                            StreamId_T                      streamId,
                                            Vector<ISelector::MetaItemSet>& rMeta,
                                            android::sp<IImageBufferHeap>&  rpHeap
                                        ) = 0;

    /**
    *    return buffer from stereoBufferPool.
    *    buffers return in this way will be kept for ZSL capture
    */
    virtual     MERROR                  returnBuffer(
                                            android::sp<IImageBufferHeap>&  rpHeap,
                                            StreamId_T                      streamId
                                        ) = 0;
    /**
    *    lock/unlock synchronizer
    *    MUST lock before trying to getResult from synchronizer
    */
    virtual     MERROR                  lock(Vector<StreamId_T>& acquiredBufferSet, Vector<StreamId_T>& rvLockedBufferSet, SYNC_MODE syncMode = NOT_SET) = 0;

    virtual     MERROR                  unlock() = 0;

    /**
    *    ZSL related APIs
    *    waitZSLBufferReady to make sure enough buffer is prepared for ZSL capture
    *    lock/unlock synchronizer for ZSL buffer
    *    MUST waitZSLBufferReady before pause pipeline
    *    MUST lock before trying to getResultZSL from synchronizer
    */
    virtual     MERROR                  waitAndLockCapture(MBOOL bNeedClearZSLQueue = MTRUE) = 0;

    virtual     MERROR                  unlockCapture() = 0;

    /**
    *    lock and deque a required buffer set
    *    It's a combination of lock and dequeBuffer
    */
    virtual    MERROR                   lockAndDequeBuffer(Vector<StreamId_T>& acquiredBufferSet, Vector<BUFFER_SET>& rvDequedBufferSet) = 0;

    /**
    *    start synchronizer
    *    MUST called AFTER all pass1 streams are added
    */
    virtual     MERROR                  start() = 0;

    /**
    *    flush synchronizer
    *    all buffer will be returned to selectors
    */
    virtual     MERROR                  flush(MBOOL flushUnSyncQueue = MTRUE) = 0;

    /**
 	*    flush capture available buffer
 	*    all buffer in capture available queue will be returned to selectors
 	*/
	virtual		MERROR					flushCaptureQueue() = 0;

	/**
    *    set IPreviewBufferUser
    *    when preview buffer ready, synchronizer will notify it
    */
    virtual     MERROR                  setPreviewBufferUser(sp<IPreviewBufferUser>) = 0;

    /**
    *    query stereo selector
    *    retruns stereo selector
    */
    virtual     sp<StereoSelector>      querySelector(StreamId_T streamId) = 0;

    virtual     void                    setDebugMode(MINT32 debug) = 0;

    virtual     void                    setZSLDelayCount(MINT32 delayCount) = 0;

    /**
    *    send query to enable frame sync for specific streams.
    *    buffer start to push to available queue after the first successful time sync buffer set
    */
    virtual     MERROR                  requestEnable(Vector<StreamId_T> streamsToSyncFromMain, Vector<StreamId_T> streamsToSyncFromAux) = 0;

    /**
    *    send query to disable frame sync for current frame sync stream set.
    *    buffer start to push to unSyncBufferQueue for next time sync buffer set,
    *    and the streams not belongs to "ToUse" will be returned immediately.
    */
    virtual     MERROR                  requestDisable(Vector<StreamId_T> streamsToUseFromMain, Vector<StreamId_T> streamsToUseFromAux) = 0;
};
/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSLegacyPipeline
};  //namespace v1
};  //namespace NSCam

#endif  //_MTK_HARDWARE_MTKCAM_V1_STEREO_BUFFER_SYNCHRONIZER_H_

