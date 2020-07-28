#ifndef __MFLLTYPES_H__
#define __MFLLTYPES_H__

#include "MfllDefs.h"

// LINUX
#include <pthread.h> // pthread_mutex_t
#include <sys/prctl.h> // prctl(PR_SET_NAME)

// STL
#include <string> // std::string
#include <map> // std::map

// MTKCAM
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

namespace mfll {

    /* Mfll error code */
    enum MfllErr
    {
        MfllErr_Ok = 0,
        MfllErr_Shooted,
        MfllErr_AlreadyExist,
        MfllErr_NotInited,
        MfllErr_BadArgument,
        MfllErr_IllegalBlendFrameNum,
        MfllErr_IllegalCaptureFrameNum,
        MfllErr_NullPointer,
        MfllErr_NotImplemented,
        MfllErr_NotSupported,
        /* This error code indicates to instance creation is failed */
        MfllErr_CreateInstanceFailed,
        /* Load image failed */
        MfllErr_LoadImageFailed,
        /* Save image failed */
        MfllErr_SaveImageFailed,
        /* Others error will be categoried here */
        MfllErr_UnexpectedError,
        /* indicates to size only */
        MfllErr_Size,
    };

    /**
     *  Mfll mode, bit map
     *    - bit 0: Zsd on or off
     *    - bit 1: is MFB
     *    - bit 2: is AIS
     *    - bit 3: is Single Capturer
     *    - bit 4: is MFHR
     *    - bit 5: is ZHDR
     *    - bit 6: is AutoHDR
     * */
    enum MfllMode
    {
        MfllMode_Off                = 0x00,
        MfllMode_NormalMfll         = 0x02,
        MfllMode_ZsdMfll            = 0x03,
        MfllMode_NormalAis          = 0x04,
        MfllMode_ZsdAis             = 0x05,
        MfllMode_NormalSingleFrame  = 0x08,
        MfllMode_ZsdSingleFrame     = 0x09,
        MfllMode_NormalMfhr         = 0x10,
        MfllMode_ZsdMfhr            = 0x11,
        MfllMode_ZhdrNormalMfll     = 0x22,
        MfllMode_ZhdrZsdMfll        = 0x23,
        MfllMode_ZhdrNormalAis      = 0x24,
        MfllMode_ZhdrZsdAis         = 0x25,
        MfllMode_AutoZhdrNormalMfll = 0x62,
        MfllMode_AutoZhdrZsdMfll    = 0x63,
        MfllMode_AutoZhdrNormalAis  = 0x64,
        MfllMode_AutoZhdrZsdAis     = 0x65,
        /* Bits */
        MfllMode_Bit_Zsd            = 0,
        MfllMode_Bit_Mfll           = 1,
        MfllMode_Bit_Ais            = 2,
        MfllMode_Bit_SingleFrame    = 3,
        MfllMode_Bit_Mfhr           = 4,
        MfllMode_Bit_Zhdr           = 5,
        MfllMode_Bit_AutoHdr        = 6,
        MfllMode_Bit_Reserve1       = 7,
        MfllMode_Bit_Reserve2       = 8,
        MfllMode_Bit_Reserve3       = 9,
        MfllMode_Bit_Reserve4       = 10,
        MfllMode_Bit_Reserve5       = 11,
        MfllMode_Bit_Reserve6       = 12,
        MfllMode_Bit_Custom1        = 13,
        MfllMode_Bit_Custom2        = 14,
        MfllMode_Bit_Custom3        = 15,
        MfllMode_Bit_Custom4        = 16,
    };

    /**
     *  An enumeration describes buffers that MFLL core uses
     */
    enum MfllBuffer
    {
        /* Captured raw, amount is: (capturedFrameNum - 1) */
        MfllBuffer_Raw = 0,
        /* Captured rrzo, amount is: (capturedFrameNum - 1) */
        MfllBuffer_Rrzo,
        /* QSize yuv, converted from captured raw, amount is: (capturedFrameNum - 1) */
        MfllBuffer_QYuv,
        /* Full yuv, converted from captured raw, amount is: (capturedFrameNum - 1) */
        MfllBuffer_FullSizeYuv, // new one
        /* Postview yuv */
        MfllBuffer_PostviewYuv,
        /* Thumbnail yuv */
        MfllBuffer_ThumbnailYuv,
        /* Get the base YUV frame of the current stage */
        MfllBuffer_BaseYuv,
        /* Get the reference frame of the current stage */
        MfllBuffer_ReferenceYuv,
        /* Get the golden frame of the current stage */
        MfllBuffer_GoldenYuv,
        /* Get the blending(output) frame of the current stage */
        MfllBuffer_BlendedYuv,
        /* Get the mixed output(final output) frame of current stage */
        MfllBuffer_MixedYuv,
        /* Get the mixed yuv workingframe of current stage */
        MfllBuffer_MixedYuvWorking,
        /* Get the mixed yuv debugframe of current stage */
        MfllBuffer_MixedYuvDebug,
        /* Get the input weighting table of the current stage */
        MfllBuffer_WeightingIn,
        /* Get the output weighting table of the current stage */
        MfllBuffer_WeightingOut,
        /* Memc working buffer, amount is: (blendingFrameNum -1) */
        MfllBuffer_AlgorithmWorking, // Memc working buffer, with index

