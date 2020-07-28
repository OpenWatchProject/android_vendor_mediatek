/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2015-2016. All rights reserved.
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
#define LOG_TAG "MfllCore"

#include "MfllCore.h"
#include <mtkcam3/feature/mfnr/MfllLog.h>
#include "MfllOperationSync.h"
#include <MfllUtilities.h>

// AOSP
#include <cutils/compiler.h>

// STL
#include <vector> // std::vector
#include <deque> // std::deque
#include <map> // std::map
#include <string> // std::string
#include <future> // std::async, std::launch
#include <fstream> // std::ifstream
#include <regex> // std::regex, std::sregex_token_iterator
#include <iterator> // std::back_inserter

#include <stdio.h>

using android::sp;
using android::Mutex;
using std::vector;
using std::map;
using std::string;
using namespace mfll;

/* if MFLL_LOG is 1, give IMfllImageBuffers with name for debug usage */
#if MFLL_LOG
#define BUFFER_NAME(name, i)      (string(name) + to_char(i)).c_str()
#else
#define BUFFER_NAME(name, i)
#endif

constexpr int32_t ver_major      = 1;
constexpr int32_t ver_minor      = 6;
constexpr int32_t ver_featured   = static_cast<int32_t>(IMfllCore::Type::DEFAULT);

//-----------------------------------------------------------------------------

template <class T>
static void swap(T &v1, T &v2)
{
    T temp = v1;
    v1 = v2;
    v2 = temp;
}

//-----------------------------------------------------------------------------

inline char to_char(int i)
{
    char c = '0';
    return (c + i);
}

static map<enum EventType, const char*> EVENTTYPE_NAME_INITIALIZER(void)
{
    static std::map<enum EventType, const char*> m;
#define EVENTTYPE_NAME_ADD_ELEMENT(x) m[x] = #x
    EVENTTYPE_NAME_ADD_ELEMENT(EventType_Init);
    EVENTTYPE_NAME_ADD_ELEMENT(EventType_AllocateRawBuffer);
    EVENTTYPE_NAME_ADD_ELEMENT(EventType_AllocateQyuvBuffer);
    EVENTTYPE_NAME_ADD_ELEMENT(EventType_AllocateYuvGolden);
    EVENTTYPE_NAME_ADD_ELEMENT(EventType_AllocateYuvWorking);
    EVENTTYPE_NAME_ADD_ELEMENT(EventType_AllocateYuvMixing);
    EVENTTYPE_NAME_ADD_ELEMENT(EventType_AllocateWeighting);
    EVENTTYPE_NAME_ADD_ELEMENT(EventType_AllocateMemc);
    EVENTTYPE_NAME_ADD_ELEMENT(EventType_Capture);
    EVENTTYPE_NAME_ADD_ELEMENT(EventType_CaptureRaw);
    EVENTTYPE_NAME_ADD_ELEMENT(EventType_CaptureYuvQ);
    EVENTTYPE_NAME_ADD_ELEMENT(EventType_CaptureEis);
    EVENTTYPE_NAME_ADD_ELEMENT(EventType_Bss);
    EVENTTYPE_NAME_ADD_ELEMENT(EventType_EncodeYuvBase);
    EVENTTYPE_NAME_ADD_ELEMENT(EventType_EncodeYuvGolden);
    EVENTTYPE_NAME_ADD_ELEMENT(EventType_MotionEstimation);
    EVENTTYPE_NAME_ADD_ELEMENT(EventType_MotionCompensation);
    EVENTTYPE_NAME_ADD_ELEMENT(EventType_Blending);
    EVENTTYPE_NAME_ADD_ELEMENT(EventType_Mixing);
    EVENTTYPE_NAME_ADD_ELEMENT(EventType_Destroy);
#undef EVENTTYPE_NAME_ADD_ELEMENT
    return m;
}
static map<enum EventType, const char*> EVENTTYPE_NAME = EVENTTYPE_NAME_INITIALIZER();


/******************************************************************************
 * M F L L
 *****************************************************************************/


const char* IMfllCore::getEventTypeName(enum EventType e)
{
    unsigned int index = (unsigned int)e;
    if (e < 0 || e >= (unsigned int)EventType_Size)
        return "Unknown_EventType";
    return EVENTTYPE_NAME[e];
}

void IMfllCore::destroyInstance(void)
{
    decStrong((void*)this);
}

/**
 *  MfllCore
 */
MfllCore::MfllCore(void)
    : m_middlewareInfo(MfllMiddlewareInfo_t())
    , m_mixYuvBufferType(MixYuvType_Unknow)
{
    mfllFunctionIn();

    /* modules */
    m_spCapturer = NULL;
    /* plug-in */
    m_spProperty = NULL;
    m_spDump = NULL;
    m_spExifInfo = IMfllExifInfo::createInstance();
    /* event dispatcher */
    m_event = IMfllEvents::createInstance();
    /* attributes */
    m_frameBlend = MFLL_BLEND_FRAME;
    m_frameCapture = MFLL_CAPTURE_FRAME;
    m_memcInstanceNum = 1;
    m_bShooted = false;
    m_sensorId = -1;
    m_iso = -1;
    m_exposure = -1;
    m_mutexShoot = PTHREAD_MUTEX_INITIALIZER;
    m_isFullSizeMc = MFLL_FULL_SIZE_MC;
    m_byPassGmv = false;
    m_postNrType = NoiseReductionType_None;
    m_caputredCount = 0;
    m_blendedCount = 0;
    m_bAsSingleFrame = 0;

    /* pointers of IMfllImageBuffers */
    m_ptrImgYuvBase = NULL;
    m_ptrImgYuvRef = NULL;
    m_ptrImgYuvGolden = NULL;
    m_ptrImgYuvBlended = NULL;
    m_ptrImgYuvMixed = NULL;
    m_ptrImgYuvThumbnail = NULL;
    m_ptrImgYuvMixedWorking = NULL;
    m_ptrImgWeightingIn = NULL;
    m_ptrImgWeightingOut = NULL;
    m_ptrImgWeightingFinal = NULL;


    /* sync objects, lock as default */
    lockSyncObject(&m_syncCapture);
    lockSyncObject(&m_syncEncodeYuvBase);
    lockSyncObject(&m_syncEncodeYuvGolden);
    lockSyncObject(&m_syncBss);
    lockSyncObject(&m_syncMixing);

    for (unsigned int i = 0; i < MFLL_MAX_FRAMES; i++) {
        m_bssIndex[i] = i;
        /* sync object for operation */
        lockSyncObject(&m_syncCapturedRaw[i]);
        lockSyncObject(&m_syncCapturedYuvQ[i]);
        lockSyncObject(&m_syncQueueYuvCapture[i]);
        lockSyncObject(&m_syncMotionEstimation[i]);
        lockSyncObject(&m_syncMotionCompensation[i]);
        lockSyncObject(&m_syncBlending[i]);
    }

    /* always uses performance service plug-in */
    m_spPerfService = MfllFeaturePerf::createInstance();
    /* always uses property plug-in */
    m_spProperty = new MfllProperty;

    mfllFunctionOut();
}

MfllCore::~MfllCore(void)
{
    MfllErr err;
    int iResult;
    MfllEventStatus_t status;

    mfllFunctionIn();
    m_event->onEvent(EventType_Destroy, status, (void*)this);

    m_event->doneEvent(EventType_Destroy, status, (void*)this);
    mfllFunctionOut();
}

enum MfllErr MfllCore::init(const MfllConfig_t &cfg)
{
    MfllErr err = MfllErr_Ok;
    MfllEventStatus_t status;

    mfllFunctionIn();
    mfllTraceCall();

    pthread_mutex_lock(&m_mutexShoot);
    if (m_bShooted) {
        err = MfllErr_Shooted;
        goto lbExit;
    }

    /* check parameter is valid or not */
    if (CC_UNLIKELY(cfg.blend_num > cfg.capture_num)) {
        mfllLogE("frame blend number is greater than capture");
        pthread_mutex_unlock(&m_mutexShoot);
        return MfllErr_BadArgument;
    }

    if (CC_UNLIKELY(cfg.capture_num <= 0)) {
        mfllLogE("frame capture number must greater than 0");
        pthread_mutex_unlock(&m_mutexShoot);
        return MfllErr_BadArgument;
    }

    m_event->onEvent(EventType_Init, status, (void*)this);

    err = do_Init(cfg);

lbExit:
    pthread_mutex_unlock(&m_mutexShoot);

    status.err = err;
    m_event->doneEvent(EventType_Init, status, (void*)this);

    if (CC_UNLIKELY(err == MfllErr_Shooted))
        mfllLogW("%s: MFLL has shooted, cannot init MFLL anymore", __FUNCTION__);

    mfllFunctionOut();
    return err;
}

enum MfllErr MfllCore::doMfll()
{
    MfllErr err = MfllErr_Ok;
    pthread_t pThread;
    pthread_t pThreadPhase1;
    void *ret;
    int registerMrpFeature = 0;

    registerMrpFeature = (m_mrpMode == MrpMode_BestPerformance ? 0 : 1);

