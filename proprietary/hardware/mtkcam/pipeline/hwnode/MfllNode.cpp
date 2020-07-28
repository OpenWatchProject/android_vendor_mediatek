
#define LOG_TAG "MtkCam/MfllNode"

#include <mtkcam/utils/std/Log.h>
#include "MFCNodeImp.h"
#include "hwnode_utilities.h"

#include <mtkcam/pipeline/hwnode/MfllNode.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>

#include <mtkcam/utils/metastore/IMetadataProvider.h>

#include <mtkcam/drv/IHalSensor.h>

/* Exif info */
#include <mtkcam/utils/exif/DebugExifUtils.h>

/* Mfll related */
#include <mtkcam/feature/mfnr/MfllLog.h>
#include <mtkcam/feature/mfnr/MfllProperty.h>
#include <mtkcam/feature/mfnr/IMfllCore.h>
#include <mtkcam/feature/mfnr/IMfllImageBuffer.h>
#include <mtkcam/feature/mfnr/IMfllCapturer.h>
#include <mtkcam/feature/mfnr/IMfllMfb.h>
#include <mtkcam/feature/mfnr/IMfllStrategy.h>
#include <mtkcam/feature/mfnr/IMfllEventListener.h>
#include <mtkcam/feature/mfnr/IMfllExifInfo.h>
#include <mtkcam/feature/mfnr/IMfllGyro.h>

#include <sys/prctl.h>
#include <sys/resource.h>
#include <cutils/properties.h>

// STL
#include <vector>
#include <queue>
#include <future>
#include <chrono>
#include <thread>

// STL from middleware
#include <mtkcam/utils/std/StlUtils.h>

#include <mtkcam/drv/iopipe/SImager/IImageTransform.h> // IImageTransform

/**
 *  Delay Frame Count
 *
 *  While using ZSD+AIS mode, the exposure and ISO may be changed during capturing.
 *  And after captured, exposure/ISO should be restored and it costs N frames to
 *  make AE stable. To avoid flash screen of preview, MfllNode will ask N frames
 *  as the original exposure/iso by querying from 3A manager. If query is failed,
 *  MfllNode use this value as default delay frame count.
 */
#define MFLLNODE_ZSDAIS_DELAY_FRAME_COUNT 3

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] @@@warning:@@@" fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] @@@error:@@@" fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)

#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils::Sync;
using namespace NS3Av3;
using namespace std;
using namespace NSIoPipe;
using namespace NSIoPipe::NSPostProc;
using namespace NSCam::NSIoPipe::NSSImager;

using mfll::IMfllCore;
using mfll::IMfllImageBuffer;
using mfll::IMfllCapturer;
using mfll::IMfllMfb;
using mfll::MfllErr;
using namespace mfll;

/**
 *  Since lots work has been done by MFCNodeImp, MfllNode only takes care about
 *  the necessary implementation that MFNR needs to do and interfaces from MFCNodeImp
 */
class MfllNodeImp final : public MFCNodeImp /* Using MFCNodeImp as common parts */
{
public:
    MfllNodeImp();

/**
 *  Implementations of MFCNode
 */
public:
    MERROR init(InitParams const &rParams);
    MERROR uninit();
    MERROR config(const ConfigParams &rParams);
    MERROR flush(void);
    MERROR queue(sp<IPipelineFrame> pFrame);
    MVOID  waitForRequestDrained(void);
    MVOID  releaseResource(void);

// General methods
public:
    inline int getCaptureFrameNum(void) { return mCaptureFrameNum; }
    inline int getBlendFrameNum(void) { return mBlendFrameNum; }
    inline int getDroppedFrameNum(void) { return mDroppedFrameNum; }

    inline std::shared_ptr<CRequestFrame>& mainRequestFrame(void)
    { return m_mainRequestFrame; }

    inline CQueue<std::shared_ptr<CRequestFrame>>& requestFrameQueue()
    { return m_vUsedRequestFrame; }

    void releaseCRequestFrame(const int &index)
    {
        if (index >= (int)m_vUsedRequestFrame.size()) {
            MY_LOGW("release CRequestFrame(%d) failed, frame not exist", index);
            return;
        }
        /* partial release RAW */
        if (m_vUsedRequestFrame[index]) {
            m_vUsedRequestFrame[index]->releaseImage(CRequestFrame::eRequestImg_FullRaw);
            m_vUsedRequestFrame[index]->releaseImage(CRequestFrame::eRequestImg_ResizedRaw);
            m_vUsedRequestFrame[index] = 0;
        }
    };

    inline enum MfllMode&           shotMode()      { return m_shotMode; }
    inline enum NoiseReductionType& postNrType()    { return m_postNrType; }

    /* makes debug exif */
    void makesDebugInfo();

public:
    /* init MFNR core and create memory chunks first */
    int doInitMfnr();

    /* to fire MFNR process once when the first IPipelineFrame got */
    int shootMfnr();

    /* process Multi Frame Noise Reduction */
    int doMultiFrameNoiseReduction();

    /* implementation of getCaptureInfo */
    static int getCaptureInfo(
            int mode,
            int sensorId,
            MfllStrategyConfig_t &capConfig,
            bool bUseCached = false);

    /* get cached capture information, which has been cached by MfllNodeImp::getCaptureInfo */
    static int getCaptureInfoCached(MfllStrategyConfig_t &cfg);

    /* set cached capture information. */
    static int setCaptureInfoCached(const MfllStrategyConfig_t &cfg);

/* Attributes */
private:
    int m_widthCapture;
    int m_heightCapture;
    int mCaptureFrameNum; // represent capture frame number
    int mDroppedFrameNum; // represent the number that frame got but dropped
    int mBlendFrameNum;
    CQueue<std::shared_ptr<CRequestFrame>> m_vUsedRequestFrame;

    /* if request JPEG debug info */
    int m_isRequestJpegExif;

    /* Saves the first CRequestFrame as the mainRequestFrame */
    std::shared_ptr<CRequestFrame> m_mainRequestFrame;

    /* MFNR thread */
    std::future<int> m_futureMFNR;

    /* MFNR related objects */
    sp<IMfllCore>       mpMfllCore;
    sp<IMfllCapturer>   mpMfllCapturer;
    sp<IMfllMfb>        mpMfb;
    sp<IMfllGyro>       mpMfllGyro;

    /* gyro info */
    std::deque< std::deque<IMfllGyro::MfllGyroInfo> >
        mGyroInfos;

    /* JPEG orientation */
    MINT32 mOrientation;