        /* Since MFNR v2.0, confidence map has been introduced. */
        MfllBuffer_ConfidenceMap,
        /* Since MFNR v3.0, MotionCompensation MV has been introduced. */
        MfllBuffer_MationCompensationMv,
        /* Since MFNR v3.0, DCESO buffer has been introduced. */
        MfllBuffer_DCESOWorking,

        /* size */
        MfllBuffer_Size
    };

    /* Image format that Mfll will invoke */
    enum ImageFormat
    {
        ImageFormat_Yuy2 = 0,   // YUV422:  size in byte = 2 x @ImageSensor
        ImageFormat_Raw12,      // RAW12:   size in byte = 1.50 x @ImageSensor
        ImageFormat_Raw10,      // RAW10:   size in byte = 1.25 x @ImageSensor
        ImageFormat_Raw8,       // for weighting table, size = 1 x @ImageSensor
        ImageFormat_Yv16,       // YVU422: size in byte = 2 x @ImageSensor
        ImageFormat_I422,       // YUV422: size in byte = 2 x @ImageSensor
        ImageFormat_I420,       // YUV420: size in byte = 1.50 x @ImageSensor
        ImageFormat_Y8,         // Y8: size in byte = 1 x @ImageSensor
        ImageFormat_Nv21,       // NV21: 2 planes, [Y]/[UV], size in byte = 1.50 x @ImageSensor
        ImageFormat_Nv12,       // NV12: 2 planes, [Y]/[VU], size in byte = 1.50 x @ImageSensor
        ImageFormat_YUV_P010,   // 420 format, 10bit, 2 plane (Y),(UV) = P010
        ImageFormat_Nv16,       // NV16:
        ImageFormat_Sta32,      // for mc mv, size = 4 x @ImageSensor
        /* size */
        ImageFormat_Size,
        ImageFormat_Unknown = ImageFormat_Size,
    };

    /**
     *  MFNR may have chance to apply SWNR or HWNR.
     */
    enum NoiseReductionType
    {
        NoiseReductionType_None = 0, // No NR applied
        NoiseReductionType_SWNR, // Applied SWNR after MFNR
        NoiseReductionType_HWNR, // Applied HWNR after MFNR
        /* size */
        NoiseReductionType_Size
    };

    /* describes MFLL RAW to YUV stage. */
    enum YuvStage
    {
        YuvStage_Unknown = 0,   // Unknown
        YuvStage_RawToYuy2,     // Stage 1
        YuvStage_RawToYv16,     // Stage 1
        YuvStage_BaseYuy2,      // Stage 1
        YuvStage_GoldenYuy2,    // Stage 3
        /* size */
        YuvStage_Size
    };

    /**
     *  MFLL provides an event classes, these are event that MFLL will invoked.
     *  If not specified param1 or param2, it will be 0.
     */
    enum EventType
    {
        EventType_Init = 0,
        EventType_AllocateRawBuffer, /* param1: integer, index of buffer */
        EventType_AllocateRrzoBuffer, /* param1: integer, index of buffer */
        EventType_AllocateQyuvBuffer, /* param1: integer, index of buffer */
        EventType_AllocateYuvBase,
        EventType_AllocateYuvGolden,
        EventType_AllocateYuvWorking,
        EventType_AllocateYuvMcWorking,
        EventType_AllocateYuvMixing,
        EventType_AllocateWeighting, /* param1: integer, index of weighting table */
        EventType_AllocateMemc, /* param1: integer, index of memc working buffer */
        EventType_Capture, /* invoke when start capturing frame and all frames are captured */
        EventType_CaptureRaw, /* param1: integer, index of captured RAW */
        EventType_CaptureRrzo, /* param1: integer, index of captured RRZO */
        EventType_CaptureYuvQ, /* param1: integer, index of captured Q size Yuv */
        EventType_CaptureEis, /* param1: integer, index of captured Eis info */
        EventType_Bss, /* param1: reference of an integer, represents number of frame to do BSS */
        EventType_EncodeYuvBase,
        EventType_EncodeYuvGolden,
        EventType_MotionEstimation, /* param1: integer, index of buffer */
        EventType_MotionCompensation,
        EventType_Blending,
        EventType_Mixing,
        EventType_Destroy,
        /* size */
        EventType_Size,
    };