    mfllFunctionIn();

    /* use mutex to protect operating shoot state */
    pthread_mutex_lock(&m_mutexShoot);
    if (CC_UNLIKELY(m_bShooted)) {
        pthread_mutex_unlock(&m_mutexShoot);
        mfllLogW("MFLL has been shooted, cancel this shoot");
        err = MfllErr_Shooted;
        goto lbExit;
    }
    else {
        m_bShooted = true;
        pthread_mutex_unlock(&m_mutexShoot);
    }

    /**
     *  M F L L    S T A R T
     *
     *
     *  R E G I S T E R    E V E N T    L I S T E N E R S
     */
    /* register Memory Reduce Plan feature */
    if (registerMrpFeature) {
        mfllLogD("Register plug-in Mrp");
        registerEventListenerNoLock(new MfllFeatureMrp, 1); // insert the event listener at the first
    }

    /* if any dump is enable, install feature dump */
    if (CC_UNLIKELY(m_spProperty->isDump())) {
        mfllLogD("Register plug-in Dump");
        MfllFeatureDump::DumpFlag f;
        f.raw = m_spProperty->isDumpRaw()           ? 1 : 0;
        f.yuv = m_spProperty->isDumpYuv()           ? 1 : 0;
        f.mfb = m_spProperty->isDumpMfb()           ? 1 : 0;
        f.mix = m_spProperty->isDumpMix()           ? 1 : 0;
        f.jpeg = m_spProperty->isDumpJpeg()         ? 1 : 0;
        f.postview = m_spProperty->isDumpPostview() ? 1 : 0;
        f.exif = m_spProperty->isDumpExif()         ? 1 : 0;
        m_spDump = new MfllFeatureDump;
        m_spDump->setDumpFlag(f);

        /* configure dump base on property */
        registerEventListenerNoLock(m_spDump);
    }

    /* if any feed is enable, install feature feed */
    if (CC_UNLIKELY(m_spProperty->isFeed())) {
        mfllLogD("Register plug-in Feed");
        MfllFeatureFeed::FeedFlag f;
        f.bfbld = m_spProperty->isFeedBfbld()       ? 1 : 0;
        f.bss   = m_spProperty->isFeedBss()         ? 1 : 0;
        f.memc  = m_spProperty->isFeedMemc()        ? 1 : 0;
        f.mfb   = m_spProperty->isFeedMfb()         ? 1 : 0;
        f.mix   = m_spProperty->isFeedMix()         ? 1 : 0;
        f.jpeg  = m_spProperty->isFeedJpeg()        ? 1 : 0;
        m_spFeed = new MfllFeatureFeed;
        m_spFeed->setFeedFlag(f);

        /* configure feed base on property */
        registerEventListenerNoLock(m_spFeed);
    }

    /**
     * Register perfservice plug-in at the end of listener stack because it needs
     * to check the ignore status
     */
    registerEventListenerNoLock(m_spPerfService);

    /* print out configuration */
    {
        mfllLogD("=========  M    F    L    L ============");
        mfllLogD("sensor id         = %d", m_sensorId);
        mfllLogD("capture num       = %d", m_frameCapture);
        mfllLogD("blend-frame       = %d", m_frameBlend.load());
        mfllLogD("full size         = %dx%d", m_width, m_height);
        mfllLogD("quater size       = %dx%d", m_qwidth, m_qheight);
        mfllLogD("full size MC      = %d", m_isFullSizeMc);
        mfllLogD("is zsd shot       = %s", isZsdMode(m_shotMode) ? "Yes" : "No");
        mfllLogD("iso               = %d", m_iso);
        mfllLogD("exposure          = %d", m_exposure);
        mfllLogD("memc mode         = %s", m_memcMode == MemcMode_Sequential ? "sequential" : "parallel");
        mfllLogD("memc threads      = %d", m_memcInstanceNum);
        mfllLogD("rwb mode          = %d", m_rwbMode);
        mfllLogD("feature dump      = %s", m_spProperty->isDump() ? "Yes" : "No");
        if (m_spProperty->isDump()) {
        mfllLogD("feature dump.raw  = %s", m_spProperty->isDumpRaw() ? "Yes" : "No");
        mfllLogD("feature dump.yuv  = %s", m_spProperty->isDumpYuv() ? "Yes" : "No");
        mfllLogD("feature dump.mfb  = %s", m_spProperty->isDumpMfb() ? "Yes" : "No");
        mfllLogD("feature dump.mix  = %s", m_spProperty->isDumpMix() ? "Yes" : "No");
        mfllLogD("feature dump.jpeg = %s", m_spProperty->isDumpJpeg() ? "Yes" : "No");
        mfllLogD("feature dump.pv   = %s", m_spProperty->isDumpPostview() ? "Yes" : "No");
        mfllLogD("feature dump.exif = %s", m_spProperty->isDumpExif() ? "Yes" : "No");
        }
        //
        mfllLogD("feature feed      = %s", m_spProperty->isFeed() ? "Yes" : "No");
        //
        if (CC_UNLIKELY( m_spProperty->isFeed() )) {
        mfllLogD("feature feed.bfbld= %s", m_spProperty->isFeedBfbld() ? "Yes" : "No");
        mfllLogD("feature feed.bss  = %s", m_spProperty->isFeedBss() ? "Yes" : "No");
        mfllLogD("feature feed.memc = %s", m_spProperty->isFeedMemc() ? "Yes" : "No");
        mfllLogD("feature feed.sf   = %s", m_spProperty->isFeedSf() ? "Yes" : "No");
        mfllLogD("feature feed.mfb  = %s", m_spProperty->isFeedMfb() ? "Yes" : "No");
        mfllLogD("feature feed.mix  = %s", m_spProperty->isFeedMix() ? "Yes" : "No");
        mfllLogD("feature feed.jpeg = %s", m_spProperty->isFeedJpeg() ? "Yes" : "No");
        }
        mfllLogD("feature mrp       = %d", registerMrpFeature);
        mfllLogD("========================================");
    }

    /**
     *  M F L L    T H R E A D S
     */

    /* thread for capture image->bss->raw2yuv */
    tellsFuturePhase1();

    /* thread to do ME&MC */
    tellsFutureMemc();

    /* thread for blending->mixing->NR->Postview->JPEG */
    tellsFuturePhase2();

    // full MFNR processing includes phase1 + phase2, hence we need to wait
    // until the threads of phase1 and phase2 finished.
    m_futurePhase1.wait();
    m_futurePhase2.wait();
    m_futureMemc.wait();
    m_futureMemory.wait();

lbExit:
    mfllFunctionOut();
    return err;
}

enum MfllErr MfllCore::doCancel()
{
    m_bypass.bypassMixing = 1;
    for (int i = MFLL_MAX_FRAMES - 1; i >= 0; i--) {
        m_bypass.bypassBlending[i] = 1;
        m_bypass.bypassMotionCompensation[i] = 1;
        m_bypass.bypassMotionEstimation[i] = 1;
    }
    //m_bypass.bypassEncodeYuvGolden = 1;
    //m_bypass.bypassEncodeYuvBase = 1;
    m_bypass.bypassBss = 1;
    m_bypass.bypassCapture = 1;
    for (int i = MFLL_MAX_FRAMES -1; i >=0; i--) {
        m_bypass.bypassAllocMemc[i] = 1;
    }
    m_bypass.bypassAllocWeighting[0] = 1;
    m_bypass.bypassAllocWeighting[1] = 1;
    m_bypass.bypassAllocYuvMixing = 1;
    m_bypass.bypassAllocYuvMcWorking = 1;
    //m_bypass.bypassAllocYuvGolden = 1;
    //m_bypass.bypassAllocYuvBase = 1;
    for (int i = MFLL_MAX_FRAMES -1; i >=0; i--) {
        m_bypass.bypassAllocQyuvBuffer[i] = 1;
        m_bypass.bypassAllocRawBuffer[i] = 1;
    }

    for (int i = MFLL_MAX_FRAMES -1; i >=0; i--) {
         syncAnnounceDone(&m_syncQueueYuvCapture[i]);  //Announce yuv bss
    }
    return MfllErr_Ok;
}

unsigned int MfllCore::getIndexByNewIndex(const unsigned int &newIndex)
{
    return m_bssIndex[newIndex];
}

int MfllCore::getSensorId(void)
{
    return m_sensorId;
}

int MfllCore::getCurrentIso(void) const
{
    return m_iso;
}

int MfllCore::getCurrentShutter(void) const
{
    return m_exposure;
}

unsigned int MfllCore::getVersion(void)
{
    return MFLL_MAKE_REVISION(ver_major, ver_minor, ver_featured);
}

std::string MfllCore::getVersionString(void)
{
    return mfll::makeRevisionString(ver_major, ver_minor, ver_featured);
}

bool MfllCore::isShooted(void)
{
    bool b = false;
    pthread_mutex_lock(&m_mutexShoot);
    b = m_bShooted;
    pthread_mutex_unlock(&m_mutexShoot);
    return b;
}

