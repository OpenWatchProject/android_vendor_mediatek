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

// MFNR Library
#include <mtkcam3/feature/mfnr/IMfllCore.h>
#include <mtkcam3/feature/mfnr/IMfllCapturer.h>
#include <mtkcam3/feature/mfnr/IMfllImageBuffer.h>

// STL
#include <vector>
#include <mutex>
#include <condition_variable>

using std::vector;
using android::sp;

namespace mfll {

class MfllYuvCapturer : public IMfllCapturer {
public:
    MfllYuvCapturer(void);
    virtual ~MfllYuvCapturer(void);


/* Implementations */
public:
    enum MfllErr    captureFrames(
                        unsigned int                            captureFrameNum,
                        std::vector< sp<IMfllImageBuffer>*  >&  raws,
                        std::vector< sp<IMfllImageBuffer>* >&   rrzos,
                        std::vector< MfllMotionVector_t* >&     gmvs,
                        std::vector< int >&                     status
                    ) override;

    enum MfllErr    captureFrames(
                        unsigned int                            captureFrameNum,
                        std::vector< sp<IMfllImageBuffer>*  >&  raws,
                        std::vector< sp<IMfllImageBuffer>* >&   rrzos,
                        std::vector< sp<IMfllImageBuffer>* >&   qyuvs,
                        std::vector< MfllMotionVector_t* >&     gmvs,
                        std::vector< int >&                     status
                    ) override;

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
                        sp<IMfllImageBuffer>*                   raws,
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

/* self Implementations */
private:
    /* real captureFrames Implementations */
    enum MfllErr    __captureFrames(
                        unsigned int                            captureFrameNum,
                        std::vector< sp<IMfllImageBuffer>* >&   raws,
                        std::vector< sp<IMfllImageBuffer>* >&   yuvs,
                        std::vector< sp<IMfllImageBuffer>* >&   qyuvs,
                        std::vector< MfllMotionVector_t* >&     gmvs,
                        std::vector< int >&                     status
                    );
    enum MfllErr    __captureFrameSingle(
                        unsigned int                            requestNo,
                        sp<IMfllImageBuffer>*                   raws,
                        sp<IMfllImageBuffer>*                   qyuvs,
                        MfllMotionVector_t*                     gmvs,
                        int&                                    status
                    );

    sp<IMfllImageBuffer>    dequeSourceImg();
    sp<IMfllImageBuffer>    dequeQyuv();
    MfllMotionVector_t      dequeGmv();


private:
    unsigned int                m_frameNum;
    enum MfllMode               m_captureMode; // indicates to capture mode.
    enum NoiseReductionType     m_postNrType;
    sp<IMfllEvents>             m_spEventDispatcher;
    IMfllCore*                  m_pCore;

    /* mutex/cond for source image (I422) */
    mutable std::mutex              m_mutexSrc;
    mutable std::condition_variable m_condSrc;
    vector< sp<IMfllImageBuffer> >  m_vSrc;

    /* mutex/cond for quarter size qyuv */
    mutable std::mutex              m_mutexQyuv;
    mutable std::condition_variable m_condQyuv;
    vector< sp<IMfllImageBuffer> >  m_vQyuv;

    /* for GMV info */
    mutable std::mutex              m_mutexGmv;
    mutable std::condition_variable m_condGmv;
    vector<MfllMotionVector_t>      m_vGmvs;

private:
    void dispatchOnEvent(const EventType &t, MfllEventStatus_t &s, void *p1, void *p2);
    void dispatchDoneEvent(const EventType &t, MfllEventStatus_t &s, void *p1, void *p2);

}; /* class MfllYuvCapturer */
}; /* namespace mfll */
#endif /* __MFLLYUVCAPTURER_H__ */
