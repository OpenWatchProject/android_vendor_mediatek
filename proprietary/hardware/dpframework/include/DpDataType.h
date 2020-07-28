#ifndef __DP_DATA_TYPE_H__
#define __DP_DATA_TYPE_H__

#ifndef __KERNEL__
#include "DpConfig.h"
#include <cutils/native_handle.h>
typedef const native_handle_t* buffer_handle_t;

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#endif
#if CONFIG_FOR_OS_WINDOWS
    #include <stddef.h>
    typedef signed char     int8_t;
    typedef unsigned char   uint8_t;
    typedef signed short    int16_t;
    typedef unsigned short  uint16_t;
    typedef signed int      int32_t;
    typedef unsigned int    uint32_t;
    typedef signed long long     int64_t;
    typedef unsigned long long   uint64_t;
#ifndef __unused
    #define __unused
#endif

inline float roundf(float x)
{
    float t;

    if (x >= 0.0) {
        t = floorf(x);
        if (t - x <= -0.5)
            t += 1.0;
        return (t);
    } else {
        t = floorf(-x);
        if (t + x <= -0.5)
            t += 1.0;
        return (-t);
    }
}
#endif  // CONFIG_FOR_OS_WINDOWS

#define DP_UNUSED(expr) do { (void)(expr); } while (0)

#ifndef MAX
    #define MAX(x, y)   ((x) >= (y))? (x): (y)
#endif // MAX

#ifndef MIN
    #define MIN(x, y)   ((x) <= (y))? (x): (y)
#endif  // MIN

#ifndef __KERNEL__
class DpStream;
class DpChannel;
class DpMutex;

class DpBasicBufferPool;
class DpAutoBufferPool;
class DpCommand;
class DpConvertBlk_NV12;
#endif

typedef unsigned long long DpJobID;
typedef int DpEngineType;

typedef enum DP_STATUS_ENUM
{
    DP_STATUS_ABORTED_BY_USER   =  4,
    DP_STATUS_ALL_TEST_DONE     =  3,
    DP_STATUS_ALL_TPIPE_DONE    =  2,
    DP_STATUS_BUFFER_DONE       =  1,
    DP_STATUS_RETURN_SUCCESS    =  0,
    DP_STATUS_INVALID_PARAX     = -1,
    DP_STATUS_INVALID_PORT      = -2,
    DP_STATUS_INVALID_PATH      = -3,
    DP_STATUS_INVALID_FILE      = -4,
    DP_STATUS_INVALID_CHANNEL   = -5,
    DP_STATUS_INVALID_BUFFER    = -6,
    DP_STATUS_INVALID_STATE     = -7,
    DP_STATUS_INVALID_ENGINE    = -8,
    DP_STATUS_INVALID_FORMAT    = -9,
    DP_STATUS_INVALID_X_INPUT   = -10,
    DP_STATUS_INVALID_Y_INPUT   = -11,
    DP_STATUS_INVALID_X_OUTPUT  = -12,
    DP_STATUS_INVALID_Y_OUTPUT  = -13,
    DP_STATUS_INVALID_X_ALIGN   = -14,
    DP_STATUS_INVALID_Y_ALIGN   = -15,
    DP_STATUS_INVALID_WIDTH     = -16,
    DP_STATUS_INVALID_HEIGHT    = -17,
    DP_STATUS_INVALID_CROP      = -18,
    DP_STATUS_INVALID_ANGLE     = -19,
    DP_STATUS_INVALID_EVENT     = -20,
    DP_STATUS_INVALID_OPCODE    = -21,
    DP_STATUS_CAN_NOT_MERGE     = -22,
    DP_STATUS_OUT_OF_MEMORY     = -23,
    DP_STATUS_BUFFER_FULL       = -24,
    DP_STATUS_BUFFER_EMPTY      = -25,
    DP_STATUS_OPERATION_FAILED  = -26,
    DP_STATUS_OVER_MAX_BRANCH   = -27,
    DP_STATUS_OVER_MAX_ENGINE   = -28,
    DP_STATUS_OVER_MAX_BACKUP   = -29,
    DP_STATUS_SCHEDULE_ERROR    = -30,
    DP_STATUS_OVER_MAX_WIDTH    = -31,
    DP_STATUS_OVER_MAX_HEIGHT   = -32,
    DP_STATUS_LEFT_EDGE_ERROR   = -33,
    DP_STATUS_RIGHT_EDGE_ERROR  = -34,
    DP_STATUS_TOP_EDGE_ERROR    = -35,
    DP_STATUS_BOTTOM_EDGE_ERROR = -36,
    DP_STATUS_X_LESS_THAN_LAST  = -37,
    DP_STATUS_Y_LESS_THAN_LAST  = -38,
    DP_STATUS_UNWANTED_X_CAL    = -39,
    DP_STATUS_LOSS_OVER_WIDTH   = -40,
    DP_STATUS_LOSS_OVER_HEIGHT  = -41,
    DP_STATUS_X_ALIGN_ERROR     = -42,
    DP_STATUS_Y_ALIGN_ERROR     = -43,
    DP_STATUS_X_OUT_OVERLAP     = -44,
    DP_STATUS_Y_OUT_OVERLAP     = -45,
    DP_STATUS_BACK_LE_FORWARD   = -46,
    DP_STATUS_UNKNOWN_ERROR     = -47,
} DP_STATUS_ENUM;

