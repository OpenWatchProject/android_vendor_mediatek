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
#define LOG_TAG "MfllCore/Capturer"

#include "MfllCapturer.h"
#include <mtkcam3/feature/mfnr/IMfllMfb.h>
#include <mtkcam3/feature/mfnr/MfllLog.h>
#include <MfllCore.h>

// AOSP
#include <cutils/compiler.h> // CC_UNLIKELY, CC_LIKELY


using namespace mfll;


MfllCapturer::MfllCapturer(void)
: m_frameNum(0)
, m_captureMode(static_cast<MfllMode>(0))
, m_postNrType(NoiseReductionType_None)
, m_pCore(NULL)
, m_mutexRaw(PTHREAD_MUTEX_INITIALIZER)
, m_condRaw(PTHREAD_COND_INITIALIZER)
, m_mutexRrzo(PTHREAD_MUTEX_INITIALIZER)
, m_condRrzo(PTHREAD_COND_INITIALIZER)
, m_mutexGmv(PTHREAD_MUTEX_INITIALIZER)
, m_condGmv(PTHREAD_COND_INITIALIZER)
, m_mutexLcso(PTHREAD_MUTEX_INITIALIZER)
{
}

MfllCapturer::~MfllCapturer(void)
{
    mfllLogD3("Delete MfllCapturer");
}

// capture RAW only
enum MfllErr MfllCapturer::captureFrames(
    unsigned int captureFrameNum,
    std::vector< sp<IMfllImageBuffer>* > &raws,
    std::vector< sp<IMfllImageBuffer>* > &rrzos,
    std::vector<MfllMotionVector_t*>     &gmvs,
    std::vector<int>                     &status
    )
{
    std::vector< sp<IMfllImageBuffer>* > ____NULL;
    return __captureFrames(
            captureFrameNum,
            raws,       // raw
            rrzos,      // rrzo
            ____NULL,   // yuv
            ____NULL,   // qyuv
            gmvs,       // gmv
            status
            );
}

// capture RAW and YUV
enum MfllErr MfllCapturer::captureFrames(
    unsigned int captureFrameNum,
    std::vector< sp<IMfllImageBuffer>* > &raws,
    std::vector< sp<IMfllImageBuffer>* > &rrzos,
    std::vector< sp<IMfllImageBuffer>* > &qyuvs,
    std::vector<MfllMotionVector_t*>     &gmvs,
    std::vector<int>                     &status
    )
{
    std::vector< sp<IMfllImageBuffer>* > ____NULL;
    return __captureFrames(
            captureFrameNum,
            raws,       // raw
            rrzos,      // rrzo
            ____NULL,   // yuv
            qyuvs,      // qyuv
            gmvs,
            status
            );      // gmv
}

enum MfllErr MfllCapturer::captureFrames(
        unsigned int captureFrameNum,
        std::vector< sp<IMfllImageBuffer>* > &raws,
        std::vector< sp<IMfllImageBuffer>* > &rrzos,
        std::vector< sp<IMfllImageBuffer>* > &yuvs,
        std::vector< sp<IMfllImageBuffer>* > &qyuvs,
        std::vector< MfllMotionVector_t* >   &gmvs,
        std::vector<int>                     &status
)
{
    return __captureFrames(
            captureFrameNum,
            raws,
            rrzos,
            yuvs,
            qyuvs,
            gmvs,
            status
            );
}

