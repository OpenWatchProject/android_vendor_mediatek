/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
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
#ifndef __MFLLYUVCAPTURER_H__
#define __MFLLYUVCAPTURER_H__


#if ((MFLL_CORE_VERSION_MAJOR >= 3) || (MFLL_CORE_VERSION_MAJOR >= 2 && MFLL_CORE_VERSION_MINOR >= 5))
#else
#error "MfllYuvCapturer v1.2 only supports MFNR Core Library version >= 2.5"
#endif

// MFNR Library
#include <mtkcam3/feature/mfnr/IMfllCore.h>
#include <mtkcam3/feature/mfnr/IMfllCapturer.h>
#include <mtkcam3/feature/mfnr/IMfllImageBuffer.h>
#include <mtkcam3/feature/mfnr/IMfllEventListener.h>

// STL
#include <vector>
#include <mutex>
#include <condition_variable>

using std::vector;
using android::sp;

namespace mfll {
/**
 *  MfllYuvCapturer v1.2 is suitable for MFNR Core Library v2.5 only. The main behavior
 *  of this revision is:
 *      1. Only supports
 *          IMfllCapturer::captureFrames(
 *                              unsigned int,
 *                              std::vector< sp<IMfllImageBuffer>* >,
 *                              std::vector< sp<IMfllImageBuffer>* >,
 *                              std::vector< MfllMotionVector_t* >,
 *                              std::vector<int>
 *          )
 *          but return MfllErr_Ok immediately. We update full size YUV, resized Y8 and
 *          global motion vector by IMfllEventListener::onEvent.
 *
 *  Notice that, caller has responsibility to enque the following data (all are necessary):
 *      1. Full size source image, usually it's YUV domain.
 *      2. Resized Y8 (or multi plane YUV), the resolution must be 1/4 or 1/8 of source image
 *         (width divides 2 or 4 and height too)
 *      3. GMV (Global Motion Vector), if no, gives (0,0) is OK.
 */
class MfllYuvCapturer : public IMfllCapturer {
public:
    MfllYuvCapturer(void);
    virtual ~MfllYuvCapturer(void);


/* Implementations */
public:
    enum MfllErr    captureFrames(
                        unsigned int captureFrameNum,
                        std::vector< sp<IMfllImageBuffer>* > &raws,
                        std::vector< sp<IMfllImageBuffer>* > &rrzos,
                        std::vector<MfllMotionVector_t*>     &gmvs,
                        std::vector<int>                     &status
                    ) override;

    /* note: this method is not supported */
    enum MfllErr    captureFrames(
                        unsigned int                            captureFrameNum,
                        std::vector< sp<IMfllImageBuffer>*  >&  raws,
                        std::vector< sp<IMfllImageBuffer>* >&   rrzos,
                        std::vector< sp<IMfllImageBuffer>* >&   qyuvs,
                        std::vector< MfllMotionVector_t* >&     gmvs,
                        std::vector< int >&                     status
                    ) override;

    /* note: this method is not supported */
    enum MfllErr    captureFrames(
                        unsigned int                            captureFrameNum,
                        std::vector< sp<IMfllImageBuffer>* >&   raws,
                        std::vector< sp<IMfllImageBuffer>* >&   rrzos,
                        std::vector< sp<IMfllImageBuffer>* >&   yuvs,
                        std::vector< sp<IMfllImageBuffer>* >&   qyuvs,
                        std::vector< MfllMotionVector_t* >&     gmvs,
                        std::vector< int >&                     status
                    ) override;

    enum MfllErr    captureFrames(
                        unsigned int                            requestNo,
                        sp<IMfllImageBuffer>*                   yuvs,
                        sp<IMfllImageBuffer>*                   qyuvs,
                        MfllMotionVector_t*                     gmvs,
                        int&                                    status
                    ) override;

    unsigned int    getCapturedFrameNum() override;

    void            setMfllCore(IMfllCore* c) override;

    void            setShotMode(const enum MfllMode& mode) override;

    void            setPostNrType(const enum NoiseReductionType& type) override;

    enum MfllErr    registerEventDispatcher(const sp<IMfllEvents>& e) override;

    enum MfllErr    queueSourceImage(sp<IMfllImageBuffer>&img) override;

    enum MfllErr    queueSourceMonoImage(sp<IMfllImageBuffer>& img) override;

    enum MfllErr    queueGmv(const MfllMotionVector_t& mv) override;

    enum MfllErr    queueSourceLcsoImage(sp<IMfllImageBuffer>& img) override;

public:
    unsigned int                m_frameNum;
    enum MfllMode               m_captureMode; // indicates to capture mode.
    enum NoiseReductionType     m_postNrType;
    sp<IMfllEvents>             m_spEventDispatcher;
    IMfllCore*                  m_pCore;

    /* mutex/cond for source image (I422) */
    mutable std::mutex              m_mutexSrc;
    vector< sp<IMfllImageBuffer> >  m_vSrc;

    /* mutex/cond for quarter size Y8 */
    mutable std::mutex              m_mutexY8;
    vector< sp<IMfllImageBuffer> >  m_vY8;

    /* for GMV info */
    mutable std::mutex              m_mutexGmv;
    vector<MfllMotionVector_t>      m_vGmvs;


public:
    /* source image sync pack */
    struct SourceImageSyncPack
    {
        enum BufferIdx
        {
            FULLSIZE_SRC                =   0,
            RESIZED_Y8,
            GMV,
            //
            SIZE
        };
        volatile int                bReadyFlags[SIZE];
        std::mutex                  mx;
        std::condition_variable     cv;
        //
        inline bool isReadyLocked()
        {
            for (const auto& i : bReadyFlags)
                if (i == 0)
                    return false;
            return true;
        }
        inline bool isReady()
        {
            std::lock_guard<std::mutex> lk(mx);
            return isReadyLocked();
        }
        //
        inline void markReadyLock(int bufferIdx)
        {
            bReadyFlags[bufferIdx] = 1;
        }
        inline void markReady(int bufferIdx)
        {
            std::lock_guard<std::mutex> lk(mx);
            markReadyLock(bufferIdx);
        }
        //
        SourceImageSyncPack() : bReadyFlags{0}
        {
        }
    };

    SourceImageSyncPack             m_srcSyncs[10];

private:
    enum MfllErr    __captureFrameSingle(
                        unsigned int                            requestNo,
                        sp<IMfllImageBuffer>*                   yuvs,
                        sp<IMfllImageBuffer>*                   qyuvs,
                        MfllMotionVector_t*                     gmvs,
                        int&                                    status
                    );
    void dispatchOnEvent(const EventType &t, MfllEventStatus_t &s, void *p1, void *p2);
    void dispatchDoneEvent(const EventType &t, MfllEventStatus_t &s, void *p1, void *p2);

}; /* class MfllYuvCapturer */
}; /* namespace mfll */
#endif /* __MFLLYUVCAPTURER_H__ */
