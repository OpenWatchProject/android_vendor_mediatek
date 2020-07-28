#ifndef __MFLLCORE_H__
#define __MFLLCORE_H__

#include <mtkcam3/feature/mfnr/MfllDefs.h>
#include <mtkcam3/feature/mfnr/MfllTypes.h>

/* interfaces */
#include <mtkcam3/feature/mfnr/IMfllCore.h>
#include <mtkcam3/feature/mfnr/IMfllImageBuffer.h>
#include <mtkcam3/feature/mfnr/IMfllCapturer.h>
#include <mtkcam3/feature/mfnr/IMfllEventListener.h>
#include <mtkcam3/feature/mfnr/IMfllEvents.h>
#include <mtkcam3/feature/mfnr/IMfllMfb.h>
#include <mtkcam3/feature/mfnr/IMfllMemc.h>
#include <mtkcam3/feature/mfnr/IMfllBss.h>
#include <mtkcam3/feature/mfnr/IMfllNvram.h>

/* plug-in */
#include <mtkcam3/feature/mfnr/MfllProperty.h>
#include "MfllFeatureMrp.h"
#include "MfllFeatureDump.h"
#include "MfllFeaturePerf.h"

/* standard libs */
#include <cstdint>
#include <semaphore.h>
#include <pthread.h> // pthread_t, pthread_mutex_t
#include <vector>
#include <string>
#include <future>

/* Android frameworks */
#include <utils/RefBase.h> // android::RefBase
#include <utils/Mutex.h> // android::Mutex
#include <utils/RWLock.h> // android::RWLock

using android::sp; // use android::sp as sp
using android::Mutex;
using android::RWLock; // use android::RWLock as sp
using std::vector; // use std::vector as vector

namespace mfll {

class MfllCore : public IMfllCore {
public:
    MfllCore(void);
    virtual ~MfllCore(void);

/* related modules */
public:
    /* Event dispatcher */
    sp<IMfllEvents> m_event;

public:
    /* module to provide captured frames */
    sp<IMfllCapturer>   m_spCapturer;
    sp<IMfllMfb>        m_spMfb;
    sp<IMfllMemc>       m_spMemc[MFLL_MAX_FRAMES];
    sp<IMfllNvram>      m_spNvramProvider;
    sp<IMfllExifInfo>   m_spExifInfo;

    /* plug-in */
    sp<MfllProperty>        m_spProperty;
    sp<MfllFeatureDump>     m_spDump;
    sp<MfllFeaturePerf>     m_spPerfService;

/* attributes */
protected:
    bool            m_bShooted;
    pthread_mutex_t m_mutexShoot; // protect shoot, makes MfllCore thread-safe
    MfllConfig_t    m_cfg;
    int             m_sensorId; // sensor Id
    unsigned int    m_frameBlend; // blending frame amount
    unsigned int    m_frameCapture; // number of frame to capture
    unsigned int    m_caputredCount; // REAL captured frame number
    unsigned int    m_blendedCount; // count frame blended in MFB stage
    unsigned int    m_bAsSingleFrame; // represents capture as single frame
    int             m_iso;
    int             m_exposure;
    unsigned int    m_memcInstanceNum; // represents how many MfllMemc to be create
    enum MfllMode   m_shotMode; // shotMode
    enum MemcMode   m_memcMode;
    enum RwbMode    m_rwbMode;
    enum MrpMode    m_mrpMode;
    enum NoiseReductionType m_postNrType;
    unsigned int    m_width; // full size
    unsigned int    m_height; // full size
    unsigned int    m_qwidth; // for 1/4 size YUV
    unsigned int    m_qheight; // for 1//4 size YUV
    int             m_isFullSizeMc; // represents is using full size MC

    /* Global motion vector retrieved by IMfllCapturer */
    unsigned int    m_bssIndex[MFLL_MAX_FRAMES];
    MfllMotionVector_t  m_globalMv[MFLL_MAX_FRAMES];