enum MfllErr MfllCapturer::__captureFrames(
    unsigned int captureFrameNum,
    std::vector< sp<IMfllImageBuffer>* > &raws,
    std::vector< sp<IMfllImageBuffer>* > &rrzos,
    std::vector< sp<IMfllImageBuffer>* > &yuvs,
    std::vector< sp<IMfllImageBuffer>* > &qyuvs,
    std::vector< MfllMotionVector_t* >   &gmvs,
    std::vector<int>                     &status
)
{
    enum MfllErr err = MfllErr_Ok;

    m_frameNum = captureFrameNum;

    /* is need full YUV */
    bool bFullYuv = yuvs.size() > 0 ? true : false;

    MfllEventStatus_t eventStatusRaw;
    MfllEventStatus_t eventStatusRrzo;
    MfllEventStatus_t eventStatusYuv;
    MfllEventStatus_t eventStatusEis;


    /* handle RAW to YUV frames first (and then, GMVs) */
    for (size_t i = 0; i < captureFrameNum; i++) {
        /* capture RAW */
        sp<IMfllImageBuffer> r;
        sp<IMfllImageBuffer> _lcso;
        sp<IMfllImageBuffer> _rrzo;
        dispatchOnEvent(EventType_CaptureRaw, eventStatusRaw, (void*)(long)i, NULL);
        dispatchOnEvent(EventType_CaptureRrzo, eventStatusRrzo, (void*)(long)i, NULL);
        if (eventStatusRaw.ignore == 0) {
            r = dequeFullSizeRaw();
            _rrzo = dequeRrzoRaw();
            _lcso = dequeLcsoRaw();
        }
        (*raws[i]) = r;
        (*rrzos[i]) = _rrzo;
        dispatchDoneEvent(EventType_CaptureRaw, eventStatusRaw, (void*)(long)i, _lcso.get());
        dispatchDoneEvent(EventType_CaptureRrzo, eventStatusRrzo, (void*)(long)i, NULL);


        if ( yuvs.size() <= 0 && qyuvs.size() <= 0 ) {
            mfllLogD("Capture RAW buffers only, idx=%zu", i);
            if (r.get() == NULL || _rrzo.get() == NULL) {
                mfllLogE("deque no raw, rrzo buffer...");
                status[i] = 1;
            }
            else {
                mfllLogD3("deque raw, rrzo buffer");
                status[i] = 0;
            }
        }
        else {
            mfllLogI("Capture RAW and YUV buffers");

            /* get MFB */
            sp<IMfllMfb> pmfb = m_pCore->getMfb();
            if (pmfb.get() == NULL) {
                mfllLogE("%s: IMfllMfb instance is NULL", __FUNCTION__);
            }

            /* capture YUV */
            dispatchOnEvent(EventType_CaptureYuvQ, eventStatusYuv, (void*)(long)i, NULL);
            /* no raw or ignored */
            if (r.get() == NULL || eventStatusYuv.ignore != 0) { // No raw
                eventStatusYuv.err = MfllErr_NullPointer;
                eventStatusYuv.ignore = 1;
                status[i] = 1;
                dispatchDoneEvent(EventType_CaptureYuvQ, eventStatusYuv, (void*)(long)i, NULL);
            }
            else {
                err = reinterpret_cast<MfllCore*>(m_pCore)->doAllocQyuvBuffer((void*)(long)i);

                // check if buffers are ready to use
                bool bBuffersReady = (err == MfllErr_Ok)
                    // check if qyuvs[i] exists
                    && (qyuvs[i] && (*qyuvs[i]).get())
                    // check if yuvs[i] exists
                    && (bFullYuv ? ( yuvs[i] && (*yuvs[i]).get()) : true);

                if (CC_UNLIKELY( ! bBuffersReady )) {
                    mfllLogE("%s: alloc QYUV buffer %zu failed", __FUNCTION__, i);
                    eventStatusYuv.err = err;
                    eventStatusYuv.ignore = 1;
                    status[i] = 1;
                    dispatchDoneEvent(EventType_CaptureYuvQ, eventStatusYuv, (void*)(long)i, NULL);
                }
                else {
                    /* tell MfllMfb what shot mode and post NR type is using */
                    pmfb->setShotMode(m_captureMode);
                    pmfb->setPostNrType(m_postNrType);

                    if (bFullYuv) {
                        mfllLogD3("%s: RAW to 2 YUVs, %dx%d, %dx%d.", __FUNCTION__,
                                (*yuvs[i])->getWidth(), (*yuvs[i])->getHeight(),
                                (*qyuvs[i])->getWidth(), (*qyuvs[i])->getHeight());

                        /**
                         *  if we've already known that it's single capture, we don't need
                         *  stage BFBLD because we use stage SF to generate the output.
                         */
                        if ( m_pCore->getCaptureFrameNum() > 1) {
                            err = pmfb->encodeRawToYuv(
                                r.get(),
                                (*yuvs[i]).get(),
                                (*qyuvs[i]).get(),
                                m_pCore->retrieveBuffer(MfllBuffer_PostviewYuv).get(),
                                MfllRect_t(), // no need cropping
                                reinterpret_cast<MfllCore*>(m_pCore)->m_postviewCropRgn,
                                YuvStage_RawToYv16
                                );
                        }

                        if (err != MfllErr_Ok) {
                            status[i] = 1;
                        }
                        else
                            status[i] = 0;
                    }
                    else {
                        /**
                         *  if we've already known that it's single capture, we don't need
                         *  stage BFBLD because we use stage SF to generate the output.
                         */
                        if ( m_pCore->getCaptureFrameNum() > 1) {
                            err = pmfb->encodeRawToYuv(
                                r.get(),
                                (*qyuvs[i]).get(),
                                m_pCore->retrieveBuffer(MfllBuffer_PostviewYuv).get(),
                                MfllRect_t(),
                                reinterpret_cast<MfllCore*>(m_pCore)->m_postviewCropRgn,
                                YuvStage_RawToYv16);
                        }

                        if (err != MfllErr_Ok) {
                            status[i] = 1;
                        }
                        else
                            status[i] = 0;
                    }
                    eventStatusYuv.err = err;
                    dispatchDoneEvent(EventType_CaptureYuvQ, eventStatusYuv, (void*)(long)i, NULL);
                }
            }
        }
    }

    /* GMV */
    for (size_t i = 0; i < captureFrameNum; i++) {
        /* GMV */
        dispatchOnEvent(EventType_CaptureEis, eventStatusEis, (void*)(long)i, NULL);
        MfllMotionVector_t mv;
        if (eventStatusEis.ignore == 0) {
            mv = dequeGmv();
            (*gmvs[i]) = mv;
        }
        dispatchDoneEvent(EventType_CaptureEis, eventStatusEis, (void*)(long)i, NULL);

    }

    mfllLogD3("Capture Done");
    return MfllErr_Ok;
}