typedef enum GAME_SWREG
{
    GAME_TDSHP_GAIN_MID   = 68,
    GAME_TDSHP_GAIN_HIGH  = 69,
    GAME_TDSHP_GAIN       = 70,
    GAME_SW_REG_END       = 71
} GAME_SWREG;

typedef enum DP_MEMORY_ENUM
{
    DP_MEMORY_VA,
    DP_MEMORY_ION,
    DP_MEMORY_PHY,
    DP_MEMORY_MVA
} DP_MEMORY_ENUM;

typedef struct DpJPEGEnc_Config_st // for JPEG port only
{
    int32_t     fileDesc;
    uint32_t    size;
    uint32_t    fQuality;
    uint32_t    soi_en;
    void        *memSWAddr[3];
} DpJPEGEnc_Config;

typedef struct DpVEnc_Config // for VENC port only
{
    /* Venc Modify + */
    unsigned long rVencDrvHandle;
    /* Venc Modify - */
    uint32_t    memYUVMVAAddr[3];
    uint32_t    memYUVMVASize[3];
    void        *memYUVSWAddr[3];
    void        *memOutputSWAddr[3];

    uint32_t*   pNumPABuffer;
    uint32_t*   pPABuffer;
    uint64_t*   pConfigFrameCount;
    uint64_t*   pDequeueFrameCount;
    DpCommand*  pVEncCommander;
    DpMutex*    pFrameMutex;

} DpVEnc_Config;


#ifndef __KERNEL__
class DpRect
{
public:

    enum
    {
        eINVALID_VALUE = -1,
        eINITIAL_VALUE = 0   //TBD, why to set as "0"?
    };

    inline DpRect(void)
        : x(eINITIAL_VALUE), sub_x(eINITIAL_VALUE),
          y(eINITIAL_VALUE), sub_y(eINITIAL_VALUE),
          w(eINITIAL_VALUE), sub_w(eINITIAL_VALUE),
          h(eINITIAL_VALUE), sub_h(eINITIAL_VALUE)
    {}

    inline DpRect(int32_t in_x, int32_t in_y, int32_t in_w, int32_t in_h,
                  uint32_t in_sub_x = 0, uint32_t in_sub_y = 0,
                  uint32_t in_sub_w = 0, uint32_t in_sub_h = 0)
        : x(in_x),
          sub_x(in_sub_x),
          y(in_y),
          sub_y(in_sub_y),
          w(in_w),
          sub_w(in_sub_w),
          h(in_h),
          sub_h(in_sub_h)
    {}

    inline DpRect(const DpRect& rt)
        : x(rt.x),
          sub_x(rt.sub_x),
          y(rt.y),
          sub_y(rt.sub_y),
          w(rt.w),
          sub_w(rt.sub_w),
          h(rt.h),
          sub_h(rt.sub_h)
    {}

    ~DpRect(void) {}

    inline DpRect& operator= (const DpRect& rval)
    {
        if (this != &rval)
        {
            x = rval.x;
            sub_x = rval.sub_x;
            y = rval.y;
            sub_y = rval.sub_y;
            w = rval.w;
            sub_w = rval.sub_w;
            h = rval.h;
            sub_h = rval.sub_h;
        }

        return *this;
    }

    int32_t x;
    uint32_t sub_x;
    int32_t y;
    uint32_t sub_y;
    int32_t w;
    uint32_t sub_w;
    int32_t h;
    uint32_t sub_h;
};
#endif

class DpColorMatrix
{
    /* Define the color matrix.
     *
     * Color matrix would be used in the following transforms:
     * YUV2RGB
     * / R \   / c00 c01 c02 \   / Y - i0 \
     * | G | = | c10 c11 c12 | * | U - i1 |
     * \ B /   \ c20 c21 c22 /   \ V - i2 /
     * YUV2YUV
     * / Y \   / c00 c01 c02 \   / Y - i0 \   / o0 \
     * | U | = | c10 c11 c12 | * | U - i1 | + | o1 |
     * \ V /   \ c20 c21 c22 /   \ V - i2 /   \ o2 /
     *
     * Coefficient value range: -4.00 ~ 3.99
     */

public:
    bool enable;
    float c00; float c01; float c02;
    float c10; float c11; float c12;
    float c20; float c21; float c22;

    inline DpColorMatrix(void)
        : enable(false),
          c00(1.0), c01(0.0), c02(0.0),
          c10(0.0), c11(1.0), c12(0.0),
          c20(0.0), c21(0.0), c22(1.0)
    {}

    inline DpColorMatrix(float in_c00, float in_c01, float in_c02,
                         float in_c10, float in_c11, float in_c12,
                         float in_c20, float in_c21, float in_c22,
                         bool in_enable = true)
        : enable(in_enable),
          c00(in_c00), c01(in_c01), c02(in_c02),
          c10(in_c10), c11(in_c11), c12(in_c12),
          c20(in_c20), c21(in_c21), c22(in_c22)
    {}

    inline DpColorMatrix(const DpColorMatrix& m)
        : enable(m.enable),
          c00(m.c00), c01(m.c01), c02(m.c02),
          c10(m.c10), c11(m.c11), c12(m.c12),
          c20(m.c20), c21(m.c21), c22(m.c22)
    {}

    ~DpColorMatrix(void) {}

