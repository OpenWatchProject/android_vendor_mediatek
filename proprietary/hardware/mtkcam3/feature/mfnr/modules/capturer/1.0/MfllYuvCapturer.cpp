/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#define LOG_TAG "MfllCore/YuvCapturer"

#include "MfllYuvCapturer.h"

// MFNR Library
#include <mtkcam3/feature/mfnr/IMfllMfb.h>
#include <mtkcam3/feature/mfnr/MfllLog.h>

// AOSP
#include <cutils/compiler.h>

using namespace mfll;

MfllYuvCapturer::MfllYuvCapturer(void)
: m_frameNum(0)
, m_captureMode(static_cast<MfllMode>(0))
, m_postNrType(NoiseReductionType_None)
, m_pCore(NULL)
{
}


MfllYuvCapturer::~MfllYuvCapturer(void)
{
    mfllLogD("Delete MfllYuvCapturer");
}


enum MfllErr MfllYuvCapturer::captureFrames(
    unsigned int captureFrameNum,
    std::vector< sp<IMfllImageBuffer>* > &raws,
    std::vector< sp<IMfllImageBuffer>* > &rrzos,
    std::vector<MfllMotionVector_t*>     &gmvs,
    std::vector<int>                     &status
    )
{
    return MfllErr_NotSupported;
}


enum MfllErr MfllYuvCapturer::captureFrames(
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
            ____NULL,   // yuv
            qyuvs,      // qyuv
            gmvs,
            status
            );      // gmv
}


enum MfllErr MfllYuvCapturer::captureFrames(
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
            yuvs,
            qyuvs,
            gmvs,
            status
            );
}

enum MfllErr MfllYuvCapturer::captureFrames(
    unsigned int                requestNo,
    sp<IMfllImageBuffer>*       yuvs,
    sp<IMfllImageBuffer>*       qyuvs,
    MfllMotionVector_t*         gmvs,
    int&                        status
)
{
    return __captureFrameSingle(
            requestNo,
            yuvs,
            qyuvs,
            gmvs,
            status
            );
}

enum MfllErr MfllYuvCapturer::__captureFrames(
    unsigned int captureFrameNum,
    std::vector< sp<IMfllImageBuffer>* >&   /* raws */,
    std::vector< sp<IMfllImageBuffer>* >&   yuvs,
    std::vector< sp<IMfllImageBuffer>* >&   qyuvs,
    std::vector< MfllMotionVector_t* >&     gmvs,
    std::vector<int>&                       status
)
{
    enum MfllErr err = MfllErr_Ok;

    m_frameNum = captureFrameNum;

    /* is need full YUV */
    bool bFullYuv = yuvs.size() > 0 ? true : false;

    MfllEventStatus_t eventStatusRaw;
    MfllEventStatus_t eventStatusYuv;
    MfllEventStatus_t eventStatusEis;

    /* handle RAW to YUV frames first (and then, GMVs) */
    for (size_t i = 0; i < captureFrameNum; i++) {
        /* capture RAW, we wont have RAWs */
        sp<IMfllImageBuffer> r;
        dispatchOnEvent(EventType_CaptureRaw, eventStatusRaw, (void*)(long)i, NULL);
        // do nothing
        dispatchDoneEvent(EventType_CaptureRaw, eventStatusRaw, (void*)(long)i, NULL);

        /* capture YUV */
        dispatchOnEvent(EventType_CaptureYuvQ, eventStatusYuv, (void*)(long)i, NULL);
        /* no raw or ignored */
        if (CC_UNLIKELY(eventStatusYuv.ignore != 0)) { // Ignored
            eventStatusYuv.err = MfllErr_NullPointer;
            eventStatusYuv.ignore = 1;
            status[i] = 1;
        }
        else {
            if (bFullYuv) {
                // full size MC, wait both source image and Y8
                auto _imgsrc = dequeSourceImg();
                auto _imgy8 = dequeY8();
                (*yuvs[i])  = _imgsrc;
                (*qyuvs[i]) = _imgy8;
            }
            else {
                auto _imgsrc = dequeSourceImg();
                (*qyuvs[i]) = _imgsrc;
            }
        }
        dispatchDoneEvent(EventType_CaptureYuvQ, eventStatusYuv, (void*)(long)i, NULL);
    }

    /* GMV */
    for (size_t i = 0; i < captureFrameNum; i++) {
        /* GMV */
        dispatchOnEvent(EventType_CaptureEis, eventStatusEis, (void*)(long)i, NULL);
        MfllMotionVector_t mv;
        if (CC_LIKELY(eventStatusEis.ignore == 0)) {
            mv = dequeGmv();
            (*gmvs[i]) = mv;
        }
        else {
            eventStatusEis.err = MfllErr_NullPointer;
            eventStatusEis.ignore = 1;
            status[i] = 1;
        }
        dispatchDoneEvent(EventType_CaptureEis, eventStatusEis, (void*)(long)i, NULL);

    }

    mfllLogD("Capture Done");
    return MfllErr_Ok;
}