unsigned int MfllCapturer::getCapturedFrameNum(void)
{
    return m_frameNum;
}

void MfllCapturer::setMfllCore(IMfllCore *c)
{
    m_pCore = c;
}

void MfllCapturer::setShotMode(const enum MfllMode &mode)
{
    m_captureMode = mode;
}

void MfllCapturer::setPostNrType(const enum NoiseReductionType &type)
{
    m_postNrType = type;
}

enum MfllErr MfllCapturer::registerEventDispatcher(const sp<IMfllEvents> &e)
{
    m_spEventDispatcher = e;
    return MfllErr_Ok;
}

enum MfllErr MfllCapturer::queueSourceImage(sp<IMfllImageBuffer> &img)
{
    enum MfllErr err = MfllErr_Ok;
    pthread_mutex_t *mx = &m_mutexRaw;
    pthread_cond_t *cond = &m_condRaw;

    pthread_mutex_lock(mx);
    {
        m_vRaws.push_back(img);
        pthread_cond_signal(cond);
        mfllLogD3("imgo is available");
    }
    pthread_mutex_unlock(mx);
    return err;
}

enum MfllErr MfllCapturer::queueSourceMonoImage(sp<IMfllImageBuffer>& /* img */)
{
    return MfllErr_NotImplemented;
}

sp<IMfllImageBuffer> MfllCapturer::dequeFullSizeRaw(void)
{
    sp<IMfllImageBuffer> img;
    pthread_mutex_t *mx = &m_mutexRaw;
    pthread_cond_t *cond = &m_condRaw;

    pthread_mutex_lock(mx); // wait if necessary
    {
        /* check buffer is available or not */
        if (m_vRaws.size() <= 0) {
            mfllLogD3("imgo is not available, wait until signal");
            pthread_cond_wait(cond, mx);
            mfllLogD3("imgo is available, got signal");
        }
        img = m_vRaws[0];
        m_vRaws.erase(m_vRaws.begin());
    }
    pthread_mutex_unlock(mx);
    return img;
}