    enum MemcMode
    {
        MemcMode_Sequential = 0,
        MemcMode_Parallel,
        /* size */
        MemcMode_Size
    };

    /* RWB sensor support mode */
    enum RwbMode
    {
        RwbMode_None = 0,
        RebMode_Mdp,
        RwbMode_GPU,
        /* size */
        RwbMode_Size
    };

    /* Memory Reduce Plan mode */
    enum MrpMode
    {
        MrpMode_BestPerformance = 0,
        MrpMode_Balance,
        /* size */
        MrpMode_Size
    };

    /* Mixing YUV buffer Type */
    enum MixYuvBufferType
    {
        MixYuvType_Unknow = 0,
        MixYuvType_Base,
        MixYuvType_Working,
        /* size */
        MixYuvType_Size
    };

    /**
     *  This enumeration describes what YUV format is as input (BFBLD).
     *  For case like using YUV capturer, the input format can be NV21 or
     *  NV12. That caller can set this value to indicate the YUV input format
     *  who wants.
     *
     *  Note: This enumeration is only supported in MFNR Core Library version >= 2.5
     */
    enum InputYuvFmt
    {
        InputYuvFmt_Nv12,
        InputYuvFmt_Nv21,
        InputYuvFmt_YUV_P010,
        InputYuvFmt_Default = InputYuvFmt_Nv12,
    };
//
//-----------------------------------------------------------------------------
//
    typedef struct MfllCoreDbgInfo
    {
        unsigned int frameMaxCapture;
        unsigned int frameMaxBlend;
        unsigned int frameCapture;
        unsigned int frameBlend;
        unsigned int iso; // saves iso to capture
        unsigned int exp; // saves exposure to capture
        unsigned int ori_iso;
        unsigned int ori_exp;
        unsigned int width; // processing image size
        unsigned int height;
        unsigned int rrzo_width; // rrzo image size
        unsigned int rrzo_height;
        unsigned int blend_yuv_width;  // blend yuv image size
        unsigned int blend_yuv_height;
        unsigned int bss_enable; // is bss applied
        unsigned int memc_skip; // bits indicates to skipped frame.
        unsigned int shot_mode;

        MfllCoreDbgInfo () noexcept
        {
            frameMaxCapture = 0;
            frameMaxBlend = 0;
            frameCapture = 0;
            frameBlend = 0;
            iso = 0;
            exp = 0;
            ori_iso = 0;
            ori_exp = 0;
            width = 0;
            height = 0;
            rrzo_width = 0;
            rrzo_height = 0;
            blend_yuv_width = 0;
            blend_yuv_height = 0;
            bss_enable = 0;
            memc_skip = 0;
            shot_mode = 0;
        }
    } MfllCoreDbgInfo_t;

    typedef struct MfllRect
    {
        int x;
        int y;
        int w;
        int h;

        MfllRect() noexcept : x(0), y(0), w(0), h(0) {}
        MfllRect(int x, int y, int w, int h) noexcept : x(x), y(y), w(w), h(h) {}
        size_t size() const noexcept { return static_cast<size_t>(w * h); }
    } MfllRect_t;

    /* sync object */
    typedef struct MfllSyncObj
    {
        pthread_mutex_t trigger;
        pthread_mutex_t done;
        MfllSyncObj(void) noexcept
            : trigger(PTHREAD_MUTEX_INITIALIZER)
            , done(PTHREAD_MUTEX_INITIALIZER)
        {
        }
    } MfllSyncObj_t;

    typedef struct MfllMotionVector
    {
        int x;
        int y;

        MfllMotionVector(void) noexcept
            : x (0), y (0) {}
    } MfllMotionVector_t;

    typedef struct MfllEventStatus
    {
        int ignore;
        enum MfllErr err;

        MfllEventStatus(void) noexcept
            : ignore(0)
            , err(MfllErr_Ok) {}
    } MfllEventStatus_t;

