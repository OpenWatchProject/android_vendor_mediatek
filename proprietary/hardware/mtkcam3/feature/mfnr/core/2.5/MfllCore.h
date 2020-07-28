/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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
#include "MfllFeatureFeed.h"
#include "MfllFeaturePerf.h"

/* thread-safe access ImageBufferPack */
#include "ImageBufferPack.h"

/* image buffer queue mechanism */
#include "IMfllImageBufferQueue.h"

/* standard libs */
#include <cstdint>
#include <semaphore.h>
#include <pthread.h> // pthread_t, pthread_mutex_t
#include <vector>
#include <string>
#include <future>
#include <atomic>

/* Android frameworks */
#include <utils/RefBase.h> // android::RefBase
#include <utils/Mutex.h> // android::Mutex
#include <utils/RWLock.h> // android::RWLock

using android::sp; // use android::sp as sp
using android::Mutex;
using android::RWLock; // use android::RWLock as sp
using std::vector; // use std::vector as vector

/* Image format definition */
#define MFLL_IMAGE_FORMAT_RAW            ImageFormat_Raw10
// due to imgo not support ImageFormat Y8/NV12/NV21,
// use the Nv16(2 planes) to tranform 2 palnes YUV422,
// and query the first palne Y for Memc algo.
#define MFLL_IMAGE_FORMAT_YUV_Q             getInputYuvFmtYuvQ()
#define MFLL_IMAGE_FORMAT_YUV_FULL          getInputYuvFmtYuvFull()
#define MFLL_IMAGE_FORMAT_YUV_BASE          getInputYuvFmtYuvBase()
#define MFLL_IMAGE_FORMAT_YUV_GOLDEN        getInputYuvFmtYuvGolden()
#define MFLL_IMAGE_FORMAT_YUV_MC_WORKING    getInputYuvFmtYuvMcWorking()
#define MFLL_IMAGE_FORMAT_YUV_WORKING       getInputYuvFmtYuvWorking()
#define MFLL_IMAGE_FORMAT_WEIGHTING         getInputYuvFmtWeightingTable()
#define MFLL_IMAGE_FORMAT_MEMC_WORKING      getInputYuvFmtMemcWorking()


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
    sp<MfllFeatureFeed>     m_spFeed;
    sp<MfllFeaturePerf>     m_spPerfService;


/* attributes */
public:
    bool            m_bShooted;
    pthread_mutex_t m_mutexShoot; // protect shoot, makes MfllCore thread-safe
    MfllConfig_t    m_cfg;
    int             m_sensorId; // sensor Id
    unsigned int    m_frameCapture; // number of frame to capture
    unsigned int    m_caputredCount; // REAL captured frame number
    std::atomic<unsigned int>    m_frameBlend; // blending frame amount
    std::atomic<unsigned int>    m_blendedCount; // count frame blended in MFB stage
    std::atomic<unsigned int>    m_bAsSingleFrame; // represents capture as single frame
    int             m_iso;
    int             m_exposure;
    unsigned int    m_memcInstanceNum; // represents how many MfllMemc to be create
    enum MfllMode   m_shotMode; // shotMode
    enum MemcMode   m_memcMode;
    enum RwbMode    m_rwbMode;
    enum MrpMode    m_mrpMode;
    enum NoiseReductionType m_postNrType;
    unsigned int    m_width; // for mfll processed yuv size (all yuv buffer before mix-out)
    unsigned int    m_height; // for mfll processed yuv size (all yuv buffer before mix-out)
    unsigned int    m_imgoWidth; // for full raw size and full size
    unsigned int    m_imgoHeight; // for full raw size and full size
    unsigned int    m_rrzoWidth; // for rrzo size
    unsigned int    m_rrzoHeight; // for rrzo size
    unsigned int    m_qwidth; // for 1/4 size YUV
    unsigned int    m_qheight; // for 1//4 size YUV
    int             m_isFullSizeMc; // represents is using full size MC
    bool            m_byPassGmv;
    int             m_uniqueKey; //MTK_PIPELINE_UNIQUE_KEY of frame 0

    /* Global motion vector retrieved by IMfllCapturer */
    unsigned int    m_bssIndex[MFLL_MAX_FRAMES];
    vector<int64_t> m_timestampSync; //for each setSyncPrivateData
    MfllMotionVector_t  m_globalMv[MFLL_MAX_FRAMES];

    /* Debug information of MfllCore */
    MfllCoreDbgInfo_t   m_dbgInfoCore;

    /* Middleware's frame information */
    std::atomic<MfllMiddlewareInfo_t> m_middlewareInfo;

    /* Mixing YUV frame buffer type */
    MixYuvBufferType m_mixYuvBufferType;
    mutable std::mutex            m_mixYuvBufferTypeMx;
    std::condition_variable       m_mixYuvBufferTypeCond;

    std::atomic<bool> m_bThumbnailDone;

    std::atomic<bool> m_bDoDownscale; // represents is using downscale denoise flow
    int m_downscaleRatio; // downscale divide factor (e.g.: 1 --> Res/1 = 100%, 2 --> Res/2 = 50%)

    std::deque<IMetadata*>      m_vMetaHal;