    inline DpColorMatrix& operator= (const DpColorMatrix& r)
    {
        if (this != &r)
        {
            enable = r.enable;
            c00 = r.c00; c01 = r.c01; c02 = r.c02;
            c10 = r.c10; c11 = r.c11; c12 = r.c12;
            c20 = r.c20; c21 = r.c21; c22 = r.c22;
        }

        return *this;
    }

    inline DpColorMatrix operator* (const DpColorMatrix& r) const
    {
        if (!enable)
        {
            if (!r.enable)
            {
                return DpColorMatrix();
            }
            return DpColorMatrix(r);
        }
        if (!r.enable)
        {
            return DpColorMatrix(*this);
        }

        return DpColorMatrix(
            c00 * r.c00 + c01 * r.c10 + c02 * r.c20, c00 * r.c01 + c01 * r.c11 + c02 * r.c21, c00 * r.c02 + c01 * r.c12 + c02 * r.c22,
            c10 * r.c00 + c11 * r.c10 + c12 * r.c20, c10 * r.c01 + c11 * r.c11 + c12 * r.c21, c10 * r.c02 + c11 * r.c12 + c12 * r.c22,
            c20 * r.c00 + c21 * r.c10 + c22 * r.c20, c20 * r.c01 + c21 * r.c11 + c22 * r.c21, c20 * r.c02 + c21 * r.c12 + c22 * r.c22);
    }

    inline bool operator== (const DpColorMatrix& r) const
    {
        if (this == &r)
        {
            return true;
        }
        if (!enable && !r.enable)
        {
            return true;
        }
        return (enable == r.enable) &&
            (c00 == r.c00) && (c01 == r.c01) && (c02 == r.c02) &&
            (c10 == r.c10) && (c11 == r.c11) && (c12 == r.c12) &&
            (c20 == r.c20) && (c21 == r.c21) && (c22 == r.c22);
    }

    inline bool operator!= (const DpColorMatrix& r) const
    {
        return !(*this == r);
    }
};

inline DpColorMatrix operator* (const float l, const DpColorMatrix& r)
{
    return DpColorMatrix(
        l * r.c00, l * r.c01, l * r.c02,
        l * r.c10, l * r.c11, l * r.c12,
        l * r.c20, l * r.c21, l * r.c22,
        r.enable);
}


typedef enum DP_PROFILE_ENUM
{
    DP_PROFILE_BT601, //Limited range
    DP_PROFILE_BT709,
    DP_PROFILE_JPEG,
    DP_PROFILE_FULL_BT601 = DP_PROFILE_JPEG,
    DP_PROFILE_BT2020,     // not support for output
    DP_PROFILE_FULL_BT709, // not support for output
    DP_PROFILE_FULL_BT2020 // not support for output
} DP_PROFILE_ENUM;


typedef enum DP_STREAM_ID_ENUM
{
    DP_BLITSTREAM       = 0x10000000,
    DP_FRAGSTREAM       = 0x20000000,
    DP_ISPSTREAM        = 0x30000000,
    DP_ASYNCBLITSTREAM  = 0x40000000,
    DP_VENCSTREAM       = 0x50000000,
    DP_UNKNOWN_STREAM   = 0xF0000000,
} DP_STREAM_ID_ENUM;

typedef enum DP_MEDIA_TYPE_ENUM
{
    MEDIA_UNKNOWN,
    MEDIA_VIDEO,
    MEDIA_PICTURE,
    MEDIA_ISP_PREVIEW,
    MEDIA_VIDEO_CODEC,
    MEDIA_ISP_CAPTURE
} DP_MEDIA_TYPE_ENUM;

typedef enum DP_ISP_FEATURE_ENUM
{
    ISP_FEATURE_DEFAULT,
    ISP_FEATURE_REFOCUS,
    ISP_FEATURE_CLEARZOOM,
    ISP_FEATURE_WCG,
    ISP_FEATURE_DRE,
    ISP_FEATURE_HFG,
} DP_ISP_FEATURE_ENUM;

typedef enum DP_DUMP_TYPE_ENUM
{
    DUMP_ISP_PRV = 1,
    DUMP_ISP_CAP,
    DUMP_ISP_PRV_CAP
} DP_DUMP_TYPE_ENUM;

typedef struct _DP_VDEC_DRV_COLORDESC_T {
    uint32_t u4ColorPrimaries; // colour_primaries emum
    uint32_t u4TransformCharacter; // transfer_characteristics emum
    uint32_t u4MatrixCoeffs; // matrix_coeffs emum
    uint32_t u4DisplayPrimariesX[3]; // display_primaries_x
    uint32_t u4DisplayPrimariesY[3]; // display_primaries_y
    uint32_t u4WhitePointX; // white_point_x
    uint32_t u4WhitePointY; // white_point_y
    uint32_t u4MaxDisplayMasteringLuminance; // max_display_mastering_luminance
    uint32_t u4MinDisplayMasteringLuminance; // min_display_mastering_luminance
    uint32_t u4MaxContentLightLevel; // max_content_light_level
    uint32_t u4MaxPicAverageLightLevel; // max_pic_average_light_level
} DP_VDEC_DRV_COLORDESC_T;

typedef enum ISP_CAPTURE_SCENRAIO_ENUM
{
    CAPTURE_SINGLE,
    CAPTURE_MULTI,
} ISP_CAPTURE_SCENRAIO_ENUM;

typedef enum VIDEO_INFO_SCENARIO_ENUM
{
    INFO_VIDEO = 0,
    INFO_GAME  = 1,
}VIDEO_INFO_SCENARIO_ENUM;