    /* shot mode */
    enum MfllMode m_shotMode;
    enum NoiseReductionType m_postNrType;

    /* uniqueKey */
    MINT32 m_uniqueKey;
};

class ListenerImp : public IMfllEventListener {
public:
    ListenerImp(MfllNodeImp *node)
    : node(node)
    {}

public:
    void onEvent(enum EventType t, MfllEventStatus_t &status, void *mfllCore, void *param1 = NULL, void *param2 __attribute__((unused)) = NULL)
    {
        IMfllCore *c = reinterpret_cast<IMfllCore*>(mfllCore);

        /* get available frame count */
        const int frameCount = node->getCaptureFrameNum() - node->getDroppedFrameNum();

        auto ignoreCapture = [this, &param1, &status, frameCount]()->void {
            int index = (int)(long long)param1;
            /* frame is not enought for encoding */
            if (index >= frameCount) {
                MY_LOGE("frame %d has been ignored for encoding YUV, MEMC, blend", index);
                status.ignore = 1;
            }
        };

        auto ignoreBss = [this, &c, &status, frameCount, param1]()->void {
            if (frameCount <= 1) {
                MY_LOGD("frames size(%d) <= 1, ignore BSS", frameCount);
                status.ignore = 1;
                return;
            }

            /* update bss frame count to the number of available frames */
            int *pBssFrameCount = reinterpret_cast<int*>(param1);
            *pBssFrameCount = frameCount;
        };

        switch (t) {
        case EventType_CaptureYuvQ:
        case EventType_CaptureEis:
        case EventType_MotionEstimation:
        case EventType_MotionCompensation:
        case EventType_Blending:
            ignoreCapture();
            break;
        case EventType_Bss:
            ignoreBss();
            break;
        default: break;
        }// switch;
    };
    void doneEvent(enum EventType t, MfllEventStatus_t &status, void *mfllCore, void *param1 __attribute__((unused)) = NULL, void *param2 __attribute__((unused)) = NULL)
    {
        /* get available frame count */
        const int frameCount = node->getCaptureFrameNum() - node->getDroppedFrameNum();
        IMfllCore *c = reinterpret_cast<IMfllCore*>(mfllCore);

        switch (t) {
        case EventType_Bss:
            if (frameCount <= 0) {
                return;
            }
            else {
                auto baseRawIdx = static_cast<int>(c->getIndexByNewIndex(0));
                /* release all RAWs except base RAW */
                for (int i = 0; i < frameCount; i++) {
                    if (baseRawIdx == i)
                        continue;

                    node->releaseCRequestFrame(i);
                }
            }
            break;

        case EventType_EncodeYuvBase:
            /* to remove postview YUV ownership of MfllCore first */
            c->releaseBuffer(mfll::MfllBuffer_PostviewYuv);

            if (frameCount <= 0) {
                /* no frame, no result */
                return;
            }
            else {
                if (node->mainRequestFrame().get() == NULL) {
                    MY_LOGE("No main request frame ...");
                    return;
                }

                /**
                 *  LABEL_EARLY_POSTVIEW:
                 *  do postview YUV here if base YUV encoding is great, or do postview at the
                 *  end of MFNR post-processing
                 *
                 *  @sa LABEL_POSTVIEW
                 */
                if (status.ignore == 0 && status.err == MfllErr_Ok) {
                    auto t1 = std::async(std::launch::async, [this, c]()->void{
                            if (node->mainRequestFrame().get() == NULL)
                                return;
                            node->mainRequestFrame()->releaseImage(CRequestFrame::eRequestImg_YuvPostview);
                        });
                    async_threads.push_back(std::move(t1));
                }
            }
            break;

        case EventType_EncodeYuvGolden:
            if (frameCount <= 0) {
                /* no frames, no result */
                return;
            }
            else {
                /* give result */
                if (node->mainRequestFrame().get() == NULL) {
                    MY_LOGE("No main request frame for update result metadata");
                    return;
                }

                IMetadata *pMainResult = node->mainRequestFrame()->metadata(CRequestFrame::eRequestMeta_OutHalResult);
                IMetadata *pMainResultApp = node->mainRequestFrame()->metadata(CRequestFrame::eRequestMeta_OutAppResult);
                IMetadata *pMfllResult = node->requestFrameQueue()[c->getIndexByNewIndex(0)]->metadata(CRequestFrame::eRequestMeta_InHalP1);
                IMetadata *pMfllResultApp = node->requestFrameQueue()[c->getIndexByNewIndex(0)]->metadata(CRequestFrame::eRequestMeta_InAppRequest);

                if (pMainResult == NULL) {
                    MY_LOGE("main result meta is NULL");
                }
                if (pMainResultApp == NULL) {
                    MY_LOGE("main app result meta is NULL");
                }
                if (pMfllResult == NULL) {
                    MY_LOGE("mfll result meta is NULL");
                }
                if (pMfllResultApp == NULL) {
                    MY_LOGE("mfll app result meta is NULL");
                }
                if (pMainResult == NULL || pMfllResult == NULL || pMainResultApp == NULL || pMfllResultApp == NULL) {
                    /* do nothing if no result */
                }
                else {
                    /**
                     *  Get the frame index after BSS applied, and update metadata
                     *  result from P1 result to Mfll result, and pass to the next node
                     */
                    *pMainResult = *pMfllResult;
                    *pMainResultApp = *pMfllResultApp;
                }

                /* release all RAWs */
                for (size_t i = 0; i < (size_t)frameCount; i++) {
                    node->releaseCRequestFrame(i);
                }
            }
            break;
        default:
            break;
        };
    };

    vector<enum EventType> getListenedEventTypes(void)
    {
        vector<enum EventType> m;
        #define LISTEN(x) m.push_back(x)
        LISTEN(EventType_Bss); // to release 1~N RAW buffers
        LISTEN(EventType_EncodeYuvBase);
        LISTEN(EventType_EncodeYuvGolden); // to release the base (index is 0) RAW buffer
        LISTEN(EventType_CaptureYuvQ);
        LISTEN(EventType_CaptureEis);
        LISTEN(EventType_MotionEstimation);
        LISTEN(EventType_MotionCompensation);
        LISTEN(EventType_Blending);
        #undef LISTEN
        return m;
    };

private:
    MfllNodeImp *node;
    std::deque< std::future<void> > async_threads;
};