    /* Mfll bypass option */
    typedef struct MfllBypassOption
    {
        volatile unsigned int bypassAllocRawBuffer[MFLL_MAX_FRAMES];
        volatile unsigned int bypassAllocRrzoBuffer[MFLL_MAX_FRAMES];
        volatile unsigned int bypassAllocQyuvBuffer[MFLL_MAX_FRAMES];
        volatile unsigned int bypassAllocYuvBase;
        volatile unsigned int bypassAllocYuvGolden;
        volatile unsigned int bypassAllocYuvWorking;
        volatile unsigned int bypassAllocYuvMcWorking;
        volatile unsigned int bypassAllocYuvMixing;
        volatile unsigned int bypassAllocWeighting[2];
        volatile unsigned int bypassAllocMemc[MFLL_MAX_FRAMES];
        volatile unsigned int bypassAllocPostview;
        volatile unsigned int bypassCapture;
        volatile unsigned int bypassBss;
        volatile unsigned int bypassEncodeYuvBase;
        volatile unsigned int bypassEncodeYuvGolden;
        volatile unsigned int bypassEncodeQYuv[MFLL_MAX_FRAMES];
        volatile unsigned int bypassMotionEstimation[MFLL_MAX_FRAMES];
        volatile unsigned int bypassMotionCompensation[MFLL_MAX_FRAMES];
        volatile unsigned int bypassBlending[MFLL_MAX_FRAMES];
        volatile unsigned int bypassMixing;
        MfllBypassOption(void) noexcept
            : bypassAllocRawBuffer{0}
            , bypassAllocRrzoBuffer{0}
            , bypassAllocQyuvBuffer{0}
            , bypassAllocYuvBase(0)
            , bypassAllocYuvGolden(0)
            , bypassAllocYuvWorking(0)
            , bypassAllocYuvMcWorking(0)
            , bypassAllocYuvMixing(0)
            , bypassAllocWeighting{0}
            , bypassAllocMemc{0}
            , bypassAllocPostview(0)
            , bypassCapture(0)
            , bypassBss(0)
            , bypassEncodeYuvBase(0)
            , bypassEncodeYuvGolden(0)
            , bypassEncodeQYuv{0}
            , bypassMotionEstimation{0}
            , bypassMotionCompensation{0}
            , bypassBlending{0}
            , bypassMixing(0)
        {}
    } MfllBypassOption_t;

    /**
     *  Capture parameter for MFNR
     */
    typedef struct MfllConfig
    {
        int             sensor_id; // opened sensor id
        int             iso; // info of adjuested ISO, which is now applied
        int             exp; // info of adjuested EXP, which is now applied
        int             original_iso; // info of the origianl ISO
        int             original_exp;
        int             blend_num;
        int             capture_num;
        int             full_size_mc;
        bool            flash_on;
        enum MfllMode   mfll_mode;
        enum RwbMode    rwb_mode;
        enum MemcMode   memc_mode;
        enum MrpMode    mrp_mode;
        enum NoiseReductionType post_nr_type;

        MfllConfig(void) noexcept
        {
            sensor_id = -1;
            iso = 0;
            exp = 0;
            original_iso = 0;
            original_exp = 0;
            blend_num = 0;
            capture_num = 0;
            full_size_mc = 0;
            flash_on = false;
            mfll_mode = static_cast<enum MfllMode>(0);
            rwb_mode = static_cast<enum RwbMode>(RwbMode_None);
            memc_mode = static_cast<enum MemcMode>(MemcMode_Parallel);
            mrp_mode = static_cast<enum MrpMode>(MrpMode_BestPerformance);
            post_nr_type = static_cast<enum NoiseReductionType>(NoiseReductionType_None);
        }
    } MfllConfig_t;

    typedef struct MfllStrategyConfig
    {
        int     iso; // [in]: caller need to fill up this field
        int     exp; // [in]: caller need to fill up this field, in us
        int     sensor_id;    // [in]: caller need to fill up this field
        int     original_iso; // [out]
        int     original_exp; // [out]
        int     frameCapture; // [out]
        int     frameBlend;   // [out]
        int     enableMfb;    // [out]
        int     isAis;        // [out]
        int     isFullSizeMc; // [out]
        int     enableDownscale; // [out]
        int     downscaleRatio;  // [out] for MFNR 2.5
        int     downscaleDividend; // [out]: for MFNR 3.0
        int     downscaleDivisor;  // [out]: for MFNR 3.0
        int     postrefine_nr;  // [out]: for MFNR 3.0
        int     postrefine_mfb;  // [out]: for MFNR 3.0
        MfllStrategyConfig(void) noexcept
            : iso(0)
            , exp(0)
            , sensor_id(0)
            , original_iso(0)
            , original_exp(0)
            , frameCapture(0)
            , frameBlend(0)
            , enableMfb(0)
            , isAis(0)
            , isFullSizeMc(0)
            , enableDownscale(0)
            , downscaleRatio(0)
            , downscaleDividend(0)
            , downscaleDivisor(0)
            , postrefine_nr(1)
            , postrefine_mfb(1)
        {}
    } MfllStrategyConfig_t;

