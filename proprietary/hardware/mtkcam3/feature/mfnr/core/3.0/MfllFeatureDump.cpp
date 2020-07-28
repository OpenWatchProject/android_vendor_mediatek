#define LOG_TAG "MfllCore/Dump"

#include "MfllFeatureDump.h"
#include <mtkcam3/feature/mfnr/MfllLog.h>
#include "MfllCore.h"

using namespace mfll;
using std::vector;
using std::string;

static const char* __get_ext_name(IMfllImageBuffer* pImg)
{
    switch (pImg->getImageFormat()){
        case ImageFormat_Yuy2:      return "yuy2";
        case ImageFormat_Raw12:     return "raw12";
        case ImageFormat_Raw10:     return "raw";
        case ImageFormat_Raw8:      return "raw8";
        case ImageFormat_Yv16:      return "yv16";
        case ImageFormat_I422:      return "i422";
        case ImageFormat_I420:      return "i420";
        case ImageFormat_Y8:        return "y";
        case ImageFormat_Nv21:      return "nv21";
        case ImageFormat_Nv12:      return "nv12";
        case ImageFormat_YUV_P010:  return "packed_word";
        case ImageFormat_Nv16:      return "nv16";
        case ImageFormat_Sta32:     return "sta32";
        default:break;
    }
    return "unknown";
}

/* if extName is nullptr, this method looks up the extension name from __get_ext_name */
static MfllErr __dump_image(
        IMfllImageBuffer* pImg,
        const char* /*prefix*/,
        const char* stageName,
        const char* bufferName,
        const char* extName,
        const MfllMiddlewareInfo_t& info,
        bool commonDump)
{
    using namespace std;
    using std::string;

    if (pImg == nullptr) {
        return MfllErr_NullPointer;
    }

    // prefix, folder + file pattern
    string _f = MFLL_DUMP_DEFAUL_PATH;
    bool isYUV10 = pImg->getImageFormat() == ImageFormat_YUV_P010;
    string bayerOrder = (isYUV10)?"s0":to_string(info.bayerOrder);

    if (isYUV10)
        _f += string(MFLL_DUMP_FILE_NAME_RAW_ALIGNMENT);
    else
        _f += string(MFLL_DUMP_FILE_NAME_OTHER_ALIGNMENT);

    auto rp = [&_f](string token, string value)
    {
        auto pos = _f.find(token);
        if ( pos != std::string::npos ) {
            _f.replace(pos, token.length(), value);
        }
    };

    char strbuf[256] = {0};

    // width, height
    rp(MFLL_DUMP_PATTERN_WIDTH,  to_string(pImg->getWidth()));
    rp(MFLL_DUMP_PATTERN_HEIGHT, to_string(pImg->getHeight()));

    // iso, shutterUs
    rp(MFLL_DUMP_PATTERN_ISO, to_string(info.iso));
    rp(MFLL_DUMP_PATTERN_EXP, to_string(info.shutterUs));

    // uniqueKey
    ::sprintf(strbuf, "%09d", info.uniqueKey);
    rp(MFLL_DUMP_PATTERN_UNIQUEKEY, strbuf);

    // frame numbuer
    ::sprintf(strbuf, "%04d", info.frameKey);
    rp(MFLL_DUMP_PATTERN_FRAMENUM, strbuf);

    // request number
    ::sprintf(strbuf, "%04d", info.requestKey);
    rp(MFLL_DUMP_PATTERN_REQUESTNUM, strbuf);

    // stage name, buffer name, extension name
    rp(MFLL_DUMP_PATTERN_STAGE, stageName);
    rp(MFLL_DUMP_PATTERN_BUFFERNAME, bufferName);
    rp(MFLL_DUMP_PATTERN_EXTENSION, extName == nullptr ? __get_ext_name(pImg) : extName);

    //PW-PH-BW
    rp(MFLL_DUMP_PATTERN_ALIGNWIDTH, to_string(pImg->getAlignedWidth()));
    rp(MFLL_DUMP_PATTERN_ALIGNHEIGHT, to_string(pImg->getAlignedHeight()));
    rp(MFLL_DUMP_PATTERN_STRIDE, to_string(pImg->getBufStridesInBytes(0)));

    // bayer order, raw bit num, sensor id
    mfllLogD("bayerOrder=%d, bit=%d", info.bayerOrder, info.rawBitNum);
    rp(MFLL_DUMP_PATTERN_BAYERORDER, bayerOrder);
    rp(MFLL_DUMP_PATTERN_BITS,      to_string(info.rawBitNum));

    mfllLogD("save image %s, commonDump(%d)", _f.c_str(), commonDump);

    return pImg->saveFile(_f.c_str(), commonDump);
}


