#define LOG_TAG "MfllCore/Mrp"

#include "MfllFeatureMrp.h"
#include <mtkcam3/feature/mfnr/MfllLog.h>
#include "MfllCore.h"

using namespace mfll;
using std::vector;

static std::vector<enum EventType> EVENTS_TO_LISTEN_INITIALIZER(void)
{
    std::vector<enum EventType> v;
    #define LISTEN(x) v.push_back(x)
    /* release RAW buffers if necessary */
    LISTEN(EventType_Bss); // when BSS done, we need to release frame buffers but not included base frame.
    LISTEN(EventType_EncodeYuvGolden); // to release base RAW frame

    /* release ME/MC related buffers */
    LISTEN(EventType_MotionCompensation);
    LISTEN(EventType_Blending);
    LISTEN(EventType_Mixing);
    #undef LISTEN
    return v;
}

static vector<enum EventType> g_eventsToListen = EVENTS_TO_LISTEN_INITIALIZER();


MfllFeatureMrp::MfllFeatureMrp(void)
{
    m_mrpMode = MrpMode_BestPerformance;
}

MfllFeatureMrp::~MfllFeatureMrp(void)
{
}

void MfllFeatureMrp::onEvent(
    enum EventType      /* t */,
    MfllEventStatus_t&  /* status */,
    void*               /* mfllCore */,
    void*               /* param1 */,
    void*               /* param2 */
    )
{
}

void MfllFeatureMrp::doneEvent(enum EventType t, MfllEventStatus_t &status, void *mfllCore, void *param1, void* /* param2 */)
{
    enum MfllErr err = MfllErr_Ok;

    mfllLogD("Mrp-doneEvent: %s", IMfllCore::getEventTypeName(t));
    mfllLogD("status.ignore = %d", status.ignore);

    switch(t) {
    /* release RAW buffers */
    case EventType_EncodeYuvGolden:
        releaseBaseRawBuffer((IMfllCore*)mfllCore);
        break;

    case EventType_Bss:
        releaseReferenceRawBuffers((IMfllCore*)mfllCore);
        break;

    case EventType_MotionCompensation:
        releaseMemcRelatedBuffers((IMfllCore*)mfllCore, (unsigned int)(long)param1);
        break;

    case EventType_Blending:
        releaseStageBlending((IMfllCore*)mfllCore, (unsigned int)(long)param1);
        break;

    case EventType_Mixing:
        releaseStageMixing((IMfllCore*)mfllCore);
        break;
    default:
        // do nothing
        break;
    } // switch(t)
}

vector<enum EventType> MfllFeatureMrp::getListenedEventTypes(void)
{
    return g_eventsToListen;
}

void MfllFeatureMrp::releaseReferenceRawBuffers(IMfllCore *pCore)
{
    enum MfllErr err = MfllErr_Ok;
    unsigned int frameNum = pCore->getCaptureFrameNum();

    mfllFunctionIn();

    for (int i = 1; i < (int)frameNum; i++) {
        mfllLogD("%s: release raw buffer %d", __FUNCTION__, i);
        pCore->releaseBuffer(MfllBuffer_Raw, i);
        mfllLogD("%s: release rrzo buffer %d", __FUNCTION__, i);
        pCore->releaseBuffer(MfllBuffer_Rrzo, i);
    }

    mfllFunctionOut();
}

void MfllFeatureMrp::releaseBaseRawBuffer(IMfllCore *pCore)
{
    enum MfllErr err = MfllErr_Ok;
    mfllFunctionIn();

    mfllLogD("%s: release raw buffer %d", __FUNCTION__, 0);
    err = pCore->releaseBuffer(MfllBuffer_Raw, 0);
    mfllLogD("%s: release rrzo buffer %d", __FUNCTION__, 0);
    err = pCore->releaseBuffer(MfllBuffer_Rrzo, 0);
    mfllFunctionOut();
}

void MfllFeatureMrp::releaseMemcRelatedBuffers(IMfllCore *pCore, unsigned int index)
{
    mfllFunctionIn();
    pCore->releaseBuffer(MfllBuffer_AlgorithmWorking, index);
    mfllFunctionOut();
}

void MfllFeatureMrp::releaseStageBlending(IMfllCore *pCore, unsigned int index)
{
    enum MfllErr err = MfllErr_Ok;
    unsigned int frameBlendTimes = pCore->getBlendFrameNum() - 1;

    mfllFunctionIn();

    /* to release reference frame and algorithm working */
    pCore->releaseBuffer(MfllBuffer_ReferenceYuv);

    /* the last round of Memc */
    if ((index + 1) >= frameBlendTimes) {
        mfllLogD("release a weighting buffer (input)");
        pCore->releaseBuffer(MfllBuffer_WeightingIn);
    }

    mfllFunctionOut();

}

void MfllFeatureMrp::releaseStageMixing(IMfllCore *pCore)
{
    mfllFunctionIn();

    pCore->releaseBuffer(MfllBuffer_BlendedYuv);
    pCore->releaseBuffer(MfllBuffer_MixedYuvWorking);
    pCore->releaseBuffer(MfllBuffer_MixedYuvDebug);
    pCore->releaseBuffer(MfllBuffer_GoldenYuv);
    pCore->releaseBuffer(MfllBuffer_WeightingOut);

    mfllFunctionOut();
}

