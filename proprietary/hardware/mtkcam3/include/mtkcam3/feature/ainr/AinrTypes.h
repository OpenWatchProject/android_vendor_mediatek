#ifndef __AINRTYPES_H__
#define __AINRTYPES_H__

#include "AinrDefs.h"

// LINUX
#include <sys/prctl.h> // prctl(PR_SET_NAME)

// STL
#include <string>  // std::string
#include <map>  // std::map
#include <mutex>
#include <tuple>  // std::tuple

// MTKCAM
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
// MTK utils
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/metadata/IMetadata.h>


namespace ainr {

    /* Ainr error code */
    enum AinrErr
    {
        AinrErr_Ok = 0,
        AinrErr_Shooted,
        AinrErr_AlreadyExist,
        AinrErr_NotInited,
        AinrErr_BadArgument,
        AinrErr_IllegalBlendFrameNum,
        AinrErr_IllegalCaptureFrameNum,
        AinrErr_NullPointer,
        AinrErr_NotImplemented,
        AinrErr_NotSupported,
        /* This error code indicates to instance creation is failed */
        AinrErr_CreateInstanceFailed,
        /* Load image failed */
        AinrErr_LoadImageFailed,
        /* Save image failed */
        AinrErr_SaveImageFailed,
        /* Others error will be categoried here */
        AinrErr_UnexpectedError,
        /* indicates to size only */
        AinrErr_Size,
    };

    /**
     *  Ainr mode, bit map
     *    - bit 0: Zsd on or off
     *    - bit 1: is MFB
     *    - bit 2: is AIS
     *    - bit 3: is Single Capturer
     *    - bit 4: is MFHR
     *    - bit 5: is ZHDR
     *    - bit 6: is AutoHDR
     * */
    enum AinrMode
    {
        AinrMode_Off                = 0x00,
        AinrMode_NormalAinr         = 0x02,
        AinrMode_ZsdAinr            = 0x03,
        AinrMode_NormalAis          = 0x04,
        AinrMode_ZsdAis             = 0x05,
        AinrMode_NormalHdr          = 0x06,
        AinrMode_ZsdHdr             = 0x07,
        AinrMode_NormalSingleFrame  = 0x08,
        AinrMode_ZsdSingleFrame     = 0x09,
        AinrMode_NormalMfhr         = 0x10,
        AinrMode_ZsdMfhr            = 0x11,
        AinrMode_ZhdrNormalAinr     = 0x22,
        AinrMode_ZhdrZsdAinr        = 0x23,
        AinrMode_ZhdrNormalAis      = 0x24,
        AinrMode_ZhdrZsdAis         = 0x25,
        AinrMode_AutoZhdrNormalAinr = 0x62,
        AinrMode_AutoZhdrZsdAinr    = 0x63,
        AinrMode_AutoZhdrNormalAis  = 0x64,
        AinrMode_AutoZhdrZsdAis     = 0x65,
        /* Bits */
        AinrMode_Bit_Zsd            = 0,
        AinrMode_Bit_Ainr           = 1,
        AinrMode_Bit_Ais            = 2,
        AinrMode_Bit_SingleFrame    = 3,
        AinrMode_Bit_Mfhr           = 4,
        AinrMode_Bit_Zhdr           = 5,
        AinrMode_Bit_AutoHdr        = 6,
        AinrMode_Bit_Reserve1       = 7,
        AinrMode_Bit_Reserve2       = 8,
        AinrMode_Bit_Reserve3       = 9,
        AinrMode_Bit_Reserve4       = 10,
        AinrMode_Bit_Reserve5       = 11,
        AinrMode_Bit_Reserve6       = 12,
        AinrMode_Bit_Custom1        = 13,
        AinrMode_Bit_Custom2        = 14,
        AinrMode_Bit_Custom3        = 15,
        AinrMode_Bit_Custom4        = 16,
    };

