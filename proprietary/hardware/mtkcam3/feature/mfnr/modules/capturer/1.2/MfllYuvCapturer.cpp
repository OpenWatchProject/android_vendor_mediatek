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
#include <MfllCore.h>

// AOSP
#include <cutils/compiler.h>

// STL
#include <chrono>

using namespace mfll;

// EventListener
namespace MfllYuvCapturer_1_2_Impl
{
class MyEventListener : public mfll::IMfllEventListener
{
public:
    MyEventListener(MfllYuvCapturer* pCapturer) : m_pCapturer(pCapturer)
    {
    }

public:
    virtual void onEvent(enum EventType t, MfllEventStatus_t& status, void* mfllCore, void* param1, void* param2)
    {
        int index = (int)(long)param1;
        int isYuvInput = (int)(long)param2;
        switch (t){
        case EventType_CaptureYuvQ:
            mfllLogD3("MfllYuvCapturer_1_2_Impl EventType_CaptureYuvQ(%d, %d)", index, isYuvInput);
            if (isYuvInput)
                onCaptureYuvQ(status, static_cast<MfllCore*>(mfllCore), index);
            break;

        default:break;
        }
    }

    virtual void doneEvent(enum EventType /*t*/, MfllEventStatus_t& /*status*/, void* /*mfllCore*/, void* /*param1 = NULL*/, void* /*param2 = NULL*/)
    {
    }

    virtual vector<enum EventType> getListenedEventTypes()
    {
        std::vector<EventType> __l =
        {
            EventType_CaptureYuvQ
        };
        return __l;
    }

public:
    void onCaptureYuvQ(MfllEventStatus_t& status, MfllCore* c, int index)
    {
        typedef mfll::MfllYuvCapturer::SourceImageSyncPack      SourceImageSyncPack;
        constexpr const int FULLSIZE_SRC = SourceImageSyncPack::FULLSIZE_SRC;
        constexpr const int RESIZED_Y8   = SourceImageSyncPack::RESIZED_Y8;
        constexpr const int GMV          = SourceImageSyncPack::GMV;

        if (status.ignore || status.err != MfllErr_Ok) {
            mfllLogW("ignore onCaptureYuvQ (ignore=%d,err=%d)", status.ignore, status.err);
            return;
        }

        /* wait sync pack ready */
        bool bOkStatus = true;
        SourceImageSyncPack& pack = m_pCapturer->m_srcSyncs[index];
        {
            std::string __log =
                std::string("wait for source pack(") + std::to_string(index) +
                std::string(")...");

            std::unique_lock<std::mutex> lk(pack.mx);

            while (true) {
                mfllAutoLog(__log.c_str());
                if ( pack.isReadyLocked() )
                    break;

                mfllLogD3("waiting, status(fullSrc,resizedY8,GMV)=(%d,%d,%d)",
                        pack.bReadyFlags[FULLSIZE_SRC],
                        pack.bReadyFlags[RESIZED_Y8],
                        pack.bReadyFlags[GMV]
                        );

                auto r = pack.cv.wait_for(lk, std::chrono::seconds(10));
                if ( r == std::cv_status::timeout ) {
                    mfllLogE("wait source pack(%d) timed out", index);
                    bOkStatus = false;
                    break;
                }
            };
        }

        if ( ! bOkStatus ) {
            mfllLogE("wait source pack failed, ignore this stage");
            status.ignore = true;
            status.err = MfllErr_NullPointer;
            return;
        }

        /* OK, update images */
        mfllLogD3("%s: index=%d", __FUNCTION__, index);
        {
            std::lock_guard<std::mutex> lk(m_pCapturer->m_mutexSrc);
            c->m_imgYuvs[index].setImgBufLocked(m_pCapturer->m_vSrc[index]);
            m_pCapturer->m_vSrc[index] = nullptr; // clear

            mfllLogD3("%s: fullYuv addr=%p", __FUNCTION__, c->m_imgYuvs[index].imgbuf.get());
            mfllLogD3("%s: size=%dx%d, format=%d(mfll)",
                    __FUNCTION__,
                    c->m_imgYuvs[index].imgbuf->getWidth(), c->m_imgYuvs[index].imgbuf->getHeight(),
                    c->m_imgYuvs[index].imgbuf->getImageFormat()
                    );
        }
        {
            std::lock_guard<std::mutex> lk(m_pCapturer->m_mutexY8);
            c->m_imgQYuvs[index].setImgBufLocked(m_pCapturer->m_vY8[index]);
            m_pCapturer->m_vY8[index] = nullptr;

            mfllLogD3("%s: resizedY8 addr=%p", __FUNCTION__, c->m_imgQYuvs[index].imgbuf.get());
            mfllLogD3("%s: size=%dx%d, format=%d(mfll)",
                    __FUNCTION__,
                    c->m_imgQYuvs[index].imgbuf->getWidth(), c->m_imgQYuvs[index].imgbuf->getHeight(),
                    c->m_imgQYuvs[index].imgbuf->getImageFormat()
                    );
        }
        {
            std::lock_guard<std::mutex> lk(m_pCapturer->m_mutexGmv);
            c->m_globalMv[index] = m_pCapturer->m_vGmvs[index];

            mfllLogD3("%s: gmv=%d,%d", __FUNCTION__, c->m_globalMv[index].x, c->m_globalMv[index].y);
        }

        status.ignore = 1; // indicate to ignore the implementation of MfllCore.
        status.err = MfllErr_Ok;
        m_pCapturer->m_frameNum++;
        return;
    }

private:
    MfllYuvCapturer* m_pCapturer;

}; // class MyEventListener
}; // namespace MfllYuvCapturer_1_2_Impl

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
    unsigned int                            captureFrameNum     __attribute__((unused)),
    std::vector< sp<IMfllImageBuffer>* >    &raws               __attribute__((unused)),
    std::vector< sp<IMfllImageBuffer>* >    &rrzos              __attribute__((unused)),
    std::vector<MfllMotionVector_t*>        &gmvs               __attribute__((unused)),
    std::vector<int>                        &status             __attribute__((unused))
    )
{
    /* OK! We wait buffer at MyEventListener::onEvent */
    return MfllErr_Ok;
}