bool MfllCore::isFullSizeMc(void)
{
    bool b = false;
    pthread_mutex_lock(&m_mutexShoot);
    b = m_bShooted;
    pthread_mutex_unlock(&m_mutexShoot);

    return b ? (m_isFullSizeMc == 1) : false;
}

enum MfllErr MfllCore::queueYuvCapture(
    int32_t                 requestNo,
    sp<IMfllImageBuffer>&   sourceImg,
    sp<IMfllImageBuffer>&   quarterImg,
    MfllMotionVector        mv)
{
    if (CC_UNLIKELY(requestNo < 0 || requestNo >= getCaptureFrameNum())) {
        mfllLogW("requestNo is out of range.");
        return MfllErr_BadArgument;
    }

    enum MfllErr err = MfllErr_Ok;

    /* display enter function log */
    mfllFunctionInInt(requestNo);
    mfllTraceCall();

    if (m_bypass.bypassEncodeQYuv[requestNo]) {
        sp<IMfllCapturer> pCapturer = getCapturer();

        if (pCapturer.get() == nullptr) {
            err = MfllErr_BadArgument;
            goto lbExit;
        }

        pCapturer->queueSourceImage(sourceImg);
        pCapturer->queueSourceMonoImage(quarterImg);
        pCapturer->queueGmv(mv);

        err = do_CaptureSingle((void*)(long)requestNo);

    } else {
        mfllLogD("%s: Bypass queueYuvCapture operation", __FUNCTION__);
    }

lbExit:
    syncAnnounceDone(&m_syncQueueYuvCapture[requestNo]);
    mfllFunctionOut();
    return err;
}

enum MfllErr MfllCore::setQueueYuvFramesCounts(uint32_t nums)
{
    mfllFunctionIn();
    enum MfllErr err = MfllErr_Ok;

    /* use mutex to protect operating shoot state */
    pthread_mutex_lock(&m_mutexShoot);
    if (CC_UNLIKELY(m_bShooted)) {
        pthread_mutex_unlock(&m_mutexShoot);
        mfllLogW("MFLL has been shooted, catnot set queue frame number");
        err = MfllErr_Shooted;
        goto lbExit;
    }
    else {
        pthread_mutex_unlock(&m_mutexShoot);
    }

    if (CC_UNLIKELY(nums > getCaptureFrameNum())){
        mfllLogW("Queue frame number(%d) is out of range.", nums);
        err = MfllErr_BadArgument;
        goto lbExit;
    }

    /* boundary blending frame number */
    if (getBlendFrameNum() >= nums) {
        m_dbgInfoCore.frameBlend = nums;
        /* by pass un-necessary operation due to no buffer, included the last frame */
        for (size_t i = (nums <= 0 ? 0 : nums-1); i < MFLL_MAX_FRAMES; i++) {
            m_bypass.bypassBlending[i] = 1;
            m_bypass.bypassMotionCompensation[i] = 1;
            m_bypass.bypassMotionEstimation[i] = 1;
            if (i+1 < MFLL_MAX_FRAMES)
                syncAnnounceDone(&m_syncQueueYuvCapture[i+1]);
        }
    }

    m_caputredCount = nums;

lbExit:
    mfllFunctionOut();
    return err;
}

enum MfllErr MfllCore::registerEventListener(const sp<IMfllEventListener> &e)
{
    bool b = false;

    pthread_mutex_lock(&m_mutexShoot);
    b = m_bShooted;
    pthread_mutex_unlock(&m_mutexShoot);

    if (CC_UNLIKELY(b)) {
        mfllLogW("%s: Mfll is shooted, ignored.", __FUNCTION__);
        return MfllErr_Shooted;
    }

    return registerEventListenerNoLock(e);
}

enum MfllErr MfllCore::registerEventListenerNoLock(const sp<IMfllEventListener> &e, int useInsert /* = 0 */)
{
    mfllFunctionIn();

    vector<enum EventType> t = e->getListenedEventTypes();
    for (size_t i = 0; i < t.size(); i++) {
        if (useInsert == 0)
            m_event->registerEventListener(t[i], e);
        else
            m_event->registerEventListenerAsFirst(t[i], e);
    }

    mfllFunctionOut();
    return MfllErr_Ok;
}


enum MfllErr MfllCore::removeEventListener(IMfllEventListener *e)
{
    bool b = false;

    pthread_mutex_lock(&m_mutexShoot);
    b = m_bShooted;
    pthread_mutex_unlock(&m_mutexShoot);

    if (CC_UNLIKELY(b)) {
        mfllLogW("%s: Mfll is shooted, ignored.", __FUNCTION__);
        return MfllErr_Shooted;
    }

    vector<enum EventType> t = e->getListenedEventTypes();
    for (size_t i = 0; i < t.size() ; i++) {
        m_event->removeEventListener(t[i], e);
    }
    return MfllErr_Ok;
}

enum MfllErr MfllCore::setBypassOption(const MfllBypassOption_t &b)
{
    bool shooted = false;

    pthread_mutex_lock(&m_mutexShoot);
    shooted = m_bShooted;
    pthread_mutex_unlock(&m_mutexShoot);

    if (CC_UNLIKELY(shooted)) {
        mfllLogW("%s: Mfll is shooted, ignored.", __FUNCTION__);
        return MfllErr_Shooted;
    }

    m_bypass = b;

    return MfllErr_Ok;
}

enum MfllErr MfllCore::setBypassOptionAsSingleFrame()
{
    mfllAutoLogFunc();

    // bypass all MEMC and blending
    for (int i = 0; i < MFLL_MAX_FRAMES; i++){
        m_bypass.bypassBlending[i] = 1;
        m_bypass.bypassMotionCompensation[i] = 1;
        m_bypass.bypassMotionEstimation[i] = 1;
    }
    m_bypass.bypassEncodeYuvBase = 1;
    m_bypass.bypassBss = 1;
    m_bypass.bypassMixing = 1;
    m_bAsSingleFrame = 1;
    return MfllErr_Ok;
}

enum MfllErr MfllCore::setCaptureResolution(unsigned int width, unsigned int height)
{
    enum MfllErr err = MfllErr_Ok;

    pthread_mutex_lock(&m_mutexShoot);
    if (CC_UNLIKELY(m_bShooted)) {
        err = MfllErr_Shooted;
        goto lbExit;
    }

    m_width = width;
    m_height = height;

    /* update debug info too */
    m_dbgInfoCore.width = width;
    m_dbgInfoCore.height = height;

lbExit:
    pthread_mutex_unlock(&m_mutexShoot);
    if (err == MfllErr_Shooted)
        mfllLogW("MFLL has shooted, cannot set resolution");
    return err;
}

enum MfllErr MfllCore::setMfb(const sp<IMfllMfb> &mfb)
{
    enum MfllErr err = MfllErr_Ok;
    pthread_mutex_lock(&m_mutexShoot);
    if (CC_UNLIKELY(m_bShooted)) {
        err = MfllErr_Shooted;
        goto lbExit;
    }

    m_spMfb = mfb;

lbExit:
    pthread_mutex_unlock(&m_mutexShoot);
    if (err == MfllErr_Shooted)
        mfllLogW("MFLL has shooted, cannot set Mfb");
    return err;
}

enum MfllErr MfllCore::setNvramProvider(const sp<IMfllNvram> &nvramProvider)
{
    enum MfllErr err = MfllErr_Ok;
    pthread_mutex_lock(&m_mutexShoot);
    if (CC_UNLIKELY(m_bShooted)) {
        err = MfllErr_Shooted;
        goto lbExit;
    }

    m_spNvramProvider = nvramProvider;

lbExit:
    pthread_mutex_unlock(&m_mutexShoot);
    if (err == MfllErr_Shooted)
        mfllLogW("MFLL has shooted, cannot set NvramProvider");
    return err;
}

enum MfllErr MfllCore::setPostviewBuffer(
        const sp<IMfllImageBuffer>& buffer,
        const MfllRect_t& srcCropRgn)
{
    m_imgYuvPostview = buffer;
    m_postviewCropRgn = srcCropRgn;
    return MfllErr_Ok;
}

enum MfllErr MfllCore::setThumbnailBuffer(
        const sp<IMfllImageBuffer>& buffer,
        const MfllRect_t& srcCropRgn)
{
    m_imgYuvThumbnail = buffer;
    m_ptrImgYuvThumbnail = &(m_imgYuvThumbnail);
    m_thumbnailCropRgn = srcCropRgn;
    return MfllErr_Ok;
}

enum MfllErr MfllCore::setMiddlewareInfo(
        const MfllMiddlewareInfo_t& info
        )
{
    m_middlewareInfo.store(info);
    return MfllErr_Ok;
}

enum MfllErr MfllCore::setMixingBuffer(
        const MixYuvBufferType& type,
        const sp<IMfllImageBuffer>& buffer
        )
{
    std::unique_lock<std::mutex> locker(m_mixYuvBufferTypeMx);
    m_mixYuvBufferType = type;
    if (type == MixYuvType_Base || buffer == nullptr) {
        m_imgYuvMixedWorking.imgbuf = nullptr;
        m_ptrImgYuvMixedWorking = nullptr;
    }
    else {
        m_imgYuvMixedWorking.imgbuf = buffer;
        m_ptrImgYuvMixedWorking = &(m_imgYuvMixedWorking);
    }
    m_mixYuvBufferTypeCond.notify_one();
    mfllLogD("%s: buffer type:%d", __FUNCTION__, type);
    return MfllErr_Ok;
}

