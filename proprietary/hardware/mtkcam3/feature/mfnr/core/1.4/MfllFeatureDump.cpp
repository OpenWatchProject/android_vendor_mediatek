#define LOG_TAG "MfllCore/Dump"

#include "MfllFeatureDump.h"
#include <mtkcam3/feature/mfnr/MfllLog.h>
#include "MfllCore.h"

#define ALIGN(w, a) (((w + (a-1)) / a) * a)

using namespace mfll;
using std::vector;
using std::string;

static std::vector<enum EventType> EVENTS_TO_LISTEN_INITIALIZER(void)
{
    std::vector<enum EventType> v;
    #define LISTEN(x) v.push_back(x)
    /* release RAW buffers if necessary */
    LISTEN(EventType_CaptureRaw);
    LISTEN(EventType_CaptureYuvQ);
    LISTEN(EventType_EncodeYuvBase);
    LISTEN(EventType_EncodeYuvGolden);
    LISTEN(EventType_Blending);
    LISTEN(EventType_Mixing);

    /* release ME/MC related buffers */
    LISTEN(EventType_MotionCompensation);
    #undef LISTEN
    return v;
}

static vector<enum EventType> g_eventsToListen = EVENTS_TO_LISTEN_INITIALIZER();


MfllFeatureDump::MfllFeatureDump(void)
    : m_filepath(MFLL_DUMP_DEFAUL_PATH)
{
    memset(&m_dumpFlag, 0, sizeof(m_dumpFlag));
}

MfllFeatureDump::~MfllFeatureDump(void)
{
}

void MfllFeatureDump::onEvent(enum EventType t, MfllEventStatus_t &status, void *mfllCore, void *param1, void *param2)
{
}

void MfllFeatureDump::doneEvent(enum EventType t, MfllEventStatus_t &status, void *mfllCore, void *param1, void *param2)
{
    enum MfllErr err = MfllErr_Ok;
    IMfllCore *c = static_cast<IMfllCore*>(mfllCore);
    unsigned int index = (unsigned int)(long)param1;

    mfllLogD("doneEvent: %s", IMfllCore::getEventTypeName(t));
    mfllLogD("status.ignore = %d", status.ignore);

    switch(t) {
    case EventType_Capture:
        dumpCapture((IMfllCore*)mfllCore);
        break;
#if 0
    case EventType_CaptureRaw:
        if (m_dumpFlag.raw)
            dump_image(c->retrieveBuffer(MfllBuffer_Raw, index).get(), DumpStage_Capture, "raw_", index);
        break;
    case EventType_CaptureYuvQ:
        if (m_dumpFlag.yuv) {
            dump_image(c->retrieveBuffer(MfllBuffer_QYuv, index).get(), DumpStage_Capture, "qyuv_", index);
            if (c->isFullSizeMc())
                dump_image(c->retrieveBuffer(MfllBuffer_FullSizeYuv, index).get(), DumpStage_Capture, "yuv_", index);
        }
        break;
#endif
    case EventType_EncodeYuvBase:
        if (m_dumpFlag.yuv)
            dump_image(c->retrieveBuffer(MfllBuffer_BaseYuv).get(), DumpStage_Base, "base_");
        break;
    case EventType_EncodeYuvGolden:
        if (m_dumpFlag.yuv)
            dump_image(c->retrieveBuffer(MfllBuffer_GoldenYuv).get(), DumpStage_Golden, "golden_");
        break;
    case EventType_Blending:
        dumpMfbStage((IMfllCore*)(mfllCore), (int)(long)param1);
        break;
    case EventType_Mixing:
        dumpMixStage((IMfllCore*)mfllCore);
        break;
    case EventType_MotionCompensation:
        if (m_dumpFlag.yuv){
            if (c->isFullSizeMc())
                dump_image(c->retrieveBuffer(MfllBuffer_FullSizeYuv, index + 1).get(), DumpStage_Memc, "memc_", index + 1);
            else
                dump_image(c->retrieveBuffer(MfllBuffer_QYuv, index + 1).get(), DumpStage_Memc, "memc_", index + 1);
        }
    } // switch(t)
}

vector<enum EventType> MfllFeatureDump::getListenedEventTypes(void)
{
    return g_eventsToListen;
}