static int getCurrent3ALocked(int sensorId, CaptureParam_T &capParam, bool bIsAis, bool bUseCached = false)
{
    static Mutex mx;
    static CaptureParam_T cachedCapParam;
    Mutex::Autolock _l(&mx);
    if (bUseCached == false) {
        std::shared_ptr<IHal3A> hal3a(
                MAKE_Hal3A(sensorId, LOG_TAG),
                [](auto *p)->void {
                    if (p) p->destroyInstance(LOG_TAG);
                }
        );
        if (hal3a == NULL) {
            MY_LOGE("create IHal3A instance failed");
            return -1;
        }

        ExpSettingParam_T expParam;
        if (bIsAis) {
            hal3a->send3ACtrl(E3ACtrl_EnableAIS, 1, 0);
        }
        hal3a->send3ACtrl(E3ACtrl_GetExposureInfo,  (MINTPTR)&expParam, 0); // for update info in ZSD mode
        hal3a->send3ACtrl(E3ACtrl_GetExposureParam, (MINTPTR)&cachedCapParam, 0);
        if (bIsAis) {
            hal3a->send3ACtrl(E3ACtrl_EnableAIS, 0, 0);
        }
    }
    capParam = cachedCapParam;
    return 0;
}

//-----------------------------------------------------------------------------
//  F U N C T I O N    I M P L M E N T A T I O N
//-----------------------------------------------------------------------------
android::sp<MfllNode>
MfllNode::createInstance(const MFCNode::MfcAttribute &attr /* = MFCNode::MfcAttribute() */)
{
    auto *p = new MfllNodeImp;
    p->shotMode() = (attr.is_time_sharing == 0) ? MfllMode_NormalMfll : MfllMode_ZsdMfll;
    p->postNrType() = NoiseReductionType_None; // TODO: maybe modified in method config()
    return (MfllNode*)p;
}

/**
 *  This method is always to be invoked at the beginning
 */
MERROR MfllNode::getCaptureInfo(int mfbMode, int &isUseMfllShot, int sensorId, void* /* custom_arg = NULL */)
{
    mfllAutoLog("MfllNode::getCaptureInfo");

#if MTKCAM_HAVE_MFB_BUILDIN_SUPPORT
    if (mfbMode == 0)
        mfbMode = MTKCAM_HAVE_MFB_BUILDIN_SUPPORT;
#endif
    // if force mfll
    int bForceMfb = MfllProperty::isForceMfll();
    if (CC_UNLIKELY( bForceMfb == 0 )) {
        MY_LOGD("Force disable MFNR");
        mfbMode = 0;
    }
    else if (CC_UNLIKELY(bForceMfb > 0)) {
        MY_LOGD("Force MFNR (bForceMfb:%d)", bForceMfb);
        mfbMode = bForceMfb;
    }

    if (mfbMode == 0)
        return OK;

    int iso = 0, exp = 0, frameBlend = 0, doMfb = 0;
    CaptureParam_T capParam;
    int result = getCurrent3ALocked(sensorId, capParam, (mfbMode == MFBMODE_AIS));
    if (result != 0) {
        MY_LOGE("getCurrent3ALocked returns %d", result);
        return UNKNOWN_ERROR;
    }
    exp = capParam.u4Eposuretime;
    iso = capParam.u4RealISO;

    MfllStrategyConfig_t cfg;
    cfg.isAis = (mfbMode == MFBMODE_AIS ? 1 : 0);
    cfg.exp = capParam.u4Eposuretime;
    cfg.iso = capParam.u4RealISO;
    result = MfllNodeImp::getCaptureInfo(mfbMode, sensorId, cfg); // update!
    if (result != 0) {
        return UNKNOWN_ERROR;
    }

    isUseMfllShot = cfg.enableMfb;
    return OK;
}

MERROR MfllNode::setCaptureInfo(
            int mfbMode,
            int sensorId,
            int /*isUseMfllShot*/,
            int iso,
            int exp,
            int frameBlend,
            int frameCapture,
            void* /* custom_arg = NULL */)
{
    MfllStrategyConfig_t cfg;
    cfg.iso = iso;
    cfg.exp = exp;
    cfg.frameBlend = frameBlend;
    cfg.frameCapture = frameCapture;

    /* update cached and read data */
    MfllNodeImp::getCaptureInfo(
            mfbMode,
            sensorId,
            cfg,
            false
            );

    /* force set configuration we need */
    cfg.isAis = (mfbMode == MFBMODE_AIS ? 1 : 0);
    cfg.iso = iso;
    cfg.exp = exp;
    cfg.frameBlend = frameBlend;
    cfg.frameCapture = frameCapture;
    return MfllNodeImp::setCaptureInfoCached(cfg);
}

MERROR MfllNode::getCaptureParamSet(
        MINT32 openId,
        Vector<CaptureParam_T> &vCaptureParams,
        Vector<NS3Av3::CaptureParam_T>* vOrigCaptureParams /* = NULL */)
{
    mfllAutoLogFunc();

    int result = 0;

    /* assume AE is fixed now ... */
    CaptureParam_T tmpCap3AParam;
    CaptureParam_T toCap3AParam;
    result = getCurrent3ALocked(openId, tmpCap3AParam, false, true); // use cached 3A info
    if (result != 0) {
        MY_LOGE("getCurrent3ALocked returns failed");
        return UNKNOWN_ERROR;
    }

    toCap3AParam = tmpCap3AParam;

    /* get catpure information */
    MfllStrategyConfig_t cfg;

    result = MfllNodeImp::getCaptureInfoCached(cfg);
    if (result != 0) {
        MY_LOGE("MfllNodeImp::getCaptureInfoCached failed, use default");
        return UNKNOWN_ERROR;
    }

    toCap3AParam.u4Eposuretime = cfg.exp;
    toCap3AParam.u4RealISO = cfg.iso;

lbExit:
    for (int i = 0; i < cfg.frameCapture; i ++)
        vCaptureParams.push_back(toCap3AParam);

    /** for ZSD, we wanna make preview stable, ask for dummpy frames */
    if (vOrigCaptureParams != NULL)
    {
        MUINT32 delayedFrames = MFLLNODE_ZSDAIS_DELAY_FRAME_COUNT;
        std::shared_ptr<IHal3A> hal3a(
                MAKE_Hal3A(openId, LOG_TAG),
                [](auto *p)->void {
                    if (p) p->destroyInstance(LOG_TAG);
                }
        );
        if (hal3a == NULL) {
            MY_LOGW("create IHal3A instance for querying delayedFrames failed, use %d frmae", delayedFrames);
        }

        hal3a->send3ACtrl(
                E3ACtrl_GetCaptureDelayFrame,
                reinterpret_cast<MINTPTR>(&delayedFrames), 0);

        vOrigCaptureParams->resize(delayedFrames);
        for (MUINT32 i = 0; i < delayedFrames; i++)
        {
            vOrigCaptureParams->editItemAt(i) = tmpCap3AParam;
        }
    }

    return OK;
}