MixYuvBufferType MfllCore::getMixingBufferType()
{
    std::unique_lock<std::mutex> locker(m_mixYuvBufferTypeMx);
    if(m_mixYuvBufferType == MixYuvType_Unknow) {
        mfllLogD("%s: wait buffer type update", __FUNCTION__);
        while (m_mixYuvBufferTypeCond.wait_for(locker,std::chrono::seconds(1)) == std::cv_status::timeout) {
            mfllLogW("%s: get mix yuv buffer type timeout, use base yuv buffer", __FUNCTION__);
            return MixYuvType_Base;
        }
    }

    if (m_mixYuvBufferType == MixYuvType_Working) {
        if (CC_UNLIKELY(m_imgYuvMixedWorking.imgbuf == nullptr)) {
            mfllLogW("%s: working buffer is nullptr, use base yuv buffer instead of working buffer", __FUNCTION__);
            return MixYuvType_Base;
        }
        else {
            mfllLogD("%s: use working buffer as mixing yuv buffer", __FUNCTION__);
        }
    }
    else {
        mfllLogD("%s: use base yuv buffer as mixing yuv buffer", __FUNCTION__);
    }

    return m_mixYuvBufferType;
}

enum MfllErr MfllCore::setSyncPrivateData(const std::deque<void*>& dataset)
{
    enum MfllErr err = MfllErr_Ok;

    pthread_mutex_lock(&m_mutexShoot);

    if (CC_LIKELY( dataset.size() >= 3)) {
        if (CC_LIKELY( m_spMfb.get() )) {
            m_spMfb->setSyncPrivateData(dataset);
        }
        else {
            err = MfllErr_NotInited;
            goto lbExit;
        }
         /* for BssOrderMapping */
        IMetadata *pHalMeta = static_cast<IMetadata*>(dataset[1]);
        m_vMetaHal.push_back(pHalMeta);
        mfllLogD("saves m_vMetaHal, size = %zu", m_vMetaHal.size());
    }
    else {
        err = MfllErr_BadArgument;
        goto lbExit;
    }

lbExit:
    pthread_mutex_unlock(&m_mutexShoot);
    if (err == MfllErr_NotInited)
        mfllLogW("%s: setSyncPrivateData fail due to m_spMfb is null", __FUNCTION__);
    else if (err == MfllErr_BadArgument)
        mfllLogW("%s: setSyncPrivateData fail due to bad argument", __FUNCTION__);
    return err;
}

sp<IMfllMfb> MfllCore::getMfb(void)
{
    sp<IMfllMfb> r;

    pthread_mutex_lock(&m_mutexShoot);
    r = m_spMfb;
    pthread_mutex_unlock(&m_mutexShoot);

    return r;
}

sp<IMfllCapturer> MfllCore::getCapturer(void)
{
    return m_spCapturer;
}

enum MfllErr MfllCore::setCapturer(const sp<IMfllCapturer> &capturer)
{
    enum MfllErr err = MfllErr_Ok;

    pthread_mutex_lock(&m_mutexShoot);
    if (CC_UNLIKELY(m_bShooted)) {
        err = MfllErr_Shooted;
        goto lbExit;
    }

    m_spCapturer = capturer;

lbExit:
    pthread_mutex_unlock(&m_mutexShoot);
    if (err == MfllErr_Shooted)
        mfllLogW("MFLL has shooted, cannot set Q resolution");
    return err;
}

sp<IMfllImageBuffer> MfllCore::retrieveBuffer(const enum MfllBuffer &s, int index /* = 0 */)
{
    enum MfllErr err = MfllErr_Ok;
    sp<IMfllImageBuffer> p = NULL;
    switch (s) {
    case MfllBuffer_Raw:
        if (CC_UNLIKELY(index < 0 || index >= static_cast<int>(getCaptureFrameNum()))) {
            mfllLogE("%s: get raw buffer but index is wrong (index, max) = (%d, %d)", __FUNCTION__, index, getCaptureFrameNum());
            err = MfllErr_BadArgument;
            goto lbExit;
        }
        p = m_imgRaws[index].getImgBufLocked();
        break;
    case MfllBuffer_QYuv:
        if (CC_UNLIKELY(index < 0 || index >= static_cast<int>(getCaptureFrameNum()))) {
            mfllLogE("%s: get qyuv buffer but index is wrong (index, max) = (%d, %d)", __FUNCTION__, index, getCaptureFrameNum());
            err = MfllErr_BadArgument;
            goto lbExit;
        }
        p = m_imgQYuvs[index].getImgBufLocked();
        break;
    case MfllBuffer_AlgorithmWorking:
        if (CC_UNLIKELY(index < 0 || index >= static_cast<int>(getCaptureFrameNum()-1))) {
            mfllLogE("%s: get algorithm working buffer but index is wrong (index, max) = (%d, %d)", __FUNCTION__, index, getCaptureFrameNum());
            err = MfllErr_BadArgument;
            goto lbExit;
        }
        p = m_imgMemc[index].getImgBufLocked();
        break;
    case MfllBuffer_FullSizeYuv:
        if (CC_UNLIKELY(index < 0 || index >= static_cast<int>(getCaptureFrameNum()))) {
            mfllLogE("%s: get full size YUV buffer but index is wrong (index, max) = (%d, %d)", __FUNCTION__, index, getCaptureFrameNum());
            err = MfllErr_BadArgument;
            goto lbExit;
        }
        p = m_imgYuvs[index].getImgBufLocked();
        break;
    case MfllBuffer_PostviewYuv:
        p = m_imgYuvPostview.getImgBufLocked();
        break;
    case MfllBuffer_ThumbnailYuv:
        p = m_imgYuvThumbnail.getImgBufLocked();
        break;
    case MfllBuffer_BaseYuv:
        p = (m_ptrImgYuvBase ? m_ptrImgYuvBase->getImgBufLocked() : nullptr);
        break;
    case MfllBuffer_GoldenYuv:
        p = (m_ptrImgYuvGolden ? m_ptrImgYuvGolden->getImgBufLocked() : nullptr);
        break;
    case MfllBuffer_ReferenceYuv:
        p = (m_ptrImgYuvRef ? m_ptrImgYuvRef->getImgBufLocked() : nullptr);
        break;
    case MfllBuffer_BlendedYuv:
        p = (m_ptrImgYuvBlended ? m_ptrImgYuvBlended->getImgBufLocked() : nullptr);
        break;
    case MfllBuffer_MixedYuv:
        p = (m_ptrImgYuvMixed ? m_ptrImgYuvMixed->getImgBufLocked() : nullptr);
        break;
    case MfllBuffer_MixedYuvWorking:
        p = (m_ptrImgYuvMixedWorking ? m_ptrImgYuvMixedWorking->getImgBufLocked() : nullptr);
        break;
    case MfllBuffer_WeightingIn:
        p = (m_ptrImgWeightingIn ? m_ptrImgWeightingIn->getImgBufLocked() : nullptr);
        break;
    case MfllBuffer_WeightingOut:
        p = (m_ptrImgWeightingOut ? m_ptrImgWeightingOut->getImgBufLocked() : nullptr);
        break;
    default:
        mfllLogE("%s: unsupport buffer %d", __FUNCTION__, (int)s);
        err = MfllErr_NotSupported;
        goto lbExit;
    }
lbExit:
    return p;
}