typedef enum USER_INFO_SCENARIO_ENUM
{
    INFO_UNKNOWN = 0,
    INFO_HWC = 1,
    INFO_GPU  = 2,
}USER_INFO_SCENARIO_ENUM;

#define MDPSETTING_MAX_SIZE (30000)

struct MDPSetting {
    void* buffer;
    uint32_t size;
    uint32_t offset;

    MDPSetting():buffer(NULL),
                 size(0),
                 offset(0)
    {
    }
};

#define DRE_DEFAULT_USERID (0x8000000000000000L)

struct DpDREParam {
    enum Cmd {
        Nothing = 0,
        Initialize = 1 << 0,
        UnInitialize = 1 << 1,
        Default = 1 << 2,
        Generate = 1 << 3,
        Apply = 1 << 4
    };

    enum DRESRAM {
        SRAM00 = 0,
        SRAM01,
        SRAMDefault
    };

    // members
    uint32_t           cmd;
    unsigned long long userId;
    void               *buffer;
    uint32_t           SRAMId;
    // tuning member
    void               *p_customSetting;
    uint32_t           customIndex;
    // app active array for FD
    uint32_t           activeWidth;
    uint32_t           activeHeight;

    DpDREParam():cmd(Cmd::Nothing),
                userId(DRE_DEFAULT_USERID),
                buffer(NULL),
                SRAMId(DRESRAM::SRAMDefault),
                p_customSetting(NULL),
                activeWidth(0),
                activeHeight(0)
    {
    }
};

struct ClearZoomParam {
    ISP_CAPTURE_SCENRAIO_ENUM captureShot;
    // tuning member
    void               *p_customSetting;

    ClearZoomParam():captureShot(CAPTURE_SINGLE),
                     p_customSetting(NULL)
    {
    }
};

struct DpHFGParam{
    // SLK param from tuning buffer
    void *p_slkParam;
    // NVRam tuning buffer
    void    *p_upperSetting;
    void    *p_lowerSetting;
    uint32_t upperISO;
    uint32_t lowerISO;
    DpHFGParam():p_slkParam(NULL),
        p_upperSetting(NULL),
        p_lowerSetting(NULL),
        upperISO(0),
        lowerISO(0)
    {
    }
};

struct VSDOFParam {
    bool     isRefocus;
    uint8_t  defaultUpTable;
    uint8_t  defaultDownTable;
    uint32_t IBSEGain;
    uint32_t switchRatio6Tap6nTap;
    uint32_t switchRatio6nTapAcc;

    VSDOFParam():isRefocus(false),
                 defaultUpTable(0),
                 defaultDownTable(0),
                 IBSEGain(0),
                 switchRatio6Tap6nTap(1),
                 switchRatio6nTapAcc(26)
    {
    }
};

struct DpVideoParam
{
    uint32_t id;
    uint32_t timeStamp;
    bool     isHDR2SDR;
    bool     isThumbnail;
    VIDEO_INFO_SCENARIO_ENUM videoScenario;
    USER_INFO_SCENARIO_ENUM  userScenario;
#if CONFIG_FOR_OS_ANDROID
    buffer_handle_t grallocExtraHandle;
#endif
    DP_VDEC_DRV_COLORDESC_T HDRinfo;
    DpDREParam dpDREParam;
    uint32_t paramTable;
    uint32_t reserved[6];   // padding and reserved

    DpVideoParam():isHDR2SDR(false)
                   ,isThumbnail(false)
                   ,videoScenario(INFO_VIDEO)
                   ,userScenario(INFO_UNKNOWN)
#if CONFIG_FOR_OS_ANDROID
                 ,grallocExtraHandle(NULL)
                 ,paramTable(0)
#endif
    {
    }
};

typedef struct
{
    bool withHist;
    uint32_t info[20];
    uint32_t iso;
    uint32_t reserved[8];  // padding and reserved
} DpImageParam;

struct DpIspParam {
    uint32_t iso;
    ClearZoomParam clearZoomParam;
    VSDOFParam vsdofParam;
    DpDREParam dpDREParam;
    DpHFGParam dpHFGParam;
    bool isIspScenario;
    uint32_t ispScenario;
    uint32_t timestamp;
    uint32_t frameNo;
    uint32_t requestNo;
    uint32_t lensId;
    char userString[8];
    bool enableDump;
    MDPSetting *p_mdpSetting;
    int LV;
    unsigned int *LCSO;
    unsigned int LCSO_Size;
    unsigned int *DCE;
    unsigned int DCE_Size;
    unsigned int *LCE;
    unsigned int LCE_Size;
    void         *p_faceInfor;
    uint32_t reserved[12];

    DpIspParam():p_mdpSetting(NULL),
	            enableDump(false),
                LV(0),
                LCSO(NULL),
                LCSO_Size(0),
                DCE(NULL),
                DCE_Size(0),
                LCE(NULL),
                LCE_Size(0),
                p_faceInfor(NULL)
    {
    }
};

union DpPqParamUnion{
    DpVideoParam video;
    DpImageParam image;
    DpIspParam isp;

    DpPqParamUnion()
    {

    }
};