//-----------------------------------------------------------------------------
//  M F L L N O D E I M P
//-----------------------------------------------------------------------------
MfllNodeImp::MfllNodeImp()
    : MFCNodeImp()
    , m_widthCapture(0)
    , m_heightCapture(0)
    , mCaptureFrameNum(0)
    , mDroppedFrameNum(0)
    , mBlendFrameNum(0)
    , m_isRequestJpegExif(0)
    , mOrientation(-1)
    , m_shotMode(MfllMode_ZsdMfll)
    , m_postNrType(mfll::NoiseReductionType_None)
    , m_uniqueKey(-1)
{
    mfllAutoLogFunc();
    /* gyro */
    mpMfllGyro = IMfllGyro::createInstance();
    if (mpMfllGyro.get()) {
        mpMfllGyro->start();
    }
}

MERROR MfllNodeImp::init(InitParams const &rParams)
{
    MERROR err = MFCNodeImp::init(rParams);
    return err;
}

MERROR MfllNodeImp::uninit()
{
    mfllAutoLogFunc();

    MFCNodeImp::uninit();

    /* check MFNR process status */
    if (m_futureMFNR.valid())
        m_futureMFNR.wait();

    return OK;
}

MERROR MfllNodeImp::config(const ConfigParams &rParams)
{
    MERROR err = MFCNodeImp::config(rParams);
    MY_LOGD("%s +", __FUNCTION__);

    if (err != OK) {
        return err;
    }
    else {
        /* get ouput yuv size and orientation */
        MSize yuvSize = rParams.vOutYuvJpeg->getImgSize();
        MSize rawSize = rParams.vInFullRaw[0]->getImgSize();
        mOrientation = rParams.vOutYuvJpeg->getTransform();

        {
            MfllStrategyConfig_t cfg;
            /* use cached value for the previous call */
            MfllNodeImp::getCaptureInfoCached(cfg);
            mCaptureFrameNum = cfg.frameCapture; // we only needs frameNum
            mBlendFrameNum = cfg.frameBlend;
            /* update mode to AIS */
            if (cfg.isAis) {
                if (shotMode() == MfllMode_NormalMfll)
                    shotMode() = MfllMode_NormalAis;
                else
                    shotMode() = MfllMode_ZsdAis;
            }
        }

        m_widthCapture = rawSize.w;
        m_heightCapture = rawSize.h;

        doInitMfnr();
    }
    MY_LOGD("%s -", __FUNCTION__);
    return err;
}

MERROR MfllNodeImp::flush()
{
    mfllAutoLogFunc();

    /* removes un-handled requests */
    MFCNodeImp::flush();

    /* wait for current operation done */
    waitForRequestDrained();

    /* flush queued request */
    CRequestFrame::flushClearAll(m_vUsedRequestFrame, this);
    return OK;
}

MERROR MfllNodeImp::queue(sp<IPipelineFrame> pFrame)
{
    shootMfnr();
    return MFCNodeImp::queue(pFrame);
}

MVOID MfllNodeImp::waitForRequestDrained()
{
    mfllAutoLogFunc ();
    /* if thread has been started, wait it. */
    if (m_futureMFNR.valid()) {
        m_futureMFNR.wait();
    }
}

MVOID MfllNodeImp::releaseResource()
{
    mfllAutoLogFunc();

    MFCNodeImp::releaseResource();
    /* clear resource that MfllNode used */
    mainRequestFrame() = 0;
    m_vUsedRequestFrame.clear();
    mpMfb = NULL;
    mpMfllCapturer = NULL;
    mpMfllCore = NULL;
}

int MfllNodeImp::doInitMfnr()
{
    mfllAutoLog("init mfll core");
    mfll::MfllBypassOption_t byPassOption;
    for (int i = 0; i < MFLL_MAX_FRAMES; i++)
        byPassOption.bypassAllocRawBuffer[i] = 1;

    /* create instances */
    mpMfllCore = IMfllCore::createInstance();
    /* capturer */
    mpMfllCapturer = IMfllCapturer::createInstance();
    mpMfllCapturer->setMfllCore(mpMfllCore.get());
    mpMfllCapturer->setShotMode(m_shotMode);
    mpMfllCapturer->setPostNrType(m_postNrType);
    /* multiframeblender */
    mpMfb = IMfllMfb::createInstance();
    mpMfb->init(getOpenId());
    mpMfb->setMfllCore(mpMfllCore.get());
    mpMfb->setShotMode(m_shotMode);
    mpMfb->setPostNrType(m_postNrType);

    mpMfllCore->setBypassOption(byPassOption);
    /* check if capture as single frame */
    if (getCaptureFrameNum() <= 1)
        mpMfllCore->setBypassOptionAsSingleFrame();
    mpMfllCore->setCapturer(mpMfllCapturer);
    mpMfllCore->setMfb(mpMfb);
    mpMfllCore->registerEventListener(new ListenerImp(this));

    if (m_widthCapture <=0 || m_heightCapture <=0) {
        MY_LOGE("capture resolution hasn't been set correctly");
        return -1;
    }
    else {
        MY_LOGD("init mfll with capture size %d x %d", m_widthCapture, m_heightCapture);
        mpMfllCore->setCaptureResolution(m_widthCapture, m_heightCapture);
    }

    mfll::MfllConfig_t cfg;

    cfg.sensor_id = mOpenId;
    cfg.capture_num = getCaptureFrameNum();
    cfg.blend_num = getBlendFrameNum();
    cfg.mfll_mode = m_shotMode;
    cfg.post_nr_type = m_postNrType;

    {
        MfllStrategyConfig_t __cfg;
        MfllNodeImp::getCaptureInfoCached(__cfg);
        cfg.full_size_mc = __cfg.isFullSizeMc;
        cfg.iso = __cfg.iso;
        cfg.exp = __cfg.exp;
        cfg.original_iso = __cfg.original_iso;
        cfg.original_exp = __cfg.original_exp;
    }

    mpMfllCore->init(cfg);

    return 0;
}

