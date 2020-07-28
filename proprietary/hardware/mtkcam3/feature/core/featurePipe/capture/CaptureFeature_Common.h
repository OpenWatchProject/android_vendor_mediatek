/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_CAMERA_CAPTURE_FEATURE_PIPE_CAPTURE_FEATURE_COMMON_H_
#define _MTK_CAMERA_CAPTURE_FEATURE_PIPE_CAPTURE_FEATURE_COMMON_H_

#include <memory>
#include <cutils/properties.h>
#include <featurePipe/core/include/MtkHeader.h>

#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <featurePipe/core/include/ImageBufferPool.h>
#include <featurePipe/core/include/FatImageBufferPool.h>
#include <featurePipe/core/include/GraphicBufferPool.h>


#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>

#include <DpIspStream.h>
#include "DebugControl.h"

#include <mtkcam/utils/hw/HwTransform.h>
#include <map>

#include <mtkcam3/feature/featurePipe/ICaptureFeaturePipe.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSCapture {

#if (MTKCAM_HAVE_AEE_FEATURE == 1)
#include <aee.h>
#define AEE_ASSERT(fmt, arg...) \
    do { \
        android::String8 const str = android::String8::format(fmt, ##arg); \
        CAM_ULOGE(NSCam::Utils::ULog::MOD_FPIPE_CAPTURE, "ASSERT(%s) fail", str.string()); \
        aee_system_exception( \
            "mtkcam/CaptureFeaturePipe", \
            NULL, \
            DB_OPT_DEFAULT, \
            str.string()); \
        raise(SIGKILL);\
    } while(0)
#else
#define AEE_ASSERT(fmt, arg...) \
    android::String8 const str = android::String8::format(fmt, ##arg); \
    CAM_ULOGE(NSCam::Utils::ULog::MOD_FPIPE_CAPTURE, "ASSERT(%s) fail", str.string());
#endif

template<typename T>
using UniquePtr = std::unique_ptr<T, std::function<MVOID(T*)>>;

MBOOL dumpToFile(IImageBuffer *buffer, const char *fmt, ...);

MUINT32 align(MUINT32 val, MUINT32 bits);

MUINT getSensorRawFmt(MINT32 sensorId);

MBOOL formatConverter(MINT32 sensorIdx, const IImageBuffer *pSrcImg, IImageBuffer *pDstImg);

typedef MUINT8 PathID_T;
typedef MUINT8 NodeID_T;
const char* NodeID2Name(MUINT8 nodeId);
const char* PathID2Name(MUINT8 pathId);
const char* TypeID2Name(MUINT8 typeId);
const char* FeatID2Name(MUINT8 featId);
const char* SizeID2Name(MUINT8 sizeId);
PathID_T FindPath(NodeID_T src, NodeID_T dst);
MBOOL GetPath(PathID_T pid, NodeID_T& src, NodeID_T& dst);

#define CAPTURE_PIPE_LOGGING_PROPERTY "vendor.debug.camera.capture.log"
#define ISP_PROFILE_WITH_IDENTITY_EFFECT EIspProfile_YUV_Reprocess

/******************************************************************************
 *  Utility
 ******************************************************************************/
template<NodeID_T nodeId>
auto getIsDumpImage() -> MBOOL
{
    static const MBOOL isDumpImg = []()
    {
        String8 prop = String8::format("vendor.debug.camera.capture.%s.img.dump", NodeID2Name(nodeId));
        MBOOL ret = property_get_int32(prop.string(), 0);
        CAM_ULOGD(NSCam::Utils::ULog::MOD_FPIPE_CAPTURE, "prop:%s, value:%d", prop.string(), ret);
        return (ret != 0);
    }();
    return isDumpImg;
}

template<typename T>
static inline MBOOL HasFeatureVSDoF(T& t)
{
    const MBOOL hasDepth = t.hasFeature(FID_DEPTH) || t.hasFeature(FID_DEPTH_3RD_PARTY);
    const MBOOL hasBokeh = t.hasFeature(FID_BOKEH) || t.hasFeature(FID_BOKEH_3RD_PARTY);
    return (hasDepth && hasBokeh) || t.hasFeature(FID_PUREBOKEH_3RD_PARTY);
}

/******************************************************************************
 *  Metadata Access
 ******************************************************************************/
/**
 * Try to get metadata value
 *
 * @param[in]  pMetadata: IMetadata instance
 * @param[in]  tag: the metadata tag to retrieve
 * @param[out]  rVal: the metadata value
 *
 *
 * @return
 *  -  true if successful; otherwise false.
 */
template <typename T>
inline MBOOL
tryGetMetadata(
    const IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        CAM_ULOGW(NSCam::Utils::ULog::MOD_FPIPE_CAPTURE, "pMetadata == NULL");
        return MFALSE;
    }
    //
    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}

/**
 * Try to set metadata value
 *
 * @param[in]  pMetadata: IMetadata instance
 * @param[in]  tag: the metadata tag to set
 * @param[in]  val: the metadata value to be configured
 *
 *
 * @return
 *  -  true if successful; otherwise false.
 */
template <typename T>
inline MVOID
trySetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        CAM_ULOGW(NSCam::Utils::ULog::MOD_FPIPE_CAPTURE, "pMetadata == NULL");
        return;
    }
    //
    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}