typedef enum PqEnableFlag
{
    /* if PQ_DEFAULT_ENABLE == 1, use default enable set by PQ */
    /* The other enable will be regardless. */
    PQ_DEFAULT_EN        = 1 << 0,
    PQ_COLOR_EN          = 1 << 1,
    PQ_SHP_EN            = 1 << 2,
    PQ_ULTRARES_EN       = 1 << 3,
    PQ_REFOCUS_EN        = 1 << 4,
    PQ_DYN_CONTRAST_EN   = 1 << 5,
    PQ_VIDEO_HDR_EN      = 1 << 6,
    PQ_DRE_EN            = 1 << 7,
    PQ_CCORR_EN          = 1 << 8,
    PQ_HFG_EN            = 1 << 9,
}PqEnableFlag;

typedef enum DP_GAMUT_ENUM
{
    DP_GAMUT_UNKNOWN = 0,
    DP_GAMUT_SRGB,
    DP_GAMUT_DISPLAY_P3,
    DP_GAMUT_BT601,
    DP_GAMUT_BT709,
    DP_GAMUT_BT2020
} DP_GAMUT_ENUM;

struct DpPqParam {
    uint32_t enable;
    DP_MEDIA_TYPE_ENUM scenario;
    DP_GAMUT_ENUM srcGamut;
    DP_GAMUT_ENUM dstGamut;

    DpPqParamUnion u;

    DpPqParam():enable(0),
                scenario(MEDIA_UNKNOWN),
                srcGamut(DP_GAMUT_SRGB),
                dstGamut(DP_GAMUT_SRGB)
    {
        memset(&u, 0, sizeof(u));
    }
};

struct DpPqConfig {
    uint32_t enSharp;
    uint32_t enUR;
    uint32_t enReFocus;
    uint32_t enDC;
    uint32_t enColor;
    uint32_t enHDR;
    uint32_t enCcorr;
    uint32_t enDRE;
    uint32_t enHFG;
};


// Format group: 0-RGB, 1-YUV, 2-Bayer raw, 3-compressed format
#define DP_COLORFMT_PACK(PACKED, LOOSE, VIDEO, PLANE, HFACTOR, VFACTOR, BITS, GROUP, SWAP_ENABLE, UNIQUEID)  \
    (((PACKED)      << 28) |                                                             \
     ((LOOSE)       << 27) |                                                             \
     ((VIDEO)       << 23) |                                                             \
     ((PLANE)       << 21) |                                                             \
     ((HFACTOR)     << 19) |                                                             \
     ((VFACTOR)     << 18) |                                                             \
     ((BITS)        << 8)  |                                                             \
     ((GROUP)       << 6)  |                                                             \
     ((SWAP_ENABLE) << 5)  |                                                             \
     ((UNIQUEID)    << 0))

#define DP_COLOR_GET_10BIT_PACKED(color)        ((0x10000000 & color) >> 28)
#define DP_COLOR_GET_10BIT_LOOSE(color)        (((0x18000000 & color) >> 27) == 1)
#define DP_COLOR_GET_10BIT_TILE_MODE(color)    (((0x18000000 & color) >> 27) == 3)
#define DP_COLOR_GET_10BIT_JUMP_MODE(color)     ((0x04000000 & color) >> 26)
#define DP_COLOR_GET_UFP_ENABLE(color)          ((0x02000000 & color) >> 25)
#define DP_COLOR_GET_INTERLACED_MODE(color)     ((0x01000000 & color) >> 24)
#define DP_COLOR_GET_BLOCK_MODE(color)          ((0x00800000 & color) >> 23)
#define DP_COLOR_GET_PLANE_COUNT(color)         ((0x00600000 & color) >> 21) /* 1-3 */
#define DP_COLOR_GET_H_SUBSAMPLE(color)         ((0x00180000 & color) >> 19) /* 0-2 */
#define DP_COLOR_GET_V_SUBSAMPLE(color)         ((0x00040000 & color) >> 18) /* 0-1 */
#define DP_COLOR_BITS_PER_PIXEL(color)          ((0x0003FF00 & color) >>  8)
#define DP_COLOR_GET_COLOR_GROUP(color)         ((0x000000C0 & color) >>  6)
#define DP_COLOR_GET_SWAP_ENABLE(color)         ((0x00000020 & color) >>  5)
#define DP_COLOR_GET_UNIQUE_ID(color)           ((0x0000001F & color) >>  0)
#define DP_COLOR_GET_HW_FORMAT(color)           ((0x0000001F & color) >>  0)

#define DP_COLOR_IS_YUV(color)                  (DP_COLOR_GET_COLOR_GROUP(color) == 1)
#define DP_COLOR_IS_UV_COPLANE(color)           ((DP_COLOR_GET_PLANE_COUNT(color) == 2) && \
                                                 DP_COLOR_IS_YUV(color))
#define DP_COLOR_GET_AUO_MODE(color)            (DP_COLOR_GET_10BIT_JUMP_MODE(color))