int MfllNodeImp::shootMfnr()
{
    /* check if invoked, make sure only invoke once */
    if (m_futureMFNR.valid())
        return 0;

    /* async invoke MNFR */
    m_futureMFNR = std::async(std::launch::async, [this]()->int {
        MY_LOGD("thread doMfnr +");
        /* check if inited, if not, init it */
        if (mpMfllCore.get() == NULL)
            doInitMfnr();

        int r = this->doMultiFrameNoiseReduction();
        /* check if the result is ok */
        int frameCapturedNum = mpMfllCore->getFrameCapturedNum(); // check if no frame captured.
        if (frameCapturedNum <= 0 || mainRequestFrame().get() == NULL) {
            MY_LOGE("MFNR failed, no output frame available or main" \
                "request frame. should invoke fake JPEG done CB");
            this->onDispatchFrame();
        }
        else {
            this->makesDebugInfo();
            this->onDispatchFrame();
        }
        MY_LOGD("thread doMfnr -");
        return r;
    });
    return 0;
}

int MfllNodeImp::doMultiFrameNoiseReduction()
{
    mfllAutoLogFunc();

    int error = 0;
    std::future<int> future_capture_frames;
    std::future<int> future_do_mfll;
    //-------------------------------------------------------------------------
    // Step 2: using MFCNodeImp::deque to get CRequestFrame and get IImageBuffer,
    //         and feed to IMfllCapturer
    //-------------------------------------------------------------------------
    // {{{
    {
        /* capture frames from pass 1 */
        future_capture_frames = std::async(std::launch::async, [this]()-> int
        {
            mfllAutoLog("doDequeFrames");
            int &frameDroppedCnt = mDroppedFrameNum; // describe counting of frame dropped

            // clear MfllGyro after all frames captured
            NSCam::ScopeWorker __lw([this](void*){
                    mpMfllGyro = NULL;
                });

            for (size_t i = 0; i < (size_t)getCaptureFrameNum(); i++) {
                mfllAutoLog("deque CRequestFrame");
                MY_LOGD("deque CRequestFrame %zu", i);
                std::shared_ptr<CRequestFrame> frame = deque();

                /**
                 *  check if timeout, which means frame is NULL, or if the status
                 *  of the frame is wrong and should be dropped.
                 *
                 *  If yes, drops this frame and wait the next
                 */
                if (frame.get() == NULL || frame->status() != 0) {
                    frameDroppedCnt++;
                    if (frame.get() == NULL) {
                        MY_LOGE("dequed as a dummpy CRequestFrame, idx= %zu", i);
                    }
                    else if (frame->status() != 0) {
                        MY_LOGW("Frame index %zu is bad and dropped. Dropped Cnt= %d", i, frameDroppedCnt);
                    }

                    /* main request is broken, discard this operation */
                    if (i == 0) {
                        mpMfllCore->doCancel();
                        frameDroppedCnt = getCaptureFrameNum(); // mark as all frame dropped
                        break; // break for-loop
                    }
                    else
                        continue;
                }

                if (m_vUsedRequestFrame.size() <= 0) {
                    /* saves the first frame as the main request frame and pipelineFrame */
                    m_mainRequestFrame = frame;

                    /* to check if exists postview yuv, if yes, register it to MfllCore */
                    do {
                        // {{{
                        auto pPostviewYuv = frame->imageBuffer(CRequestFrame::eRequestImg_YuvPostview);
                        if (pPostviewYuv == NULL)
                            break;

                        sp<IMfllImageBuffer> __postview = IMfllImageBuffer::createInstance("postview");
                        if (__postview.get() == NULL)
                            break;

                        auto appMeta = frame->metadata(CRequestFrame::eRequestMeta_InAppRequest);

                        if (appMeta == NULL) {
                            mfllLogE("cannot calculate the crop rgn of postview, use default");
                            break;
                        }

                        mfll::MfllRect_t __srcCropRgn;
                        MRect appCropRgn;
                        if (MFCNodeImp::retrieveScalerCropRgn(appMeta, appCropRgn)) {
                            __srcCropRgn.x = appCropRgn.p.x;
                            __srcCropRgn.y = appCropRgn.p.y;
                            __srcCropRgn.w = appCropRgn.s.w;
                            __srcCropRgn.h = appCropRgn.s.h;
                        }
                        __postview->setImageBuffer(reinterpret_cast<void*>(pPostviewYuv));
                        mpMfllCore->setPostviewBuffer(__postview, __srcCropRgn);
                        // }}}
                    } while (0);

                    MINT32 uniqueKey = -1;
                    IMetadata::getEntry<MINT32>(
                            frame->metadata(CRequestFrame::eRequestMeta_InHalP1),
                            MTK_PIPELINE_UNIQUE_KEY,
                            uniqueKey);
                    if (uniqueKey != -1) {
                        m_uniqueKey = uniqueKey;
                    }

                    /**
                     *  Update is request exif information due to Hal metadata may
                     *  be updated by IHal3A
                     */
                    MUINT8 isRequestExif = 0;
                    IMetadata::getEntry<MUINT8>(
                            frame->metadata(CRequestFrame::eRequestMeta_InHalP1),
                            MTK_HAL_REQUEST_REQUIRE_EXIF,
                            isRequestExif);
                    m_isRequestJpegExif = isRequestExif;
                }

                m_vUsedRequestFrame.enque(frame);

                MFCNodeImp::dumpExifInfo(*frame->metadata(CRequestFrame::eRequestMeta_InHalP1));

                /* retrieve metadata */
                IMetadata *pAppMeta = frame->metadata(CRequestFrame::eRequestMeta_InAppRequest);
                IMetadata *pHalMeta = frame->metadata(CRequestFrame::eRequestMeta_InHalP1);
                /* retrieve IMetadata for P2 driver usage*/
                IImageBuffer *pImageBuffer = frame->imageBuffer(CRequestFrame::eRequestImg_FullRaw);
                /* retrieve LCSO RAW IImageBuffer from P1 */
                IImageBuffer *pImageLcsoRaw = frame->imageBuffer(CRequestFrame::eRequestImg_LcsoRaw);
                if (pImageLcsoRaw) {
                    MY_LOGD("%s: got LCSO histogram RAW buffer --> %p", __FUNCTION__,
                            pImageBuffer);
                }

                /* create IMfllImageBuffer of RAW */
                sp<IMfllImageBuffer> mfllImgBuf = IMfllImageBuffer::createInstance("raw");
                mfllImgBuf->setImageBuffer(pImageBuffer);

                /* retrieve sensor timestamp & gyro info */
                mfllTraceBegin("get gyro info");
                MINT64 sensorTs = 0;
                std::deque<IMfllGyro::MfllGyroInfo> gyroInfo;
                if (IMetadata::getEntry<MINT64>(pAppMeta, MTK_SENSOR_TIMESTAMP, sensorTs)) {
                    MINT64 sensorDuration = 100 * 1000000L; // NS
                    if (!IMetadata::getEntry<MINT64>(
                                pAppMeta, MTK_SENSOR_EXPOSURE_TIME, sensorDuration)) {
                        mfllLogW("%s: cannot get sensor duration, assume to %" PRId64 "  ns",
                                __FUNCTION__, sensorDuration);
                    }

                    // check duration, at least, duration must be larger than
                    // (update_interval * (gyro_info_per_frame + safe_padding)
                    int64_t updateDuration =
                        (MFLL_GYRO_NUMBER_PER_FRAME + 1) // safe padding: 1
                        * MFLL_GYRO_DEFAULT_INTERVAL_MS
                        * 1000000L;

                    if (sensorDuration < updateDuration)
                        sensorDuration = updateDuration;


                    /* fetch gyro info */
                    if (mpMfllGyro.get()) {
                        mfllLogD("frame(%zu), timestamp=%" PRId64 ", duration=%" PRId64 "(ns)", i, sensorTs, sensorDuration);
                        // assume duration is 100 ms (frame rate)
                        auto __gyroInfos = mpMfllGyro->takeGyroInfo(sensorTs - sensorDuration, sensorTs);
                        // {{{ for debug
                        for (const auto& itr : __gyroInfos) {
                            mfllLogD(
                                    "got gyro scope: (ts,x,y,z)=(%" PRId64 ",%f,%f,%f)",
                                    itr.timestamp,
                                    itr.vector.x(),
                                    itr.vector.y(),
                                    itr.vector.z()
                                    );
                        }
                        // }}}

                        // we need at least MFLL_GYRO_NUMBER_PER_FRAME gyro info
                        if (__gyroInfos.size() < MFLL_GYRO_NUMBER_PER_FRAME) {
                            mfllLogW("%s: got no gyro info, or not enough. size=%zu",
                                    __FUNCTION__, __gyroInfos.size());
                        }
                        else {
                            // pick up the info around middle of __gyroInfos
                            auto __itr =
                                __gyroInfos.begin() +
                                ((__gyroInfos.size() / 2) - (MFLL_GYRO_NUMBER_PER_FRAME / 2));

                            for (size_t i = 0; i < MFLL_GYRO_NUMBER_PER_FRAME; i++, __itr++) {
                                gyroInfo.push_back(std::move(*__itr));
                            }
                        }
                    }
                }
                else {
                    mfllLogD("cannot get frame(%zu)'s timestamp, ignored gyro info", i);
                }

                /* saves gyro info */
                mGyroInfos.push_back(std::move(gyroInfo));
                mfllTraceEnd();

                /* retrieve GMV */
                MfllMotionVector_t mv = [this, &pHalMeta, &i]() -> MfllMotionVector_t {
                    MfllMotionVector    mv;
                    MSize               rzoSize;
                    MRect               p1ScalarRgn;
                    MBOOL               ret = MTRUE;

                    /* to get GMV info and the working resolution */
                    ret = retrieveGmvInfo(pHalMeta, mv.x, mv.y, rzoSize);
                    if (ret == MTRUE) {
                        ret = IMetadata::getEntry<MRect>(
                                pHalMeta,
                                MTK_P1NODE_SCALAR_CROP_REGION,
                                p1ScalarRgn);
                    }

                    /* if works, mapping it from rzoDomain to MfllCore domain */
                    if (ret == MTRUE) {
                        /* the first frame, set GMV as zero */
                        if (i == 0) {
                            mv.x = 0;
                            mv.y = 0;
                        }

                        MY_LOGD("%s: GMV(x,y)=(%d,%d), unit based on resized RAW",
                                __FUNCTION__,
                                mv.x, mv.y);
                        MY_LOGD("%s: p1node scalar crop rgion (width): %d, gmv domain(width): %d",
                                __FUNCTION__, p1ScalarRgn.s.w, rzoSize.w);
                        /**
                         *  the cropping crops height only, not for width. Hence, just
                         *  simply uses width to calculate the ratio.
                         */
                        float ratio =
                            static_cast<float>(p1ScalarRgn.s.w)
                            /
                            static_cast<float>(rzoSize.w)
                            ;
                        MY_LOGD("%s: ratio = %f", __FUNCTION__, ratio);

                        // we don't need floating computing because GMV is formated
                        // with 8 bits floating point
                        mv.x *= ratio;
                        mv.y *= ratio;

                        /* normalization */
                        mv.x = mv.x >> 8;
                        mv.y = mv.y >> 8;

                        /**
                         *  assume the ability of EIS algo, which may seach near by
                         *  N pixels only, so if the GMV is more than N pixels,
                         *  we clip it
                         */
                        auto CLIP = [](int x, const int n) -> int {
                            if (x < -n)     return -n;
                            else if(x > n)  return n;
                            else            return x;
                        };

                        /**
                         *  Hence we've already known that search region is 32 by 32
                         *  pixel based on RRZO domain, we can map it to full size
                         *  domain and makes clip if it's out-of-boundary.
                         */
                        int c = static_cast<int>(ratio * 32.0f);
                        mv.x = CLIP(mv.x, c);
                        mv.y = CLIP(mv.y, c);

                        MY_LOGI("%s: GMV'(x,y)=(%d,%d), unit: Mfll domain",
                                __FUNCTION__,
                                mv.x, mv.y);
                    }
                    return mv;
                }();

                /* get address of IMetadata */
                std::deque<void*> metaset;
                metaset.push_back(static_cast<void*>(pAppMeta));
                metaset.push_back(static_cast<void*>(pHalMeta));
                metaset.push_back(static_cast<void*>(pImageLcsoRaw));

                /* set sync object to IMfllMfb */
                mpMfb->setSyncPrivateData(metaset);
                /* enque IMfllImageBuffer */
                mpMfllCapturer->queueSourceImage(mfllImgBuf);
                /* enque GMV info */
                mpMfllCapturer->queueGmv(mv);

                if (i == 0) { // the first CRequestFrame which contains output YUV
                    IImageBuffer *imgBufYuvJpeg = frame->imageBuffer(CRequestFrame::eRequestImg_YuvJpeg);
                    IImageBuffer *imgbufYuvThumbnail = frame->imageBuffer(CRequestFrame::eRequestImg_YuvThumbnail);
                    if (imgBufYuvJpeg == NULL) {
                        MY_LOGE("YUV JPEG buffer is NULL");
                        return -1;
                    }
                    if (imgbufYuvThumbnail == NULL) {
                        MY_LOGE("YUV thumbnail buffer is NULL");
                        return -1;
                    }
                }
            } // for N frames

            /* prepare dummy frame if any frame dropped */
            for (int i = 0; i < frameDroppedCnt; i++) {
                sp<IMfllImageBuffer> dummpy = IMfllImageBuffer::createInstance("dummpy");
                /* enque dummpy frame */
                mpMfllCapturer->queueSourceImage(dummpy);
                /* enque GMV info */
                mpMfllCapturer->queueGmv(MfllMotionVector_t());

                std::shared_ptr<CRequestFrame> dummpyFrame(NULL);
                m_vUsedRequestFrame.enque(dummpyFrame);
            }

            return 0;
        }); // std::async::launch
    } // Step 2
    // }}}

    //-------------------------------------------------------------------------
    // Step 3: Invoke IMfllCore::doMfll to do MFNR
    //-------------------------------------------------------------------------
    // {{{
    {
        mfllAutoLog("doMfnr");
        CAM_TRACE_BEGIN("doMfll");
        {
            /* async invoke */
            future_do_mfll = std::async(std::launch::async, [this]()->int{
                this->mpMfllCore->doMfll(); // synchronized call
                return 0;
            });

            /* wait frames are feed to IMfllCapturer */
            future_capture_frames.wait();

            /* wait MFNR processing finish */
            future_do_mfll.wait();
        }
        CAM_TRACE_END();
    } // step 3
    // }}}

    //-------------------------------------------------------------------------
    // Step 4: process out YUV to JPEG node
    //-------------------------------------------------------------------------
    // {{{
    {
        mfllAutoLog("doYuvJpeg");
        /* ok. convert to output yuv for jpeg node */
        CAM_TRACE_BEGIN("doThumbnail");
        {
            sp<IMfllImageBuffer> img = mpMfllCore->retrieveBuffer(mfll::MfllBuffer_MixedYuv);
            if (img.get() == NULL) {
                mfllLogE("%s: mixed output buffer is NULL", __FUNCTION__);
                error = -1;
                goto lbExit;
            }

            if (mpMfllCore->getFrameCapturedNum() <= 0) {
                MY_LOGE("No frame has been captured by MfllCore");
                error = -1;
                goto lbExit;
            }

            if (mainRequestFrame().get() == NULL) {
                MY_LOGE("No mainRequestFrame for output");
                error = -1;
                goto lbExit;
            }

            IImageBuffer *yuvSrc = (IImageBuffer*)img->getImageBuffer();
            IImageBuffer *yuvJpeg = mainRequestFrame()->imageBuffer(CRequestFrame::eRequestImg_YuvJpeg);
            IImageBuffer *yuvThmb = mainRequestFrame()->imageBuffer(CRequestFrame::eRequestImg_YuvThumbnail);
            IImageBuffer *yuvPostview = mainRequestFrame()->imageBuffer(CRequestFrame::eRequestImg_YuvPostview);
            IMetadata    *appMeta = mainRequestFrame()->metadata(CRequestFrame::eRequestMeta_OutAppResult);
            IMetadata    *halMeta = mainRequestFrame()->metadata(CRequestFrame::eRequestMeta_OutHalResult);
            MSize dst1Size = (mOrientation == eTransform_ROT_90 || mOrientation == eTransform_ROT_270)
                ? MSize(yuvJpeg->getImgSize().h, yuvJpeg->getImgSize().w)
                : yuvJpeg->getImgSize();
            MRect rectDst1(MPoint(0, 0), dst1Size);
            MRect rectDst2(MPoint(0, 0), yuvThmb->getImgSize());
            MRect rectSrc;
            if (!MFCNodeImp::retrieveScalerCropRgn(appMeta, rectSrc)) {
                /* if no crop region in metadata, use full size as default */
                MY_LOGW("No crop region in app metadata, use default");
                rectSrc.p = MPoint(0, 0);
                rectSrc.s = yuvSrc->getImgSize();
            }

            /**
             *  LABEL_POSTVIEW:
             *  if yuvPostview still exisits, do it again and clear request
             *
             *  @sa LABEL_EARLY_POSTVIEW
             */
            do {
                // {{{
                if (yuvPostview == NULL)
                    break;
                // mark as error due to it's no data, and postview YUV should not be sent
                mainRequestFrame()->markError(CRequestFrame::eRequestImg_YuvPostview);
                mainRequestFrame()->releaseImage(CRequestFrame::eRequestImg_YuvPostview);
                //}}}
            } while(0);

            generateOutputYuv(
                    mOpenId,
                    yuvSrc,
                    yuvJpeg,
                    yuvThmb,
                    calCrop(rectSrc, rectDst1),
                    calCrop(rectSrc, rectDst2),
                    mOrientation,
                    appMeta,
                    halMeta,
                    MTRUE /* for Mfll this argument (isMfll) set to be true */
                    );
        }
        CAM_TRACE_END();
    } // Step 4
    // }}}

lbExit:
    return error;
}