enum MfllErr MfllYuvCapturer::__captureFrameSingle(
    unsigned int requestNo,
    sp<IMfllImageBuffer>*                   yuvs,
    sp<IMfllImageBuffer>*                   qyuvs,
    MfllMotionVector_t*                     gmvs,
    int&                                    status

)
{
    enum MfllErr err = MfllErr_Ok;

    if (CC_UNLIKELY(requestNo < 0 || requestNo >= MFLL_MAX_FRAMES))
        return MfllErr_BadArgument;

    m_frameNum++;

    /* is need full YUV */
    bool bFullYuv = yuvs != nullptr ? true : false;

    MfllEventStatus_t eventStatusRaw;
    MfllEventStatus_t eventStatusYuv;
    MfllEventStatus_t eventStatusEis;

    /* handle RAW to YUV frames first (and then, GMVs) */
    {
        /* capture RAW, we wont have RAWs */
        sp<IMfllImageBuffer> r;
        dispatchOnEvent(EventType_CaptureRaw, eventStatusRaw, (void*)(long)requestNo, NULL);
        // do nothing
        dispatchDoneEvent(EventType_CaptureRaw, eventStatusRaw, (void*)(long)requestNo, NULL);

        /* capture YUV */
        dispatchOnEvent(EventType_CaptureYuvQ, eventStatusYuv, (void*)(long)requestNo, NULL);
        /* no raw or ignored */
        if (CC_UNLIKELY(eventStatusYuv.ignore != 0)) { // Ignored
            eventStatusYuv.err = MfllErr_NullPointer;
            eventStatusYuv.ignore = 1;
            status = 1;
        }
        else {
            if (bFullYuv) {
                // full size MC, wait both source image and Y8
                auto _imgsrc = dequeSourceImg();
                auto _imgy8 = dequeY8();
                (*yuvs)  = _imgsrc;
                (*qyuvs) = _imgy8;
            }
            else {
                auto _imgsrc = dequeSourceImg();
                (*qyuvs) = _imgsrc;
            }
        }
        dispatchDoneEvent(EventType_CaptureYuvQ, eventStatusYuv, (void*)(long)requestNo, NULL);
    }

    /* GMV */
    {
        /* GMV */
        dispatchOnEvent(EventType_CaptureEis, eventStatusEis, (void*)(long)requestNo, NULL);
        MfllMotionVector_t mv;
        if (CC_LIKELY(eventStatusEis.ignore == 0)) {
            mv = dequeGmv();
            (*gmvs) = mv;
        }
        else {
            eventStatusEis.err = MfllErr_NullPointer;
            eventStatusEis.ignore = 1;
            status = 1;
        }
        dispatchDoneEvent(EventType_CaptureEis, eventStatusEis, (void*)(long)requestNo, NULL);

    }

    mfllLogD("Capture Done");
    return MfllErr_Ok;
}

unsigned int MfllYuvCapturer::getCapturedFrameNum()
{
    return m_frameNum;
}


void MfllYuvCapturer::setMfllCore(IMfllCore *c)
{
    m_pCore = c;
}


void MfllYuvCapturer::setShotMode(const enum MfllMode &mode)
{
    m_captureMode = mode;
}


void MfllYuvCapturer::setPostNrType(const enum NoiseReductionType &type)
{
    m_postNrType = type;
}


enum MfllErr MfllYuvCapturer::registerEventDispatcher(const sp<IMfllEvents> &e)
{
    m_spEventDispatcher = e;
    return MfllErr_Ok;
}


enum MfllErr MfllYuvCapturer::queueSourceImage(sp<IMfllImageBuffer>& img)
{
    {
        std::lock_guard<std::mutex> __l(m_mutexSrc);
        m_vSrc.push_back(img);
        m_condSrc.notify_one();
    }
    return MfllErr_Ok;
}


enum MfllErr MfllYuvCapturer::queueSourceMonoImage(sp<IMfllImageBuffer>& img)
{
    {
        std::lock_guard<std::mutex> __l(m_mutexY8);
        m_vY8.push_back(img);
        m_condY8.notify_one();
    }
    return MfllErr_Ok;
}


enum MfllErr MfllYuvCapturer::queueGmv(const MfllMotionVector_t &mv)
{
    {
        std::lock_guard<std::mutex> __l(m_mutexGmv);
        m_vGmvs.push_back(mv);
        m_condGmv.notify_one();
    }
    return MfllErr_Ok;
}

enum MfllErr MfllYuvCapturer::queueSourceLcsoImage(sp<IMfllImageBuffer>& /* img */)
{
    return MfllErr_NotImplemented;
}

sp<IMfllImageBuffer> MfllYuvCapturer::dequeY8()
{
    sp<IMfllImageBuffer> img;
    {
        std::unique_lock<std::mutex> __l(m_mutexY8);
        while (m_vY8.size() <= 0)
            m_condY8.wait(__l);

        img = m_vY8.front();
        m_vY8.erase(m_vY8.begin());
    }
    return img;
}


sp<IMfllImageBuffer> MfllYuvCapturer::dequeSourceImg()
{
    sp<IMfllImageBuffer> img;
    {
        std::unique_lock<std::mutex> __l(m_mutexSrc);
        while (m_vSrc.size() <= 0)
            m_condSrc.wait(__l);

        img = m_vSrc.front();
        m_vSrc.erase(m_vSrc.begin());
    }
    return img;
}


MfllMotionVector_t MfllYuvCapturer::dequeGmv(void)
{
    MfllMotionVector_t mv;
    {
        std::unique_lock<std::mutex> __l(m_mutexGmv);

        // wait until there's a GMV data
        while (m_vGmvs.size() <= 0) {
            m_condGmv.wait(__l);
        }

        mv = m_vGmvs.front();
        m_vGmvs.erase(m_vGmvs.begin());
    }
    return mv;
}


void MfllYuvCapturer::dispatchOnEvent(const EventType &t, MfllEventStatus_t &s, void *p1, void *p2)
{
    if (CC_LIKELY(m_spEventDispatcher.get())) {
        m_spEventDispatcher->onEvent(t, s, (void*)m_pCore, p1, p2);
    }
}


void MfllYuvCapturer::dispatchDoneEvent(const EventType &t, MfllEventStatus_t &s, void *p1, void *p2)
{
    if (CC_LIKELY(m_spEventDispatcher.get())) {
        m_spEventDispatcher->doneEvent(t, s, (void*)m_pCore, p1, p2);
    }
}