typedef enum DP_COLOR_ENUM
{
    DP_COLOR_UNKNOWN        = 0,

    DP_COLOR_FULLG8         = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0,  8, 2,  0, 21),
    DP_COLOR_FULLG10        = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0, 10, 2,  0, 21),
    DP_COLOR_FULLG12        = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0, 12, 2,  0, 21),
    DP_COLOR_FULLG14        = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0, 14, 2,  0, 21),
    DP_COLOR_UFO10          = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0, 10, 2,  0, 24),

    DP_COLOR_BAYER8         = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0,  8, 2,  0, 20),
    DP_COLOR_BAYER10        = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0, 10, 2,  0, 20),
    DP_COLOR_BAYER12        = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0, 12, 2,  0, 20),
    DP_COLOR_BAYER22        = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0, 24, 2,  0, 20), // fix 24 bits for pixel
    DP_COLOR_RGB48          = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0, 48, 0,  0, 23),
    //for Bayer+Mono raw-16
    DP_COLOR_RGB565_RAW     = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0, 16, 2,  0, 0),

    DP_COLOR_BAYER8_UNPAK   = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0,  8, 2,  0, 22), // fix 16 bits for pixel
    DP_COLOR_BAYER10_UNPAK  = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0, 10, 2,  0, 22), // fix 16 bits for pixel
    DP_COLOR_BAYER12_UNPAK  = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0, 12, 2,  0, 22), // fix 16 bits for pixel
    DP_COLOR_BAYER14_UNPAK  = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0, 14, 2,  0, 22), // fix 16 bits for pixel
    DP_COLOR_BAYER16_UNPAK  = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0, 16, 2,  0, 22), // fix 16 bits for pixel

    // Unified format
    DP_COLOR_GREY           = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0,  8, 1,  0, 7),

    DP_COLOR_RGB565         = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0, 16, 0,  0, 0),
    DP_COLOR_BGR565         = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0, 16, 0,  1, 0),
    DP_COLOR_RGB888         = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0, 24, 0,  1, 1),
    DP_COLOR_BGR888         = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0, 24, 0,  0, 1),
    DP_COLOR_RGBA8888       = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0, 32, 0,  1, 2),
    DP_COLOR_BGRA8888       = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0, 32, 0,  0, 2),
    DP_COLOR_ARGB8888       = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0, 32, 0,  1, 3),
    DP_COLOR_ABGR8888       = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0, 32, 0,  0, 3),

    DP_COLOR_UYVY           = DP_COLORFMT_PACK(0, 0, 0, 1, 1, 0, 16, 1,  0, 4),
    DP_COLOR_VYUY           = DP_COLORFMT_PACK(0, 0, 0, 1, 1, 0, 16, 1,  1, 4),
    DP_COLOR_YUYV           = DP_COLORFMT_PACK(0, 0, 0, 1, 1, 0, 16, 1,  0, 5),
    DP_COLOR_YVYU           = DP_COLORFMT_PACK(0, 0, 0, 1, 1, 0, 16, 1,  1, 5),

    DP_COLOR_I420           = DP_COLORFMT_PACK(0, 0, 0, 3, 1, 1,  8, 1,  0, 8),
    DP_COLOR_YV12           = DP_COLORFMT_PACK(0, 0, 0, 3, 1, 1,  8, 1,  1, 8),
    DP_COLOR_I422           = DP_COLORFMT_PACK(0, 0, 0, 3, 1, 0,  8, 1,  0, 9),
    DP_COLOR_YV16           = DP_COLORFMT_PACK(0, 0, 0, 3, 1, 0,  8, 1,  1, 9),
    DP_COLOR_I444           = DP_COLORFMT_PACK(0, 0, 0, 3, 0, 0,  8, 1,  0, 10),
    DP_COLOR_YV24           = DP_COLORFMT_PACK(0, 0, 0, 3, 0, 0,  8, 1,  1, 10),

    DP_COLOR_NV12           = DP_COLORFMT_PACK(0, 0, 0, 2, 1, 1,  8, 1,  0, 12),
    DP_COLOR_NV21           = DP_COLORFMT_PACK(0, 0, 0, 2, 1, 1,  8, 1,  1, 12),
    DP_COLOR_NV16           = DP_COLORFMT_PACK(0, 0, 0, 2, 1, 0,  8, 1,  0, 13),
    DP_COLOR_NV61           = DP_COLORFMT_PACK(0, 0, 0, 2, 1, 0,  8, 1,  1, 13),
    DP_COLOR_NV24           = DP_COLORFMT_PACK(0, 0, 0, 2, 0, 0,  8, 1,  0, 14),
    DP_COLOR_NV42           = DP_COLORFMT_PACK(0, 0, 0, 2, 0, 0,  8, 1,  1, 14),

    // Mediatek proprietary format
    //Frame mode + Block mode + UFO
    DP_COLOR_420_BLKP_UFO   = DP_COLORFMT_PACK(0, 0, 5, 2, 1, 1, 256, 1, 0, 12),
    //Frame mode + Block mode + UFO AUO
    DP_COLOR_420_BLKP_UFO_AUO   = DP_COLORFMT_PACK(0, 0, 13, 2, 1, 1, 256, 1, 0, 12),
    //Frame mode + Block mode
    DP_COLOR_420_BLKP       = DP_COLORFMT_PACK(0, 0, 1, 2, 1, 1, 256, 1, 0, 12),
    //Field mode + Block mode
    DP_COLOR_420_BLKI       = DP_COLORFMT_PACK(0, 0, 3, 2, 1, 1, 256, 1, 0, 12),
    //Frame mode
    DP_COLOR_422_BLKP       = DP_COLORFMT_PACK(0, 0, 1, 1, 1, 0, 512, 1, 0, 4),

    DP_COLOR_IYU2           = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0, 24,  1, 0, 25),
    DP_COLOR_YUV444         = DP_COLORFMT_PACK(0, 0, 0, 1, 0, 0, 24,  1, 0, 30),

    // Mediatek proprietary 10bit format
    DP_COLOR_RGBA1010102    = DP_COLORFMT_PACK(1, 0, 0, 1, 0, 0, 32,  0, 1, 2),
    DP_COLOR_BGRA1010102    = DP_COLORFMT_PACK(1, 0, 0, 1, 0, 0, 32,  0, 0, 2),
    DP_COLOR_ARGB1010102    = DP_COLORFMT_PACK(1, 0, 0, 1, 0, 0, 32,  0, 1, 3),
    DP_COLOR_ABGR1010102    = DP_COLORFMT_PACK(1, 0, 0, 1, 0, 0, 32,  0, 0, 3),
    //Packed 10bit UYVY
    DP_COLOR_UYVY_10P       = DP_COLORFMT_PACK(1, 0, 0, 1, 1, 0, 20,  1, 0, 4),
    //Packed 10bit NV21
    DP_COLOR_NV12_10P       = DP_COLORFMT_PACK(1, 0, 0, 2, 1, 1, 10,  1, 0, 12),
    DP_COLOR_NV21_10P       = DP_COLORFMT_PACK(1, 0, 0, 2, 1, 1, 10,  1, 1, 12),
    //Frame mode + Block mode
    DP_COLOR_420_BLKP_10_H          = DP_COLORFMT_PACK(1, 0, 1, 2, 1, 1, 320, 1, 0, 12),
    //Frame mode + HEVC tile mode
    DP_COLOR_420_BLKP_10_V          = DP_COLORFMT_PACK(1, 1, 1, 2, 1, 1, 320, 1, 0, 12),
    //Frame mode + Block mode + Jump
    DP_COLOR_420_BLKP_10_H_JUMP     = DP_COLORFMT_PACK(1, 0, 9, 2, 1, 1, 320, 1, 0, 12),
    //Frame mode + HEVC tile mode + Jump
    DP_COLOR_420_BLKP_10_V_JUMP     = DP_COLORFMT_PACK(1, 1, 9, 2, 1, 1, 320, 1, 0, 12),
    //Frame mode + Block mode
    DP_COLOR_420_BLKP_UFO_10_H      = DP_COLORFMT_PACK(1, 0, 5, 2, 1, 1, 320, 1, 0, 12),
    //Frame mode + HEVC tile mode
    DP_COLOR_420_BLKP_UFO_10_V      = DP_COLORFMT_PACK(1, 1, 5, 2, 1, 1, 320, 1, 0, 12),
    //Frame mode + Block mode + Jump
    DP_COLOR_420_BLKP_UFO_10_H_JUMP = DP_COLORFMT_PACK(1, 0, 13, 2, 1, 1, 320, 1, 0, 12),
    //Frame mode + HEVC tile mode + Jump
    DP_COLOR_420_BLKP_UFO_10_V_JUMP = DP_COLORFMT_PACK(1, 1, 13, 2, 1, 1, 320, 1, 0, 12),

    // Loose 10bit format
    DP_COLOR_UYVY_10L       = DP_COLORFMT_PACK(0, 1, 0, 1, 1, 0, 20,  1, 0, 4),
    DP_COLOR_VYUY_10L       = DP_COLORFMT_PACK(0, 1, 0, 1, 1, 0, 20,  1, 1, 4),
    DP_COLOR_YUYV_10L       = DP_COLORFMT_PACK(0, 1, 0, 1, 1, 0, 20,  1, 0, 5),
    DP_COLOR_YVYU_10L       = DP_COLORFMT_PACK(0, 1, 0, 1, 1, 0, 20,  1, 1, 5),
    DP_COLOR_NV12_10L       = DP_COLORFMT_PACK(0, 1, 0, 2, 1, 1, 16,  1, 0, 12),
    DP_COLOR_NV21_10L       = DP_COLORFMT_PACK(0, 1, 0, 2, 1, 1, 16,  1, 1, 12),
    DP_COLOR_NV16_10L       = DP_COLORFMT_PACK(0, 1, 0, 2, 1, 0, 16,  1, 0, 13),
    DP_COLOR_NV61_10L       = DP_COLORFMT_PACK(0, 1, 0, 2, 1, 0, 16,  1, 1, 13),
    DP_COLOR_YV12_10L       = DP_COLORFMT_PACK(0, 1, 0, 3, 1, 1, 16,  1, 1, 8),
    DP_COLOR_I420_10L       = DP_COLORFMT_PACK(0, 1, 0, 3, 1, 1, 16,  1, 0, 8),

    DP_COLOR_YV12_10P       = DP_COLORFMT_PACK(1, 0, 0, 3, 1, 1, 10,  1, 1, 8),
    DP_COLOR_I422_10P       = DP_COLORFMT_PACK(1, 0, 0, 3, 1, 0, 10,  1, 0, 9),
    DP_COLOR_NV16_10P       = DP_COLORFMT_PACK(1, 0, 0, 2, 1, 0, 10,  1, 0, 13),
    DP_COLOR_NV61_10P       = DP_COLORFMT_PACK(1, 0, 0, 2, 1, 0, 10,  1, 1, 13),

    DP_COLOR_I422_10L       = DP_COLORFMT_PACK(0, 1, 0, 3, 1, 0, 16,  1, 0, 9),
} DP_COLOR_ENUM;