enum MfllErr MfllCore::releaseBuffer(const enum MfllBuffer &s, int index /* = 0 */)
{
    enum MfllErr err = MfllErr_Ok;
    mfllLogD("%s: (type:%d, index:%d)", __FUNCTION__, s, index);
    switch (s) {
    case MfllBuffer_Raw:
        if (CC_UNLIKELY(index < 0 || index >= static_cast<int>(getCaptureFrameNum()))) {
            mfllLogE("%s: raw buffer index is out of range (index, max) = (%d, %d)", __FUNCTION__, index, getCaptureFrameNum());
            err = MfllErr_BadArgument;
            goto lbExit;
        }
        m_imgRaws[index] = 0;
        break;
    case MfllBuffer_QYuv:
        if (CC_UNLIKELY(index < 0 || index >= static_cast<int>(getCaptureFrameNum()))) {
            mfllLogE("%s: qyuv buffer index is out of range (index, max) = (%d, %d)", __FUNCTION__, index, getCaptureFrameNum());
            err = MfllErr_BadArgument;
            goto lbExit;
        }
        m_imgQYuvs[index] = 0;
        break;
    case MfllBuffer_AlgorithmWorking:
        if (CC_UNLIKELY(index < 0 || index >= static_cast<int>(getCaptureFrameNum()-1))) {
            mfllLogE("%s: algorithm working buffer but index is wrong (index, max) = (%d, %d)", __FUNCTION__, index, getCaptureFrameNum());
            err = MfllErr_BadArgument;
            goto lbExit;
        }
        m_imgMemc[index] = 0;
        break;
    case MfllBuffer_FullSizeYuv:
        if (CC_UNLIKELY(index < 0 || index >= static_cast<int>(getCaptureFrameNum()))) {
            mfllLogE("%s: qyuv buffer index is out of range (index, max) = (%d, %d)", __FUNCTION__, index, getCaptureFrameNum());
            err = MfllErr_BadArgument;
            goto lbExit;
        }
        m_imgYuvs[index] = 0;
        break;
    case MfllBuffer_PostviewYuv:
        m_imgYuvPostview = 0;
        break;
    case MfllBuffer_ThumbnailYuv:
        m_imgYuvThumbnail = 0;
        break;
    case MfllBuffer_BaseYuv:
        if (m_ptrImgYuvBase) (*m_ptrImgYuvBase) = 0;
        break;
    case MfllBuffer_GoldenYuv:
        if (m_ptrImgYuvGolden) (*m_ptrImgYuvGolden) = 0;
        break;
    case MfllBuffer_ReferenceYuv:
        if (m_ptrImgYuvRef) (*m_ptrImgYuvRef) = 0;
        break;
    case MfllBuffer_BlendedYuv:
        if (m_ptrImgYuvBlended) (*m_ptrImgYuvBlended) = 0;
        break;
    case MfllBuffer_MixedYuv:
        if (m_ptrImgYuvMixed) (*m_ptrImgYuvMixed) = 0;
        break;
    case MfllBuffer_MixedYuvWorking:
        if (m_ptrImgYuvMixedWorking) (*m_ptrImgYuvMixedWorking) = 0;
        break;
    case MfllBuffer_WeightingIn:
        if (m_ptrImgWeightingIn) (*m_ptrImgWeightingIn) = 0;
        break;
    case MfllBuffer_WeightingOut:
        if (m_ptrImgWeightingOut) (*m_ptrImgWeightingOut) = 0;
        break;
    default:
        mfllLogE("%s: unsupport buffer %d", __FUNCTION__, (int)s);
        err = MfllErr_NotSupported;
        goto lbExit;
    }
lbExit:
    return err;
}

enum MfllErr MfllCore::updateExifInfo(unsigned int key, uint32_t value)
{
    return m_spExifInfo->updateInfo(key, value); // this operator is thread-safe
}

const sp<IMfllExifInfo>& MfllCore::getExifContainer()
{
    return m_spExifInfo;
}

MfllMiddlewareInfo_t MfllCore::getMiddlewareInfo() const
{
    return m_middlewareInfo.load();
}

void MfllCore::updateImageBufferQueueLimit()
{
    int maxFullSize = (m_isFullSizeMc
            ? m_frameCapture + 4
            : 3
            );
    int maxQSize = (m_isFullSizeMc
            ? m_frameCapture
            : m_frameCapture + 1
            );
    int maxWeighting = 2; // fix to 2
    int maxMemc = getMemcInstanceNum();

    m_imgQueue.fsize        = IMfllImageBufferQueue::getInstance(Flag_FullSize);
    m_imgQueue.qsize        = IMfllImageBufferQueue::getInstance(Flag_QuarterSize);
    m_imgQueue.weighting    = IMfllImageBufferQueue::getInstance(Flag_WeightingTable);
    m_imgQueue.algo         = IMfllImageBufferQueue::getInstance(Flag_Algorithm);

    // check if queue is available, if yes, update it
    auto update__ = [](std::shared_ptr<IMfllImageBufferQueue>& q, int l, IMfllImageBuffer_Flag_t f)
    {
        if (q.get() != nullptr)
            q->updateInstanceLimit(l);
        else {
            mfllLogD("ImageBufferQueue(flag=%d) doesn't exist", f);
        }
    };

    update__(m_imgQueue.fsize,      maxFullSize,    Flag_FullSize);
    update__(m_imgQueue.qsize,      maxQSize,       Flag_QuarterSize);
    update__(m_imgQueue.weighting,  maxWeighting,   Flag_WeightingTable);
    update__(m_imgQueue.algo,       maxMemc,        Flag_Algorithm);
}

/**
 *  ---------------------------------------------------------------------------
 *  MFLL OPERATIONS
 *
 *  These functions are operations that MFLL will invoke. Function name template
 *  is do{$action} where {$action} is the meaniful name for the operation
 *  -----------------------------------------------------------------------------
 */

/**
 *  The first operation of MFLL is to capture frames, but it's necessary to wait
 *  RAW buffers and QYUV buffers for capture are ready.
 *
 *  We have to capture getBlendFrameNum() RAW and QYUV frames by IMfllCapturer
 */
enum MfllErr MfllCore::doCapture(JOB_VOID)
{
    enum MfllErr err = MfllErr_Ok;
    MfllEventStatus_t status;

    /* invokes events */
    m_event->onEvent(EventType_Capture, status, this);
    /* display enter function log */
    mfllFunctionIn();
    mfllTraceCall();

    if (m_bypass.bypassCapture || status.ignore) {
        mfllLogD("%s: Bypass capture operation", __FUNCTION__);
        status.ignore = 1;
    }
    else {
        err = do_Capture(__arg);
        int times = static_cast<int>(getCaptureFrameNum());
        for (int index = 0 ; index < times ; index++)
            syncAnnounceDone(&m_syncQueueYuvCapture[index]);
    }

lbExit:
    status.err = err;
    m_event->doneEvent(EventType_Capture, status, this);
    syncAnnounceDone(&m_syncCapture);
    mfllFunctionOut();
    return err;
}

/**
 *  Best Shot Selection should takes captured images
 */
enum MfllErr MfllCore::doBss(intptr_t intptrframeCount)
{
    enum MfllErr err = MfllErr_Ok;
    MfllEventStatus_t status;

    int bssFrameCount = intptrframeCount;

    /* conditions */
    syncWaitDone(&m_syncCapture);
#if SUPPORT_YUV_BSS
    syncWaitDone(&m_syncQueueYuvCapture[m_caputredCount-1]);
#endif
    /* display enter function log */
    mfllFunctionIn();
    mfllTraceCall();
    /* trigger events */
    m_event->onEvent(EventType_Bss, status, this, (void*)&bssFrameCount);

    /* check property if force disable BSS */
    if (CC_UNLIKELY(m_spProperty->getBss() <= 0)) {
        mfllLogD("%s: Bypass bss due to force disable by property", __FUNCTION__);
        m_bypass.bypassBss = 1;
    }

    if (m_bypass.bypassBss || status.ignore || (bssFrameCount <= 0)) {
        mfllLogD("%s: Bypass bss", __FUNCTION__);
        status.ignore = 1;
        m_dbgInfoCore.bss_enable = 0;
    }
    else {
        m_dbgInfoCore.bss_enable = 1;

        /* if frame capture number is smaller that target, assign it */
        if (static_cast<int>(getCaptureFrameNum()) < bssFrameCount)
            bssFrameCount = getCaptureFrameNum();

        /* if real frame captured is smaller that target, assign it */
        if (static_cast<int>(m_caputredCount) < bssFrameCount)
            bssFrameCount = static_cast<int>(m_caputredCount);

        err = do_Bss(bssFrameCount);
    }

lbExit:
    status.err = err;
    m_event->doneEvent(EventType_Bss, status, this, (void*)&bssFrameCount);

    syncAnnounceDone(&m_syncBss);

    // late release, except RAW index 0
    for (size_t i = 1; i < MFLL_MAX_FRAMES; i++) {
        m_imgRaws[i].releaseBufLocked();
    }

    mfllFunctionOut();

    return err;
}

/**
 *  To encode base RAW buffer to YUV base buffer. This operation must be invoked
 *  after doBss() has done and YUV base buffer is available to use. Hence the
 *  conditions are:
 *
 *  1. Buffer of YUV base is ready
 *  2. BSS has been done
 */
enum MfllErr MfllCore::doEncodeYuvBase(JOB_VOID)
{
    enum MfllErr err = MfllErr_Ok;
    MfllEventStatus_t status;

    /* conditions */
    syncWaitDone(&m_syncQueueYuvCapture[0]); //Base Image for Me
    syncWaitDone(&m_syncBss);
    /* display enter function log */
    mfllFunctionIn();
    mfllTraceCall();
    /* invokes events */
    m_event->onEvent(EventType_EncodeYuvBase, status, this);

    if (m_bypass.bypassEncodeYuvBase || status.ignore) {
        mfllLogD("%s: Bypass encoding YUV base", __FUNCTION__);
        status.ignore = 1;
    }
    else {
        err = do_EncodeYuvBase(__arg);
    }

lbExit:
    status.err = err;
    m_event->doneEvent(EventType_EncodeYuvBase, status, this);
    syncAnnounceDone(&m_syncEncodeYuvBase);
    mfllFunctionOut();
    return err;
}

/**
 *  To encode base RAW buffer to YUV golden buffer. This operation must be invoked
 *  after doBss() has done and YUV golden buffer is available to use.
 *
 *  1. Buffer of YUV golden is ready
 *  2. BSS has been done
 */