    typedef struct MfllMiddlewareInfo
    {
        int     uniqueKey;      // represent the unique number from middleware
        int     frameKey;       // represent the frame number from middleware
        int     requestKey;     // actually, MFNR Core Lib doesn't need requestNo.
        int     iso;            // the iso info
        int     shutterUs;      // shutter time info, in microsecond
        int     bayerOrder;     // raw order format
        int     rawBitNum;      // raw bits
        int     postrefine_nr;  // [out]: for MFNR 3.0
        int     postrefine_mfb;  // [out]: for MFNR 3.0

        MfllMiddlewareInfo() noexcept
            : uniqueKey(0), frameKey(0), requestKey(0), iso(0), shutterUs(0)
            , bayerOrder(0), rawBitNum(0), postrefine_nr(1), postrefine_mfb(1) {}
    } MfllMiddlewareInfo_t;
}; /* namespace mfll */

/******************************************************************************
 * Utilities
 ******************************************************************************/
namespace mfll {

/**
 *  Check if the mode is ZSD or note.
 *  @param m            Mode to check
 *  @return             If the mode is ZSD mode, returns true
 */
inline bool isZsdMode(const enum MfllMode& m) noexcept
{
    return !!(m & (1 << MfllMode_Bit_Zsd)); // see MfllMode, bit 0 represents if it's ZSD mode or not
}

inline bool isMfllMode(const enum MfllMode& m) noexcept
{
    return !!(m & (1 << MfllMode_Bit_Mfll));
}

inline bool isAisMode(const enum MfllMode& m) noexcept
{
    return !!(m  & (1 << MfllMode_Bit_Ais));
}

inline bool isSingleFrameMode(const enum MfllMode& m) noexcept
{
    return !!(m & (1 << MfllMode_Bit_SingleFrame));
}

inline bool isMfhrMode(const enum MfllMode& m) noexcept
{
    return !!(m & (1 << MfllMode_Bit_Mfhr));
}

inline bool isZhdrMode(const enum MfllMode& m) noexcept
{
    return !!(m & (1 << MfllMode_Bit_Zhdr));
}

inline bool isAutoHdr(const enum MfllMode& m) noexcept
{
    return !!(m & (1 << MfllMode_Bit_AutoHdr));
}

inline bool isCustomHint1(const enum MfllMode& m) noexcept
{
    return !!(m & (1 << MfllMode_Bit_Custom1));
}

inline bool isCustomHint2(const enum MfllMode& m) noexcept
{
    return !!(m & (1 << MfllMode_Bit_Custom2));
}

inline bool isCustomHint3(const enum MfllMode& m) noexcept
{
    return !!(m & (1 << MfllMode_Bit_Custom3));
}

inline bool isCustomHint4(const enum MfllMode& m) noexcept
{
    return !!(m & (1 << MfllMode_Bit_Custom4));
}

inline int32_t getCustomHint(const enum MfllMode& m) noexcept
{
    int32_t customHint = MTK_CUSTOM_HINT_0;
    if (isCustomHint1(m)) {
        customHint = MTK_CUSTOM_HINT_1;
    }
    else if (isCustomHint2(m)) {
        customHint = MTK_CUSTOM_HINT_2;
    }
    else if (isCustomHint3(m)) {
        customHint = MTK_CUSTOM_HINT_3;
    }
    else if (isCustomHint4(m)) {
        customHint = MTK_CUSTOM_HINT_4;
    }
    return customHint;
}

#define MFLL_THREAD_NAME(x) mfll::setThreadName(x)
inline void setThreadName(const char* name)
{
    ::prctl(PR_SET_NAME, name, 0, 0, 0);
}

inline uint32_t makeGmv32bits(short x, short y)
{
    return (uint32_t) y << 16 | (x & 0x0000FFFF);
}

}; /* namespace mfll */
#endif /* __MFLLTYPES_H__ */
