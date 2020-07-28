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

#include <log/log.h>
#include <android/log.h>

#include <memory>
#include <utils/String8.h>
#include <cutils/properties.h>

#include "core/DebugControl.h"

//#include <mtkcam/utils/hw/HwTransform.h>
#include <map>

#include <utils/metadata/IMetadata.h>

#include <IFeaturePipe.h>


using namespace android;
using namespace NSCam::Utils::Format;

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {

#if (MTKCAM_HAVE_AEE_FEATURE == 1)
#include <aee.h>
#define AEE_ASSERT(fmt, arg...) \
    do { \
        android::String8 const str = android::String8::format(fmt, ##arg); \
        CAM_LOGE("ASSERT(%s) fail", str.string()); \
        aee_system_exception( \
            "postalgo/FeaturePipe", \
            NULL, \
            DB_OPT_DEFAULT, \
            str.string()); \
        raise(SIGKILL);\
    } while(0)
#else
#define AEE_ASSERT(fmt, arg...) \
    android::String8 const str = android::String8::format(fmt, ##arg); \
    CAM_LOGE("ASSERT(%s) fail", str.string());
#endif

template<typename T>
using UniquePtr = std::unique_ptr<T, std::function<MVOID(T*)>>;

MBOOL copyImageBuffer(IImageBuffer *src, IImageBuffer *dst);

MBOOL dumpToFile(IImageBuffer *buffer, const char *fmt, ...);

MUINT32 align(MUINT32 val, MUINT32 bits);

//MUINT getSensorRawFmt(MINT32 sensorId);

typedef MUINT8 PathID_T;
typedef MUINT8 NodeID_T;
const char* NodeID2Name(MUINT8 nodeId);
const char* PathID2Name(MUINT8 pathId);
const char* TypeID2Name(MUINT8 typeId);
const char* FeatID2Name(MUINT8 featId);
const char* SizeID2Name(MUINT8 sizeId);
PathID_T FindPath(NodeID_T src, NodeID_T dst);
const NodeID_T* GetPath(PathID_T pid);


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
        //ALOGD("TMP","prop:%s, value:%d", prop.string(), ret);
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
        //ALOGW("TMP","pMetadata == NULL");
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
        //ALOGW("TMP","pMetadata == NULL");
        return;
    }
    //
    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}


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

int gcd(int num1, int num2);
int lcm(int num1, int num2);
class HwStrideAlignment
{
private:
    typedef int BitPerPixel;
    typedef int WidthAlign;
    typedef MUINT Format_T;
    static std::map<BitPerPixel, WidthAlign> ALIGNMENT;
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

#define ImgFmt2Name(fmt) (fmt == 0) ? "unknown" : queryImageFormatName(fmt).c_str()
// Define YUV repeat count
#define MAX_YUV_REPEAT_NUM 6

} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com

#endif // _MTK_CAMERA_CAPTURE_FEATURE_PIPE_CAPTURE_FEATURE_COMMON_H_