enum MfllErr MfllCore::doEncodeYuvGolden(JOB_VOID)
{
    enum MfllErr err = MfllErr_Ok;
    MfllEventStatus_t status;

    /* conditions */
    syncWaitDone(&m_syncQueueYuvCapture[0]); //Base Image for Me
    syncWaitDone(&m_syncBss);
    /* display enter function log */
    mfllFunctionIn();
    mfllTraceCall();
    /* invokes events */
    m_event->onEvent(EventType_EncodeYuvGolden, status, this); // invokes events

    if (m_bypass.bypassEncodeYuvGolden || status.ignore) {
        mfllLogD("%s: Bypass encoding YUV golden", __FUNCTION__);
        status.ignore = 1;
    }
    else {
        err = do_EncodeYuvGolden(__arg);
    }

lbExit:
    status.err = err;
    m_event->doneEvent(EventType_EncodeYuvGolden, status, this);
    syncAnnounceDone(&m_syncEncodeYuvGolden);
    mfllFunctionOut();
    return err;
}

/**
 *  Calculate motion estimation using MfllMemc. This operation must be invoked
 *  while these conditions are matched:
 *
 *  1. BSS has been done
 */
enum MfllErr MfllCore::doMotionEstimation(void *void_index)
{
    unsigned int index = (unsigned int)(long)void_index;
    enum MfllErr err = MfllErr_Ok;
    MfllEventStatus_t status;

    /* conditions */
    syncWaitDone(&m_syncBss); // we dont need to wait capture due to BSS does

    /* conditions */
    syncWaitDone(&m_syncQueueYuvCapture[0]); //Base Image for Me
    syncWaitDone(&m_syncQueueYuvCapture[index + 1]); //Ref Image for Me

    mfllFunctionInInt(index);
    mfllTraceCall();

    /* invokes event */
    m_event->onEvent(EventType_MotionEstimation, status, this, (void*)(long)index);

    if (m_bypass.bypassMotionEstimation[index] || status.ignore) {
        mfllLogD("%s: Bypass motion estimation(%d)", __FUNCTION__, index);
        status.ignore = 1;
    }
    else {
        err = do_MotionEstimation(void_index);
    }

lbExit:
    status.err = err;
    m_event->doneEvent(EventType_MotionEstimation, status, this, (void*)(long)index);
    syncAnnounceDone(&m_syncMotionEstimation[index]);
    mfllFunctionOut();
    return err;
}

/**
 *  Motion compensation is a pure software algorithm, the i-th motion compensation
 *  should be executed if only if:
 *
 *  1. The i-th motion estimation has been done.
 *  2. The (i-1)-th compensation has been done (if i > 1)
 */
enum MfllErr MfllCore::doMotionCompensation(void *void_index)
{
    const unsigned int index = (unsigned int)(long)void_index;
    enum MfllErr err = MfllErr_Ok;
    MfllEventStatus_t status;

    /* conditions */
    syncWaitDone(&m_syncMotionEstimation[index]);
    if (index > 0)
        syncWaitDone(&m_syncMotionCompensation[index - 1]);

    mfllFunctionInInt(index);
    mfllTraceCall();

    /* trigger events */
    m_event->onEvent(EventType_MotionCompensation, status, this, (void*)(long)index);

    if (m_bypass.bypassMotionCompensation[index] || status.ignore) {
        mfllLogD("%s: Bypass motion compensation(%d)", __FUNCTION__, index);
        status.ignore = 1;
    }
    else {
        err = do_MotionCompensation(void_index);
    }

lbExit:
    status.err = err;

    if (status.ignore || err != MfllErr_Ok) {
        m_dbgInfoCore.memc_skip = (1 << index);
    }

    m_event->doneEvent(EventType_MotionCompensation, status, this, (void*)(long)index);
    syncAnnounceDone(&m_syncMotionCompensation[index]);
    mfllFunctionOut();
    return err;
}

/**
 *  Blending using MTK hardware, furthurmore, operation blending depends on that
 *  the previous blended output. Therefore, the conditions of this operaion are:
 *
 *  1. YUV working buffer for blending is ready.
 *  2. The i-th motion compensation has been done.
 *  3. The (i-1)-th blending has been done.
 */
enum MfllErr MfllCore::doBlending(void *void_index)
{
    const unsigned int index = (unsigned int)(long)void_index;
    enum MfllErr err = MfllErr_Ok;
    MfllEventStatus_t status;

    /* conditions */
    syncWaitDone(&m_syncEncodeYuvBase);
    syncWaitDone(&m_syncMotionCompensation[index]);
    if (index > 0)
        syncWaitDone(&m_syncBlending[index - 1]);

    mfllFunctionInInt(index);
    mfllTraceCall();

    /* if blended, ping-pong the base YUV <-> blended YUV buffers */
    if (m_blendedCount > 0) {
        /* ping-pong YUV base <--> YUV blended */
        swap<ImageBufferPack*>(m_ptrImgYuvBase, m_ptrImgYuvBlended);
        swap<ImageBufferPack*>(m_ptrImgWeightingIn, m_ptrImgWeightingOut);
    }
    else {
        m_ptrImgYuvBase = &(m_imgYuvBase);
        m_ptrImgYuvBlended = &(m_imgYuvBlended);
        m_ptrImgWeightingIn = &(m_imgWeighting[0]);
        m_ptrImgWeightingOut = &(m_imgWeighting[1]);
    }

    m_ptrImgYuvRef = m_isFullSizeMc ? &(m_imgYuvs[index + 1]) : &(m_imgQYuvs[index +1]);

    /**
     *  before blending, set final weighting table as input weighting table,
     *  if blending ok, m_ptrImgWeightingFinal will be assigned to
     *  m_ptrImgWeightingOut
     */
    m_ptrImgWeightingFinal = m_ptrImgWeightingIn;


    /* trigger events */
    m_event->onEvent(EventType_Blending, status, this, (void*)(long)index);

    if (m_bypass.bypassBlending[index] || status.ignore) {
        mfllLogD("%s: Bypass blending(%d)", __FUNCTION__, index);
        status.ignore = 1;
    }
    else {
        err = do_Blending(void_index);
    }

lbExit:
    status.err = err;

    /* if ignored or something wrong, ping-pong back if blended count > 0 */
    if (status.ignore || err != MfllErr_Ok) {
        /**
         *  if m_blendedCount > 0 which means YUV base and blended buffers have been
         *  exchanged hence we need exchange them back
         */
        if (m_blendedCount > 0) {
            /* ping-pong YUV base <--> YUV blended */
            swap<ImageBufferPack*>(m_ptrImgYuvBase, m_ptrImgYuvBlended);
            swap<ImageBufferPack*>(m_ptrImgWeightingIn, m_ptrImgWeightingOut);
        }
    }

    m_event->doneEvent(EventType_Blending, status, this, (void*)(long)index);
    syncAnnounceDone(&m_syncBlending[index]);
    /* release Memory */
    mfllTraceBegin("Bld_FreeMem");
    {
        /* release buffer */
        m_imgYuvs[index + 1].releaseBufLocked();
        m_imgQYuvs[index + 1].releaseBufLocked();
    }
    mfllTraceEnd();
    mfllFunctionOut();
    return err;
}

/**
 *  Mixing a blended frame and a golden frame. Therefore, we must have a blended
 *  frame which means that this operation must wait until blending done. Hence
 *  the conditions are
 *
 *  1. YUV mixing output frame buffer ready
 *  2. All blending has been done.
 */
enum MfllErr MfllCore::doMixing(JOB_VOID)
{
    enum MfllErr err = MfllErr_Ok;
    MfllEventStatus_t status;

    /* conditions */
    syncWaitDone(&m_syncEncodeYuvBase);
    syncWaitDone(&m_syncEncodeYuvGolden);
    for (int i = 0; i < (int)getBlendFrameNum() - 1; i++)
        syncWaitDone(&m_syncBlending[i]);

    /* display enter function log */
    mfllFunctionIn();
    mfllTraceCall();

    /* if working on mixing, check buffer type first */
    if (getMixingBufferType() == MixYuvType_Working) {
        mfllLogD("%s: use working buffer as output", __FUNCTION__);
        m_ptrImgYuvMixed = m_ptrImgYuvMixedWorking;
    }
    else {
        mfllLogD("%s: re-use YUV base frame as output", __FUNCTION__);
        m_ptrImgYuvMixed = m_ptrImgYuvBase;
    }

    /* trigger events */
    m_event->onEvent(EventType_Mixing, status, this);

    /* update debug info */
    m_dbgInfoCore.frameBlend = m_blendedCount;

    /* check if all blending are ignored, if yes, ignore mix stage too */
    if (m_blendedCount <= 0)
    {
        mfllLogW("%s: bypass mixing stage due to no blending applied", __FUNCTION__);
            m_bypass.bypassMixing = 1;
    }