// Legacy for 6589 compatible
typedef DP_COLOR_ENUM DpColorFormat;

#define eYUV_420_3P             DP_COLOR_I420
#define eYUV_420_2P_YUYV        DP_COLOR_YUYV
#define eYUV_420_2P_UYVY        DP_COLOR_UYVY
#define eYUV_420_2P_YVYU        DP_COLOR_YVYU
#define eYUV_420_2P_VYUY        DP_COLOR_VYUY
#define eYUV_420_2P_ISP_BLK     DP_COLOR_420_BLKP
#define eYUV_420_2P_VDO_BLK     DP_COLOR_420_BLKI
#define eYUV_422_3P             DP_COLOR_I422
#define eYUV_422_2P             DP_COLOR_NV16
#define eYUV_422_I              DP_COLOR_YUYV
#define eYUV_422_I_BLK          DP_COLOR_422_BLKP
#define eYUV_444_3P             DP_COLOR_I444
#define eYUV_444_2P             DP_COLOR_NV24
#define eYUV_444_1P             DP_COLOR_YUV444
#define eBAYER8                 DP_COLOR_BAYER8
#define eBAYER10                DP_COLOR_BAYER10
#define eBAYER12                DP_COLOR_BAYER12
#define eRGB565                 DP_COLOR_RGB565
#define eBGR565                 DP_COLOR_BGR565
#define eRGB888                 DP_COLOR_RGB888
#define eBGR888                 DP_COLOR_BGR888
#define eARGB8888               DP_COLOR_ARGB8888
#define eABGR8888               DP_COLOR_ABGR8888
#define DP_COLOR_XRGB8888       DP_COLOR_ARGB8888
#define DP_COLOR_XBGR8888       DP_COLOR_ABGR8888
#define eRGBA8888               DP_COLOR_RGBA8888
#define eBGRA8888               DP_COLOR_BGRA8888
#define eXRGB8888               DP_COLOR_XRGB8888
#define eXBGR8888               DP_COLOR_XBGR8888
#define DP_COLOR_RGBX8888       DP_COLOR_RGBA8888
#define DP_COLOR_BGRX8888       DP_COLOR_BGRA8888
#define eRGBX8888               DP_COLOR_RGBX8888
#define eBGRX8888               DP_COLOR_BGRX8888
#define ePARGB8888              DP_COLOR_PARGB8888
#define eXARGB8888              DP_COLOR_XARGB8888
#define ePABGR8888              DP_COLOR_PABGR8888
#define eXABGR8888              DP_COLOR_XABGR8888
#define eGREY                   DP_COLOR_GREY
#define eI420                   DP_COLOR_I420
#define eYV12                   DP_COLOR_YV12
#define eIYU2                   DP_COLOR_IYU2