    enum DumpLevel
    {
        AinrDumpClose   = 0,
        AinrDumpPartial = 1,
        AinrDumpWorking = 2,
        AinrDumpAll     = 3,
    };

    enum AinrFeatureType
    {
        AINR_SINGLE = 0,
        AINR_ONLY   = 1,
        AINR_YHDR   = 2,
        AIHDR       = 3,
    };

    enum AinrOnlineDeviceMode
    {
        ONLINE_DEVICE_MODE_DISABLE = 0,
        ONLINE_DEVICE_MODE_DUMP_DBG_TUNING = 1,
        ONLINE_DEVICE_MODE_ENABLE = 2,
    };

    enum class AinrUpkMode: int32_t {
        UPK_DEFAULT = 0,
        UPK_CPU     = 1,
        UPK_VPU     = 2,
    };

    enum class AinrCoreMode: uint32_t {
        Full_SAMPLE = 0,
        Down_SAMPLE = 1,
    };

    enum class exposureType : uint32_t {
        SSE = 0,
        SE = 1,
        NE = 2,
        LE = 3,
        BYPASS = 255,
    };

    enum class formatType : int32_t {
        RAW  = 0,
        BLOB = 1,
        YUV  = 2
    };
//
//-----------------------------------------------------------------------------
//
    typedef struct AinrAeInfo {
        int64_t exposureTime;
        int32_t sensorGain;
        int32_t ispGain;

        AinrAeInfo(void) noexcept
            : exposureTime(0)
            , sensorGain(0)
            , ispGain(0)
            {}
    } AinrAeInfo_t;

    /**
     *  Capture parameter for AINR RAW
     */
    typedef struct AinrConfig
    {
        int              sensor_id; // opened sensor id
        unsigned int     captureNum;
        unsigned int     imgoWidth;
        unsigned int     imgoHeight;
        unsigned int     rrzoWidth;
        unsigned int     rrzoHeight;
        int              requestNum;
        int              frameNum;
        uint32_t         uniqueKey;
        uint32_t         dgnGain;
        size_t           imgoStride;
        bool             needTileMode;
        AinrFeatureType  algoType;
        bool             needDRC;

        AinrConfig(void) noexcept
            : sensor_id(0)
            , captureNum(0)
            , imgoWidth(0)
            , imgoHeight(0)
            , rrzoWidth(0)
            , rrzoHeight(0)
            , requestNum(0)
            , frameNum(0)
            , uniqueKey(0)
            , dgnGain(0)
            , imgoStride(0)
            , needTileMode(true)
            , algoType(AINR_SINGLE)
            , needDRC(false)
            {}
    } AinrConfig_t;

    typedef struct AinrStrategyConfig
    {
        // input
        int           sensorId;
        int           ainrIso;
        NSCam::MSize  size;

        // output
        std::vector<exposureType>
                      vEvTypes;
        exposureType  bssEvTypes;
        int           frameCapture;
        int           ainrIsoTh;
        bool          enableAinr;

        AinrStrategyConfig(void) noexcept
            : sensorId(0)
            , ainrIso(0)
            , size(NSCam::MSize())
            , bssEvTypes(exposureType::NE)
            , frameCapture(0)
            , ainrIsoTh(0)
            , enableAinr(false)
        {}
    } AinrStrategyConfig_t;

    typedef struct AiHgInfo
    {
        // Algo input
        NSCam::MSize feoSize;
        // Algo output: get from algo
        int32_t workingBufSize;

        AiHgInfo(void) noexcept
            : workingBufSize(0)
            {}
    } AiHgInfo;