/**
 *  Operation locks, these mthods may be invoked in different threads, we have to
 *  make them thread-safe.
 */
protected:
    std::mutex          m_opLockDoAllocRawBuffer[MFLL_MAX_FRAMES];
    std::mutex          m_opLockDoAllocRrzoBuffer[MFLL_MAX_FRAMES];
    std::mutex          m_opLockDoAllocQYuvBuffer[MFLL_MAX_FRAMES];
    std::mutex          m_opLockDoAllocYuvBase;
    std::mutex          m_opLockDoAllocYuvGolden;
    std::mutex          m_opLockDoAllocYuvWorking;
    std::mutex          m_opLockDoAllocYuvMcWorking;
    std::mutex          m_opLockDoAllocWeighting[2];
    std::mutex          m_opLockDoAllocMemcWorking[MFLL_MAX_FRAMES];


/* public attributes */
public:
    /*  Bypass option can let you bypass any operation but conditions are still necessary */
    MfllBypassOption_t  m_bypass; // MfllBypassOption_t

    /* used image buffers */
    /* phiscal buffer, costs memory */
    ImageBufferPack     m_imgRaws[MFLL_MAX_FRAMES];
    ImageBufferPack     m_imgRrzos[MFLL_MAX_FRAMES];
    ImageBufferPack     m_imgYuvs[MFLL_MAX_FRAMES]; // may save full size or 1/4 size yuv buffers
    ImageBufferPack     m_imgQYuvs[MFLL_MAX_FRAMES]; // saves 1/4 size yuv for ME if using full size MC
    ImageBufferPack     m_imgYuvMcWorking; // for Motion Compensation working
    ImageBufferPack     m_imgYuvPostview;
    ImageBufferPack     m_imgYuvThumbnail;
    ImageBufferPack     m_imgYuvBase; // for blending/mixing stage
    ImageBufferPack     m_imgYuvGolden; // for blending/mixing stage
    ImageBufferPack     m_imgYuvBlended; // for blending/mixing stage
    ImageBufferPack     m_imgWeighting[2]; // ping-pong buffers, takes 2
    ImageBufferPack     m_imgMemc[MFLL_MAX_FRAMES]; // working buffer for MEME
    ImageBufferPack     m_imgYuvMixedWorking; // for mixing stage by working buffer

    MfllRect_t           m_postviewCropRgn; // zoom crop region, if the size is 0, do not apply crop
    MfllRect_t           m_thumbnailCropRgn; // zoom crop region, if the size is 0, do not apply crop

    /**
     *  During blending, MFLL will use two buffers as ping-pong buffers, e.g.:
     *  1st: imgBase    + imgRef = imgBlended    (ping)
     *  2nd: imgBlended + imgRef = imgBase       (pong)
     *  3rd: imgBase    + imgRef = imgBlended    (ping)
     *  ...
     *
     *  Hence here we use these pointers to indicate the REAL meanful buffers.
     */
    ImageBufferPack*    m_ptrImgYuvBase; // indicates to full size base YUV
    ImageBufferPack*    m_ptrImgYuvRef; // indicates to any size reference YUV
    ImageBufferPack*    m_ptrImgYuvGolden; // indicates to golden YUV (noraml YUV)
    ImageBufferPack*    m_ptrImgYuvBlended; // indicates to blended YUV (same size with YUV base)
    ImageBufferPack*    m_ptrImgYuvMixed; // indicates to mixed YUV
    ImageBufferPack*    m_ptrImgYuvMixedWorking; // indicates to mixing YUV by working buffer
    ImageBufferPack*    m_ptrImgYuvThumbnail; // indicates to thumbnail YUV
    ImageBufferPack*    m_ptrImgWeightingIn; // indicates to input of weighting table during blending
    ImageBufferPack*    m_ptrImgWeightingOut; // indicates to output of weighting table during blending
    ImageBufferPack*    m_ptrImgWeightingFinal; // indicates to final weighting for mixing

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
    MfllSyncObj_t m_syncAllocYuvQ[MFLL_MAX_FRAMES]; // annonced done while a single QYUV buffer is allocated
    MfllSyncObj_t m_syncCapturedRaw[MFLL_MAX_FRAMES]; // annonced done while a single RAW is captured
    MfllSyncObj_t m_syncCapturedYuvQ[MFLL_MAX_FRAMES]; // annonced done while a single QYUV is captured
    MfllSyncObj_t m_syncQueueYuvCapture[MFLL_MAX_FRAMES]; // annonced done while a new data is inputed
    MfllSyncObj_t m_syncEncodeYuvBase;
    MfllSyncObj_t m_syncEncodeYuvGolden;
    MfllSyncObj_t m_syncBss;
    MfllSyncObj_t m_syncMotionEstimation[MFLL_MAX_FRAMES];
    MfllSyncObj_t m_syncMotionCompensation[MFLL_MAX_FRAMES];
    MfllSyncObj_t m_syncBlending[MFLL_MAX_FRAMES];
    MfllSyncObj_t m_syncMixing;


