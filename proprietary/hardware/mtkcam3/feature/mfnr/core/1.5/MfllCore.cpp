#define LOG_TAG "MfllCore"

#include "MfllCore.h"
#include <mtkcam3/feature/mfnr/MfllLog.h>
#include <MfllUtilities.h>

#include <utils/threads.h> // ANDROID_PRIORITY_FOREGROUND
#include <cutils/log.h> // XLOG
#include <sys/time.h>

#include <stdio.h>

// STL
#include <vector> // std::vector
#include <deque> // std::deque
#include <map> // std::map
#include <string> // std::string
#include <future> // std::async, std::launch
#include <fstream> // std::ifstream
#include <regex> // std::regex, std::sregex_token_iterator
#include <iterator> // std::back_inserter


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

//-----------------------------------------------------------------------------
// phase 2 protector
//-----------------------------------------------------------------------------
static std::mutex g_mx_phase2locker;
static void phase2_lock()
{
    g_mx_phase2locker.lock();
}

static void phase2_unlock()
{
    g_mx_phase2locker.unlock();
}

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

const pthread_attr_t PTHREAD_DEFAULT_ATTR = {
    0, NULL, 1024 * 1024, 4096, SCHED_OTHER, ANDROID_PRIORITY_FOREGROUND
};

/* This thread will allocate all buffers sequentially */
static void* thread_allocate_memory(void *arg)
{
    void *ret = 0;
    mfllFunctionIn();

    MfllCore *pCore = (MfllCore*)arg;
    int frameNum = (int)pCore->getBlendFrameNum();
    bool bIsMultiFrame = (frameNum - 1) <= 0 ? false : true;

    /* allocate raw buffers frames */
    for(int i = 0; i < frameNum; i++)
        pCore->doAllocRawBuffer((void*)(long)i);

    /* allocate QYUV buffers */
    for(int i = 0; i < frameNum; i++)
        pCore->doAllocQyuvBuffer((void*)(long)i);

    /* ME/MC working buffer */
    for(int i = 0; i < pCore->getMemcInstanceNum() && bIsMultiFrame; i++)
        pCore->doAllocMemcWorking((void*)(long)i);

    if (bIsMultiFrame) {
        /* MC working buffer */
        pCore->doAllocYuvMcWorking(NULL);

        /* After BSS done, we can convert RAW to YUV base */
        pCore->doAllocYuvBase(NULL);

        /* YUV working buffer for blending stage */
        pCore->doAllocYuvWorking(NULL);

        /* Allocate weighting table */
        pCore->doAllocWeighting((void*)(long)0);
        pCore->doAllocWeighting((void*)(long)1);
    }

    /* After BSS done, we can convert RAW to YUV golden too (is not urgent) */
    pCore->doAllocYuvGolden(NULL);

lbExit:
    pCore->decStrong(pCore);
    mfllFunctionOut();
    return ret;
}

/**
 *  Thread phase1 has responsibility to capture frames, execute BSS, and encode
 *  YUV base/golden frames.
 */
static void* thread_phase1(void *arg)
{
    void *ret = 0;
    mfllFunctionIn();
    MfllCore *c = (MfllCore*)arg;

    c->doCapture(NULL);
    c->doBss((void*)(long long)c->getBlendFrameNum());
    c->doEncodeYuvBase(NULL);
    c->doEncodeYuvGolden(NULL);

    c->decStrong(c);
    mfllFunctionOut();
    return ret;
}

/**
 *  Since our hardware only provides that only one a pass2 driver, all the
 *  operation invokes pass2 driver will be processed in this thread.
 *
 *  Due to pass2 driver is a thread-safe and FIFO mechanism, every operation
 *  can be dispatched as a sub thread to ask pass2 driver for operation using
 *  greedy algorithm.
 */
static void* thread_phase2(void *arg)
{
    void *ret = 0;
    mfllFunctionIn();
    MfllCore *c = (MfllCore*)arg;
    int times = (int)c->getBlendFrameNum() - 1;

    for (int i = 0; i < times; i++)
        c->doBlending((void*)(long)i);

    c->doMixing(NULL);
    c->decStrong(c);
    mfllFunctionOut();
    return ret;
}

/**
 *  Thread motion estimation will be invoked parallelly. There will be
 *  (m_frameBlend - 1) mfll_thread_me
 */
typedef struct _memc_attr {
    MfllCore *pCore;
    vector<int> indexQueue;
} memc_attr_t;

static void* thread_memc(void *arg_memc_attr)
{
    enum MfllErr err = MfllErr_Ok;
    memc_attr_t *pMemcAttr = (memc_attr_t*)arg_memc_attr;

    MfllCore *c = pMemcAttr->pCore;

    mfllFunctionIn();

    for (size_t i = 0; i < pMemcAttr->indexQueue.size(); i++ ) {
        int index = pMemcAttr->indexQueue[i];
        void *void_index = (void*)(long)index;
        std::future<enum MfllErr> future_me = std::async(std::launch::async, [&c, &void_index]()->enum MfllErr{
                return c->doMotionEstimation(void_index);
            });
        future_me.wait();
        err = future_me.get();
        if (err != MfllErr_Ok) {
            mfllLogW("%s: ME(%d) failed, ignore MC and blending(%d)", __FUNCTION__, index, index);
            c->m_bypass.bypassMotionCompensation[index] = 1;
            c->m_bypass.bypassBlending[index] = 1;
        }

        err = pMemcAttr->pCore->doMotionCompensation(void_index);
        if (err != MfllErr_Ok) {
            mfllLogW("%s: MC(%d) failed, ignore blending(%d)", __FUNCTION__, index, index);
            c->m_bypass.bypassBlending[index] = 1; // mark no need blending
        }
    }

lbExit:
    mfllFunctionOut();
    return (void*)(long)err;

}

static void* thread_memc_parallel(void *arg)
{
    void *ret = 0;
    mfllFunctionIn();
    MfllCore *c = (MfllCore*)arg;

    int times = (int)c->getBlendFrameNum() - 1;
    int instanceNum = (int)c->getMemcInstanceNum();
    int threadsNum = (times <= instanceNum ? times : instanceNum);

    mfllLogD("%s: times to blend(%d), MEMC instanceNum(%d), threadsNum(%d)", __FUNCTION__, times, instanceNum, threadsNum);

    memc_attr_t *attrs = new memc_attr_t[threadsNum];
    pthread_t *pThreads = new pthread_t[threadsNum];
    pthread_attr_t pthreadAttr = PTHREAD_DEFAULT_ATTR;

    /* create threadsNum threads for executing ME/MC */
    for (int i = 0; i < times; i++) {
        attrs[i % threadsNum].indexQueue.push_back(i);
        attrs[i % threadsNum].pCore = c;
    }

    for (int i = 0; i < threadsNum; i++) {
        pthread_create((pThreads + i), &pthreadAttr, thread_memc, (void*)&attrs[i]);
    }

    /* sync threads */
    for (int i = 0; i < threadsNum; i++) {
        void *r;
        pthread_join(*(pThreads + i), (void**)&r);
    }

lbExit:
    delete [] pThreads;
    delete [] attrs;

    c->decStrong(c);
    mfllFunctionOut();
    return ret;
}

/* this thread will process ME/MC sequentially */
static void* thread_memc_seq(void *arg)
{
    enum MfllErr err = MfllErr_Ok;

    mfllFunctionIn();

    MfllCore *c = (MfllCore*)arg;
    int times = (int)c->getBlendFrameNum() - 1;

    memc_attr_t memcAttr;
    memcAttr.pCore = c;
    for (int i = 0; i < times; i++) {
        memcAttr.indexQueue.push_back(i);
    }

    err = (enum MfllErr)(long)thread_memc((void*)&memcAttr);

    c->decStrong(c);
    mfllFunctionOut();

    return (void*)(long)err;
}