    typedef struct AiHgParam
    {
        // hgParam
        NSCam::IImageBuffer *imgoBuf;
        NSCam::IImageBuffer *rrzoBuf;
        NSCam::IImageBuffer *baseFeBuf;
        NSCam::IImageBuffer *refFeBuf;
        NSCam::IImageBuffer *baseFmBuf;
        NSCam::IImageBuffer *refFmBuf;
        NSCam::IImageBuffer *gridX;
        NSCam::IImageBuffer *gridY;
        NSCam::MRect cropRect;
        NSCam::MSize gridMapSize;

        AiHgParam(void) noexcept
            : imgoBuf(nullptr)
            , rrzoBuf(nullptr)
            , baseFeBuf(nullptr)
            , refFeBuf(nullptr)
            , baseFmBuf(nullptr)
            , refFmBuf(nullptr)
            , gridX(nullptr)
            , gridY(nullptr)
            {}
    } AiHgParam_t;

    typedef struct AiBasePackage
    {
        NSCam::IImageBuffer *inBuf;
        NSCam::IImageBuffer *outBuf;

        AiBasePackage(void) noexcept
            : inBuf(nullptr)
            , outBuf(nullptr)
            {}
    } AiBasePackage_t;

    typedef struct AiRefPackage
    {
        NSCam::IImageBuffer *rgBuf;
        NSCam::IImageBuffer *gbBuf;
        NSCam::IImageBuffer *outBuf;

        AiRefPackage(void) noexcept
            : rgBuf(nullptr)
            , gbBuf(nullptr)
            , outBuf(nullptr)
            {}
    } AiRefPackage_t;

    typedef struct AiParam
    {
        int32_t width;
        int32_t height;
        int32_t stride;
        int32_t outBits; // 10 or 12
        AinrFeatureType algoType;
        int32_t obOfst[4];
        int32_t dgnGain[4];

        AiParam(void) noexcept
            : width(0)
            , height(0)
            , stride(0)
            , outBits(10)
            , algoType(AINR_ONLY)
            {}
    } AiParam_t;

    typedef struct AiCoreParam
    {
        // Typedef
        using Buffers = std::vector<NSCam::IImageBuffer*>;
        // tuple<expostureTime (ns), sensorGain, ispGain>
        using ExpInfoTuples
                = std::vector<std::tuple<int64_t, int, int>>;
        using IndexEvPairs
            = std::vector<std::pair<int, int>>;

        // Structure
        AinrCoreMode sampleMode;
        bool isTileMode;
        int32_t captureNum;
        Buffers inBuffers;
        ExpInfoTuples expInfos;
        IndexEvPairs sortedPairs;
        NSCam::IImageBuffer* outBuffer;
        uint32_t uniqueKey;
        int32_t requestNum;
        int32_t frameNum;

        AiCoreParam(void) noexcept
            : sampleMode(AinrCoreMode::Full_SAMPLE)
            , isTileMode(true)
            , captureNum(0)
            , outBuffer(nullptr)
            , uniqueKey(0)
            , requestNum(0)
            , frameNum(0)
            {}
    } AiCoreParam_t;

    class Pass2DataPack final
    {
    public:
        Pass2DataPack(size_t buf_size) : size(buf_size)
        {
            if (buf_size > 0) {
                this->data = std::unique_ptr<char[]>(new char[buf_size]{0});
            }
        }
        ~Pass2DataPack() = default;


    public:
        inline std::unique_lock<std::mutex> uniqueLock()
        {
            return std::unique_lock<std::mutex>(mx);
        }
        inline std::mutex& getLocker() { return mx; }
        inline void lock()      { mx.lock(); }
        inline void unlock()    { mx.unlock(); }
        inline void notifyOne()  { cv.notify_one(); }
        inline void notifyAll()  { cv.notify_all(); }
        inline void wait(std::unique_lock<std::mutex>&& l) { cv.wait(l); }

        inline char* getBuf()   { return data.get(); }
        inline size_t getSize() const { return size; }


    private:
        size_t                  size;
        std::unique_ptr<char[]> data;
        // for thread sync
        std::mutex              mx;
        std::condition_variable cv;
    };