/******************************************************************************
 *  Utility for crop
 ******************************************************************************/
inline MINT32 div_round(MINT32 const numerator, MINT32 const denominator) {
    return (( numerator < 0 ) ^ (denominator < 0 )) ?
        (( numerator - denominator/2)/denominator) : (( numerator + denominator/2)/denominator);
}

struct vector_f //vector with floating point
{
    MPoint  p;
    MPoint  pf;

                                vector_f(
                                        MPoint const& rP = MPoint(),
                                        MPoint const& rPf = MPoint()
                                        )
                                    : p(rP)
                                    , pf(rPf)
                                {}
};

struct simpleTransform
{
    // just support translation than scale, not a general formulation
    // translation
    MPoint    tarOrigin;
    // scale
    MSize     oldScale;
    MSize     newScale;

                                simpleTransform(
                                        MPoint rOrigin = MPoint(),
                                        MSize  rOldScale = MSize(),
                                        MSize  rNewScale = MSize()
                                        )
                                    : tarOrigin(rOrigin)
                                    , oldScale(rOldScale)
                                    , newScale(rNewScale)
                                {}
};

// transform MPoint
inline MPoint transform(simpleTransform const& trans, MPoint const& p) {
    return MPoint(
            div_round( (p.x - trans.tarOrigin.x) * trans.newScale.w, trans.oldScale.w),
            div_round( (p.y - trans.tarOrigin.y) * trans.newScale.h, trans.oldScale.h)
            );
};

inline MPoint inv_transform(simpleTransform const& trans, MPoint const& p) {
    return MPoint(
            div_round( p.x * trans.oldScale.w, trans.newScale.w) + trans.tarOrigin.x,
            div_round( p.y * trans.oldScale.h, trans.newScale.h) + trans.tarOrigin.y
            );
};

inline int int_floor(float x) {
    int i = (int)x;
    return i - (i > x);
}

// transform vector_f
inline vector_f transform(simpleTransform const& trans, vector_f const& p) {
    MFLOAT const x = (p.p.x + (p.pf.x/(MFLOAT)(1u<<31))) * trans.newScale.w / trans.oldScale.w;
    MFLOAT const y = (p.p.y + (p.pf.y/(MFLOAT)(1u<<31))) * trans.newScale.h / trans.oldScale.h;
    int const x_int = int_floor(x);
    int const y_int = int_floor(y);
    return vector_f(
            MPoint(x_int, y_int),
            MPoint((x - x_int) * (1u<<31), (y - y_int) * (1u<<31))
            );
};

inline vector_f inv_transform(simpleTransform const& trans, vector_f const& p) {
    MFLOAT const x = (p.p.x + (p.pf.x/(MFLOAT)(1u<<31))) * trans.oldScale.w / trans.newScale.w;
    MFLOAT const y = (p.p.y + (p.pf.y/(MFLOAT)(1u<<31))) * trans.oldScale.h / trans.newScale.h;
    int const x_int = int_floor(x);
    int const y_int = int_floor(y);
    return vector_f(
            MPoint(x_int, y_int),
            MPoint((x - x_int) * (1u<<31), (y - y_int) * (1u<<31))
            );
};

// transform MSize
inline MSize transform(simpleTransform const& trans, MSize const& s) {
    return MSize(
            div_round( s.w * trans.newScale.w, trans.oldScale.w),
            div_round( s.h * trans.newScale.h, trans.oldScale.h)
            );
};

inline MSize inv_transform(simpleTransform const& trans, MSize const& s) {
    return MSize(
            div_round( s.w * trans.oldScale.w, trans.newScale.w),
            div_round( s.h * trans.oldScale.h, trans.newScale.h)
            );
};

// transform MRect
inline MRect transform(simpleTransform const& trans, MRect const& r) {
    return MRect(transform(trans, r.p), transform(trans, r.s));
};

inline MRect inv_transform(simpleTransform const& trans, MRect const& r) {
    return MRect(inv_transform(trans, r.p), inv_transform(trans, r.s));
};

class CropCalculator : public RefBase {
public:

    CropCalculator(const std::vector<MINT32> sensorList, MUINT32 uLogLevel, MUINT32 uDualMode);
public:
    struct Factor : public LightRefBase<Factor> {
        MSize               mSensorSize;
        // P1 Crop Info
        MRect               mP1SensorCrop;
        MRect               mP1DmaCrop;
        MSize               mP1ResizerSize;
        // Transform Matrix
        NSCamHW::HwMatrix   mActive2Sensor;
        NSCamHW::HwMatrix   mSensor2Active;
        simpleTransform     mSensor2Resizer;
        MINT32              mSensorMode;
        // Target crop: cropRegion (active array coorinate)
        // not applied eis's mv yet, but the crop area is already reduced by EIS ratio.
        MRect               mActiveCrop;
        MRect               mSensorCrop;
        MRect               mP2RegionCrop; // for removing black edge
        float               mFovDiff_X;
        float               mFovDiff_Y;
        MBOOL               mbExistActiveCrop = MTRUE;
        // EIS
        MBOOL               mEnableEis;
        vector_f            mActiveEisMv;   //active array coor.
        vector_f            mSensorEisMv;   //sensor coor.
        vector_f            mResizerEisMv;  //resized coor.

        MBOOL               isEqual(
                                sp<Factor> pFactor
                            ) const;

        MVOID               dump() const;

    };

    sp<Factor>              getFactor(
                                const IMetadata* inApp,
                                const IMetadata* inHal,
                                const MINT32 sensorId
                            );

    static MVOID            evaluate(
                                MSize const &srcSize,
                                MSize const &dstSize,
                                MRect &srcCrop
                            );

    static MVOID            evaluate(
                                MSize const &srcSize,
                                MSize const &dstSize,
                                MRect const &srcCropRegion,
                                MRect &srcCrop
                            );

    MVOID                   evaluate(
                                sp<Factor> pFactor,
                                MSize const &dstSize,
                                MRect &srcCrop,
                                MBOOL const bResized = MFALSE,
                                MBOOL const bCheckCropRegion = MFALSE
                            ) const;

    MBOOL                   refineBoundary(
                                MSize const &bufSize,
                                MRect &crop
                            ) const;

    const MRect&            getActiveArray(MINT32 sensorId) {
                                return mTransformers[sensorId]->mActiveArray;
                            };

    struct eis_region {
        MUINT32 x_int;
        MUINT32 x_float;
        MUINT32 y_int;
        MUINT32 y_float;
        MSize s;
#if SUPPORT_EIS_MV
        MUINT32 x_mv_int;
        MUINT32 x_mv_float;
        MUINT32 y_mv_int;
        MUINT32 y_mv_float;
        MUINT32 is_from_zzr;
#endif
    };

    MBOOL                   queryEisRegion(
                                const IMetadata*, eis_region&
                            ) const;

private:

    struct Transformer : public LightRefBase<Transformer> {
        MRect                   mActiveArray;
        sp<Factor>              mpLastFactor;
        mutable Mutex           mLock;
    };

    MUINT32                 mLogLevel;
    MUINT32                 mDualMode;

    std::map<MINT32, sp<Transformer>> mTransformers;
};

class IBooster
{
public:
    using Ptr = UniquePtr<IBooster>;

public:
    static Ptr createInstance(const std::string& name);

public:
    virtual ~IBooster() = 0;

public:
    virtual const std::string& getName() = 0;

    virtual MVOID enable() = 0;

    virtual MVOID disable() = 0;
};
using IBoosterPtr = IBooster::Ptr;

enum SensorAliasName
{
    eSAN_None,
    eSAN_Master,
    eSAN_Sub_01
};

enum SensorConfigType
{
    eSCT_None,
    eSCT_BayerBayer,
    eSCT_BayerMono
};

enum RawImageType
{
    eRIT_None,
    eRIT_Imgo,
    eRIT_Rrzo
};

union IspProfileHint
{
    const uint64_t mValue;
    struct
    {
        uint8_t mSensorAliasName    :8;
        uint8_t mSensorConfigType   :8;
        uint8_t mRawImageType       :8;
    };

    IspProfileHint()
    : IspProfileHint(eSAN_None, eSCT_None, eRIT_None)
    {

    }

    IspProfileHint(uint8_t sensorAliasName, uint8_t sensorConfigType, uint8_t rawImageType)
    : mValue(0)
    {
        mSensorAliasName = sensorAliasName;
        mSensorConfigType = sensorConfigType;
        mRawImageType = rawImageType;
    }

    struct Compare
    {
       bool inline operator() (const IspProfileHint& lhs, const IspProfileHint& rhs) const
       {
           return lhs.mValue < rhs.mValue;
       }
    };
};

struct IspProfileInfo
{
    const char* mName;
    MUINT8      mValue;