    if (m_bypass.bypassMixing || status.ignore) {
        mfllLogD("%s: Bypass mixing", __FUNCTION__);
        status.ignore = 1;
    }
    else {

        err = do_Mixing(__arg);
    }

lbExit:
    if (status.ignore || err != MfllErr_Ok) {
        if (err != MfllErr_Ok)
            mfllLogE("%s: mix failed with code %d, using YUV golden as output", __FUNCTION__, err);

        /* if using Yuv working buffer(which is set by caller), we need one more
        * MDP to covert image from m_ptrImgYuvGolden to m_ptrImgYuvMixed */
        if (getMixingBufferType() == MixYuvType_Working) {
            mfllLogD("%s: using working type MixedYuv, costs one more MDP", __FUNCTION__);
            IMfllImageBuffer* pImgSrc = m_ptrImgYuvGolden->imgbuf.get();
            IMfllImageBuffer* pImgDst = m_ptrImgYuvMixed ->imgbuf.get();
            MfllRect_t dstRect(0, 0, pImgSrc->getWidth(), pImgSrc->getHeight());

            err = m_spMfb->convertYuvFormatByMdp(
                    pImgSrc, // source
                    pImgDst, // destination 1
                    nullptr, // destination 2
                    dstRect, // ROI 1
                    MfllRect_t(), // ROI 2
                    YuvStage_Unknown
                    );

            if (__builtin_expect( err != MfllErr_Ok, false )) {
                mfllLogE("%s: convertYuvFormatByMdp returns error(%#x)",
                        __FUNCTION__, err);
                ImageBufferPack::swap(m_ptrImgYuvMixed, m_ptrImgYuvGolden);
                err = MfllErr_Ok;
            }

        }
        else {
            /* using golden YUV as output */
            ImageBufferPack::swap(m_ptrImgYuvMixed, m_ptrImgYuvGolden);
        }
    }

    /* no matter how, saves debug info */
    m_spExifInfo->updateInfo(m_dbgInfoCore);

    status.err = err;
    m_event->doneEvent(EventType_Mixing, status, this);
    syncAnnounceDone(&m_syncMixing);
    mfllFunctionOut();

    mfllTraceBegin("mix_freeMem");
    {
        if (m_ptrImgYuvGolden)
            m_ptrImgYuvGolden->releaseBufLocked();
        if (m_ptrImgYuvBlended)
            m_ptrImgYuvBlended->releaseBufLocked();
    }
    mfllTraceEnd();
    return err;
}

enum MfllErr MfllCore::doAllocRawBuffer(void *void_index)
{
    unsigned int index = (unsigned int)(long)void_index;
    enum MfllErr err = MfllErr_Ok;
    MfllEventStatus_t status;

    /* thread-safe operation lock */
    std::lock_guard<std::mutex> lk(m_opLockDoAllocRawBuffer[index]);

    mfllFunctionIn();
    mfllTraceCall();
    /* trigger events */
    m_event->onEvent(EventType_AllocateRawBuffer, status, this, (void*)(long)index);

    if (m_bypass.bypassAllocRawBuffer[index] || status.ignore) {
        mfllLogD("%s: Bypass allocate raw buffers", __FUNCTION__);
        status.ignore = 1;
    }
    else {
        err = do_AllocRawBuffer(void_index);
    }

lbExit:
    status.err = err;
    m_event->doneEvent(EventType_AllocateRawBuffer, status, this, (void*)(long)index);
    mfllFunctionOut();
    m_bypass.bypassAllocRawBuffer[index] = 1; // ignore the next round
    return err;
}


enum MfllErr MfllCore::doAllocQyuvBuffer(void *void_index)
{
    unsigned int index = (unsigned int)(long)void_index;
    enum MfllErr err = MfllErr_Ok;
    MfllEventStatus_t status;

    /* thread-safe operation lock */
    std::lock_guard<std::mutex> lk(m_opLockDoAllocQYuvBuffer[index]);

    mfllFunctionIn();
    mfllTraceCall();

    /* trigger events */
    m_event->onEvent(EventType_AllocateQyuvBuffer, status, this, (void*)(long)index);


    if (m_bypass.bypassAllocQyuvBuffer[index] || status.ignore) {
        mfllLogD("%s: Bypass allocate QYUV buffers", __FUNCTION__);
        status.ignore = 1;
    }
    else {
        err = do_AllocQyuvBuffer(void_index);
    }

lbExit:
    status.err = err;
    m_event->doneEvent(EventType_AllocateQyuvBuffer, status, this, (void*)(long)index);
    m_bypass.bypassAllocQyuvBuffer[index] = 1; // ignore the next round
    mfllFunctionOut();
    return err;
}

enum MfllErr MfllCore::doAllocYuvBase(JOB_VOID)
{
    enum MfllErr err = MfllErr_Ok;
    MfllEventStatus_t status;

    /* thread-safe operation lock */
    std::lock_guard<std::mutex> lk(m_opLockDoAllocYuvBase);

    mfllFunctionIn();
    mfllTraceCall();

    /* trigger events */
    m_event->onEvent(EventType_AllocateYuvBase, status, this);

    if (m_bypass.bypassAllocYuvBase || status.ignore) {
        mfllLogD("%s: Bypass allocate YUV base buffer", __FUNCTION__);
        status.ignore = 1;
    }
    else {
        err = do_AllocYuvBase(__arg);
    }

lbExit:
    status.err = err;
    m_event->doneEvent(EventType_AllocateYuvBase, status, this);
    m_bypass.bypassAllocYuvBase = 1; // ignore the next round
    mfllFunctionOut();
    return err;
}


enum MfllErr MfllCore::doAllocYuvGolden(JOB_VOID)
{
    enum MfllErr err = MfllErr_Ok;
    MfllEventStatus_t status;

    /* thread-safe operation lock */
    std::lock_guard<std::mutex> lk(m_opLockDoAllocYuvGolden);

    mfllFunctionIn();
    mfllTraceCall();

    /* trigger events */
    m_event->onEvent(EventType_AllocateYuvGolden, status, this);

    if (m_bypass.bypassAllocYuvGolden || status.ignore) {
        mfllLogD("%s: Bypass allocate YUV golen buffer", __FUNCTION__);
        status.ignore = 1;
    }
    else {
        err = do_AllocYuvGolden(__arg);
    }

lbExit:
    status.err = err;
    m_event->doneEvent(EventType_AllocateYuvGolden, status, this);
    m_bypass.bypassAllocYuvGolden = 1;
    mfllFunctionOut();
    return err;
}

enum MfllErr MfllCore::doAllocYuvWorking(JOB_VOID)
{
    enum MfllErr err = MfllErr_Ok;
    MfllEventStatus_t status;

    /* thread-safe operation lock */
    std::lock_guard<std::mutex> lk(m_opLockDoAllocYuvWorking);

    mfllFunctionIn();
    mfllTraceCall();

    /* trigger events */
    m_event->onEvent(EventType_AllocateYuvWorking, status, this);

    if (m_bypass.bypassAllocYuvWorking || status.ignore) {
        mfllLogD("%s: Bypass allocate YUV working(mixing) buffer", __FUNCTION__);
        status.ignore = 1;
    }
    else {
        err = do_AllocYuvWorking(__arg);
    }

lbExit:
    status.err = err;
    m_event->doneEvent(EventType_AllocateYuvWorking, status, this);
    m_bypass.bypassAllocYuvWorking = 1;
    mfllFunctionOut();
    return err;
}

/**
 *  To allocate a YUV buffer (m_qwidth x m_qheight) for Motion Compensation working
 */
enum MfllErr MfllCore::doAllocYuvMcWorking(JOB_VOID)
{
    enum MfllErr err = MfllErr_Ok;
    MfllEventStatus_t status;

    /* thread-safe operation lock */
    std::lock_guard<std::mutex> lk(m_opLockDoAllocYuvMcWorking);

    mfllFunctionIn();
    mfllTraceCall();

    /* trigger events */
    m_event->onEvent(EventType_AllocateYuvMcWorking, status, this);

    if (m_bypass.bypassAllocYuvMcWorking || status.ignore) {
        mfllLogD("%s: Bypass allocate YUV MC working buffer", __FUNCTION__);
        status.ignore = 1;
    }
    else {
        err = do_AllocYuvMcWorking(__arg);
    }

lbExit:
    status.err = err;
    m_event->doneEvent(EventType_AllocateYuvMcWorking, status, this);
    m_bypass.bypassAllocYuvMcWorking = 1;
    mfllFunctionOut();
    return err;
}

enum MfllErr MfllCore::doAllocWeighting(void *void_index)
{
    const unsigned int index = (unsigned int)(long)void_index;
    enum MfllErr err = MfllErr_Ok;
    MfllEventStatus_t status;

    /* index should be smaller than 2 or it's the critical condition */
    if (CC_UNLIKELY(index >= 2)) {
        mfllLogE("%s: index(%d) of weighting table is greater than 2", __FUNCTION__, index);
        abort();
    }

    /* thread-safe operation lock */
    std::lock_guard<std::mutex> lk(m_opLockDoAllocWeighting[index]);

    mfllFunctionIn();
    mfllTraceCall();

    /* trigger events */
    m_event->onEvent(EventType_AllocateWeighting, status, this, (void*)(long)index);