enum MfllErr MfllCapturer::queueGmv(const MfllMotionVector_t &mv)
{
    enum MfllErr err = MfllErr_Ok;
    pthread_mutex_t *mx = &m_mutexGmv;
    pthread_cond_t *cond = &m_condGmv;

    pthread_mutex_lock(mx);
    {
        m_vGmvs.push_back(mv);
        pthread_cond_signal(cond);
    }
    pthread_mutex_unlock(mx);
    return err;
}

enum MfllErr MfllCapturer::queueSourceLcsoImage(sp<IMfllImageBuffer>& img)
{
    enum MfllErr err = MfllErr_Ok;
    pthread_mutex_t *mx = &m_mutexLcso;

    pthread_mutex_lock(mx);
    {
        m_vLcso.push_back(img);
    }
    pthread_mutex_unlock(mx);
    return err;
}

enum MfllErr MfllCapturer::queueSourceRrzoImage(sp<IMfllImageBuffer>& img)
{
    enum MfllErr err = MfllErr_Ok;
    pthread_mutex_t *mx = &m_mutexRrzo;
    pthread_cond_t *cond = &m_condRrzo;

    pthread_mutex_lock(mx);
    {
        m_vRrzo.push_back(img);
        pthread_cond_signal(cond);
        mfllLogD3("rrzo is available");
    }
    pthread_mutex_unlock(mx);
    return err;
}

MfllMotionVector_t MfllCapturer::dequeGmv(void)
{
    MfllMotionVector_t mv;
    pthread_mutex_t *mx = &m_mutexGmv;
    pthread_cond_t *cond = &m_condGmv;

    pthread_mutex_lock(mx);
    {
        if (m_vGmvs.size() <= 0) {
            pthread_cond_wait(cond, mx);
        }

        mv = m_vGmvs[0];
        m_vGmvs.erase(m_vGmvs.begin());
    }
    pthread_mutex_unlock(mx);
    return mv;
}

sp<IMfllImageBuffer> MfllCapturer::dequeLcsoRaw(void)
{
    sp<IMfllImageBuffer> img;
    pthread_mutex_t *mx = &m_mutexLcso;

    pthread_mutex_lock(mx);
    {
        if (m_vLcso.size() <= 0) {
            // do nothing
        }
        else {
            img = m_vLcso[0];
            m_vLcso.erase(m_vLcso.begin());
        }
    }
    pthread_mutex_unlock(mx);
    return img;
}

sp<IMfllImageBuffer> MfllCapturer::dequeRrzoRaw(void)
{
    sp<IMfllImageBuffer> img;
    pthread_mutex_t *mx = &m_mutexRrzo;
    pthread_cond_t *cond = &m_condRrzo;

    pthread_mutex_lock(mx);
    {
        /* check rrzo is available or not */
        if (m_vRrzo.size() <= 0) {
            mfllLogD3("rrzo is not available, wait until signal");
            pthread_cond_wait(cond, mx);
            mfllLogD3("rrzo is available, got signal");
        }
        img = m_vRrzo[0];
        m_vRrzo.erase(m_vRrzo.begin());
    }
    pthread_mutex_unlock(mx);
    return img;
}

void MfllCapturer::dispatchOnEvent(const EventType &t, MfllEventStatus_t &s, void *p1, void *p2)
{
    if (m_spEventDispatcher.get()) {
        m_spEventDispatcher->onEvent(t, s, (void*)m_pCore, p1, p2);
    }
}

void MfllCapturer::dispatchDoneEvent(const EventType &t, MfllEventStatus_t &s, void *p1, void *p2)
{
    if (m_spEventDispatcher.get()) {
        m_spEventDispatcher->doneEvent(t, s, (void*)m_pCore, p1, p2);
    }
}