    class Pass2Async final
    {
    public:
        Pass2Async() = default;
        ~Pass2Async() = default;
    public:
        inline std::unique_lock<std::mutex> uniqueLock()
        {
            return std::unique_lock<std::mutex>(mx);
        }
        inline std::mutex& getLocker() { return mx; }
        inline void lock()      { mx.lock(); }
        inline void unlock()    { mx.unlock(); }
        inline void notifyOne()  { cv.notify_one(); }
        inline void notifyAll()  { cv.notify_all(); }
        inline void wait(std::unique_lock<std::mutex>&& l) { cv.wait(l); }
    private:
        // for thread sync
        std::mutex              mx;
        std::condition_variable cv;
    };

    typedef struct AinrPipelinePack {
        NSCam::IMetadata* appMeta;
        NSCam::IMetadata* halMeta;
        NSCam::IMetadata* appMetaDynamic;
        NSCam::IMetadata* outHalMeta;
        NSCam::IImageBuffer* imgoBuffer;
        NSCam::IImageBuffer* rrzoBuffer;
        //
        AinrPipelinePack()
            : appMeta(nullptr)
            , halMeta(nullptr)
            , appMetaDynamic(nullptr)
            , outHalMeta(nullptr)
            , imgoBuffer(nullptr)
            , rrzoBuffer(nullptr)
        {}
    } AinrPipelinePack_t;

    typedef struct AinrMetaPack {
        NSCam::IMetadata appMeta;
        NSCam::IMetadata halMeta;
    } AinrMetaPack_t;
}; /* namespace ainr */

/******************************************************************************
 * Utilities
 ******************************************************************************/
namespace ainr {

/**
 *  Check if the mode is ZSD or note.
 *  @param m            Mode to check
 *  @return             If the mode is ZSD mode, returns true
 */
inline bool isZsdMode(const enum AinrMode& m) noexcept
{
    return !!(m & (1 << AinrMode_Bit_Zsd)); // see AinrMode, bit 0 represents if it's ZSD mode or not
}

inline bool isAinrMode(const enum AinrMode& m) noexcept
{
    return !!(m & (1 << AinrMode_Bit_Ainr));
}

inline bool isAisMode(const enum AinrMode& m) noexcept
{
    return !!(m  & (1 << AinrMode_Bit_Ais));
}

inline bool isSingleFrameMode(const enum AinrMode& m) noexcept
{
    return !!(m & (1 << AinrMode_Bit_SingleFrame));
}

inline bool isMfhrMode(const enum AinrMode& m) noexcept
{
    return !!(m & (1 << AinrMode_Bit_Mfhr));
}

inline bool isZhdrMode(const enum AinrMode& m) noexcept
{
    return !!(m & (1 << AinrMode_Bit_Zhdr));
}

inline bool isAutoHdr(const enum AinrMode& m) noexcept
{
    return !!(m & (1 << AinrMode_Bit_AutoHdr));
}

inline bool isCustomHint1(const enum AinrMode& m) noexcept
{
    return !!(m & (1 << AinrMode_Bit_Custom1));
}

inline bool isCustomHint2(const enum AinrMode& m) noexcept
{
    return !!(m & (1 << AinrMode_Bit_Custom2));
}

inline bool isCustomHint3(const enum AinrMode& m) noexcept
{
    return !!(m & (1 << AinrMode_Bit_Custom3));
}

inline bool isCustomHint4(const enum AinrMode& m) noexcept
{
    return !!(m & (1 << AinrMode_Bit_Custom4));
}

inline int32_t getCustomHint(const enum AinrMode& m) noexcept
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

//#define AINR_THREAD_NAME(x) mfll::setThreadName(x)
inline void setThreadName(const char* name)
{
    ::prctl(PR_SET_NAME, name, 0, 0, 0);
}

inline uint32_t makeGmv32bits(short x, short y)
{
    return (uint32_t) y << 16 | (x & 0x0000FFFF);
}

}; /* namespace ainr */
#endif /* __AINRTYPES_H__ */