static std::vector<enum EventType> EVENTS_TO_LISTEN_INITIALIZER(void)
{
    std::vector<enum EventType> v;
    #define LISTEN(x) v.push_back(x)
    /* release RAW buffers if necessary */
    LISTEN(EventType_CaptureRaw);
    LISTEN(EventType_CaptureRrzo);
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

void MfllFeatureDump::onEvent(
        enum EventType      /* t */,
        MfllEventStatus_t&  /* status */,
        void*               /* mfllCore */,
        void*               /* param1 */,
        void*               /* param2 */
        )
{
}

void MfllFeatureDump::doneEvent(enum EventType t, MfllEventStatus_t &status, void *mfllCore, void *param1, void* param2)
{
    enum MfllErr err = MfllErr_Ok;
    IMfllCore *c = static_cast<IMfllCore*>(mfllCore);
    unsigned int index = (unsigned int)(long)param1;
    MfllMiddlewareInfo_t info = c->getMiddlewareInfo();
    info.frameKey = static_cast<int>(index);

    mfllLogD("doneEvent: %s", IMfllCore::getEventTypeName(t));
    mfllLogD("status.ignore = %d", status.ignore);


    switch(t) {
    case EventType_Capture:
        break;
#if 0
    case EventType_CaptureRaw:
        if (m_dumpFlag.raw) {
            __dump_image(
                    c->retrieveBuffer(MfllBuffer_Raw, index).get(),
                    MFLL_DUMP_FILE_NAME_RAW,
                    "bfbld",
                    "raw",
                    "packed_word",
                    info);

            // dump LCSO if exists
            if (param2) {
                IMfllImageBuffer* _pLcso = static_cast<IMfllImageBuffer*>(param2);
                __dump_image(
                        _pLcso,
                        MFLL_DUMP_FILE_NAME_OTHER,
                        "bfbld",
                        "lcso",
                        "lcso",
                        info);
            }
        }
        break;

    case EventType_CaptureRrzo:
        if (m_dumpFlag.raw) {
            __dump_image(
                    c->retrieveBuffer(MfllBuffer_Rrzo, index).get(),
                    MFLL_DUMP_FILE_NAME_RAW,
                    "bfbld",
                    "rrzo",
                    "packed_word",
                    info);
        }
        break;


    case EventType_CaptureYuvQ:
        if (m_dumpFlag.yuv) {
            if (c->isFullSizeMc()) {
                __dump_image(
                        c->retrieveBuffer(MfllBuffer_QYuv, index).get(),
                        MFLL_DUMP_FILE_NAME_OTHER,
                        "bfbld",
                        "qyuv",
                        "y",
                        info);

                __dump_image(
                        c->retrieveBuffer(MfllBuffer_FullSizeYuv, index).get(),
                        MFLL_DUMP_FILE_NAME_OTHER,
                        "bfbld",
                        "yuv",
                        "nv12",
                        info);
            }
            else {
                __dump_image(
                        c->retrieveBuffer(MfllBuffer_QYuv, index).get(),
                        MFLL_DUMP_FILE_NAME_OTHER,
                        "bfbld",
                        "qyuv",
                        "nv16",
                        info);
            }
        }
        break;
#endif
    case EventType_EncodeYuvBase:
        if (m_dumpFlag.yuv) {
            __dump_image(
                    c->retrieveBuffer(MfllBuffer_BaseYuv).get(),
                    MFLL_DUMP_FILE_NAME_OTHER,
                    "bfbld",
                    "base",
                    c->retrieveBuffer(MfllBuffer_BaseYuv).get()?nullptr:"nv12",
                    info,
                    true);
        }
        break;
    case EventType_EncodeYuvGolden:
        if (m_dumpFlag.yuv) {
            __dump_image(
                    c->retrieveBuffer(MfllBuffer_GoldenYuv).get(),
                    MFLL_DUMP_FILE_NAME_OTHER,
                    "single",
                    "yuv",
                    c->retrieveBuffer(MfllBuffer_GoldenYuv).get()?nullptr:"nv12",
                    info,
                    true);
        /* dceso */
        __dump_image(
                c->retrieveBuffer(MfllBuffer_DCESOWorking).get(),
                MFLL_DUMP_FILE_NAME_OTHER,
                "single",
                "dceso",
                "sta32",
                info,
                false);
        }
        break;
    case EventType_Blending:
        dumpMfbStage((IMfllCore*)(mfllCore), (int)(long)param1);
        break;
    case EventType_Mixing:
        dumpMixStage((IMfllCore*)mfllCore);
        break;
    case EventType_MotionCompensation:
        if (m_dumpFlag.yuv){
#if 0
            if (c->isFullSizeMc()) {
                __dump_image(
                        c->retrieveBuffer(MfllBuffer_FullSizeYuv, index + 1).get(),
                        MFLL_DUMP_FILE_NAME_OTHER,
                        "memc",
                        "yuv",
                        "nv12",
                        info);
            }
            else {
                __dump_image(
                        c->retrieveBuffer(MfllBuffer_QYuv, index + 1).get(),
                        MFLL_DUMP_FILE_NAME_OTHER,
                        "memc",
                        "yuv",
                        "nv12",
                        info);
            }
#endif
            // dump confidence map if exists
            auto confMap = c->retrieveBuffer(MfllBuffer_ConfidenceMap, index); // start from index 0
            if (confMap.get()) {
                __dump_image(
                        confMap.get(),
                        MFLL_DUMP_FILE_NAME_OTHER,
                        "memc",
                        "conf",
                        "y",
                        info,
                        false);
            }

            // dump motion compensation mv if exists
            auto mcMv = c->retrieveBuffer(MfllBuffer_MationCompensationMv, index); // start from index 0
            if (mcMv.get()) {
                __dump_image(
                        mcMv.get(),
                        MFLL_DUMP_FILE_NAME_OTHER,
                        "memc",
                        "mcmv",
                        "sta32",
                        info,
                        false);
            }
        }
    default:
        // do nothing
        break;
    } // switch(t)
}

vector<enum EventType> MfllFeatureDump::getListenedEventTypes(void)
{
    return g_eventsToListen;
}

void MfllFeatureDump::dump_image(
        IMfllImageBuffer*   /* pImg */,
        const DumpStage&    /* stage */,
        const char*         /* name */,
        int                 /* number */)
{
}

void MfllFeatureDump::dumpCapture(IMfllCore* /* pCore */)
{
}

void MfllFeatureDump::dumpBaseStage(IMfllCore* /* pCore */)
{
}

void MfllFeatureDump::dumpMfbStage(IMfllCore *pCore, int index)
{
    enum MfllErr err = MfllErr_Ok;
    MfllCore *c = (MfllCore*)pCore;
    MfllMiddlewareInfo_t info = pCore->getMiddlewareInfo();
    info.frameKey = index;

    if (m_dumpFlag.mfb == 0)
        return;

    mfllAutoLogFunc();

    auto dp = [&](IMfllImageBuffer* pBuf, const char* bufferName, bool commonDump, const char* extName = nullptr)
    {
        __dump_image(pBuf, MFLL_DUMP_FILE_NAME_OTHER, "mfb", bufferName, extName, info, commonDump);
    };

    /* base frame */
    dp( c->retrieveBuffer(MfllBuffer_BaseYuv).get(), "base_in", true/*, "nv12"*/);

    /* reference frame */
    dp( c->retrieveBuffer(MfllBuffer_ReferenceYuv).get(), "ref_in", true/*, "nv12"*/);

    /* weighting input */
    dp( c->retrieveBuffer(MfllBuffer_WeightingIn).get(), "wt_in", false, "y");

    if (MfllProperty::getMfbP2DirectLink()) {
        if (index+2 >= pCore->getFrameCapturedNum()) { // index + 2 = blended + 1 = capture num
            mfllLogD("%s: MFB direct link at blend(%d), total_frames(%d), discard last MFB dump", __FUNCTION__, index+1, pCore->getFrameCapturedNum());
            return;
        }
    }

    /* weighting output */
    dp( c->retrieveBuffer(MfllBuffer_WeightingOut).get(), "wt_out", false, "y");

    /* output frame */
    dp( c->retrieveBuffer(MfllBuffer_BlendedYuv).get(), "blended", true/*, "nv12"*/);
}

void MfllFeatureDump::dumpMixStage(IMfllCore *pCore)
{
    MfllCore *c = (MfllCore*)pCore;
    MfllMiddlewareInfo_t info = pCore->getMiddlewareInfo();

    if (m_dumpFlag.mix == 0)
        return;

    mfllAutoLogFunc();

    auto dp = [&](IMfllImageBuffer* pBuf, const char* bufferName, bool commonDump, const char* extName = nullptr)
    {
        __dump_image(pBuf, MFLL_DUMP_FILE_NAME_OTHER_ALIGNMENT, "afbld", bufferName, extName, info, commonDump);
    };

    /* output frame */
    if (c->getMixingBufferType() == MixYuvType_Working) {
        dp( c->retrieveBuffer(MfllBuffer_MixedYuv).get(), "mixed_working", true);
    }
    else {
        dp( c->retrieveBuffer(MfllBuffer_MixedYuv).get(), "mixed", true/*, "nv12"*/);
    }

    if (m_dumpFlag.mix == 2) {
        mfllLogD("Dump mix out only");
        return;
    }

    /* golden frame */
    dp( c->retrieveBuffer(MfllBuffer_GoldenYuv).get(), "golden", true/*, "nv12"*/);

    /* output debug frame*/
    dp( c->retrieveBuffer(MfllBuffer_MixedYuvDebug).get(), "mixed_debug", true);

    /* thumbnail frame */
    dp( c->retrieveBuffer(MfllBuffer_ThumbnailYuv).get(), "thumbnail", true, "yuy2");

    /* dceso */
    dp( c->retrieveBuffer(MfllBuffer_DCESOWorking).get(), "dceso", false, "sta32");

    if (!MfllProperty::getMfbP2DirectLink()) {
        /* base frame */
        dp( c->retrieveBuffer(MfllBuffer_BlendedYuv).get(), "blended", true/*, "nv12"*/);
        /* weighting */
        dp( c->retrieveBuffer(MfllBuffer_WeightingOut).get(), "weighting", false, "y");
    }

}