// ----------------------------------------------------------------------------
// helper to clear sp<IMfllImageBuffer> with a mutex locked
// ----------------------------------------------------------------------------
static void release_imagebuffer_w_locker(
        Mutex& mx,
        sp<IMfllImageBuffer>* pImg1,
        sp<IMfllImageBuffer>* pImg2 = nullptr,
        sp<IMfllImageBuffer>* pImg3 = nullptr
)
{
    Mutex::Autolock _l(mx);
    auto safe_release__ = [](sp<IMfllImageBuffer>* pBuf)->void
    {
        if (pBuf) *pBuf = NULL;
    };
    safe_release__(pImg1);
    safe_release__(pImg2);
    safe_release__(pImg3);
}


/******************************************************************************
 * M F L L
 *****************************************************************************/
IMfllCore* IMfllCore::createInstance(int, int, IMfllCore::Type)
{
    return (IMfllCore*)new MfllCore;
}

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
    m_postNrType = NoiseReductionType_None;
    m_caputredCount = 0;
    m_blendedCount = 0;

    /* set image buffer smart pointers to NULL */
    m_imgYuvBase = NULL;
    m_imgYuvGolden = NULL;
    m_imgYuvBlended = NULL;
    m_imgWeighting[0] = NULL;
    m_imgWeighting[1] = NULL;

    /* pointers of IMfllImageBuffers */
    m_ptrImgYuvBase = NULL;
    m_ptrImgYuvRef = NULL;
    m_ptrImgYuvGolden = NULL;
    m_ptrImgYuvBlended = NULL;
    m_ptrImgYuvMixed = NULL;
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
        /* set image buffer smart pointers = NULL */
        m_imgRaws[i] = NULL;
        m_imgYuvs[i] = NULL;
        m_imgQYuvs[i] = NULL;
        m_imgMemc[i] = NULL;
        m_bssIndex[i] = i;
        /* sync object for operation */
        lockSyncObject(&m_syncCapturedRaw[i]);
        lockSyncObject(&m_syncCapturedYuvQ[i]);
        lockSyncObject(&m_syncMotionEstimation[i]);
        lockSyncObject(&m_syncMotionCompensation[i]);
        lockSyncObject(&m_syncBlending[i]);
    }

    /* always uses performance service plug-in */
    m_spPerfService = MfllFeaturePerf::createInstance();
    /* always uses property plug-in */
    m_spProperty = new MfllProperty;

    mfllLogD("Create MfllCore version: %s",((std::string)getVersionString()).c_str());
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
    if (cfg.blend_num > cfg.capture_num) {
        mfllLogE("frame blend number is greater than capture");
        pthread_mutex_unlock(&m_mutexShoot);
        return MfllErr_BadArgument;
    }

    if (cfg.capture_num <= 0) {
        mfllLogE("frame capture number must greater than 0");
        pthread_mutex_unlock(&m_mutexShoot);
        return MfllErr_BadArgument;
    }

    m_event->onEvent(EventType_Init, status, (void*)this);

    /* copy configuration */
    /* Update catprue frame number and blend frame number */
    m_iso = cfg.iso;
    m_exposure = cfg.exp;
    m_frameBlend = cfg.blend_num;
    m_frameCapture = cfg.capture_num;
    m_sensorId = cfg.sensor_id;
    m_shotMode = cfg.mfll_mode;
    m_rwbMode = cfg.rwb_mode;
    m_mrpMode = cfg.mrp_mode;
    m_memcMode = cfg.memc_mode;
    m_isFullSizeMc = cfg.full_size_mc != 0 ? 1 : 0;

    /**
     *  Using (number of blending - 1) as MEMC threads num
     *  or the default should be m_memcInstanceNum = MFLL_MEMC_THREADS_NUM;
     */
    m_memcInstanceNum = m_frameBlend - 1;

    /* update debug info */
    m_dbgInfoCore.frameCapture = m_frameCapture;
    m_dbgInfoCore.frameBlend = m_frameBlend;
    m_dbgInfoCore.iso = m_iso;
    m_dbgInfoCore.exp = m_exposure;
    m_dbgInfoCore.ori_iso = cfg.original_iso;
    m_dbgInfoCore.ori_exp = cfg.original_exp;
    if (m_shotMode == MfllMode_NormalMfll || m_shotMode == MfllMode_ZsdMfll)
        m_dbgInfoCore.shot_mode = 1;
    else if (m_shotMode == MfllMode_NormalAis || m_shotMode == MfllMode_ZsdAis)
        m_dbgInfoCore.shot_mode = 2;
    else
        m_dbgInfoCore.shot_mode = 0;


    /* assign pointers to real buffer */
    m_ptrImgYuvBase = &(m_imgYuvBase);
    m_ptrImgYuvGolden = &(m_imgYuvGolden);
    m_ptrImgYuvBlended = &(m_imgYuvBlended);
    m_ptrImgWeightingIn = &(m_imgWeighting[0]);
    m_ptrImgWeightingOut = &(m_imgWeighting[1]);

    /* update m_qwidth and m_qheight to half of capture resolution */
    m_qwidth = m_width / 2;
    m_qheight = m_height / 2;

    /* init NVRAM provider */
    if (CC_UNLIKELY(m_spNvramProvider.get() == NULL)) {
        mfllLogW("%s: m_spNvramProvider has not been set, try to create one", __FUNCTION__);
        m_spNvramProvider = IMfllNvram::createInstance();
        if (CC_UNLIKELY(m_spNvramProvider.get() == NULL)) {
            err = MfllErr_UnexpectedError;
            mfllLogE("%s: create NVRAM provider failed", __FUNCTION__);
            goto lbExit;
        }
        IMfllNvram::ConfigParams nvramCfg;
        nvramCfg.iSensorId = cfg.sensor_id;
        nvramCfg.bFlashOn  = cfg.flash_on;
        m_spNvramProvider->init(nvramCfg);
    }

    /* create MEMC instance */
    for (size_t i = 0; i < getMemcInstanceNum(); i++) {
        m_spMemc[i] = IMfllMemc::createInstance();
        m_spMemc[i]->setMfllCore(this);
        m_spMemc[i]->setMotionEstimationResolution(m_qwidth, m_qheight);
        if (m_isFullSizeMc)
            m_spMemc[i]->setMotionCompensationResolution(m_width, m_height);
        else
            m_spMemc[i]->setMotionCompensationResolution(m_qwidth, m_qheight);
        if (m_spMemc[i]->init(m_spNvramProvider) != MfllErr_Ok) {
            mfllLogE("%s: init MfllMemc failed with code", __FUNCTION__);
        }
    }

    if (m_mrpMode == MrpMode_BestPerformance) {
        m_futureAllocMemory = std::async(std::launch::async, [this]()->void {
            this->incStrong((void*)this);
            thread_allocate_memory((void*)this);
        });
    }

lbExit:
    pthread_mutex_unlock(&m_mutexShoot);

    status.err = err;
    m_event->doneEvent(EventType_Init, status, (void*)this);

    if (err == MfllErr_Shooted)
        mfllLogW("%s: MFLL has shooted, cannot init MFLL anymore", __FUNCTION__);

    mfllFunctionOut();
    return err;
}

enum MfllErr MfllCore::doMfll()
{
    MfllErr err = MfllErr_Ok;
    std::vector<pthread_t> vThreadToJoin;
    pthread_t pThread;
    pthread_t pThreadPhase1;
    void *ret;
    std::vector<pthread_t>::iterator itr;
    int registerMrpFeature = 0;


    /* function pointer to memc */
    void* (*ptr_thread_memc)(void*) = thread_memc_seq;

    registerMrpFeature = (m_mrpMode == MrpMode_BestPerformance ? 0 : 1);