    IspProfileInfo(const char* name, MUINT8 value)
    : mName(name)
    , mValue(value)
    {

    }
};
#define MAKE_ISP_PROFILE_INFO(PROFILE_NAME) IspProfileInfo(#PROFILE_NAME, PROFILE_NAME)

class IspProfileManager final
{
public:
    IspProfileManager() = delete;

public:
    static const IspProfileInfo& get(const IspProfileHint& hint);
};

class FovCalculator : public RefBase
{
public:
    struct FovInfo
    {
        MRect mFOVCropRegion;
        MSize mDestinationSize;
    };

public:
    FovCalculator(const std::vector<MINT32>& sensorIndexes, const MBOOL isDualCam);

    auto getIsEnable() const -> MBOOL;

    auto getFovInfo(MINT32 sensorIndex, FovInfo& fovInfo) const -> MBOOL;

    auto transform(MINT32 sensorIndex, const MPoint& in, MPoint& out) const -> MBOOL;

    auto transform(MINT32 sensorIndex, const MPoint& inLeftTop, const MPoint& inRightBottom, MPoint& outLeftTop, MPoint& outRightBottom) const -> MBOOL;

private:
    auto init(const std::vector<MINT32>& sensorIndexes) -> MVOID;

private:
    std::map<MINT32, FovInfo>   mFOVInfos;
    MBOOL                       mEnable     = MTRUE;
    MBOOL                       mIsDualCam  = MFALSE;
};

class IHolder
{
public:
    static std::unique_ptr<IHolder> createResizerInstance(IImageBuffer* pBuf, const MSize& size);

public:
    virtual ~IHolder() = 0;
};

enum NVRAM_TYPE
{
    NVRAM_TYPE_DRE,
    NVRAM_TYPE_CZ,
    NVRAM_TYPE_SWNR_THRES,
    NVRAM_TYPE_DSDN,
    NVRAM_TYPE_HFG
};

const void *
getTuningFromNvram(
    MUINT32 openId,
    MUINT32& idx,
    MINT32 magicNo,
    MINT32 type ,
    const IMetadata* pInHalMeta,
    MBOOL enableLog,
    MBOOL isISPHIDL,
    MINT32 useIspProfile = -1);

enum DumpFilter {
    DUMP_ROUND1_IMGI      = 1 << 0,
    DUMP_ROUND1_LCEI      = 1 << 1,
    DUMP_ROUND1_WDMAO     = 1 << 2,
    DUMP_ROUND1_WROTO     = 1 << 3,
    DUMP_ROUND1_IMG2O     = 1 << 4,
    DUMP_ROUND1_IMG3O     = 1 << 5,
    DUMP_ROUND2_IMGI      = 1 << 6,
    DUMP_ROUND2_WDMAO     = 1 << 7,
    DUMP_ROUND2_WROTO     = 1 << 8,
    DUMP_ROUND2_IMG2O     = 1 << 9,
    DUMP_ROUND2_IMG3O     = 1 << 10,
    DUMP_ROUND1_DP0       = 1 << 11,
    DUMP_ROUND1_DP1       = 1 << 12,
    DUMP_ROUND1_TIMGO     = 1 << 13,
    DUMP_ROUND2_TIMGO     = 1 << 14,
};

int gcd(int num1, int num2);
int lcm(int num1, int num2);
class HwStrideAlignment
{
private:
    typedef int BitPerPixel;
    typedef int WidthAlign;
    typedef int HeightAlign;
    typedef MUINT Format_T;
    static std::map<BitPerPixel, WidthAlign> ALIGNMENT_WIDTH;
    static std::map<BitPerPixel, HeightAlign> ALIGNMENT_HEIGHT;
public:
    /**
     * @brief query the buffer alignment
     * @param [in] fmt buffer format
     * @param [in] currentAlign current buffer alignment if exist
     * @return
     * - buffer alignment for allocation
     */
    static MSize queryFormatAlignment(
        const Format_T& fmt,
        const MSize& currentAlign = MSize(0, 0)
    );
};

#define ImgFmt2Name(fmt) (fmt == 0) ? "unknown" : Utils::Format::queryImageFormatName(fmt).c_str()
// Define YUV repeat count
#define MAX_YUV_REPEAT_NUM 6
MVOID dumpQParams(int logLevel, NSIoPipe::QParams& rInputQParam);

static inline MINT64 generatePQUserID(MINT32 frameNo, MUINT8 sensorID, MBOOL isPhysicalStream)
{
    return ((frameNo << 4) | (sensorID << 1) | (int)isPhysicalStream);
}

} // NSCapture
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_CAPTURE_FEATURE_PIPE_CAPTURE_FEATURE_COMMON_H_