enum MfllErr MfllYuvCapturer::captureFrames(
    unsigned int                            captureFrameNum     __attribute__((unused)),
    std::vector< sp<IMfllImageBuffer>* >    &raws               __attribute__((unused)),
    std::vector< sp<IMfllImageBuffer>* >    &rrzos              __attribute__((unused)),
    std::vector< sp<IMfllImageBuffer>* >    &qyuvs              __attribute__((unused)),
    std::vector<MfllMotionVector_t*>        &gmvs               __attribute__((unused)),
    std::vector<int>                        &status             __attribute__((unused))
    )
{
    return MfllErr_NotImplemented;
}


enum MfllErr MfllYuvCapturer::captureFrames(
        unsigned int                            captureFrameNum __attribute__((unused)),
        std::vector< sp<IMfllImageBuffer>* >    &raws           __attribute__((unused)),
        std::vector< sp<IMfllImageBuffer>* >    &rrzos          __attribute__((unused)),
        std::vector< sp<IMfllImageBuffer>* >    &yuvs           __attribute__((unused)),
        std::vector< sp<IMfllImageBuffer>* >    &qyuvs          __attribute__((unused)),
        std::vector< MfllMotionVector_t* >      &gmvs           __attribute__((unused)),
        std::vector<int>                        &status         __attribute__((unused))
)
{
    return MfllErr_NotImplemented;
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

        static int isYuvInput = 1;

        /* capture YUV */
        dispatchOnEvent(EventType_CaptureYuvQ, eventStatusYuv, (void*)(long)requestNo, (void*)(long)isYuvInput);
        /* no raw or ignored */
        if (CC_UNLIKELY(eventStatusYuv.ignore != 0)) { // Ignored
            eventStatusYuv.err = MfllErr_NullPointer;
            eventStatusYuv.ignore = 1;
            status = 1;
        }
        dispatchDoneEvent(EventType_CaptureYuvQ, eventStatusYuv, (void*)(long)requestNo, (void*)(long)isYuvInput);
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
    bool bNeedRegisterEventListener = (m_pCore == nullptr);

    m_pCore = c;

    if (bNeedRegisterEventListener) {
        /* register event listener */
        sp<IMfllEventListener> listener(
                new MfllYuvCapturer_1_2_Impl::MyEventListener(this)
                );

        static_cast<MfllCore*>(c)->registerEventListenerNoLock(listener);
    }
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
    size_t cnt = 0;
    /* push_back element */
    {
        std::lock_guard<std::mutex> __l(m_mutexSrc);
        cnt = m_vSrc.size();
        m_vSrc.push_back(img);
    }

    /* mark ready and notify condition_variable */
    {
        std::lock_guard<std::mutex> __l(m_srcSyncs[cnt].mx);
        m_srcSyncs[cnt].markReadyLock(SourceImageSyncPack::FULLSIZE_SRC);
        m_srcSyncs[cnt].cv.notify_one();
    }

    return MfllErr_Ok;
}


enum MfllErr MfllYuvCapturer::queueSourceMonoImage(sp<IMfllImageBuffer>& img)
{
    size_t cnt = 0;
    /* push_back element */
    {
        std::lock_guard<std::mutex> __l(m_mutexY8);
        cnt = m_vY8.size();
        m_vY8.push_back(img);
    }

    /* mark ready and notify condition_variable */
    {
        std::lock_guard<std::mutex> __l(m_srcSyncs[cnt].mx);
        m_srcSyncs[cnt].markReadyLock(SourceImageSyncPack::RESIZED_Y8);
        m_srcSyncs[cnt].cv.notify_one();
    }

    return MfllErr_Ok;
}


enum MfllErr MfllYuvCapturer::queueGmv(const MfllMotionVector_t &mv)
{
    size_t cnt = 0;
    /* push_back element */
    {
        std::lock_guard<std::mutex> __l(m_mutexGmv);
        cnt = m_vGmvs.size();
        m_vGmvs.push_back(mv);
    }

    /* mark ready and notify condition_variable */
    {
        std::lock_guard<std::mutex> __l(m_srcSyncs[cnt].mx);
        m_srcSyncs[cnt].markReadyLock(SourceImageSyncPack::GMV);
        m_srcSyncs[cnt].cv.notify_one();
    }

    return MfllErr_Ok;
}

enum MfllErr MfllYuvCapturer::queueSourceLcsoImage(sp<IMfllImageBuffer>& /* img */)
{
    mfllLogW("%s is not implemented yet", __FUNCTION__);
    return MfllErr_NotImplemented;
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