/* implementations */
public:
    virtual enum MfllErr init(const MfllConfig_t &cfg);
    virtual std::deque<IMetadata*> getMetaHal()
    {
        return m_vMetaHal;
    }
    virtual enum MfllErr doMfll();
    virtual enum MfllErr doCancel();
    virtual unsigned int getBlendFrameNum(void) { return m_frameBlend.load(); }
    virtual unsigned int getCaptureFrameNum(void) { return m_frameCapture; }
    virtual unsigned int getFrameBlendedNum(void) { return m_blendedCount.load(); }
    virtual unsigned int getFrameCapturedNum(void) { return m_caputredCount; }
    virtual unsigned int getMemcInstanceNum(void) { return m_memcInstanceNum; }
    virtual unsigned int getIndexByNewIndex(const unsigned int &newIndex);
    virtual int getSensorId(void);
    virtual int getCurrentIso(void) const;
    virtual int getCurrentShutter(void) const;
    virtual unsigned int getVersion(void);
    virtual std::string getVersionString(void);
    virtual bool isShooted(void);
    virtual bool isThumbnailDone(void) { return m_bThumbnailDone.load(); }
    virtual bool isFullSizeMc(void);
    virtual enum MfllErr queueYuvCapture(int32_t requestNo, sp<IMfllImageBuffer>& sourceImg, sp<IMfllImageBuffer>& quarterImg, MfllMotionVector mv);
    virtual enum MfllErr setQueueYuvFramesCounts(uint32_t nums);
    virtual enum MfllErr registerEventListener(const sp<IMfllEventListener> &e);
    virtual enum MfllErr registerEventListenerNoLock(const sp<IMfllEventListener> &e, int useInsert = 0);
    virtual enum MfllErr removeEventListener(IMfllEventListener *e);
    virtual enum MfllErr setBypassOption(const MfllBypassOption_t &b);
    virtual enum MfllErr setBypassOptionAsSingleFrame();
    virtual enum MfllErr setCaptureResolution(unsigned int width, unsigned int height);
    virtual enum MfllErr setRrzoResolution(unsigned int width, unsigned int height);
    virtual enum MfllErr setCapturer(const sp<IMfllCapturer> &capturer);
    virtual enum MfllErr setMfb(const sp<IMfllMfb> &mfb);
    virtual enum MfllErr setNvramProvider(const sp<IMfllNvram> &nvramProvider);
    virtual enum MfllErr setPostviewBuffer(
            const sp<IMfllImageBuffer>& buffer,
            const MfllRect_t& srcCropRgn);
    virtual enum MfllErr setThumbnailBuffer(
            const sp<IMfllImageBuffer>& buffer,
            const MfllRect_t& srcCropRgn);
    virtual enum MfllErr setMiddlewareInfo(const MfllMiddlewareInfo_t& info);
    virtual sp<IMfllMfb> getMfb(void);
    virtual sp<IMfllCapturer> getCapturer(void);
    virtual sp<IMfllImageBuffer> retrieveBuffer(const enum MfllBuffer &s, int index = 0);
    virtual enum MfllErr releaseBuffer(const enum MfllBuffer &s, int index = 0);
    virtual enum MfllErr updateExifInfo(unsigned int key, uint32_t value);
    virtual const sp<IMfllExifInfo>& getExifContainer();
    virtual MfllMiddlewareInfo_t getMiddlewareInfo() const;

    virtual enum MfllErr setMixingBuffer(const MixYuvBufferType& type, const sp<IMfllImageBuffer>& buffer);
    virtual MixYuvBufferType getMixingBufferType();
    virtual enum MfllErr setInputYuvFormat(const InputYuvFmt);
    virtual InputYuvFmt getInputYuvFormat() const;
    virtual enum MfllErr getBlendFrameSize(int &w, int &h);

    virtual enum MfllErr setSyncPrivateData(const std::deque<void*>& dataset);

    virtual void setDownscaleInfo(bool downscale_enabled, int downscale_ratio, int /*downscale_dividend = 0*/, int /*downscale_divisor = 0*/)
                 {
                     m_bDoDownscale = downscale_enabled;
                     m_downscaleRatio = downscale_ratio;
                     _updateSingeFrameBypassConfigLocked();
                 }
    virtual bool isSingleFrame(void){ return m_bAsSingleFrame.load(); }
    virtual bool isDownscale(void) { return m_bDoDownscale.load(); }
    virtual int  getDownscaleRatio(void) { return m_downscaleRatio; }