//-----------------------------------------------------------------------------
// Singleton strategy
//-----------------------------------------------------------------------------
#define __MAGICNUM__ 12345678
static struct __singletonStrategy{
    sp<IMfllStrategy>       strategy;
    sp<IMfllNvram>          nvram;
    Mutex                   mx;
    MfllStrategyConfig_t    cfg;
    int                     magic_num;
} singletonStrategy;

int MfllNodeImp::getCaptureInfo(int /*mode*/, int sensorId, MfllStrategyConfig_t &capConfig, bool bUseCached)
{
    enum MfllErr err = MfllErr_Ok;

    /* get singletonStrategy */
    Mutex &mx                           = singletonStrategy.mx;
    sp<IMfllStrategy> &strategy         = singletonStrategy.strategy;
    sp<IMfllNvram> &nvram               = singletonStrategy.nvram;
    MfllStrategyConfig_t &__cachedCfg   = singletonStrategy.cfg;
    int &magic_num                      = singletonStrategy.magic_num;

    Mutex::Autolock _l(&mx); // lock first

    /* check magic num is still there while using cached value */
    if (bUseCached) {
        if (magic_num == __MAGICNUM__) {
            bUseCached = false; // if yes, do not use cached because there's no cached value.
        }
    }

    if (bUseCached) {
        capConfig = __cachedCfg;
    }
    else {

        MfllStrategyConfig_t config = capConfig;

        if (strategy.get() == NULL) {
            strategy = IMfllStrategy::createInstance();
            if (strategy.get() == NULL) {
                MY_LOGE("%s: create IMfllStrategy instance failed", __FUNCTION__);
                return -1;
            }
        }
        if( nvram.get() == NULL) {
            nvram = IMfllNvram::createInstance();
            if (nvram.get() == NULL) {
                MY_LOGE("%s: create IMfllNvram instance failed", __FUNCTION__);
                return -1;
            }
        }
        /* IMfllNvram */
        err = nvram->init(sensorId);
        if (err != MfllErr_Ok) {
            MY_LOGE("%s: init IMfllNvram failed, code=%d", __FUNCTION__, err);
            return -1;
        }
        /* IMfllStrategy */
        err = strategy->init(nvram);
        if (err != MfllErr_Ok) {
            MY_LOGE("%s: init IMfllStrategy failed, code=%d", __FUNCTION__, err);
            return -1;
        }
        /* query strategy info */
        err = strategy->queryStrategy(config, &config);
        if (err != MfllErr_Ok) {
            MY_LOGE("%s: IMfllStrategy::queryStrategy failed, code=%d", __FUNCTION__, err);
            return -1;
        }

        //---------------------------------------------------------------------
        // always use MfllShot to do single capture if using AIS mode
        //---------------------------------------------------------------------
        if (config.isAis && config.enableMfb == 0) {
            config.enableMfb = 1;
            config.frameCapture = 1;
            config.frameBlend = 1;
        }

        //---------------------------------------------------------------------
        // Start to check parameters and update
        //---------------------------------------------------------------------
        /* check blend_num */
        if (config.enableMfb) {
            if (config.frameBlend > config.frameCapture) {
                config.frameBlend = config.frameCapture;
            }
        }

        /* print log */
        MY_LOGD("%s: original (exp,iso)=(%d,%d)", __FUNCTION__, capConfig.exp, capConfig.iso);
        MY_LOGD("%s: final    (exp,iso)=(%d,%d)", __FUNCTION__, config.exp, config.iso);
        MY_LOGD("%s: blend_num    = %d", __FUNCTION__, config.frameBlend);
        MY_LOGD("%s: capture_num  = %d", __FUNCTION__, config.frameCapture);
        MY_LOGD("%s: doMfb        = %d", __FUNCTION__, config.enableMfb);
        MY_LOGD("%s: isAis        = %d", __FUNCTION__, config.isAis);
        MY_LOGD("%s: full_size_mc = %d", __FUNCTION__, config.isFullSizeMc);

        /* saves original */
        config.original_exp = capConfig.iso;
        config.original_iso = capConfig.exp;

        /* feedback */
        capConfig = config;

        /* update cached */
        __cachedCfg = config;

        /* clear magic number */
        magic_num = 0;
    }
    return 0;
}