    /* Debug information of MfllCore */
    MfllCoreDbgInfo_t   m_dbgInfoCore;

    /* async call to allocate memory chunks */
    std::future<void>   m_futureAllocMemory;

    std::deque<IMetadata*>      m_vMetaHal;

/* public attributes */
public:
    /*  Bypass option can let you bypass any operation but conditions are still necessary */
    MfllBypassOption_t  m_bypass; // MfllBypassOption_t

    /* used image buffers */
    /* phiscal buffer, costs memory */
    sp<IMfllImageBuffer> m_imgRaws[MFLL_MAX_FRAMES];
    sp<IMfllImageBuffer> m_imgYuvs[MFLL_MAX_FRAMES]; // may save full size or 1/4 size yuv buffers
    sp<IMfllImageBuffer> m_imgQYuvs[MFLL_MAX_FRAMES]; // saves 1/4 size yuv for ME if using full size MC
    sp<IMfllImageBuffer> m_imgYuvMcWorking; // for Motion Compensation working
    sp<IMfllImageBuffer> m_imgYuvPostview;
    sp<IMfllImageBuffer> m_imgYuvBase; // for blending/mixing stage
    sp<IMfllImageBuffer> m_imgYuvGolden; // for blending/mixing stage
    sp<IMfllImageBuffer> m_imgYuvBlended; // for blending/mixing stage
    sp<IMfllImageBuffer> m_imgWeighting[2]; // ping-pong buffers, takes 2
    sp<IMfllImageBuffer> m_imgMemc[MFLL_MAX_FRAMES]; // working buffer for MEME

    MfllRect_t           m_postviewCropRgn; // zoom crop region, if the size is 0, do not apply crop