/* helper function */
public:
    /* Check the current condition, if we need resized Y8 buffers */
    virtual bool _isNeedResizedYBuffer(void);
    /* helper function to update bypass configuration */
    virtual void _updateSingeFrameBypassConfigLocked(void);



/* threads, we use std::async to begin threads */
public:
    struct FuturePack
    {
        std::future<void>   fu;
        std::mutex          mx;
        inline void wait() { if (fu.valid()) fu.wait(); }
    };

public:
    virtual void tellsFutureAllocateMemory();
    virtual void tellsFuturePhase1();
    virtual void tellsFuturePhase2();
    virtual void tellsFutureMemc();

protected:
    FuturePack m_futureMemory;
    FuturePack m_futurePhase1;
    FuturePack m_futurePhase2;
    FuturePack m_futureMemc;

/* Image Buffer Queue mechanism */
protected:
    struct {
        std::shared_ptr<IMfllImageBufferQueue> fsize;
        std::shared_ptr<IMfllImageBufferQueue> qsize;
        std::shared_ptr<IMfllImageBufferQueue> weighting;
        std::shared_ptr<IMfllImageBufferQueue> algo;
    } m_imgQueue;

    virtual void updateImageBufferQueueLimit();

/* internal image buffer format scenarios */
public:
    std::atomic<InputYuvFmt> m_internalYuvFmt;

    virtual ImageFormat getInputYuvFmtYuvQ() const;
    virtual ImageFormat getInputYuvFmtYuvFull() const;
    virtual ImageFormat getInputYuvFmtYuvBase() const;
    virtual ImageFormat getInputYuvFmtYuvGolden() const;
    virtual ImageFormat getInputYuvFmtYuvMcWorking() const;
    virtual ImageFormat getInputYuvFmtYuvWorking() const;
    virtual ImageFormat getInputYuvFmtWeightingTable() const;
    virtual ImageFormat getInputYuvFmtMemcWorking() const;