int MfllNodeImp::getCaptureInfoCached(MfllStrategyConfig_t &cfg)
{
    return MfllNodeImp::getCaptureInfo(0, 0, cfg, true);
}

int MfllNodeImp::setCaptureInfoCached(const MfllStrategyConfig_t &cfg)
{
    /* get singletonStrategy */
    MfllStrategyConfig_t &__cachedCfg   = singletonStrategy.cfg;
    Mutex &mx                           = singletonStrategy.mx;
    int &magic_num                      = singletonStrategy.magic_num;

    Mutex::Autolock _l(&mx);
    //---------------------------------------------------------------------
    // Start to check parameters and update
    //---------------------------------------------------------------------
    /* print log */
    MY_LOGD("%s: set cached capture info directly", __FUNCTION__);
    MY_LOGD("%s: original (exp,iso)=(%d,%d)", __FUNCTION__, __cachedCfg.exp, __cachedCfg.iso);
    MY_LOGD("%s: final    (exp,iso)=(%d,%d)", __FUNCTION__, cfg.exp, cfg.iso);
    MY_LOGD("%s: blend_num   = %d", __FUNCTION__, cfg.frameBlend);
    MY_LOGD("%s: capture_num = %d", __FUNCTION__, cfg.frameCapture);
    MY_LOGD("%s: doMfb       = %d", __FUNCTION__, cfg.enableMfb);
    MY_LOGD("%s: isAis       = %d", __FUNCTION__, cfg.isAis);
    __cachedCfg = cfg;
    magic_num = 0;
    return 0;
}
#undef __MAGICNUM__