void MfllFeatureDump::dump_image(IMfllImageBuffer *pImg, const DumpStage &stage, const char *name, int number)
{
    char filename[512];
    unsigned int w, h;
    unsigned int aligned_w, aligned_h;
    ImageFormat f;
    const char *extFmt = NULL;
    const char *yv16 = "yv16";
    const char *qyuv = "qyuv";
    if (pImg == NULL) {
        mfllLogE("%s: img buffer is NULL, can not save %s", __FUNCTION__, name);
        return;
    }
    pImg->getResolution(w, h);
    /* using aligned width and height as output information */
    pImg->getAligned(aligned_w, aligned_h);
    f = pImg->getImageFormat();
    w = ALIGN(w, aligned_w);
    h = ALIGN(h, aligned_h);
    switch(f) {
    case ImageFormat_Yuy2:  extFmt = "yuy2";        break;
    case ImageFormat_Yv16:  extFmt = "yv16";        break;
    case ImageFormat_I422:  extFmt = "i422";        break;
    case ImageFormat_Y8:
    case ImageFormat_Raw8:  extFmt = "y";           break;
    case ImageFormat_Raw10: extFmt = "raw";         break;
    case ImageFormat_Raw12: extFmt = "raw12";       break;
    default:                extFmt = "unknown";     break;
    }

    /* workaround for imageformat yv16 */
    if(strcmp(qyuv,name)==0){
       snprintf(
        filename,
        sizeof(filename),
        "%s_%02d_%s_%dx%d_%d.%s",
        m_filepath.c_str(), (int)stage, name, w, h, number, yv16
    );
    }else{
    snprintf(
        filename,
        sizeof(filename),
        "%s_%02d_%s_%dx%d_%d.%s",
        m_filepath.c_str(), (int)stage, name, w, h, number, extFmt
    );
        }

    mfllLogD("%s: save image to %s", __FUNCTION__, filename);
    pImg->saveFile(filename);
}

void MfllFeatureDump::dumpCapture(IMfllCore *pCore)
{
    enum MfllErr err = MfllErr_Ok;
    DumpStage s = DumpStage_Capture;
    MfllCore *c = (MfllCore*)pCore;

    mfllAutoLogFunc();

    if (m_dumpFlag.raw) {
        for (int i = 0; i < c->getCaptureFrameNum(); i++) {
            dump_image(c->m_imgRaws[i].get(), s, "raw", i);
        }
    }

    if (m_dumpFlag.yuv) {
        for (int i = 0; i < c->getCaptureFrameNum(); i++) {
            dump_image(
                c->m_imgQYuvs[i].get(),
                s,
                "qyuv",
                i);
        }
    }
}

void MfllFeatureDump::dumpBaseStage(IMfllCore *pCore)
{
    if (m_dumpFlag.yuv == 0)
        return;

    DumpStage s = DumpStage_Base;

    mfllAutoLogFunc();

    MfllCore *c = reinterpret_cast<MfllCore*>(pCore);

    /* dump base frame */
    dump_image(c->retrieveBuffer(MfllBuffer_BaseYuv).get(), s, "base");
}

void MfllFeatureDump::dumpMfbStage(IMfllCore *pCore, int index)
{
    enum MfllErr err = MfllErr_Ok;
    DumpStage s = DumpStage_Mfb;
    MfllCore *c = (MfllCore*)pCore;

    if (m_dumpFlag.mfb == 0)
        return;

    mfllAutoLogFunc();

    /* base frame */
    dump_image(c->retrieveBuffer(MfllBuffer_BaseYuv).get(), s, "base_in", index);

    /* reference frame */
    dump_image(c->retrieveBuffer(MfllBuffer_ReferenceYuv).get(), s, "ref_in", index);

    /* weighting input */
    dump_image(c->retrieveBuffer(MfllBuffer_WeightingIn).get(), s, "wt_in", index);

    /* weighting output */
    dump_image(c->retrieveBuffer(MfllBuffer_WeightingOut).get(), s, "wt_out", index);

    /* output frame */
    dump_image(c->retrieveBuffer(MfllBuffer_BlendedYuv).get(), s, "blended", index);
}

void MfllFeatureDump::dumpMixStage(IMfllCore *pCore)
{
    DumpStage s = DumpStage_Mix;
    MfllCore *c = (MfllCore*)pCore;

    if (m_dumpFlag.mix == 0)
        return;

    mfllAutoLogFunc();

    /* base frame */
    dump_image(c->retrieveBuffer(MfllBuffer_BlendedYuv).get(), s, "blended");

    /* golden frame */
    dump_image(c->retrieveBuffer(MfllBuffer_GoldenYuv).get(), s, "golden");

    /* output frame */
    dump_image(c->retrieveBuffer(MfllBuffer_MixedYuv).get(), s, "output");

    /* weighting */
    dump_image(c->retrieveBuffer(MfllBuffer_WeightingOut).get(), s, "weighting");
}