/* MFLL operations, user should not invoke these functions directly */
public:

    /**
     *  We make all the jobs with the same prototype: void*. Because we want to make some job threads
     *  with job queue to execute these jobs.
     */
    #define JOB_VOID intptr_t  __arg __attribute__((unused))
    /* doCapture will output N full size RAW and N 1/4 Size YUV */
    virtual enum MfllErr doCapture(JOB_VOID);
    /* Wait until all captured buffers are ready, using BSS to pick the best base frame */
    virtual enum MfllErr doBss(intptr_t intptrframeCount);
    /* After the base frame has been selected, we can encode it to YUV */
    virtual enum MfllErr doEncodeYuvBase(JOB_VOID);
    virtual enum MfllErr doEncodeYuvGolden(JOB_VOID);
    virtual enum MfllErr doEncodeQYuv(void *void_index);
    virtual enum MfllErr doMotionEstimation(void *void_index);
    virtual enum MfllErr doMotionCompensation(void *void_index);
    virtual enum MfllErr doBlending(void *void_index);
    virtual enum MfllErr doMixing(JOB_VOID);


public:
/* Buffer allocations */
    virtual enum MfllErr doAllocRawBuffer(void *void_index);
    virtual enum MfllErr doAllocRrzoBuffer(void *void_index);
    virtual enum MfllErr doAllocQyuvBuffer(void *void_index);
    virtual enum MfllErr doAllocYuvBase(JOB_VOID);
    virtual enum MfllErr doAllocYuvGolden(JOB_VOID);
    virtual enum MfllErr doAllocYuvWorking(JOB_VOID);
    virtual enum MfllErr doAllocYuvMcWorking(JOB_VOID);
    virtual enum MfllErr doAllocWeighting(void *void_index);
    virtual enum MfllErr doAllocMemcWorking(void *void_index);


/**
 *  MFLL implementations, we separate interfaces and implementations for derived
 *  features
 */
protected:
    // interfaces
    virtual enum MfllErr do_Init(const MfllConfig_t &cfg);
    // internal
    virtual enum MfllErr do_Capture(JOB_VOID);
    virtual enum MfllErr do_CaptureSingle(void* void_index);
    virtual enum MfllErr do_Bss(intptr_t intptrframeCount);
    virtual enum MfllErr do_EncodeQYuv(void* void_index);
    virtual enum MfllErr do_EncodeYuvBase(JOB_VOID);
    virtual enum MfllErr do_EncodeYuvGolden(JOB_VOID);
    virtual enum MfllErr do_EncodeYuvGoldenDownscale(JOB_VOID);
    virtual enum MfllErr do_MotionEstimation(void* void_index);
    virtual enum MfllErr do_MotionCompensation(void* void_index);
    virtual enum MfllErr do_Blending(void* void_index);
    virtual enum MfllErr do_Mixing(JOB_VOID);
    // memory related
    virtual enum MfllErr do_AllocRawBuffer(void* void_index);
    virtual enum MfllErr do_AllocRrzoBuffer(void* void_index);
    virtual enum MfllErr do_AllocQyuvBuffer(void* void_index);
    virtual enum MfllErr do_AllocYuvBase(JOB_VOID);
    virtual enum MfllErr do_AllocYuvGolden(JOB_VOID);
    virtual enum MfllErr do_AllocYuvWorking(JOB_VOID);
    virtual enum MfllErr do_AllocYuvMcWorking(JOB_VOID);
    virtual enum MfllErr do_AllocWeighting(void* void_index);
    virtual enum MfllErr do_AllocMemcWorking(void* void_index);


/* MFLL sync object operations */
public:
    static void lockSyncObject(MfllSyncObj_t *pSyncObj);
    static void unlockSyncObject(MfllSyncObj_t *pSyncObj);
    static void syncWaitTrigger(MfllSyncObj_t *pSyncObj);
    static void syncAnnounceTrigger(MfllSyncObj_t *pSyncObj);
    static void syncWaitDone(MfllSyncObj_t *pSyncObj);
    static void syncAnnounceDone(MfllSyncObj_t *pSyncObj);


}; /* class MfllCore */
}; /* namespace mfll */
#endif /* __MFLLCORE_H__ */