void MfllNodeImp::makesDebugInfo()
{
    enum MfllErr err = MfllErr_Ok;

    if (mainRequestFrame().get() == NULL) {
        return;
    }

    IMetadata *pMeta = mainRequestFrame()->metadata(CRequestFrame::eRequestMeta_OutHalResult);
    /* check if request exif ? */
    if (m_isRequestJpegExif == 0) {
        MY_LOGD("%s: No request exif", __FUNCTION__);
        return;
    }
    MY_LOGD("%s: prepare MF debug info", __FUNCTION__);

    /* makes exif info here */
    sp<IMfllExifInfo> exifInfo = mpMfllCore->getExifContainer();
    if (exifInfo.get() == NULL) {
        MY_LOGE("%s: get exif info from MfllCore failed", __FUNCTION__);
        return;
    }

    exifInfo->updateInfo(singletonStrategy.nvram.get());

    /* update gyro info, use sendCommand */
    std::deque<void*> dataset(1, static_cast<void*>(&mGyroInfos));
    err = exifInfo->sendCommand("ADD_GYRO_INFO", dataset);
    if (err == MfllErr_NotImplemented) {
        mfllLogD("%s: add gyro info hasn't been implemented", __FUNCTION__);
    }
    else if (err != MfllErr_Ok) {
        mfllLogD("%s: add gyro info failed with code %#x", __FUNCTION__, err);
    }
    else {
        mfllLogD("%s: added gyro info", __FUNCTION__);
    }

    /* packing to IMetadata for JPEG node */
    IMetadata exifMeta;
    {
        IMetadata::IEntry entry= pMeta->entryFor(MTK_3A_EXIF_METADATA);
        if(!entry.isEmpty())
            exifMeta = entry.itemAt(0, Type2Type<IMetadata>());
    }

    /* set debug information into debug Exif metadata */
    DebugExifUtils::setDebugExif(
            DebugExifUtils::DebugExifType::DEBUG_EXIF_MF,
            static_cast<MUINT32>(MTK_MF_EXIF_DBGINFO_MF_KEY),
            static_cast<MUINT32>(MTK_MF_EXIF_DBGINFO_MF_DATA),
            exifInfo->getInfoMap(),
            &exifMeta);

    /* update debug Exif metadata */
    IMetadata::IEntry entry_exif(MTK_3A_EXIF_METADATA);
    entry_exif.push_back(exifMeta, Type2Type<IMetadata>());
    pMeta->update(entry_exif.tag(), entry_exif);
}