    if (m_bypass.bypassAllocWeighting[index] || status.ignore) {
        mfllLogD("%s: Bypass allocate weighting table(%d)", __FUNCTION__, index);
        status.ignore = 1;
    }
    else {
        err = do_AllocWeighting(void_index);
    }

lbExit:
    status.err = err;
    m_event->doneEvent(EventType_AllocateWeighting, status, this, (void*)(long)index);
    m_bypass.bypassAllocWeighting[index] = 1;
    mfllFunctionOut();
    return err;
}

enum MfllErr MfllCore::doAllocMemcWorking(void *void_index)
{
    unsigned int index = (unsigned int)(long)void_index;
    enum MfllErr err = MfllErr_Ok;
    MfllEventStatus_t status;

    /* thread-safe operation lock */
    std::lock_guard<std::mutex> lk(m_opLockDoAllocMemcWorking[index]);

    mfllFunctionIn();
    mfllTraceCall();

    /* trigger events */
    m_event->onEvent(EventType_AllocateMemc, status, this, (void*)(long)index);

    if (m_bypass.bypassAllocMemc[index] || status.ignore) {
        mfllLogD("%s: Bypass allocate MEMC working buffer(%d)", __FUNCTION__, index);
        status.ignore = 1;
    }
    else {
        err = do_AllocMemcWorking(void_index);
    }

lbExit:
    status.err = err;
    m_event->doneEvent(EventType_AllocateMemc, status, this, (void*)(long)index);
    m_bypass.bypassAllocMemc[index] = 1;
    mfllFunctionOut();
    return err;
}


void MfllCore::tellsFutureAllocateMemory()
{
    std::lock_guard<std::mutex> __l(m_futureMemory.mx);

    if (m_futureMemory.fu.valid()) // has told.
        return;

    m_futureMemory.fu = std::async(std::launch::async, [this]()->void
    {
        MFLL_THREAD_NAME("thread_allocate_memory");

        mfllLogD("future allocate memory +");

        int frameNum = (int)getBlendFrameNum();
        bool bIsMultiFrame = (frameNum - 1) <= 0 ? false : true;

        /* allocate raw buffers frames */
        for(int i = 0; i < frameNum; i++)
            doAllocRawBuffer((void*)(long)i);

        /* allocate QYUV buffers */
        for (int i = 0; i < frameNum; i++) {
            doAllocQyuvBuffer((void*)(long)i);
        }

        /* ME/MC working buffer */
        for(int i = 0; i < static_cast<int>(getMemcInstanceNum()) && bIsMultiFrame; i++) {
            doAllocMemcWorking((void*)(long)i);
        }

        if (bIsMultiFrame) {
            /* MC working buffer */
            doAllocYuvMcWorking(NULL);

            /* After BSS done, we can convert RAW to YUV base */
            doAllocYuvBase(NULL);

            /* YUV working buffer for blending stage */
            doAllocYuvWorking(NULL);

            /* weighting table 1 */
            doAllocWeighting((void*)(long)0);

            /* weighting table 2 */
            doAllocWeighting((void*)(long)1);
        }

        /* After BSS done, we can convert RAW to YUV golden too (is not urgent) */
        doAllocYuvGolden(NULL);

        mfllLogD("future allocate memory -");
    });
}

void MfllCore::tellsFuturePhase1()
{
    std::lock_guard<std::mutex> __l(m_futurePhase1.mx);

    if (m_futurePhase1.fu.valid())
        return;

    m_futurePhase1.fu = std::async(std::launch::async, [this]() -> void {
        MFLL_THREAD_NAME("thread_phase1");

        mfllLogD("future phase1 +");

        doCapture(NULL);
        doBss(getBlendFrameNum());
        doEncodeYuvBase(NULL);
        doEncodeYuvGolden(NULL);

        mfllLogD("future phase1 -");
    });
}

void MfllCore::tellsFuturePhase2()
{
    std::lock_guard<std::mutex> __l(m_futurePhase2.mx);

    if (m_futurePhase2.fu.valid())
        return;

    m_futurePhase2.fu = std::async(std::launch::async, [this]()->void
    {
        MFLL_THREAD_NAME("thread_phase2");

        mfllLogD("future phase2 +");

        int times = static_cast<int>(getBlendFrameNum() - 1);

        for (int i = 0; i < times; i++)
            doBlending((void*)(long)i);

        MfllOperationSync::getInstance()->removeJob(MfllOperationSync::JOB_MFB);

        doMixing(NULL);

        mfllLogD("future phase2 -");
    });
}

void MfllCore::tellsFutureMemc()
{
    std::lock_guard<std::mutex> __l(m_futureMemc.mx);

    if (m_futureMemc.fu.valid())
        return;

    // create thread
    m_futureMemc.fu = std::async(std::launch::async, [this]()->void
    {
        MFLL_THREAD_NAME("thread_memc");

        mfllLogD("future memc +");

        int times = static_cast<int>(getBlendFrameNum() - 1);
        int instanceNum = static_cast<int>(getMemcInstanceNum());
        int threadsNum = (times <= instanceNum ? times : instanceNum);

        mfllLogD("times to blend(%d), MEMC instanceNum(%d), threadsNum(%d)",
            times,
            instanceNum,
            threadsNum);

        // describes a function to do MEMC
        auto funcProcessMemc = [this](int index)->void
        {
            MFLL_THREAD_NAME("thread_me");

            mfllLogD("funcProcessMemc(%d) +", index);

            MfllErr err = MfllErr_Ok;

            /* set thread priority to algorithm threads */
            // {{{
            // Changes the current thread's priority, the algorithm threads will
            // inherits this value.
            int _priority = MfllProperty::readProperty(Property_AlgoThreadsPriority, MFLL_ALGO_THREADS_PRIORITY);
            int _oripriority = 0;
            // We give higher thread priority at the first MEMC thread because we
            // want the first MEMC finished ASAP.
            int _result = setThreadPriority(_priority - (index == 0 ? 1 : 0), _oripriority);
            if (CC_UNLIKELY( _result != 0 )) {
                mfllLogW("set algo threads priority failed(err=%d)", _result);
            }
            else {
                mfllLogD("set algo threads priority to %d", _priority);
            }
            // }}}

            err = doMotionEstimation((void*)(long)index);

            if (CC_UNLIKELY(err != MfllErr_Ok)) {
                mfllLogW("ME(%d) failed, ignore MC and blending(%d)", index, index);
                m_bypass.bypassMotionCompensation[index] = 1;
                m_bypass.bypassBlending[index] = 1;
            }

            if (CC_UNLIKELY(doMotionCompensation((void*)(long)index) != MfllErr_Ok)) {
                mfllLogW("MC(%d) failed, ignore blending(%d)", index, index);
                m_bypass.bypassBlending[index] = 1; // mark no need blending
            }

            // algorithm threads have been forked,
            // if priority set OK, reset it back to the original one
            if (CC_LIKELY( _result == 0 )) {
                _result = setThreadPriority( _oripriority, _oripriority );
                if (CC_UNLIKELY( _result != 0 )) {
                    mfllLogE("set priority back failed, weird!");
                }
            }

            mfllLogD("funcProcessMemc(%d) -", index);
        };

        // create job queue for threads
        std::vector< std::deque<int> > jobQueue(threadsNum);

        // dispatch job to threads
        for (int i = 0; i < times; i++) {
            jobQueue[i%threadsNum].push_back(i);
        }

        // start threads
        std::deque< std::future<void> > futureTeller;
        for (int i = 0; i < threadsNum; i++) {
            auto __future = std::async(std::launch::async, [&jobQueue, funcProcessMemc, i]()->void
            {
                for (auto index : jobQueue[i]) {

                    funcProcessMemc(index);
                }
            });
            futureTeller.push_back(std::move(__future));
        }
    });
}

void MfllCore::lockSyncObject(MfllSyncObj_t *pSyncObj)
{
    pthread_mutex_lock(&pSyncObj->trigger);
    pthread_mutex_lock(&pSyncObj->done);
}

void MfllCore::unlockSyncObject(MfllSyncObj_t *pSyncObj)
{
    pthread_mutex_unlock(&pSyncObj->trigger);
    pthread_mutex_unlock(&pSyncObj->done);
}

void MfllCore::syncWaitTrigger(MfllSyncObj_t *pSyncObj)
{
    pthread_mutex_lock(&pSyncObj->trigger);
    pthread_mutex_unlock(&pSyncObj->trigger);
}

void MfllCore::syncAnnounceTrigger(MfllSyncObj_t *pSyncObj)
{
    pthread_mutex_unlock(&pSyncObj->trigger);
}

void MfllCore::syncWaitDone(MfllSyncObj_t *pSyncObj)
{
    pthread_mutex_lock(&pSyncObj->done);
    pthread_mutex_unlock(&pSyncObj->done);
}

void MfllCore::syncAnnounceDone(MfllSyncObj_t *pSyncObj)
{
    pthread_mutex_unlock(&pSyncObj->done);
}

// include implementations
#include "MfllCoreImpl.hpp"