    mfllFunctionIn();

    /* use mutex to protect operating shoot state */
    pthread_mutex_lock(&m_mutexShoot);
    if (m_bShooted) {
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
    if (m_spProperty->isDump()) {
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

        /* construct dump file path and prefix */
        static unsigned int serialNumber = 0;
        static char szSerialNumber[32];
        sprintf(szSerialNumber, "%04d", serialNumber++);

        m_spDump->setFilePath(
                std::string(MFLL_DUMP_DEFAUL_PATH)
                .append("MFLL")
                .append(szSerialNumber)
                .append("_exp_")
                .append(std::to_string(m_exposure))
                .append("_iso_")
                .append(std::to_string(m_iso))
                );

        /* configure dump base on property */
        registerEventListenerNoLock(m_spDump);
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
        mfllLogD("blend-frame       = %d", m_frameBlend);
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
        mfllLogD("feature mrp       = %d", registerMrpFeature);
        mfllLogD("========================================");
    }

    /**
     *  M F L L    T H R E A D S
     */

    /* assign ME/MC thread according memc_mode */
    switch(m_memcMode){
        case MemcMode_Sequential:
            ptr_thread_memc = &thread_memc_seq;
            break;
        case MemcMode_Parallel:
            ptr_thread_memc = &thread_memc_parallel;
            break;
    }
    /* thread for capture image->bss->raw2yuv */
    createThread(&pThreadPhase1, thread_phase1);

    /* thread to do ME&MC */
    createThread(&pThread, (*ptr_thread_memc));
    vThreadToJoin.push_back(pThread);

    /* thread for blending->mixing->NR->Postview->JPEG */
    createThread(&pThread, thread_phase2);
    vThreadToJoin.push_back(pThread);

    /**
     *  Ok, here we need to sync threads.
     */
    /* Phase1 should be always joined. */
    err = joinThread(&pThreadPhase1);

    /* if not async call, we have to join these threads */
    for( itr = vThreadToJoin.begin(); itr != vThreadToJoin.end(); itr++) {
        pthread_t t = (*itr);
        pthread_join(t, &ret);
    }

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
    m_bypass.bypassEncodeYuvGolden = 1;
    m_bypass.bypassEncodeYuvBase = 1;
    m_bypass.bypassBss = 1;
    m_bypass.bypassCapture = 1;
    for (int i = MFLL_MAX_FRAMES -1; i >=0; i--) {
        m_bypass.bypassAllocMemc[i] = 1;
    }
    m_bypass.bypassAllocWeighting[0] = 1;
    m_bypass.bypassAllocWeighting[1] = 1;
    m_bypass.bypassAllocYuvMixing = 1;
    m_bypass.bypassAllocYuvMcWorking = 1;
    m_bypass.bypassAllocYuvGolden = 1;
    m_bypass.bypassAllocYuvBase = 1;
    for (int i = MFLL_MAX_FRAMES -1; i >=0; i--) {
        m_bypass.bypassAllocQyuvBuffer[i] = 1;
        m_bypass.bypassAllocRawBuffer[i] = 1;
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
    return MFLL_MAKE_REVISION(
            MFLL_CORE_VERSION_MAJOR,
            MFLL_CORE_VERSION_MINOR,
            static_cast<int>(IMfllCore::Type::DEFAULT));
}

std::string MfllCore::getVersionString(void)
{
    return mfll::makeRevisionString(
            MFLL_CORE_VERSION_MAJOR,
            MFLL_CORE_VERSION_MINOR,
            static_cast<int>(IMfllCore::Type::DEFAULT));
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

enum MfllErr MfllCore::registerEventListener(const sp<IMfllEventListener> &e)
{
    bool b = false;

    pthread_mutex_lock(&m_mutexShoot);
    b = m_bShooted;
    pthread_mutex_unlock(&m_mutexShoot);

    if (b) {
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

    if (b) {
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

    if (shooted) {
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
    return MfllErr_Ok;
}

enum MfllErr MfllCore::setCaptureResolution(unsigned int width, unsigned int height)
{
    enum MfllErr err = MfllErr_Ok;

    pthread_mutex_lock(&m_mutexShoot);
    if (m_bShooted) {
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
    if (m_bShooted) {
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
    return MfllErr_NotSupported;
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
    return m_spMfb;
}

sp<IMfllCapturer> MfllCore::getCapturer(void)
{
    return m_spCapturer;
}

enum MfllErr MfllCore::setCapturer(const sp<IMfllCapturer> &capturer)
{
    enum MfllErr err = MfllErr_Ok;

    pthread_mutex_lock(&m_mutexShoot);
    if (m_bShooted) {
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
        if (index < 0 || index >= getCaptureFrameNum()) {
            mfllLogE("%s: get raw buffer but index is wrong (index, max) = (%d, %d)", __FUNCTION__, index, getCaptureFrameNum());
            err = MfllErr_BadArgument;
            goto lbExit;
        }
        p = m_imgRaws[index];
        break;
    case MfllBuffer_QYuv:
        if (index < 0 || index >= getCaptureFrameNum()) {
            mfllLogE("%s: get qyuv buffer but index is wrong (index, max) = (%d, %d)", __FUNCTION__, index, getCaptureFrameNum());
            err = MfllErr_BadArgument;
            goto lbExit;
        }
        p = m_imgQYuvs[index];
        break;
    case MfllBuffer_AlgorithmWorking:
        if (index < 0 || index >= (getCaptureFrameNum()-1)) {
            mfllLogE("%s: get algorithm working buffer but index is wrong (index, max) = (%d, %d)", __FUNCTION__, index, getCaptureFrameNum());
            err = MfllErr_BadArgument;
            goto lbExit;
        }
        p = m_imgMemc[index];
        break;
    case MfllBuffer_FullSizeYuv:
        if (index < 0 || index >= getCaptureFrameNum()) {
            mfllLogE("%s: get full size YUV buffer but index is wrong (index, max) = (%d, %d)", __FUNCTION__, index, getCaptureFrameNum());
            err = MfllErr_BadArgument;
            goto lbExit;
        }
        p = m_imgYuvs[index];
        break;
    case MfllBuffer_PostviewYuv:
        p = m_imgYuvPostview;
        break;
    case MfllBuffer_BaseYuv:
        p = (*m_ptrImgYuvBase);
        break;
    case MfllBuffer_GoldenYuv:
        p = (*m_ptrImgYuvGolden);
        break;
    case MfllBuffer_ReferenceYuv:
        p = (*m_ptrImgYuvRef);
        break;
    case MfllBuffer_BlendedYuv:
        p = (*m_ptrImgYuvBlended);
        break;
    case MfllBuffer_MixedYuv:
        p = (*m_ptrImgYuvMixed);
        break;
    case MfllBuffer_WeightingIn:
        p = (*m_ptrImgWeightingIn);
        break;
    case MfllBuffer_WeightingOut:
        p = (*m_ptrImgWeightingOut);
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
    switch (s) {
    case MfllBuffer_Raw:
        if (index < 0 || index >= getCaptureFrameNum()) {
            mfllLogE("%s: raw buffer index is out of range (index, max) = (%d, %d)", __FUNCTION__, index, getCaptureFrameNum());
            err = MfllErr_BadArgument;
            goto lbExit;
        }
        m_imgRaws[index] = 0;
        break;
    case MfllBuffer_QYuv:
        if (index < 0 || index >= getCaptureFrameNum()) {
            mfllLogE("%s: qyuv buffer index is out of range (index, max) = (%d, %d)", __FUNCTION__, index, getCaptureFrameNum());
            err = MfllErr_BadArgument;
            goto lbExit;
        }
        m_imgQYuvs[index] = 0;
        break;
    case MfllBuffer_AlgorithmWorking:
        if (index < 0 || index >= (getCaptureFrameNum()-1)) {
            mfllLogE("%s: algorithm working buffer but index is wrong (index, max) = (%d, %d)", __FUNCTION__, index, getCaptureFrameNum());
            err = MfllErr_BadArgument;
            goto lbExit;
        }
        m_imgMemc[index] = 0;
        break;
    case MfllBuffer_FullSizeYuv:
        if (index < 0 || index >= getCaptureFrameNum()) {
            mfllLogE("%s: qyuv buffer index is out of range (index, max) = (%d, %d)", __FUNCTION__, index, getCaptureFrameNum());
            err = MfllErr_BadArgument;
            goto lbExit;
        }
        m_imgYuvs[index] = 0;
        break;
    case MfllBuffer_PostviewYuv:
        m_imgYuvPostview = 0;
        break;
    case MfllBuffer_BaseYuv:
        (*m_ptrImgYuvBase) = 0;
        break;
    case MfllBuffer_GoldenYuv:
        (*m_ptrImgYuvGolden) = 0;
        break;
    case MfllBuffer_ReferenceYuv:
        (*m_ptrImgYuvRef) = 0;
        break;
    case MfllBuffer_BlendedYuv:
        (*m_ptrImgYuvBlended) = 0;
        break;
    case MfllBuffer_MixedYuv:
        (*m_ptrImgYuvMixed) = 0;
        break;
    case MfllBuffer_WeightingIn:
        (*m_ptrImgWeightingIn) = 0;
        break;
    case MfllBuffer_WeightingOut:
        (*m_ptrImgWeightingOut) = 0;
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
        /**
         *  N O T I C E
         *
         *  Aquire buffers should be invoked by MfllCapturer
         *  1. IMfllCore::doAllocRawBuffer
         *  2. IMfllCore::doAllocQyuvBuffer
         */
        /* check if IMfllCapturer has been assigned */
        if (m_spCapturer.get() == NULL) {
            mfllLogD("%s: create MfllCapturer", __FUNCTION__);
            m_spCapturer = IMfllCapturer::createInstance();
            if (m_spCapturer.get() == NULL) {
                mfllLogE("%s: create MfllCapturer instance", __FUNCTION__);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
        }

        vector< sp<IMfllImageBuffer>* > raws;
        vector< sp<IMfllImageBuffer>* > yuvs;
        vector< sp<IMfllImageBuffer>* > qyuvs;
        vector<MfllMotionVector_t*>     gmvs;
        vector< int >                   rStatus;

        /* prepare output buffer */
        for (int i = 0; i < (int)getCaptureFrameNum(); i++) {
            raws.push_back(&m_imgRaws[i]);
            qyuvs.push_back(&m_imgQYuvs[i]);
            gmvs.push_back(&m_globalMv[i]);
            rStatus.push_back(0);

            if (m_isFullSizeMc)
                yuvs.push_back(&m_imgYuvs[i]);
        }

        /* register event dispatcher and set MfllCore instance */
        m_spCapturer->setMfllCore((IMfllCore*)this);
        err = m_spCapturer->registerEventDispatcher(m_event);

        if (err != MfllErr_Ok) {
            mfllLogE("%s: MfllCapture::registerEventDispatcher failed with code %d", __FUNCTION__, err);
            goto lbExit;
        }

        /* Catpure frames */
        mfllLogD("Capture frames!");

        if (m_isFullSizeMc)
            err = m_spCapturer->captureFrames(getCaptureFrameNum(), raws, yuvs, qyuvs, gmvs, rStatus);
        else
            err = m_spCapturer->captureFrames(getCaptureFrameNum(), raws, qyuvs, gmvs, rStatus);
        if (err != MfllErr_Ok) {
            mfllLogE("%s: MfllCapture::captureFrames failed with code %d", __FUNCTION__, err);
            goto lbExit;
        }
        /* check if force set GMV to zero */
        if (m_spProperty->getForceGmvZero() > 0) {
            for (int i = 0; i < MFLL_MAX_FRAMES; i++) {
                m_globalMv[i].x = 0;
                m_globalMv[i].y = 0;
            }
        }

        // {{{ checks captured buffers
        /* check result, resort buffers, and update frame capture number and blend number if need */
        {
            vector< sp<IMfllImageBuffer> > r; // raw buffer;
            vector< sp<IMfllImageBuffer> > q; // qyuv buffer
            vector< sp<IMfllImageBuffer> > y; // full size yuv buffer
            vector< MfllMotionVector_t   > m; // motion vector

            size_t okCount = 0; // counting ok frame numbers

            /* check the status from Capturer, if status is 0 means ok */
            for (size_t i = 0; i < (size_t)getCaptureFrameNum(); i++) {
                /* If not failed, save to vector */
                if (rStatus[i] == 0) {
                    r.push_back(m_imgRaws[i]);
                    q.push_back(m_imgQYuvs[i]);
                    y.push_back(m_imgYuvs[i]);
                    m.push_back(m_globalMv[i]);
                    okCount++;
                }
            }

            m_dbgInfoCore.frameCapture = okCount;
            mfllLogD("capture done, ok count=%u, num of capture = %d",
                    okCount, getCaptureFrameNum());

            m_caputredCount = okCount;

            /* if not equals, something wrong */
            if (okCount != (size_t)getCaptureFrameNum()) {
                /* sort available buffers continuously */
                for (size_t i = 0; i < okCount; i++) {
                    m_imgRaws[i] = r[i];
                    m_imgYuvs[i] = y[i];
                    m_imgQYuvs[i] = q[i];
                    m_globalMv[i] = m[i];
                }

                /* boundary blending frame number */
                if (getBlendFrameNum() > okCount) {
                    m_dbgInfoCore.frameBlend = okCount;
                    /* by pass un-necessary operation due to no buffer, included the last frame */
                    for (size_t i = (okCount <= 0 ? 0 : okCount - 1); i < getBlendFrameNum(); i++) {
                        m_bypass.bypassMotionEstimation[i] = 1;
                        m_bypass.bypassMotionCompensation[i] = 1;
                        m_bypass.bypassBlending[i] = 1;
                    }
                }
            }
        } // }}}
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
enum MfllErr MfllCore::doBss(void *void_frameCount)
{
    enum MfllErr err = MfllErr_Ok;
    MfllEventStatus_t status;

    int bssFrameCount = (int)(long long)void_frameCount;

    /* conditions */
    syncWaitDone(&m_syncCapture);
    /* display enter function log */
    mfllFunctionIn();
    mfllTraceCall();
    /* trigger events */
    m_event->onEvent(EventType_Bss, status, this, (void*)&bssFrameCount);

    /* check property if force disable BSS */
    if (m_spProperty->getBss() <= 0) {
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

        sp<IMfllBss> bss = IMfllBss::createInstance();
        if (bss.get() == NULL) {
            mfllLogE("%s: create IMfllBss instance fail", __FUNCTION__);
            m_dbgInfoCore.bss_enable = 0;
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }
        bss->setMfllCore(this);

        if (bss->init(m_spNvramProvider) != MfllErr_Ok) {
            mfllLogE("%s: init BSS failed, ignore BSS", __FUNCTION__);
            err = MfllErr_UnexpectedError;
            m_dbgInfoCore.bss_enable = 0;
            goto lbExit;
        }

        /* if frame capture number is smaller that target, assign it */
        if (getCaptureFrameNum() < bssFrameCount)
            bssFrameCount = getCaptureFrameNum();

        /* if real frame captured is smaller that target, assign it */
        if (m_caputredCount < bssFrameCount)
            bssFrameCount = m_caputredCount;

        std::vector< sp<IMfllImageBuffer> > imgs;
        std::vector< MfllMotionVector_t >   mvs;
        std::vector< int64_t > tss;

        for (size_t i = 0; i < bssFrameCount; i++) {
            imgs.push_back(m_imgQYuvs[i]);
            mvs.push_back(m_globalMv[i]);
        }

        std::vector<int> newIndex = bss->bss(imgs, mvs, tss);
        if (newIndex.size() <= 0) {
            mfllLogE("%s: do bss failed", __FUNCTION__);
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }

        // sort items
        std::vector< sp<IMfllImageBuffer> > newRaws;
        std::vector< sp<IMfllImageBuffer> > newQYuvs;
        std::vector< sp<IMfllImageBuffer> > newYuvs;
        for (size_t i = 0; i < bssFrameCount; i++) {
            int index = newIndex[i]; // new index
            newRaws.push_back(m_imgRaws[index]);
            newQYuvs.push_back(m_imgQYuvs[index]);
            newYuvs.push_back(m_imgYuvs[index]);
            m_bssIndex[i] = index;
            mfllLogD("%s: new index (%u)-->(%d)", __FUNCTION__, i, index);
            /**
             *  mvs will be sorted by Bss, we don't need to re-sort it again,
             *  just update it
             */
            m_globalMv[i] = mvs[i];
        }
        for (size_t i = 0; i < bssFrameCount; i++) {
            m_imgRaws[i] = newRaws[i];
            m_imgQYuvs[i] = newQYuvs[i];
            m_imgYuvs[i] = newYuvs[i];
        }

        // check frame to skip
        size_t frameNumToSkip = bss->getSkipFrameCount();
        if (frameNumToSkip > 0) {
            mfllLogD("%s: skip frame count = %u", __FUNCTION__, frameNumToSkip);
            for (size_t i = getCaptureFrameNum() - frameNumToSkip; i < getCaptureFrameNum(); i++) {
                mfllLogD("%s: skip frame index = %u", __FUNCTION__, i);
                release_imagebuffer_w_locker(m_lockAllocRaws[i], &m_imgRaws[i]);
                release_imagebuffer_w_locker(
                        m_lockAllocQYuvs[i],
                        &m_imgQYuvs[i],
                        &m_imgYuvs[i]
                        );
                m_bypass.bypassMotionEstimation[i] = 1;
                m_bypass.bypassMotionCompensation[i] = 1;
                m_bypass.bypassBlending[i] = 1;
            }
        }
    }

lbExit:
    status.err = err;
    m_event->doneEvent(EventType_Bss, status, this, (void*)&bssFrameCount);

    syncAnnounceDone(&m_syncBss);

    // late release
    for (size_t i = 1; i < MFLL_MAX_FRAMES; i++) {
        release_imagebuffer_w_locker(m_lockAllocRaws[i], &m_imgRaws[i]);
    }

    mfllFunctionOut();

    /* enter phase 2 */
    phase2_lock();

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
        err = doAllocYuvBase(NULL);
        if (err != MfllErr_Ok) {
            mfllLogE("%s: allocate YUV base failed", __FUNCTION__);
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }

        if (m_spMfb.get() == NULL) {
            mfllLogE("%s: MfllMfb is NULL, cannot encode", __FUNCTION__);
            status.ignore = 1;
            goto lbExit;
        }

        m_spMfb->init(m_sensorId);
        m_spMfb->setShotMode(m_shotMode);
        m_spMfb->setPostNrType(m_postNrType);
        mfllLogD("%s: encode base raw to yuv", __FUNCTION__);
        err = m_spMfb->encodeRawToYuv(
                m_imgRaws[0].get(),         // source RAW
                m_imgYuvBase.get(),         // output 1
                m_imgYuvPostview.get(),     // output 2
                MfllRect_t(),               // no crop for output 1
                m_postviewCropRgn,          // postview cropping region
                YuvStage_BaseYuy2);
        if (err != MfllErr_Ok) {
            mfllLogE("%s: Encode RAW to YUV fail", __FUNCTION__);
            goto lbExit;
        }
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
        err = doAllocYuvGolden(NULL);
        if (err != MfllErr_Ok) {
            mfllLogE("%s: allocate YUV golden failed", __FUNCTION__);
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }

        if (m_spMfb.get() == NULL) {
            mfllLogE("%s: MfllMfb is NULL, cannot encode", __FUNCTION__);
            status.ignore = 1;
            goto lbExit;
        }

        m_spMfb->init(m_sensorId);
        m_spMfb->setShotMode(m_shotMode);
        m_spMfb->setPostNrType(m_postNrType);
        err = m_spMfb->encodeRawToYuv(m_imgRaws[0].get(), m_imgYuvGolden.get(), YuvStage_GoldenYuy2);
        if (err != MfllErr_Ok) {
            mfllLogE("%s: Encode RAW to YUV fail", __FUNCTION__);
            goto lbExit;
        }
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
    std::string _log = std::string("start ME") + to_char(index);

    /* conditions */
    syncWaitDone(&m_syncBss); // we dont need to wait capture due to BSS does

    mfllFunctionInInt(index);
    mfllTraceCall();

    /* invokes event */
    m_event->onEvent(EventType_MotionEstimation, status, this, (void*)(long)index);

    if (m_bypass.bypassMotionEstimation[index] || status.ignore) {
        mfllLogD("%s: Bypass motion estimation(%d)", __FUNCTION__, index);
        status.ignore = 1;
    }
    else {
        mfllAutoLog(_log.c_str());
        unsigned int memcIndex = index % getMemcInstanceNum();
        sp<IMfllMemc> memc = m_spMemc[memcIndex];
        if (memc.get() == NULL) {
            mfllLogE("%s: MfllMemc is necessary to be created first (index=%d)", __FUNCTION__, index);
            err = MfllErr_NullPointer;
            goto lbExit;
        }

        err = doAllocMemcWorking((void*)(long)memcIndex);
        if (err != MfllErr_Ok) {
            mfllLogE("%s: allocate MEMC working buffer(%d) failed", __FUNCTION__, memcIndex);
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }
        /* set motion vector */
        memc->setMotionVector(m_globalMv[index + 1].x, m_globalMv[index + 1].y);
        memc->setAlgorithmWorkingBuffer(m_imgMemc[memcIndex]);
        memc->setMeBaseImage(m_imgQYuvs[0]);
        memc->setMeRefImage(m_imgQYuvs[index + 1]);

        err = memc->motionEstimation();
        if (err != MfllErr_Ok) {
            memc->giveupMotionCompensation();
            mfllLogE("%s: IMfllMemc::motionEstimation failed, returns %d", __FUNCTION__, (int)err);
            goto lbExit;
        }
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
    unsigned int index = (unsigned int)(long)void_index;
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

    /* using full size YUV if is using full size MC */
    sp<IMfllImageBuffer> *pMcRef = (m_isFullSizeMc == 0)
        ? &m_imgQYuvs[index + 1]
        : &m_imgYuvs[index + 1];

    if (m_bypass.bypassMotionCompensation[index] || status.ignore) {
        mfllLogD("%s: Bypass motion compensation(%d)", __FUNCTION__, index);
        status.ignore = 1;
    }
    else {
        unsigned int memcIndex = index % getMemcInstanceNum();
        sp<IMfllMemc> memc = m_spMemc[memcIndex];

        if (memc.get() == NULL) {
            mfllLogE("%s: MfllMemc is necessary to be created first (index=%d)", __FUNCTION__, index);
            err = MfllErr_NullPointer;
            goto lbExit;
        }

        /* check if we need to do MC or not */
        if (memc->isIgnoredMotionCompensation()) {
            mfllLogD("%s: ignored motion compensation & blending", __FUNCTION__);
            m_bypass.bypassBlending[index] = 1;
            goto lbExit;
        }

        /* allocate YUV MC working buffer if necessary */
        err = doAllocYuvMcWorking(NULL);
        if (err != MfllErr_Ok) {
            mfllLogE("%s: allocate YUV MC working buffer failed", __FUNCTION__);
            goto lbExit;
        }

        /* setting up mc */
        memc->setMcRefImage(*pMcRef);
        memc->setMcDstImage(m_imgYuvMcWorking);

        err = memc->motionCompensation();
        if (err != MfllErr_Ok) {
            mfllLogE("%s: IMfllMemc::motionCompensation failed, returns %d", __FUNCTION__, (int)err);
            goto lbExit;
        }

        /* exchange buffer from Dst->Src */
        {
            sp<IMfllImageBuffer> _t = *pMcRef;
            *pMcRef = m_imgYuvMcWorking;
            m_imgYuvMcWorking = _t;
        }

        /* sync CPU cache to HW */
        (*pMcRef)->syncCache(); // CPU->HW
    }

lbExit:
    status.err = err;

    if (status.ignore || err != MfllErr_Ok) {
        /* if failed or ignored, no need to convert image format */
        m_dbgInfoCore.memc_skip = (1 << index);
    }
    else {
        /* convert image format to Yuy2 without alignment for blending or mixing */
        (*pMcRef)->setAligned(2, 2);
        (*pMcRef)->convertImageFormat(ImageFormat_Yuy2);
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
    unsigned int index = (unsigned int)(long)void_index;
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
        swap<sp<IMfllImageBuffer>*>(m_ptrImgYuvBase, m_ptrImgYuvBlended);
        swap<sp<IMfllImageBuffer>*>(m_ptrImgWeightingIn, m_ptrImgWeightingOut);
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
        err = doAllocYuvWorking(NULL);
        if (err != MfllErr_Ok) {
            mfllLogE("%s: allocate YUV working buffer failed", __FUNCTION__);
            goto lbExit;
        }

        err = doAllocWeighting((void*)(long)0);
        err = doAllocWeighting((void*)(long)1);
        if (err != MfllErr_Ok) {
            mfllLogE("%s: allocate weighting buffer 0 or 1 failed", __FUNCTION__);
            goto lbExit;
        }

        if (m_spMfb.get() == NULL) {
            mfllLogD("%s: create IMfllMfb instance", __FUNCTION__);
            m_spMfb = IMfllMfb::createInstance();
            if (m_spMfb.get() == NULL) {
                mfllLogE("%s: m_spMfb is NULL", __FUNCTION__);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
        }

        err = m_spMfb->init(m_sensorId);
        m_spMfb->setShotMode(m_shotMode);
        m_spMfb->setPostNrType(m_postNrType);

        if (err != MfllErr_Ok) {
            mfllLogE("%s: m_spMfb init failed with code %d", __FUNCTION__, (int)err);
            goto lbExit;
        }

        /* do blending */
        mfllLogD("%s: do blending now", __FUNCTION__);

        /**
         * while index == 0, which means the first time to blend, the input weighting
         * table should be sent
         */
        err = m_spMfb->blend(
            (*m_ptrImgYuvBase).get(),
            (*m_ptrImgYuvRef).get(),
            (*m_ptrImgYuvBlended).get(),
            (index == 0) ? NULL : (*m_ptrImgWeightingIn).get(),
            (*m_ptrImgWeightingOut).get()
        );

        if (err != MfllErr_Ok) {
            mfllLogE("%s: Mfb failed with code %d", __FUNCTION__, (int)err);
            goto lbExit;
        }
        else {
            /* save for mixing */
            m_ptrImgWeightingFinal = m_ptrImgWeightingOut;
            m_blendedCount++; // count blended frame
        }
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
            swap<sp<IMfllImageBuffer>*>(m_ptrImgYuvBase, m_ptrImgYuvBlended);
            swap<sp<IMfllImageBuffer>*>(m_ptrImgWeightingIn, m_ptrImgWeightingOut);
        }
    }

    m_event->doneEvent(EventType_Blending, status, this, (void*)(long)index);
    syncAnnounceDone(&m_syncBlending[index]);
    /* release Memory */
    mfllTraceBegin("Bld_FreeMem");
    {
        /* release buffer */
        release_imagebuffer_w_locker(
                m_lockAllocQYuvs[index + 1],
                &m_imgQYuvs[index + 1],
                &m_imgYuvs[index + 1]
                );
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

    /* re-use YUV base frame as output */
    m_ptrImgYuvMixed = m_ptrImgYuvBase;

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
        if (m_spMfb.get() == NULL) {
            m_spMfb = IMfllMfb::createInstance();
            if (m_spMfb.get() == NULL) {
                mfllLogE("%s: create MFB instance failed", __FUNCTION__);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
        }

        /* set sensor ID and private data for sync */
        err = m_spMfb->init(m_sensorId);
        m_spMfb->setShotMode(m_shotMode);
        m_spMfb->setPostNrType(m_postNrType);

        if (err != MfllErr_Ok) {
            mfllLogE("%s: init MFB instance failed with code %d", __FUNCTION__, (int)err);
            goto lbExit;
        }

        /* mixing */
        mfllLogD("%s: do mixing", __FUNCTION__);
        err = m_spMfb->mix(
            (*m_ptrImgYuvBlended).get(),
            (*m_ptrImgYuvGolden).get(),
            (*m_ptrImgYuvMixed).get(),
            (*m_ptrImgWeightingFinal).get()
        );
    }

lbExit:
    if (status.ignore || err != MfllErr_Ok) {
        if (err != MfllErr_Ok)
            mfllLogE("%s: mix failed with code %d, using YUV golden as output", __FUNCTION__, err);

        /* using golden YUV as output */
        sp<IMfllImageBuffer> tp = *m_ptrImgYuvMixed;
        *m_ptrImgYuvMixed = *m_ptrImgYuvGolden;
        *m_ptrImgYuvGolden = tp;
    }

    /* no matter how, saves debug info */
    m_spExifInfo->updateInfo(m_dbgInfoCore);

    status.err = err;
    m_event->doneEvent(EventType_Mixing, status, this);
    syncAnnounceDone(&m_syncMixing);
    mfllFunctionOut();

    /* exits phase 2 */
    phase2_unlock();

    mfllTraceBegin("mix_freeMem");
    {
        // these pointers may related to buffer:
        //  1. m_imgYuvBase
        //  2. m_imgYuvGolden
        //  3. m_imgYuvBlended
        // due to the fact that there's race condition between buffer allocation
        // and release, we need lock mutex to protected.
        Mutex::Autolock __l1(m_lockAllocYuvBase);
        Mutex::Autolock __l2(m_lockAllocYuvGolden);
        Mutex::Autolock __l3(m_lockAllocYuvWorking);
        *m_ptrImgYuvGolden = NULL;
        *m_ptrImgYuvBlended = NULL;
    }
    mfllTraceEnd();
    return err;
}

enum MfllErr MfllCore::doAllocRawBuffer(void *void_index)
{
    unsigned int index = (unsigned int)(long)void_index;
    enum MfllErr err = MfllErr_Ok;
    MfllEventStatus_t status;

    Mutex::Autolock _l(m_lockAllocRaws[index]);

    mfllFunctionIn();
    mfllTraceCall();
    /* trigger events */
    m_event->onEvent(EventType_AllocateRawBuffer, status, this, (void*)(long)index);

    if (m_bypass.bypassAllocRawBuffer[index] || status.ignore) {
        mfllLogD("%s: Bypass allocate raw buffers", __FUNCTION__);
        status.ignore = 1;
    }
    else {
        /* create IMfllImageBuffer instances */
        IMfllImageBuffer *pImg = m_imgRaws[index].get();
        if (pImg == NULL) {
            pImg = IMfllImageBuffer::createInstance(BUFFER_NAME("raw_", index));
            if (pImg == NULL) {
                mfllLogE("%s: create IMfllImageBuffer(%d) failed", __FUNCTION__, index);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
            m_imgRaws[index] = pImg;
        }

        /* if not init, init it */
        if (!pImg->isInited()) {
            pImg->setImageFormat(ImageFormat_Raw10);
            pImg->setResolution(m_width, m_height);
            err = pImg->initBuffer();

            if (err != MfllErr_Ok) {
                mfllLogE("%s: init raw buffer(%d) failed", __FUNCTION__, index);
                err = MfllErr_UnexpectedError;
                goto lbExit;
            }
        }
        else {
            status.ignore = 1;
            mfllLogD("%s: raw buffer %d is inited, ignore here", __FUNCTION__, index);
        }
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

    Mutex::Autolock _l(m_lockAllocQYuvs[index]);

    mfllFunctionIn();
    mfllTraceCall();
    /* trigger events */
    m_event->onEvent(EventType_AllocateQyuvBuffer, status, this, (void*)(long)index);

    if (m_bypass.bypassAllocQyuvBuffer[index] || status.ignore) {
        mfllLogD("%s: Bypass allocate QYUV buffers", __FUNCTION__);
        status.ignore = 1;
    }
    else {
        IMfllImageBuffer *pImg = m_imgQYuvs[index].get();
        IMfllImageBuffer *pFullSizeImg = m_imgYuvs[index].get();

        if (pImg == NULL) {
            pImg = IMfllImageBuffer::createInstance(BUFFER_NAME("qyuv_", index), Flag_QuarterSize);
            if (pImg == NULL) {
                mfllLogE("%s: create QYUV buffer instance (%d) failed", __FUNCTION__, index);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
            m_imgQYuvs[index] = pImg;
        }

        /* full size YUV (if necessary) */
        if (m_isFullSizeMc && pFullSizeImg == NULL) {
            pFullSizeImg = IMfllImageBuffer::createInstance(BUFFER_NAME("fyuv_", index), Flag_FullSize);
            if (pFullSizeImg == NULL) {
                mfllLogE("%s: create full size YUV buffer instance (%d) failed", __FUNCTION__, index);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
            m_imgYuvs[index] = pFullSizeImg;
        }

        if (!pImg->isInited()) {
            pImg->setImageFormat(m_isFullSizeMc ? ImageFormat_Y8 : ImageFormat_I422);
            pImg->setAligned(16, 16);
            pImg->setResolution(m_qwidth, m_qheight);
            err = pImg->initBuffer();
            if (err != MfllErr_Ok) {
                mfllLogE("%s: init QYUV buffer(%d) failed", __FUNCTION__, index);
                goto lbExit;
            }
        }
        else {
            status.ignore = 1;
        }

        /* full size YUV */
        if (m_isFullSizeMc && pFullSizeImg->isInited() == false) {
            pFullSizeImg->setImageFormat(ImageFormat_I422);
            pFullSizeImg->setAligned(16, 16);
            pFullSizeImg->setResolution(m_width, m_height);
            err = pFullSizeImg->initBuffer();
            if (err != MfllErr_Ok) {
                mfllLogE("%s: init full size YUV buffer (%d) failed", __FUNCTION__, index);
                goto lbExit;
            }
        }
        /* full size YUV, and it's been inited */
        else if (m_isFullSizeMc){
            status.ignore = 1;
            pFullSizeImg->setAligned(16, 16);
            pFullSizeImg->convertImageFormat(ImageFormat_I422);
        }
        else {
            /* not full size, don't care */
        }
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

    Mutex::Autolock _l(m_lockAllocYuvBase);

    mfllFunctionIn();
    mfllTraceCall();

    /* trigger events */
    m_event->onEvent(EventType_AllocateYuvBase, status, this);

    if (m_bypass.bypassAllocYuvBase || status.ignore) {
        mfllLogD("%s: Bypass allocate YUV base buffer", __FUNCTION__);
        status.ignore = 1;
    }
    else {

        IMfllImageBuffer *pImg = m_imgYuvBase.get();
        if (pImg == NULL) {
            pImg = IMfllImageBuffer::createInstance(BUFFER_NAME("yuv_base", 0), Flag_FullSize);
            if (pImg == NULL) {
                mfllLogE("%s: create YUV base buffer instance failed", __FUNCTION__);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
            m_imgYuvBase = pImg;
        }

        if (!pImg->isInited()) {
            pImg->setImageFormat(ImageFormat_Yuy2);
            pImg->setResolution(m_width, m_height);
            pImg->setAligned(16, 16); // always uses 16 pixels aligned
            err = pImg->initBuffer();

            if (err != MfllErr_Ok) {
                mfllLogE("%s: init YUV base buffer failed", __FUNCTION__);
                err = MfllErr_UnexpectedError;
                goto lbExit;
            }
            pImg->setAligned(2, 2);
            pImg->convertImageFormat(ImageFormat_Yuy2);
        }
        else {
            status.ignore = 1;
            if (pImg->getImageFormat() != ImageFormat_Yuy2) {
                pImg->setAligned(2, 2);
                pImg->convertImageFormat(ImageFormat_Yuy2);
            }
        }
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

    Mutex::Autolock _l(m_lockAllocYuvGolden);

    mfllFunctionIn();
    mfllTraceCall();

    /* trigger events */
    m_event->onEvent(EventType_AllocateYuvGolden, status, this);

    if (m_bypass.bypassAllocYuvGolden || status.ignore) {
        mfllLogD("%s: Bypass allocate YUV golen buffer", __FUNCTION__);
        status.ignore = 1;
    }
    else {
        IMfllImageBuffer *pImg = m_imgYuvGolden.get();
        if (pImg == NULL) {
            pImg = IMfllImageBuffer::createInstance(BUFFER_NAME("yuv_golden", 0), Flag_FullSize);
            if (pImg == NULL) {
                mfllLogE("%s: create YUV golden instance failed", __FUNCTION__);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
            m_imgYuvGolden = pImg;
        }

        if (!pImg->isInited()) {
            pImg->setImageFormat(ImageFormat_Yuy2);
            pImg->setResolution(m_width, m_height);
            pImg->setAligned(16, 16);
            err = pImg->initBuffer();
            if (err != MfllErr_Ok) {
                mfllLogE("%s: init YUV golden buffer failed", __FUNCTION__);
                err = MfllErr_UnexpectedError;
                goto lbExit;
            }
            /* re-map the IImageBuffers due to aligned changed */
            pImg->setAligned(2, 2);
            pImg->convertImageFormat(ImageFormat_Yuy2);
        }
        else {
            status.ignore = 1;
            if (pImg->getImageFormat() != ImageFormat_Yuy2) {
                pImg->setAligned(2, 2);
                pImg->convertImageFormat(ImageFormat_Yuy2);
            }
        }
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

    Mutex::Autolock _l(m_lockAllocYuvWorking);

    mfllFunctionIn();
    mfllTraceCall();

    /* trigger events */
    m_event->onEvent(EventType_AllocateYuvWorking, status, this);

    if (m_bypass.bypassAllocYuvWorking || status.ignore) {
        mfllLogD("%s: Bypass allocate YUV working(mixing) buffer", __FUNCTION__);
        status.ignore = 1;
    }
    else {
        IMfllImageBuffer *pImg = m_imgYuvBlended.get();
        if (pImg == NULL) {
            pImg = IMfllImageBuffer::createInstance(BUFFER_NAME("yuv_blended",0), Flag_FullSize);
            if (pImg == NULL) {
                mfllLogE("%s: create YUV blended buffer instance failed", __FUNCTION__);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
            m_imgYuvBlended = pImg;
        }

        if (!pImg->isInited()) {
            pImg->setImageFormat(ImageFormat_Yuy2);
            pImg->setResolution(m_width, m_height);
            pImg->setAligned(16, 16);
            err = pImg->initBuffer();
            if (err != MfllErr_Ok) {
                mfllLogE("%s: init YUV blended buffer failed", __FUNCTION__);
                err = MfllErr_UnexpectedError;
                goto lbExit;
            }
            pImg->setAligned(2, 2);
            pImg->convertImageFormat(ImageFormat_Yuy2);
        }
        else {
            status.ignore = 1;
            if (pImg->getImageFormat() != ImageFormat_Yuy2) {
                pImg->setAligned(2, 2);
                pImg->convertImageFormat(ImageFormat_Yuy2);
            }
            mfllLogD("%s: yuv blending(working) is inited, ignored here", __FUNCTION__);
        }
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

    Mutex::Autolock _l(m_lockAllocYuvMcWorking);

    mfllFunctionIn();
    mfllTraceCall();

    /* trigger events */
    m_event->onEvent(EventType_AllocateYuvMcWorking, status, this);

    if (m_bypass.bypassAllocYuvMcWorking || status.ignore) {
        mfllLogD("%s: Bypass allocate YUV MC working buffer", __FUNCTION__);
        status.ignore = 1;
    }
    else {
        IMfllImageBuffer *pImg = m_imgYuvMcWorking.get();
        if (pImg == NULL) {
            pImg = IMfllImageBuffer::createInstance("yuv_mc_working", m_isFullSizeMc ? Flag_FullSize : Flag_QuarterSize);
            if (pImg == NULL) {
                mfllLogE("%s: create YUV MC working buffer instance failed", __FUNCTION__);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
            m_imgYuvMcWorking = pImg;
        }

        if (!pImg->isInited()) {
            pImg->setImageFormat(ImageFormat_Yuy2);
            if (m_isFullSizeMc) {
                pImg->setResolution(m_width, m_height);
            }
            else
                pImg->setResolution(m_qwidth, m_qheight);
            pImg->setAligned(16, 16);
            err = pImg->initBuffer();
            if (err != MfllErr_Ok) {
                mfllLogE("%s: init YUV MC working buffer failed", __FUNCTION__);
                err = MfllErr_UnexpectedError;
                goto lbExit;
            }
            pImg->setAligned(2, 2);
            pImg->convertImageFormat(ImageFormat_Yuy2);
        }
        else {
            if (pImg->getImageFormat() != ImageFormat_Yuy2) {
                pImg->setAligned(2, 2);
                pImg->convertImageFormat(ImageFormat_Yuy2);
            }
            status.ignore = 1;
        }
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
    unsigned int index = (unsigned int)(long)void_index;
    enum MfllErr err = MfllErr_Ok;
    MfllEventStatus_t status;

    /* index should be smaller than 2 or it's the critical condition */
    if (index >= 2) {
        mfllLogE("%s: index(%d) of weighting table is greater than 2", __FUNCTION__, index);
        abort();
    }

    Mutex::Autolock _l(m_lockAllocWeighting[index]);

    mfllFunctionIn();
    mfllTraceCall();

    /* trigger events */
    m_event->onEvent(EventType_AllocateWeighting, status, this, (void*)(long)index);

    if (m_bypass.bypassAllocWeighting[index] || status.ignore) {
        mfllLogD("%s: Bypass allocate weighting table(%d)", __FUNCTION__, index);
        status.ignore = 1;
    }
    else {
        IMfllImageBuffer *pImg = m_imgWeighting[index].get();
        if (pImg == NULL) {
            pImg = IMfllImageBuffer::createInstance(BUFFER_NAME("wt_", index));
            if (pImg == NULL) {
                mfllLogE("%s: create weighting table(%d) buffer instance failed", __FUNCTION__, index);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
            m_imgWeighting[index] = pImg;
        }

        if (!pImg->isInited()) {
            pImg->setImageFormat(ImageFormat_Raw8);
            pImg->setResolution(m_width, m_height);
            err = pImg->initBuffer();
            if (err != MfllErr_Ok) {
                mfllLogE("%s: init weighting table(%d) buffer failed", __FUNCTION__, index);
                err = MfllErr_UnexpectedError;
                goto lbExit;
            }
        }
        else {
            status.ignore = 1;
            mfllLogD("%s: weighting table %d is inited, ignored here", __FUNCTION__, index);
        }
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

    Mutex::Autolock _l(m_lockAllocMemc[index]);

    mfllFunctionIn();
    mfllTraceCall();

    /* trigger events */
    m_event->onEvent(EventType_AllocateMemc, status, this, (void*)(long)index);

    if (m_bypass.bypassAllocMemc[index] || status.ignore) {
        mfllLogD("%s: Bypass allocate MEMC working buffer(%d)", __FUNCTION__, index);
        status.ignore = 1;
    }
    else {
        int bufferIndex = index % getMemcInstanceNum();
        IMfllImageBuffer *pImg = m_imgMemc[bufferIndex].get();
        if (pImg == NULL) {
            pImg = IMfllImageBuffer::createInstance(BUFFER_NAME("memc_", bufferIndex));
            if (pImg == NULL) {
                mfllLogE("%s: create MEMC working buffer(%d) instance failed", __FUNCTION__, bufferIndex);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
            m_imgMemc[bufferIndex] = pImg;
        }

        if (!pImg->isInited()) {
            sp<IMfllMemc> memc = m_spMemc[bufferIndex];
            if (memc.get() == NULL) {
                mfllLogE("%s: memc instance(index %d) is NULL", __FUNCTION__, bufferIndex);
                err = MfllErr_NullPointer;
                goto lbExit;
            }
            size_t bufferSize = 0;
            err = memc->getAlgorithmWorkBufferSize(&bufferSize);
            if (err != MfllErr_Ok) {
                mfllLogE("%s: get algorithm working buffer size fail", __FUNCTION__);
                goto lbExit;
            }

            /* check if 2 bytes alignment */
            if (bufferSize % 2 != 0) {
                mfllLogW("%s: algorithm working buffer size is not 2 bytes alignment, make it is", __FUNCTION__);
                bufferSize += 1;
            }

            pImg->setImageFormat(ImageFormat_Raw8);
            pImg->setAligned(2, 2); // always using 2 bytes align
            pImg->setResolution(bufferSize/2, 2);
            err = pImg->initBuffer();
            if (err != MfllErr_Ok) {
                mfllLogE("%s: init MEMC working buffer(%d) failed", __FUNCTION__, bufferIndex);
                err = MfllErr_UnexpectedError;
                goto lbExit;
            }
        }
        else {
            status.ignore = 1;
            mfllLogD("%s: memc buffer %d is inited, ignored here", __FUNCTION__, bufferIndex);
        }
    }

lbExit:
    status.err = err;
    m_event->doneEvent(EventType_AllocateMemc, status, this, (void*)(long)index);
    m_bypass.bypassAllocMemc[index] = 1;
    mfllFunctionOut();
    return err;
}

enum MfllErr MfllCore::createThread(pthread_t *pThread, void *(*routine)(void*), bool bDistach /* = false */)
{
    pthread_attr_t attr = PTHREAD_DEFAULT_ATTR;
    mfllFunctionIn();

    /* increase lifetime, the routine should invoke decStrong */
    incStrong(this);

    pthread_create(pThread, &attr, routine, (void*)this);
    if (bDistach) {
        pthread_detach(*pThread);
    }
    mfllFunctionOut();
    return MfllErr_Ok;
}

enum MfllErr MfllCore::joinThread(pthread_t *pThread)
{
    enum MfllErr err = MfllErr_Ok;
    void *ret;

    mfllFunctionIn();

    int r = pthread_join(*pThread, &ret);
    if (r == EINVAL) {
        mfllLogE("Join pthread %#x failed, perhaps it's not a joinable thread", static_cast<void*>(pThread));
        err = MfllErr_UnexpectedError;
    }
    mfllFunctionOut();
    return err;
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