#define eYV21                   DP_COLOR_I420
#define eNV12_BLK               DP_COLOR_420_BLKP
#define eNV12_BLK_FCM           DP_COLOR_420_BLKI
#define eYUV_420_3P_YVU         DP_COLOR_YV12

#define eNV12_BP                DP_COLOR_420_BLKP
#define eNV12_BI                DP_COLOR_420_BLKI
#define eNV12                   DP_COLOR_NV12
#define eNV21                   DP_COLOR_NV21
#define eI422                   DP_COLOR_I422
#define eYV16                   DP_COLOR_YV16
#define eNV16                   DP_COLOR_NV16
#define eNV61                   DP_COLOR_NV61
#define eUYVY                   DP_COLOR_UYVY
#define eVYUY                   DP_COLOR_VYUY
#define eYUYV                   DP_COLOR_YUYV
#define eYVYU                   DP_COLOR_YVYU
#define eUYVY_BP                DP_COLOR_422_BLKP
#define eI444                   DP_COLOR_I444
#define eNV24                   DP_COLOR_NV24
#define eNV42                   DP_COLOR_NV42
#define DP_COLOR_YUY2           DP_COLOR_YUYV
#define eYUY2                   DP_COLOR_YUY2
#define eY800                   DP_COLOR_GREY
//#define eIYU2
#define eMTKYUV                 DP_COLOR_422_BLKP

#define eCompactRaw1            DP_COLOR_BAYER10

#define eNV12_10L               DP_COLOR_NV12_10L
#define eNV21_10L               DP_COLOR_NV21_10L

#define eNV12_10P               DP_COLOR_NV12_10P
#define eNV21_10P               DP_COLOR_NV21_10P

enum DpInterlaceFormat
{
    eInterlace_None,
    eTop_Field,
    eBottom_Field
};

enum DpSecure
{
    DP_SECURE_NONE  = 0,
    DP_SECURE       = 1,
    DP_SECURE_PROTECTED = 2,
    DP_SECURE_SHIFT = 8
};

enum DpBlitUser
{
    DP_BLIT_HWC0 = 0,
    DP_BLIT_HWC1 = 1,
    DP_BLIT_HWC2 = 2,
    DP_BLIT_HWC3 = 3,
    DP_BLIT_HWC4 = 4,
    DP_BLIT_HWC5 = 5,

    DP_BLIT_GENERAL_USER = DP_BLIT_HWC0,
    DP_BLIT_GPU = DP_BLIT_HWC1,
    // GPU_2
    // RESERVED
    DP_BLIT_HWC_120FPS = DP_BLIT_HWC4,
    DP_BLIT_ADDITIONAL_DISPLAY = DP_BLIT_HWC5,
};

#define MAX_NUM_READBACK_REGS (1024)
#define MAX_NUM_READBACK_PA_BUFFER (MAX_NUM_READBACK_REGS * 2)

#define MAX_FRAME_INFO_SIZE (1024)

#define VENC_ENABLE_FLAG    (0x08967)

#endif  // __DP_DATA_TYPE_H__