    /**
     *  During blending, MFLL will use two buffers as ping-pong buffers, e.g.:
     *  1st: imgBase    + imgRef = imgBlended    (ping)
     *  2nd: imgBlended + imgRef = imgBase       (pong)
     *  3rd: imgBase    + imgRef = imgBlended    (ping)
     *  ...
     *
     *  Hence here we use these pointers to indicate the REAL meanful buffers.
     */
    sp<IMfllImageBuffer> *m_ptrImgYuvBase; // indicates to full size base YUV
    sp<IMfllImageBuffer> *m_ptrImgYuvRef; // indicates to any size reference YUV
    sp<IMfllImageBuffer> *m_ptrImgYuvGolden; // indicates to golden YUV (noraml YUV)
    sp<IMfllImageBuffer> *m_ptrImgYuvBlended; // indicates to blended YUV (same size with YUV base)
    sp<IMfllImageBuffer> *m_ptrImgYuvMixed; // indicates to mixed YUV
    sp<IMfllImageBuffer> *m_ptrImgWeightingIn; // indicates to input of weighting table during blending
    sp<IMfllImageBuffer> *m_ptrImgWeightingOut; // indicates to output of weighting table during blending
    sp<IMfllImageBuffer> *m_ptrImgWeightingFinal; // indicates to final weighting for mixing

/**
 *  Synchronization objects
 *
 *  We're using pthread_mutex_t for thread synchronization, here we defined
 *  that the default mutexes state to locked for these operations (blending, mixing or ME/MC).
 *  All the operations have different condition which means
 *  to invoke an operaion, the operation will be executed after conditions matched.
 *  For example: doEncodeYuvBase, which means to encode base RAW frame to YUV frame.
 *  The conditions of this operation will be the base frame has been decided.
 *
 *  So we will invoke just like:
 *
 *  // wait RAW buffers buffers are ready to use
 *  syncWaitDone(&m_syncBss);
 *
 *  to wait conditions matched.
 */
public:
    /* operation sync objects */
    MfllSyncObj_t m_syncCapture; // annonced done while capture is done
    MfllSyncObj_t m_syncCapturedRaw[MFLL_MAX_FRAMES]; // annonced done while a single RAW is captured
    MfllSyncObj_t m_syncCapturedYuvQ[MFLL_MAX_FRAMES]; // annonced done while a single QYUV is captured
    MfllSyncObj_t m_syncEncodeYuvBase;
    MfllSyncObj_t m_syncEncodeYuvGolden;
    MfllSyncObj_t m_syncBss;
    MfllSyncObj_t m_syncMotionEstimation[MFLL_MAX_FRAMES];
    MfllSyncObj_t m_syncMotionCompensation[MFLL_MAX_FRAMES];
    MfllSyncObj_t m_syncBlending[MFLL_MAX_FRAMES];
    MfllSyncObj_t m_syncMixing;

/* thread safe protector */
public:
    mutable Mutex m_lockAllocRaws[MFLL_MAX_FRAMES];
    mutable Mutex m_lockAllocQYuvs[MFLL_MAX_FRAMES];
    mutable Mutex m_lockAllocMemc[MFLL_MAX_FRAMES];
    mutable Mutex m_lockAllocYuvBase;
    mutable Mutex m_lockAllocYuvGolden;
    mutable Mutex m_lockAllocYuvWorking;
    mutable Mutex m_lockAllocYuvMcWorking;
    mutable Mutex m_lockAllocWeighting[2];

/* implementations */
public:
    virtual enum MfllErr init(const MfllConfig_t &cfg);
    virtual std::deque<IMetadata*> getMetaHal()
    {
        return m_vMetaHal;
    }
    virtual enum MfllErr doMfll();
    virtual enum MfllErr doCancel();
    virtual unsigned int getBlendFrameNum(void) { return m_frameBlend; }
    virtual unsigned int getCaptureFrameNum(void) { return m_frameCapture; }
    virtual unsigned int getFrameBlendedNum(void) { return m_blendedCount; }
    virtual unsigned int getFrameCapturedNum(void) { return m_caputredCount; }
    virtual unsigned int getMemcInstanceNum(void) { return m_memcInstanceNum; }
    virtual unsigned int getIndexByNewIndex(const unsigned int &newIndex);
    virtual int getSensorId(void);
    virtual int getCurrentIso(void) const;
    virtual int getCurrentShutter(void) const;
    virtual unsigned int getVersion(void);
    virtual std::string getVersionString(void);
    virtual bool isShooted(void);
    virtual bool isFullSizeMc(void);
    virtual enum MfllErr registerEventListener(const sp<IMfllEventListener> &e);
    virtual enum MfllErr registerEventListenerNoLock(const sp<IMfllEventListener> &e, int useInsert = 0);
    virtual enum MfllErr removeEventListener(IMfllEventListener *e);
    virtual enum MfllErr setBypassOption(const MfllBypassOption_t &b);
    virtual enum MfllErr setBypassOptionAsSingleFrame();
    virtual enum MfllErr setCaptureResolution(unsigned int width, unsigned int height);
    virtual enum MfllErr setCapturer(const sp<IMfllCapturer> &capturer);
    virtual enum MfllErr setMfb(const sp<IMfllMfb> &mfb);
    virtual enum MfllErr setNvramProvider(const sp<IMfllNvram> &nvramProvider);
    virtual enum MfllErr setPostviewBuffer(
            const sp<IMfllImageBuffer>& buffer,
            const MfllRect_t& srcCropRgn);
    virtual enum MfllErr setThumbnailBuffer(
            const sp<IMfllImageBuffer>& buffer,
            const MfllRect_t& srcCropRgn);
    virtual enum MfllErr setMiddlewareInfo(const MfllMiddlewareInfo_t&) { return MfllErr_NotImplemented; }
    virtual sp<IMfllMfb> getMfb(void);
    virtual sp<IMfllCapturer> getCapturer(void);
    virtual sp<IMfllImageBuffer> retrieveBuffer(const enum MfllBuffer &s, int index = 0);
    virtual enum MfllErr releaseBuffer(const enum MfllBuffer &s, int index = 0);
    virtual enum MfllErr updateExifInfo(unsigned int key, uint32_t value);
    virtual const sp<IMfllExifInfo>& getExifContainer();
    virtual MfllMiddlewareInfo_t getMiddlewareInfo() const { return MfllMiddlewareInfo_t(); }
    virtual enum MfllErr setMixingBuffer(const MixYuvBufferType& type, const sp<IMfllImageBuffer>& buffer) {return MfllErr_NotImplemented; };
    virtual MixYuvBufferType getMixingBufferType() { return MixYuvType_Base; };
    virtual enum MfllErr setSyncPrivateData(const std::deque<void*>& dataset);
    virtual enum MfllErr setInputYuvFormat(const InputYuvFmt) { return MfllErr_NotImplemented; }
    virtual InputYuvFmt getInputYuvFormat() const { return InputYuvFmt_Default; }


/* MFLL operations, user should not invoke these functions directly */
public:

    /**
     *  We make all the jobs with the same prototype: void*. Because we want to make some job threads
     *  with job queue to execute these jobs.
     */
    #define JOB_VOID intptr_t  __arg __attribute__((unused))
    /* doCapture will output N full size RAW and N 1/4 Size YUV */
    enum MfllErr doCapture(JOB_VOID);
    /* Wait until all captured buffers are ready, using BSS to pick the best base frame */
    enum MfllErr doBss(intptr_t intptrframeCount);
    /* After the base frame has been selected, we can encode it to YUV */
    enum MfllErr doEncodeYuvBase(JOB_VOID);
    enum MfllErr doEncodeYuvGolden(JOB_VOID);
    enum MfllErr doMotionEstimation(void *void_index);
    enum MfllErr doMotionCompensation(void *void_index);
    enum MfllErr doBlending(void *void_index);
    enum MfllErr doMixing(JOB_VOID);
    enum MfllErr doNoiseReduction(JOB_VOID);
    enum MfllErr doCreatePostview(JOB_VOID);
    enum MfllErr doEncodeThumbnail(JOB_VOID);
    enum MfllErr doEncodeJpeg(JOB_VOID);

/* Buffer allocations */
    enum MfllErr doAllocRawBuffer(void *void_index);
    enum MfllErr doAllocQyuvBuffer(void *void_index);
    enum MfllErr doAllocYuvBase(JOB_VOID);
    enum MfllErr doAllocYuvGolden(JOB_VOID);
    enum MfllErr doAllocYuvWorking(JOB_VOID);
    enum MfllErr doAllocYuvMcWorking(JOB_VOID);
    enum MfllErr doAllocWeighting(void *void_index);
    enum MfllErr doAllocMemcWorking(void *void_index);

public:
    /**
     *  To create a thread within MfllCore as the argument, and also increase "this"(MfllCore*) lifetime,
     *  caller must to invoke MfllCore::decStrong(void*) to decrease lifetime counter when the thread
     *  goes to the end.
     */
    enum MfllErr createThread(pthread_t *pThread, void *(*routine)(void*), bool bDistach = false);
    enum MfllErr joinThread(pthread_t *pThread);

/* MFLL sync object operations */
public:
    static void lockSyncObject(MfllSyncObj_t *pSyncObj);
    static void unlockSyncObject(MfllSyncObj_t *pSyncObj);
    static void syncWaitTrigger(MfllSyncObj_t *pSyncObj);
    static void syncAnnounceTrigger(MfllSyncObj_t *pSyncObj);
    static void syncWaitDone(MfllSyncObj_t *pSyncObj);
    static void syncAnnounceDone(MfllSyncObj_t *pSyncObj);

/* static methods */
public:
    /**
     *  To create a MFLL default configuration
     *  @return             - A dynamically created struct MfllConfig object.
     *  @notice             - Caller has responsibility to release handle.
     */
    static MfllConfig_t* createMfllConfig(void);

}; /* class MfllCore */
}; /* namespace mfll */

#endif /* __MFLLCORE_H__ */
