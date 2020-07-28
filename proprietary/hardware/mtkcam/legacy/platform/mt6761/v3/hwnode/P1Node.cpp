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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#define LOG_TAG "MtkCam/P1NodeImp"
//
#include <mtkcam/Log.h>
#include <mtkcam/utils/common.h>
#include "BaseNode.h"
#include "HwEventIrq.h"
#include <mtkcam/v3/hwnode/P1Node.h>
#include <mtkcam/v3/stream/IStreamInfo.h>
#include <mtkcam/v3/stream/IStreamBuffer.h>
#include <mtkcam/v3/utils/streambuf/IStreamBufferPool.h>
//
#include <utils/RWLock.h>
#include <utils/Thread.h>
//
#include <sys/prctl.h>
#include <sys/resource.h>
#include <system/thread_defs.h>

//
#include <mtkcam/v3/hal/IHal3A.h>
#include <mtkcam/iopipe/CamIO/IHalCamIO.h>
#include <mtkcam/iopipe/CamIO/INormalPipe.h>
#include <vector>
#include <iopipe/CamIO/PortMap.h>/*#include <iopipe_FrmB/CamIO/PortMap_FrmB.h>*/
#include <mtkcam/featureio/eis_hal.h>
#include <mtkcam/algorithm/libeis/MTKEis.h>
//
#include <mtkcam/v3/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/metadata/client/mtk_metadata_tag.h>
//
#include "Profile.h"
#include <mtkcam/Trace.h>
#include <cutils/properties.h>
#include <utils/Atomic.h>
//
#include <mtkcam/imageio/ispio_utility.h> //(SUPPORT_SCALING_CROP)
#include <mtkcam/metadata/IMetadataProvider.h>
#include "hwnode_utilities.h"
//
#include <mtkcam/utils/imagebuf/IIonImageBufferHeap.h>
#include <mtkcam/utils/imagebuf/IDummyImageBufferHeap.h>
//
#include <mtkcam/iopipe/PostProc/IFeatureStream.h> //(SUPPORT_HQC)
//
using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
//using namespace NSCam::Utils;
using namespace NSCam::Utils::Sync;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NSCam::NSIoPipe::NSPostProc;
using namespace NS3Av3;
using namespace NSImageio;
using namespace NSIspio;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

/******************************************************************************
 *
 ******************************************************************************/
#define SUPPORT_3A              (1)
#define SUPPORT_ISP             (1)
#define SUPPORT_PERFRAME_CTRL   (0)
#define SUPPORT_EIS             (1) // needed by 3DNR

#define SUPPORT_HQC             (1) // High Quality Capture

#define SUPPORT_SCALING_CROP    (1)
#define SUPPORT_SCALING_CROP_IMGO   (SUPPORT_SCALING_CROP && (0))
#define SUPPORT_SCALING_CROP_RRZO   (SUPPORT_SCALING_CROP && (1))

#define FORCE_EIS_ON                (SUPPORT_EIS && (0))
#define FORCE_3DNR_ON               (SUPPORT_EIS && (0))
#define DISABLE_BLOB_DUMMY_BUF      (0)

/******************************************************************************
 *
 ******************************************************************************/
#define FUNCTION_IN             MY_LOGD_IF(1<=mLogLevel, "+");
#define FUNCTION_OUT            MY_LOGD_IF(1<=mLogLevel, "-");
#define PUBLIC_API_IN           MY_LOGD_IF(1<=mLogLevel, "API +");
#define PUBLIC_API_OUT          MY_LOGD_IF(1<=mLogLevel, "API -");
#define MY_LOGD1(...)           MY_LOGD_IF(1<=mLogLevel, __VA_ARGS__)
#define MY_LOGD2(...)           MY_LOGD_IF(2<=mLogLevel, __VA_ARGS__)

#define P1THREAD_POLICY         (SCHED_OTHER)
#define P1THREAD_PRIORITY       (ANDROID_PRIORITY_FOREGROUND-2)

#define P1SOFIDX_INIT_VAL       (0)

/******************************************************************************
 *
 ******************************************************************************/
namespace {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// a singleton class to record the instances information
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class InstanceInfo {
    public:
                        InstanceInfo()
                            : mLock()
                            , mLog(MFALSE)
                            , mCount(0)
                            {};

        virtual         ~InstanceInfo(){};

        void            setLog(MBOOL enable)
                        {
                            Mutex::Autolock lock(mLock);
                            mLog = enable;
                        };

        void            addCnt()
                        {
                            Mutex::Autolock lock(mLock);
                            mCount ++;
                            MY_LOGD_IF(mLog, "Count++ : %d", mCount);
                        };

        void            delCnt()
                        {
                            Mutex::Autolock lock(mLock);
                            mCount --;
                            MY_LOGD_IF(mLog, "Count-- : %d", mCount);
                        };

        MUINT32         getCnt()
                        {
                            Mutex::Autolock lock(mLock);
                            //MY_LOGD_IF(mLog, "Count : %d", mCount);
                            return mCount;
                        };

    private:
        mutable Mutex   mLock;
        MBOOL           mLog;
        MUINT32         mCount;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class AAAResult {
    protected:
        struct info{
            sp<IPipelineFrame> spFrame;
            IMetadata          resultVal;
            MUINT32            mFlag;
            info()
                : spFrame(0)
                , resultVal()
                , mFlag(0)
                {}
        };

    protected:
        enum KeyType {
            KeyType_StrobeState = 1,
            KeyType_Rest        = 2, //1,2,4,8,...
        };

    protected:
        mutable Mutex              mLock;
        KeyedVector<MUINT32, info> mData; //key: magicnum, val: info
        MUINT32                    mAllKey;

    public:

        AAAResult()
            : mLock()
            , mData()
            , mAllKey(KeyType_Rest)
            //, mAllKey(KeyType_StrobeState|KeyType_Rest)
            {}

        void add(MUINT32 magicNum, MUINT32 key, MUINT32 val)
        {
             Mutex::Autolock lock(mLock);
             if(key != MTK_FLASH_STATE) {
                 //unSupported
                 return;
             }

             IMetadata::IEntry entry(MTK_FLASH_STATE);
             entry.push_back(val, Type2Type< MUINT8 >()); //{MTK_FLASH_STATE, MUINT8}
             ssize_t i = mData.indexOfKey(magicNum);
             if(i < 0) {
                 info data;
                 data.resultVal.update(MTK_FLASH_STATE, entry);

    data.mFlag |= KeyType_StrobeState;
                 mData.add(magicNum, data);
             } else {
                 info& data = mData.editValueFor(magicNum);
                 data.resultVal.update(MTK_FLASH_STATE, entry);

    data.mFlag |= KeyType_StrobeState;
             }
        }

        void add(MUINT32 magicNum, sp<IPipelineFrame> pframe, IMetadata &rVal)
        {
             Mutex::Autolock lock(mLock);
             ssize_t i = mData.indexOfKey(magicNum);
             if(i < 0) {
                 info data;
                 data.spFrame = pframe;
                 data.resultVal = rVal;

data.mFlag |= KeyType_Rest;
                 mData.add(magicNum, data);
             } else {
                 info& data = mData.editValueFor(magicNum);
                 data.spFrame = pframe;
                 data.resultVal += rVal;
                 data.mFlag |= KeyType_Rest;
             }
        }

        const info& valueFor(const MUINT32& magicNum) const {
            return mData.valueFor(magicNum);
        }

        bool isCompleted(MUINT32 magicNum) {
            Mutex::Autolock lock(mLock);
            return (mData.valueFor(magicNum).mFlag & mAllKey) == mAllKey;
        }

        void removeItem(MUINT32 key) {
            Mutex::Autolock lock(mLock);
            mData.removeItem(key);
        }

        void clear() {
            debug();
            Mutex::Autolock lock(mLock);
            mData.clear();
        }

        void debug() {
            Mutex::Autolock lock(mLock);
            for(size_t i = 0; i < mData.size(); i++) {
                MY_LOGW_IF((mData.valueAt(i).mFlag & KeyType_StrobeState) == 0,
                           "No strobe result: (%d)", mData.keyAt(i));
                MY_LOGW_IF((mData.valueAt(i).mFlag & KeyType_Rest) == 0,
                           "No rest result: (%d)", mData.keyAt(i));
            }
        }
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class Storage {

    protected:
        typedef DefaultKeyedVector<MINTPTR, sp<IImageBuffer> >  MapType;
        MapType                    mvStorageQ;
        mutable Mutex              mStorageLock;
        MINT32                     mLogEnable;
    public:
                                   Storage()
                                       : mvStorageQ()
                                       , mStorageLock()
                                       , mLogEnable(0)
                                       {}

        virtual                   ~Storage(){};

        void                       init(MINT32 logEnable)
                                   {
                                       mvStorageQ.clear();
                                       mLogEnable = logEnable;
                                   }

        void                       uninit()
                                   {
                                       mvStorageQ.clear();
                                   }

        void                       enque(sp<IImageStreamBuffer> const& key, sp<IImageBuffer> &value) {
                                       Mutex::Autolock lock(mStorageLock);
                                       MY_LOGD_IF(mLogEnable, "Storage-enque::(key)%p/(val)%p",
                                           key.get(), value.get());
                                       MY_LOGD_IF(mLogEnable, "Info::(val-pa)0x%zx/%d/%d/%zu/%zu/%zu",
                                        value->getBufPA(0),value->getImgSize().w, value->getImgSize().h,
                                        value->getBufStridesInBytes(0), value->getBufSizeInBytes(0), value->getPlaneCount());

                                       mvStorageQ.add(reinterpret_cast<MINTPTR>(key.get()), value);
                                   };


        sp<IImageBuffer>           deque(MINTPTR key) {
                                       Mutex::Autolock lock(mStorageLock);
                                       sp<IImageBuffer> pframe = mvStorageQ.valueFor(key);
                                       if (pframe != NULL)
                                       {
                                           mvStorageQ.removeItem(key); //should un-mark
                                           MY_LOGD_IF(mLogEnable, "Storage-deque::(key)0x%zx/(val)%p",
                                            key, pframe.get());
                                           MY_LOGD_IF(mLogEnable, "(val-pa)0x%zx",
                                            pframe->getBufPA(0));
                                           return pframe;
                                       }
                                       return NULL;
                                   }
        sp<IImageBuffer>           query(MINTPTR key) {
                                       Mutex::Autolock lock(mStorageLock);
                                       sp<IImageBuffer> pframe = mvStorageQ.valueFor(key);
                                       if (pframe != NULL)
                                       {
                                           MY_LOGD_IF(mLogEnable, "Storage-deque::(key)0x%zx/(val)%p",
                                            key, pframe.get());
                                           MY_LOGD_IF(mLogEnable, "Info::(val-pa)0x%zx",
                                            pframe->getBufPA(0));
                                           return pframe;
                                       }
                                       return NULL;
                                   }
};

MUINT32 HQC_ON;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class DummyBuffer
    : public RefBase
{
    public:
        MINT32                  mLogLevel;
        MUINT32                 mSize;
        MUINT32                 mSlot;
        MUINT32                 mTotal;
        MINT32                  mHeapId;
        MUINTPTR                mVAddr;
        MUINTPTR                mPAddr;
        sp<IImageBuffer>        mDummyBuffer;
        android::String8        mName;

    public:
                                DummyBuffer(
                                    char const* szName,
                                    MINT32 enLog)
                                    : mLogLevel(enLog)
                                    , mSize(0)
                                    , mSlot(0)
                                    , mTotal(0)
                                    , mHeapId(-1)
                                    , mVAddr(0)
                                    , mPAddr(0)
                                    , mDummyBuffer(NULL)
                                    , mName(szName)
                                    , mUsingLock()
                                    , mUsingCount(0)
                                {
                                    MY_LOGD1("");
                                }

        virtual                 ~DummyBuffer()
                                {
                                    MY_LOGD1("");
                                };

        MERROR                  init(
                                    sp<IImageStreamInfo> const& imgStreamInfo,
                                    int numHardwareBuffer);

        MERROR                  init(
                                    MUINT32 const imgStreamSize,
                                    int numHardwareBuffer);

        MERROR                  uninit(void);

        MERROR                  count(MBOOL isIncrease)
                                {
                                    Mutex::Autolock _l(mUsingLock);
                                    if (mDummyBuffer == NULL) {
                                        MY_LOGE("dummy buffer not create");
                                        return BAD_VALUE;
                                    }
                                    if (isIncrease) {
                                        mUsingCount ++;
                                    } else {
                                        mUsingCount --;
                                    }
                                    MY_LOGD1("%s dummy buffer : change(%d) to "
                                        "count(%d)", mName.string(),
                                        isIncrease, mUsingCount);
                                    return OK;
                                };

        MINT32                  query(void)
                                {
                                    Mutex::Autolock _l(mUsingLock);
                                    return mUsingCount;
                                };

    private:
        mutable Mutex           mUsingLock;
        MINT32                  mUsingCount;
};

//#if (SUPPORT_SCALING_CROP)
#ifdef MAX
#undef MAX
#endif
#ifdef MIN
#undef MIN
#endif
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#ifdef ALIGN_UPPER
#undef ALIGN_UPPER
#endif
#ifdef ALIGN_LOWER
#undef ALIGN_LOWER
#endif
#define ALIGN_UPPER(x,a)    (((x)+((typeof(x))(a)-1))&~((typeof(x))(a)-1))
#define ALIGN_LOWER(x,a)    (((x))&~((typeof(x))(a)-1))

#ifdef RESIZE_RATIO_MAX_10X
#undef RESIZE_RATIO_MAX_10X
#endif
#define RESIZE_RATIO_MAX_10X   (4)

#if SUPPORT_HQC
enum P1_HQC_STATE{
    P1_HQC_STATE_NONE = 0,  // HQC not start
    P1_HQC_STATE_REQ,       // HQC request arrival
    P1_HQC_STATE_SET,       // HQC set to 3A
    P1_HQC_STATE_ENQ,       // HQC was En-queued
    P1_HQC_STATE_DEQ        // HQC was De-queued
};
#endif

/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
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

/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MBOOL
trySetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return MFALSE;
    }
    //
    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
inline MBOOL
isEISOn(
    IMetadata* const inApp
)
{
    if (inApp == NULL) {
        return false;
    }
    MUINT8 eisMode = MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF;
    if(!tryGetMetadata<MUINT8>(inApp,
        MTK_CONTROL_VIDEO_STABILIZATION_MODE, eisMode)) {
        //MY_LOGW_IF(1, "no MTK_CONTROL_VIDEO_STABILIZATION_MODE");
    }
#if FORCE_EIS_ON
    eisMode = MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON;
#endif
    return eisMode == MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON;
}

/******************************************************************************
 *
 ******************************************************************************/
inline MBOOL
is3DNROn(
    IMetadata* const inApp
)
{
    if (inApp == NULL) {
        return false;
    }
    MINT32 e3DnrMode = MTK_NR_FEATURE_3DNR_MODE_OFF;
    if(!tryGetMetadata<MINT32>(inApp,
        MTK_NR_FEATURE_3DNR_MODE, e3DnrMode)) {
        //MY_LOGW_IF(1, "no MTK_NR_FEATURE_3DNR_MODE");
    }
#if FORCE_3DNR_ON
    e3DnrMode = MTK_NR_FEATURE_3DNR_MODE_ON;
#endif
    return e3DnrMode == MTK_NR_FEATURE_3DNR_MODE_ON;
}

#if SUPPORT_HQC
/******************************************************************************
 *
 ******************************************************************************/
inline MBOOL
isHighQualityCaptureOn(
    IMetadata* const inApp,
    IHal3A* p3A
)
{
    char value[PROPERTY_VALUE_MAX];
    ::property_get("debug.HQC.on", value, "0");
    HQC_ON = ::atoi(value);
    MY_LOGD_IF(1,"HQC_ON(%d), default value off",HQC_ON);
    if(!HQC_ON)
        return false;
    if (inApp == NULL) {
        return false;
    }
    MUINT8 mode = 0;
    // HQC on by (still-capure) and (flash-trigger)
    if (!tryGetMetadata<MUINT8>(inApp,
        MTK_CONTROL_CAPTURE_INTENT, mode) ||
        mode != MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE) {
        return false;
    }
    MY_LOGD("HighQualityCapture mode(%d)", mode);
    if ((p3A == NULL) || (p3A->checkCapFlash() == false)) {
        return false;
    }
    MY_LOGD("HighQualityCapture On");
    return true;
}
#endif

/******************************************************************************
 *
 ******************************************************************************/
MBOOL calculateCropInfoFull(
    MUINT32 pixelMode,
    MSize const& sensorSize,
    MSize const& bufferSize,
    MRect const& querySrcRect,
    MRect& resultSrcRect,
    MSize& resultDstSize,
    MINT32 mLogLevel = 0
)
{
    if ((querySrcRect.size().w == sensorSize.w) &&
        (querySrcRect.size().h == sensorSize.h)) {
        return false;
    }
    if ((querySrcRect.size().w > bufferSize.w || // cannot over buffer size
        querySrcRect.size().h > bufferSize.h) ||
        (((querySrcRect.leftTop().x + querySrcRect.size().w) > sensorSize.w) ||
        ((querySrcRect.leftTop().y + querySrcRect.size().h) > sensorSize.h))
        ) {
        MY_LOGD_IF((1 <= mLogLevel), "calculateCropInfoFull input invalid "
            "pixelMode(%d) sensorSize(%dx%d) bufferSize(%dx%d) "
            "querySrcRect_size(%dx%d) querySrcRect_start(%d,%d)", pixelMode,
            sensorSize.w, sensorSize.h, bufferSize.w, bufferSize.h,
            querySrcRect.size().w, querySrcRect.size().h,
            querySrcRect.leftTop().x, querySrcRect.leftTop().y);
        return false;
    }
    // TODO: query the valid value, currently do not crop in IMGO
    resultDstSize = MSize(sensorSize.w, sensorSize.h);
    resultSrcRect = MRect(MPoint(0, 0), resultDstSize);

    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL calculateCropInfoResizer(
    MUINT32 pixelMode,
    MSize const& sensorSize,
    MSize const& bufferSize,
    MRect const& querySrcRect,
    MRect& resultSrcRect,
    MSize& resultDstSize,
    MINT32 mLogLevel = 0
)
{
    if ((querySrcRect.size().w == sensorSize.w) &&
        (querySrcRect.size().h == sensorSize.h)) {
        return false;
    }
    if ((((querySrcRect.leftTop().x + querySrcRect.size().w) > sensorSize.w) ||
        ((querySrcRect.leftTop().y + querySrcRect.size().h) > sensorSize.h))
        ) {
        MY_LOGD_IF((1 <= mLogLevel), "calculateCropInfoResizer input invalid "
            "pixelMode(%d) sensorSize(%dx%d) bufferSize(%dx%d) "
            "querySrcRect_size(%dx%d) querySrcRect_start(%d,%d)", pixelMode,
            sensorSize.w, sensorSize.h, bufferSize.w, bufferSize.h,
            querySrcRect.size().w, querySrcRect.size().h,
            querySrcRect.leftTop().x, querySrcRect.leftTop().y);
        return false;
    }
    //
    MPoint::value_type src_crop_x = querySrcRect.leftTop().x;
    MPoint::value_type src_crop_y = querySrcRect.leftTop().y;
    MSize::value_type src_crop_w = querySrcRect.size().w;
    MSize::value_type src_crop_h = querySrcRect.size().h;
    MSize::value_type dst_size_w = 0;
    MSize::value_type dst_size_h = 0;
    if (querySrcRect.size().w < bufferSize.w) {
        dst_size_w = querySrcRect.size().w;
        NSImageio::NSIspio::ISP_QUERY_RST queryRst;
        NSImageio::NSIspio::ISP_QuerySize(
                                NSImageio::NSIspio::EPortIndex_RRZO,
                                NSImageio::NSIspio::ISP_QUERY_X_PIX|
                                NSImageio::NSIspio::ISP_QUERY_STRIDE_PIX|
                                NSImageio::NSIspio::ISP_QUERY_STRIDE_BYTE,
                                eImgFmt_FG_BAYER10,
                                dst_size_w,
                                queryRst,
                                pixelMode == 0 ?
                                NSImageio::NSIspio::ISP_QUERY_1_PIX_MODE :
                                NSImageio::NSIspio::ISP_QUERY_2_PIX_MODE
                                );
        dst_size_w = MIN((MSize::value_type)queryRst.x_pix, sensorSize.w);
        src_crop_w = dst_size_w;
        if (src_crop_w > querySrcRect.size().w) {
            if ((src_crop_x + src_crop_w) > sensorSize.w) {
                src_crop_x = sensorSize.w - src_crop_w;
            }
        }
    } else {
        if ((src_crop_w * RESIZE_RATIO_MAX_10X) > (bufferSize.w * 10)) {
            MY_LOGW("calculateCropInfoResizer resize width invalid "
                    "(%d):(%d)", src_crop_w, bufferSize.w);
            return false;
        }
        dst_size_w = bufferSize.w;
    }
    if (querySrcRect.size().h < bufferSize.h) {
        dst_size_h = querySrcRect.size().h;
        dst_size_h = MIN(ALIGN_UPPER(dst_size_h, 2), sensorSize.h);
        src_crop_h = dst_size_h;
        if (src_crop_h > querySrcRect.size().h) {
            if ((src_crop_y + src_crop_h) > sensorSize.h) {
                src_crop_y = sensorSize.h - src_crop_h;
            }
        }
    } else {
        if ((src_crop_h * RESIZE_RATIO_MAX_10X) > (bufferSize.h * 10)) {
            MY_LOGW("calculateCropInfoResizer resize height invalid "
                    "(%d):(%d)", src_crop_h, bufferSize.h);
            return false;
        }
        dst_size_h = bufferSize.h;
    }
    resultDstSize = MSize(dst_size_w, dst_size_h);
    resultSrcRect = MRect(MPoint(src_crop_x, src_crop_y),
                            MSize(src_crop_w, src_crop_h));
    return true;
}
//#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  .
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class P1NodeImp
    : public BaseNode
    , public P1Node
    , public IHal3ACb
    , protected Thread
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    struct QueNode_T {
        MUINT32                            magicNum;
        MUINT32                            sofIdx;
        sp<IPipelineFrame>                 appFrame;
        sp<IImageStreamBuffer>             halFrame_full;
        sp<IImageStreamBuffer>             halFrame_resizer;
        sp<IImageBuffer>                   img_full;
        sp<IImageBuffer>                   img_resizer;
        sp<IImageBuffer>                   img_opaque;
        MUINT32                            reqOut;
        MUINT32                            reqType;
        #if (SUPPORT_SCALING_CROP)
        MSize                              dstSize_full;
        MSize                              dstSize_resizer;
        MRect                              cropRect_full;
        MRect                              cropRect_resizer;
        #endif
        QueNode_T()
            : magicNum(0)
            , sofIdx(P1SOFIDX_INIT_VAL)
            , appFrame(NULL)
            , halFrame_full(NULL)
            , halFrame_resizer(NULL)
            , img_opaque(NULL)
            , reqOut(REQ_OUT_NONE)
            , reqType(REQ_TYPE_UNKNOWN)
            , img_full(NULL)
            , img_resizer(NULL)
        {}
    };

    typedef Vector<QueNode_T> Que_T;

    enum REQ_OUT
    {
        REQ_OUT_NONE            = (0x0),
        REQ_OUT_RESIZER         = (0x1 << 0),
        REQ_OUT_FULL            = (0x1 << 1),
        REQ_OUT_OPAQUE           = (0x1 << 2),
        REQ_OUT_ALL             = (0xFFFFFFFF)
    };

    enum REQ_TYPE
    {
        REQ_TYPE_UNKNOWN        = 0,
        REQ_TYPE_NORMAL,
        REQ_TYPE_PADDING,
        REQ_TYPE_DUMMY,
        REQ_TYPE_REDO,
        REQ_TYPE_YUV
    };

protected:  ////                    Data Members. (Config)
    mutable RWLock                  mConfigRWLock;
    MBOOL                           mInit;

    SortedVector<StreamId_T>        mInStreamIds;
    sp<IMetaStreamInfo>             mInAppMeta;
    sp<IMetaStreamInfo>             mOutAppMeta;
    sp<IMetaStreamInfo>             mInHalMeta;
    sp<IMetaStreamInfo>             mOutHalMeta;
    ImageStreamInfoSetT             mvOutImage_full;
    sp<IImageStreamInfo>            mOutImage_resizer;
    sp<IImageStreamInfo>            mInImage_opaque;
    sp<IImageStreamInfo>            mOutImage_opaque;
    sp<IImageStreamInfo>            mInImage_yuv;
    RawParams                       mRawParams;
    SensorParams                    mSensorParams;
    sp<IImageStreamBufferPoolT>     mpStreamPool_full;
    sp<IImageStreamBufferPoolT>     mpStreamPool_resizer;

    Storage                         mImageStorage;
    InstanceInfo*                   mpInstanceInfo;

protected:  ////                    Data Members. (System capability)
    static const int                mNumInMeta = 2;
    static const int                mNumOutMeta = 3;
    int                             m3AProcessedDepth;
    int                             mNumHardwareBuffer;
    int                             mDelayframe;

protected:  ////
    MUINT32                         mlastNum;
    MUINT32                         mlastNonReprocNum;
    MUINT32                         mlastProcessedNum = -1;
    mutable Mutex                   mlastNumLock;

protected:  ////                    Data Members. (Hardware)
    mutable Mutex                   mHardwareLock;
    MBOOL                           mActive;
    MBOOL                           mCamActive;
    MBOOL                           m3AActive;
    MBOOL                           mEisActive;
    INormalPipe_FrmB*               mpCamIO;
    IHal3A*                         mp3A;
    HwEventIrq*                     mpIEventIrq;
    #if SUPPORT_EIS
    EisHal_R*                       mpEIS;
    #endif
    #if SUPPORT_SCALING_CROP
    MRect                           mActiveArray;
    MUINT32                         mPixelMode;
    #endif
    sp<DummyBuffer>                 mDummyBufferFull;
    sp<DummyBuffer>                 mDummyBufferResizer;
    sp<IImageBuffer>                mImageBufferFull;
    sp<IImageBuffer>                mImageBufferResizer;
    android::String8                mImageBufferNameFull;
    android::String8                mImageBufferNameResizer;
    MBOOL                           mImageBufferLockedFull;
    MBOOL                           mImageBufferLockedResizer;
    mutable Mutex                   mImageBufferLock;
    #if SUPPORT_HQC
    P1_HQC_STATE                    mHighQualityCaptureState;
    MUINT32                         mHighQualityCaptureNum;
    MVOID*                          mHighQualityCaptureAddr;
    mutable Mutex                   mHighQualityCaptureLock;
    Condition                       mHighQualityCaptureCond;
    MBOOL                           mHighQualityCaptureForceDisable;
    #endif

protected:  ////                    Data Members. (Queue: Request)
    mutable Mutex                   mRequestQueueLock;
    Que_T                           mRequestQueue;

protected:  ////                    Data Members. (Queue: 3Alist)
    mutable Mutex                   mControls3AListLock;
    List<MetaSet_T>                 mControls3AList;
    Condition                       mControls3AListCond;

protected:  ////
    AAAResult                       m3AStorage;

protected:  ////                    Data Members. (Queue: Processing)
    mutable Mutex                   mProcessingQueueLock;
    Condition                       mProcessingQueueCond;
    Que_T                           mProcessingQueue;

protected: ////                    Data Members. (Queue: Redo)
    mutable Mutex                   mRedoQueueLock;
    Que_T                           mRedoQueue;

protected:  ////                    Data Members. (Queue: Yuv)
    mutable Mutex                   mYuvQueueLock;
    Vector<QueNode_T>               mYuvQueue;

protected:  ////
    mutable Mutex                   mReprocessLock;
    Condition                       mReprocessCond;
    MBOOL                           mReprocessGoing;

protected:  ////                    Data Members. (Queue: drop)
    mutable Mutex                   mDropQueueLock;
    Vector<MUINT>                   mDropQueue;

protected:  ////                    Data Members.
    mutable Mutex                   mThreadLock;
    Condition                       mThreadCond;
    Que_T                           mdummyQueue;

protected:  ////                    Data Members.
    DurationProfile                 mDequeThreadProfile;

protected:  ////                    Data Members.
    mutable Mutex                   mPublicLock;
    mutable Mutex                   mReprocLock;
    mutable Mutex                   mTransferLock;

protected:  ////                    Data Members.
    MINT32                          mInFlightRequestCnt;

protected:
    MINT32                          mLogLevel;

protected:  ////                    Operations.
    MVOID                           onProcessFrame(
                                        MBOOL initial,
                                        MUINT32 magicNum = 0,
                                        MUINT32 sofIdx = P1SOFIDX_INIT_VAL
                                    );

    MVOID                           onProcess3AResult(
                                        MUINT32 magicNum,
                                        MUINT32 key,
                                        MUINT32 val
                                    );

    MVOID                           onProcessEIS(
                                        QueNode_T const &pFrame,
                                        IMetadata &resultEIS,
                                        QBufInfo const &deqBuf
                                    );

    MVOID                           onProcessEnqueFrame(
                                        QueNode_T &pFrame
                                    );

    MERROR                          onProcessDequedFrame(
                                    );

    MERROR                          onProcessDropFrame(
                                    );

    MBOOL                           getProcessingFrame_ByAddr(
                                        IImageBuffer* const imgBuffer,
                                        MUINT32 magicNum,
                                        QueNode_T &pFrame
                                    );

    QueNode_T                       getProcessingFrame_ByNumber(
                                        MUINT32 magicNum
                                    );


    MVOID                           onHandleFlush(
                                        MBOOL wait
                                    );

    MVOID                           onReturnProcessingFrame(
                                        QueNode_T const& pFrame,
                                        QBufInfo const &deqBuf,
                                        MetaSet_T const &result3A,
                                        IMetadata const &resultEIS
                                    );

    MVOID                           onFlushProcessingFrame(
                                        QueNode_T const& pFrame
                                    );

    MVOID                           onFlushRequestFrame(
                                        QueNode_T const& pFrame
                                    );

    MVOID                           onRedoFrame(
                                        MUINT32 currentNum,
                                        MBOOL onFlush,
                                        MBOOL onLock
                                    );

    MVOID                           onYuvFrame(
                                        MUINT32 currentNum,
                                        MBOOL onFlush,
                                        MBOOL onLock
                                    );

    MVOID                           createNode(sp<IPipelineFrame> appframe,
                                               Que_T *Queue,
                                               Mutex *QueLock,
                                               List<MetaSet_T> *list,
                                               Mutex *listLock
                                    );

    MVOID                           createNode(Que_T &Queue);

protected:  ////                    Hardware Operations.
    MERROR                          hardwareOps_start(
                                    );

    MERROR                          hardwareOps_enque(
                                        QueNode_T &pFrame,
                                        MBOOL dummy,
                                        MBOOL block
                                    );

    MERROR                          hardwareOps_deque(
                                        QBufInfo &deqBuf,
                                        MetaSet_T &result3A
                                    );

    MERROR                          hardwareOps_stop(
                                    );

#if SUPPORT_HQC
    MERROR                          hardwareOps_suspend(
                                    );

    MERROR                          hardwareOps_resume(
                                    );

    void                            highQualityCapture_init(
                                        MUINT32 setMagicNum
                                    );

    void                            highQualityCapture_set(
                                        P1_HQC_STATE state
                                    );

    void                            highQualityCapture_setAddr(
                                        MVOID* addr
                                    );

    P1_HQC_STATE                    highQualityCapture_get(
                                    );

    MVOID*                          highQualityCapture_getAddr(
                                    );

    MBOOL                           highQualityCapture_check(
                                        MUINT32 checkMagicNum
                                    );

    void                            highQualityCapture_uninit(
                                    );

    MVOID                           createPureBuf(
                                        QueNode_T const& node,
                                        QBufInfo &enBuf
                                    );

    MERROR                          onReprocessRawFrame(
                                        QueNode_T &pFrame,
                                        QBufInfo &deqBuf,
                                        MetaSet_T &result3A
                                    );
#endif

    MERROR                          hwDummyBuffer_init(
                                        void
                                    );

    MERROR                          hwDummyBuffer_acquire(
                                        sp<DummyBuffer> const& dummyBuffer,
                                        char const* szName,
                                        sp<IImageBuffer> & imageBuffer
                                    );

    MERROR                          hwDummyBuffer_return(
                                        sp<DummyBuffer>& dummyBuffer
                                    );

    MERROR                          hwDummyBuffer_release(
                                        sp<DummyBuffer>& dummyBuffer
                                    );

    MERROR                          hwDummyBuffer_uninit(
                                        void
                                    );

    MERROR                          hwImageBuffer_init(
                                        sp<IImageStreamInfo>
                                        const& imgStreamInfo
                                    );

    MERROR                          hwImageBuffer_init(
                                        MBOOL isFull,
                                        MINT32 format,
                                        MSize size,
                                        MUINT32 stride,
                                        char const* szName
                                    );

    MERROR                          hwImageBuffer_lock(
                                        MBOOL isFull,
                                        sp<IImageBuffer> & imageBuffer
                                    );

    MERROR                          hwImageBuffer_unlock(
                                        MBOOL isFull
                                    );

    MERROR                          hwImageBuffer_uninit(
                                        void
                                    );

    MVOID                           generateAppMeta(
                                        sp<IPipelineFrame> const &request,
                                        MetaSet_T const &result3A,
                                        QBufInfo const &deqBuf,
                                        IMetadata &appMetadata
                                    );

    MVOID                           generateHalMeta(
                                        MetaSet_T const &result3A,
                                        QBufInfo const &deqBuf,
                                        IMetadata const &resultEIS,
                                        IMetadata const &inHalMetadata,
                                        IMetadata &halMetadata
                                    );

    MVOID                           createFullBuf(
                                        sp<IImageBuffer> const &pImageBuffer,
                                        MUINT32 num,
                                        MUINT32 idx,
                                        QBufInfo &enBuf
                                    );

    MVOID                           createResizerBuf(
                                        sp<IImageBuffer> const &pImageBuffer,
                                        MUINT32 num,
                                        MUINT32 idx,
                                        QBufInfo &enBuf
                                    );

    #if (SUPPORT_SCALING_CROP)
    MVOID                           createFullBuf(
                                        QueNode_T const& node,
                                        QBufInfo &enBuf
                                    );

    MVOID                           createResizerBuf(
                                        QueNode_T const& node,
                                        QBufInfo &enBuf
                                    );

    MVOID                           prepareCropInfo(
                                       IMetadata* pMetadata,
                                       QueNode_T& node
                                   );
    #endif

    MERROR                          check_config(
                                        ConfigParams const& rParams
                                    );

protected:  ///

    MERROR                          lockMetadata(
                                        sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        IMetadata &rMetadata
                                    );

    MERROR                          returnLockedMetadata(
                                        sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        MBOOL success = MTRUE
                                    );

    MERROR                          returnUnlockedMetadata(
                                        sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId
                                    );

    MERROR                          lock_and_returnMetadata(
                                        sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        IMetadata &metadata
                                    );


    MERROR                          lockImageBuffer(
                                        sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        sp<IImageStreamBuffer>  &pOutpImageStreamBuffer,
                                        sp<IImageBuffer> &rImageBuffer
                                    );

    MERROR                          returnLockedImageBuffer(
                                        sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        sp<IImageBuffer> const& pImageBuffer,
                                        MBOOL success = MTRUE
                                    );

    MERROR                          returnUnlockedImageBuffer(
                                        sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId
                                    );

    MERROR                          lockImageBuffer(
                                        sp<IImageStreamBuffer> const& pStreamBuffer,
                                        sp<IImageBuffer> &pImageBuffer
                                    );

    MERROR                          returnLockedImageBuffer(
                                        sp<IImageBuffer> const &pImageBuffer,
                                        sp<IImageStreamBuffer> const &pStreamBuffer,
                                        sp<IImageStreamBufferPoolT> const &pStreamPool
                                    );

    MERROR                          returnUnlockedImageBuffer(
                                        sp<IImageStreamBuffer> const& pStreamBuffer,
                                        sp<IImageStreamBufferPoolT> const &pStreamPool
                                    );

    MUINT32                         get_and_increase_magicnum()
                                    {
                                        Mutex::Autolock _l(mlastNumLock);
                                        MUINT32 ret = mlastNum++;
                                        //skip num = 0 as 3A would callback 0 when request stack is empty
                                        //skip -1U as a reserved number to indicate that which would never happen in 3A queue
                                        if(ret==0 || ret==-1U) ret = mlastNum++;
                                        return ret;
                                    }
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations in base class Thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    virtual void                    requestExit();

    // Good place to do one-time initializations
    virtual status_t                readyToRun();

private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool                    threadLoop();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
                                    P1NodeImp(InstanceInfo* pInstanceInfo);
    virtual                        ~P1NodeImp();
    virtual MERROR                  config(ConfigParams const& rParams);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNode Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.

    virtual MERROR                  init(InitParams const& rParams);

    virtual MERROR                  uninit();

    virtual MERROR                  flush();

    virtual MERROR                  flush(
                                        android::sp<IPipelineFrame> const &pFrame
                                    );

    virtual MERROR                  queue(
                                        sp<IPipelineFrame> pFrame
                                    );

public:     ////                    Operations.

    virtual void                    doNotifyCb (
                                        MINT32  _msgType,
                                        MINTPTR _ext1,
                                        MINTPTR _ext2,
                                        MINTPTR _ext3
                                    );

    static void                     doNotifyDropframe(MUINT magicNum, void* cookie);

};
};  //namespace


/******************************************************************************
 *
 ******************************************************************************/
P1NodeImp::
P1NodeImp(InstanceInfo* pInstanceInfo)
    : BaseNode()
    , P1Node()
    , mConfigRWLock()
    , mInit(MTRUE)

    , mImageStorage()
    , mpInstanceInfo(pInstanceInfo)
    //
    , m3AProcessedDepth(3)
    , mNumHardwareBuffer(3)
    , mDelayframe(4)
    , mlastNum(1)
    , mlastNonReprocNum(-1)
    , mlastNumLock()
    , mHardwareLock()
    , mActive(MFALSE)
    , mCamActive(MFALSE)
    , m3AActive(MFALSE)
    , mEisActive(MFALSE)
    , mpCamIO(NULL)
    , mp3A(NULL)
    , mpIEventIrq(NULL)
    #if SUPPORT_EIS
    , mpEIS(NULL)
    #endif
    //
    , mPixelMode(0)
    //
    , mDummyBufferFull(NULL)
    , mDummyBufferResizer(NULL)
    //
    , mImageBufferFull(NULL)
    , mImageBufferResizer(NULL)
    , mImageBufferNameFull("")
    , mImageBufferNameResizer("")
    , mImageBufferLockedFull(false)
    , mImageBufferLockedResizer(false)
    , mImageBufferLock()
    //
    #if SUPPORT_HQC
    , mHighQualityCaptureState(P1_HQC_STATE_NONE)
    , mHighQualityCaptureNum(0)
    , mHighQualityCaptureAddr(NULL)
    , mHighQualityCaptureLock()
    , mHighQualityCaptureCond()
    , mHighQualityCaptureForceDisable(false)
    #endif
    //
    , mRequestQueueLock()
    , mRequestQueue()
    //
    , mControls3AListLock()
    , mControls3AList()
    , mControls3AListCond()
    //
    , m3AStorage()
    //
    , mProcessingQueueLock()
    , mProcessingQueueCond()
    , mProcessingQueue()
    //
    , mRedoQueueLock()
    , mRedoQueue()
    //
    , mYuvQueueLock()
    , mYuvQueue()
    //
    , mReprocessLock()
    , mReprocessCond()
    , mReprocessGoing(MFALSE)
    //
    , mDropQueueLock()
    , mDropQueue()
    //
    , mThreadLock()
    , mThreadCond()
    //
    , mdummyQueue()
    //
    , mDequeThreadProfile("P1Node::deque", 15000000LL)
    , mInFlightRequestCnt(0)
{
    char cLogLevel[PROPERTY_VALUE_MAX];
    ::property_get("debug.camera.log", cLogLevel, "0");
    mLogLevel = ::atoi(cLogLevel);
    if ( 0 == mLogLevel ) {
        ::property_get("debug.camera.log.p1node", cLogLevel, "0");
        mLogLevel = ::atoi(cLogLevel);
    }
    #if SUPPORT_HQC
    {
        char cDisableHQC[PROPERTY_VALUE_MAX] = {0};
        ::property_get("debug.camera.disable_hqc", cDisableHQC, "0");
        mHighQualityCaptureForceDisable =
            (::atoi(cDisableHQC) > 0) ? true : false;
        MY_LOGI_IF(mHighQualityCaptureForceDisable,
            "HighQualityCapture - Force Disable");
    }
    #endif
    if (mpInstanceInfo != NULL) {
        mpInstanceInfo->setLog((mLogLevel >= 1) ? MTRUE : MFALSE);
        mpInstanceInfo->addCnt();
    }
}


/******************************************************************************
 *
 ******************************************************************************/
P1NodeImp::
~P1NodeImp()
{
    MY_LOGD("");
    if (mpInstanceInfo != NULL) {
        mpInstanceInfo->delCnt();
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
init(InitParams const& rParams)
{
    FUNCTION_IN

    {
        RWLock::AutoWLock _l(mConfigRWLock);
        //
        mOpenId  = rParams.openId;
        mNodeId  = rParams.nodeId;
        mNodeName= rParams.nodeName;
    }

    MERROR err = run("P1NodeImp::init");

    FUNCTION_OUT

    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
uninit()
{
    PUBLIC_API_IN

    // flush the left frames if exist
    onHandleFlush(MFALSE);

    requestExit();

    hwImageBuffer_uninit();

    hwDummyBuffer_uninit();

    PUBLIC_API_OUT

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
check_config(ConfigParams const& rParams)
{
    CAM_TRACE_CALL();

    if (rParams.pInAppMeta == NULL ) {
        MY_LOGE("in metadata is null");
        return BAD_VALUE;
    }

    if (rParams.pOutAppMeta == NULL) {
        MY_LOGE("out app metadata is null");
        return BAD_VALUE;
    }

    if (rParams.pOutHalMeta == NULL) {
        MY_LOGE("out hal metadata is null");
        return BAD_VALUE;
    }

    if(rParams.pvOutImage_full.size() == 0 &&
        rParams.pOutImage_resizer == NULL &&
        rParams.pOutImage_opaque == NULL &&
        rParams.pInImage_opaque == NULL) {
        MY_LOGE("image IO is empty");
    }

    if (rParams.pvOutImage_full.size() == 0 && rParams.pOutImage_resizer == NULL) {
        MY_LOGE("image is empty");
        return BAD_VALUE;
    }

    if (rParams.pStreamPool_full == NULL && rParams.pStreamPool_resizer == NULL) {
        MY_LOGE("image pool is empty");
        return BAD_VALUE;
    }

    if (rParams.pStreamPool_full != NULL && rParams.pvOutImage_full.size() == 0) {
        MY_LOGE("wrong full input");
        return BAD_VALUE;
    }

    if (rParams.pStreamPool_resizer != NULL && rParams.pOutImage_resizer == NULL) {
        MY_LOGE("wrong resizer input");
        return BAD_VALUE;
    }

    //
    {
        RWLock::AutoWLock _l(mConfigRWLock);
        //
        if(rParams.pInAppMeta != 0) {
            mInAppMeta = rParams.pInAppMeta;
            mInStreamIds.add(mInAppMeta->getStreamId());
        } else {
            mInAppMeta = NULL;
            mInStreamIds.clear();
        }
        //
        if(rParams.pInHalMeta != 0) {
            mInHalMeta = rParams.pInHalMeta;
            mInStreamIds.add(mInHalMeta->getStreamId());
        } else {
            mInHalMeta = NULL;
            mInStreamIds.clear();
        }
        mOutAppMeta          = (rParams.pOutAppMeta != 0)?         rParams.pOutAppMeta         : NULL;
        mOutHalMeta          = (rParams.pOutHalMeta != 0)?         rParams.pOutHalMeta         : NULL;
        mOutImage_resizer    = (rParams.pOutImage_resizer != 0)?   rParams.pOutImage_resizer   : NULL;
        mpStreamPool_full    = (rParams.pStreamPool_full != 0)?    rParams.pStreamPool_full    : NULL;
        mpStreamPool_resizer = (rParams.pStreamPool_resizer != 0)? rParams.pStreamPool_resizer : NULL;
        if(rParams.pvOutImage_full.size() != 0) {
            mvOutImage_full  = rParams.pvOutImage_full;
        } else {
            mvOutImage_full.clear();
        }

        mSensorParams = rParams.sensorParams;
        mInImage_opaque = ((rParams.pInImage_opaque != 0) ?
                        rParams.pInImage_opaque : NULL);
        mOutImage_opaque = ((rParams.pOutImage_opaque != 0) ?
                        rParams.pOutImage_opaque : NULL);

        mInImage_yuv = (rParams.pInImage_yuv != 0)? rParams.pInImage_yuv : NULL;

        mRawParams = rParams.rawParams;

        MY_LOGI("[Config] sensor(M:%d S:%dx%d)"
            " raw(F:0x%x S:%dx%d std:%d len:%d)",
            mSensorParams.mode, mSensorParams.size.w, mSensorParams.size.h,
            mRawParams.format, mRawParams.size.w, mRawParams.size.h,
            mRawParams.stride, mRawParams.length);
        MY_LOGI("[Config] Meta Id: In(0x%x) OutApp(0x%x) OutHal(0x%x)",
            mInAppMeta->getStreamId(),
            mOutAppMeta->getStreamId(),
            mOutHalMeta->getStreamId());
        for(size_t i = 0; i < mvOutImage_full.size(); i++) {
            MY_LOGI("[Config] Image Id: OutFull[%d](0x%x) - %dx%d",
                i, mvOutImage_full[i]->getStreamId(),
                mvOutImage_full[i]->getImgSize().w,
                mvOutImage_full[i]->getImgSize().h);
        }
        if (mOutImage_resizer != NULL) {
            MY_LOGI("[Config] Image Id: OutResizer(0x%x) - %dx%d",
                mOutImage_resizer->getStreamId(),
                mOutImage_resizer->getImgSize().w,
                mOutImage_resizer->getImgSize().h);
        }
        if (mOutImage_opaque != NULL) {
            MY_LOGI("[Config] Image Id: OutOpaque(0x%x) - %dx%d",
                mOutImage_opaque->getStreamId(),
                mOutImage_opaque->getImgSize().w,
                mOutImage_opaque->getImgSize().h);
        }
        if (mInImage_opaque != NULL) {
            MY_LOGI("[Config] Image Id: InOpaque(0x%x) - %dx%d",
                mInImage_opaque->getStreamId(),
                mInImage_opaque->getImgSize().w,
                mInImage_opaque->getImgSize().h);
        }

        if (mInImage_yuv != NULL) {
            MY_LOGI("[Config] Image Id: InOpaque(0x%x) - %dx%d",
                mInImage_yuv->getStreamId(),
                mInImage_yuv->getImgSize().w,
                mInImage_yuv->getImgSize().h);
        }
    }

    MY_LOGI("[Config] In Meta Id: %#" PRIx64 ", Out APP Meta Id: %#" PRIx64 ", Out Hal Meta Id: %#" PRIx64 " ",
            mInAppMeta->getStreamId(), mOutAppMeta->getStreamId(), mOutHalMeta->getStreamId());

    for(size_t i = 0; i < mvOutImage_full.size(); i++) {
        MY_LOGI("[Config] full image Id: %#" PRIx64 " (%d,%d)",
                 mvOutImage_full[i]->getStreamId(), mvOutImage_full[i]->getImgSize().w, mvOutImage_full[i]->getImgSize().h);
    }
    if (mOutImage_resizer != NULL) {
        MY_LOGI("[Config] resizer image Id: %#" PRIx64 " (%d,%d)",
                 mOutImage_resizer->getStreamId(), mOutImage_resizer->getImgSize().w, mOutImage_resizer->getImgSize().h);
    }

#if (SUPPORT_SCALING_CROP)
    {
        sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(getOpenId());
        if( ! pMetadataProvider.get() ) {
            MY_LOGE(" ! pMetadataProvider.get() ");
            return DEAD_OBJECT;
        }
        IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();
        if( tryGetMetadata<MRect>(&static_meta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, mActiveArray) ) {
            MY_LOGD_IF(1,"active array(%d, %d, %dx%d)",
                    mActiveArray.p.x, mActiveArray.p.y, mActiveArray.s.w, mActiveArray.s.h);
        } else {
            MY_LOGE("no static info: MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION");
            return UNKNOWN_ERROR;
        }
    }
#endif

    return OK;

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
config(ConfigParams const& rParams)
{
    PUBLIC_API_IN

    Mutex::Autolock _l(mPublicLock);
    //(1) check
    MERROR err = check_config(rParams);
    if (err != OK) {
        return err;
    }

    //(2) configure hardware

    if(mActive) {
        MY_LOGD("active=%d", mActive);
        onHandleFlush(MFALSE);
        err = hwDummyBuffer_uninit();
        if (err != OK) {
            return err;
        }
    }

    err = hwDummyBuffer_init();
    if (err != OK) {
        return err;
    }

    err = hardwareOps_start();
    if (err != OK) {
        return err;
    }

    PUBLIC_API_OUT

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
queue(
    sp<IPipelineFrame> pFrame
)
{
    PUBLIC_API_IN

    CAM_TRACE_CALL();
    MY_LOGD1("active=%d", mActive);

    #if SUPPORT_HQC
    if (highQualityCapture_get() > P1_HQC_STATE_NONE) {
        {
            Mutex::Autolock _l(mHighQualityCaptureLock);
            MY_LOGD1("wait HQC + (num: %d)", mHighQualityCaptureNum);
            status_t status = mHighQualityCaptureCond.wait(mHighQualityCaptureLock);
            MY_LOGD1("wait HQC - (num: %d)", mHighQualityCaptureNum);
            if  ( OK != status ) {
                MY_LOGW(
                    "wait HQC status:%d:%s, mHighQualityCaptureNum:%d",
                    status, ::strerror(-status), mHighQualityCaptureNum
                );
            }
        }
        hardwareOps_resume();
    }
    #endif

    if (!mActive) {
        hardwareOps_start();
    }

    {
        Mutex::Autolock _l(mControls3AListLock);

        //block condition 1: if pipeline is full
        while (mControls3AList.size() > (size_t)m3AProcessedDepth) {
            MY_LOGD1("wait: %zu > %d", mControls3AList.size(), m3AProcessedDepth);
            status_t status = mControls3AListCond.wait(mControls3AListLock);
            MY_LOGD1("wait-");
            if  ( OK != status ) {
                MY_LOGW(
                    "wait status:%d:%s, mControls3AList.size:%zu",
                    status, ::strerror(-status), mControls3AList.size()
                );
            }
        }
        //compensate to the number of mProcessedDepth
        while(mControls3AList.size() < (size_t)m3AProcessedDepth) {
            createNode(NULL, NULL, NULL, &mControls3AList, NULL);
        }

        //push node from appFrame
        createNode(pFrame, &mRequestQueue, &mRequestQueueLock, &mControls3AList, NULL);
        android_atomic_inc(&mInFlightRequestCnt);
        ATRACE_INT("P1_request_cnt", android_atomic_acquire_load(&mInFlightRequestCnt));
    }

    if ((mInImage_opaque != NULL) || (mInImage_yuv != NULL)) {
        Mutex::Autolock _l(mReprocessLock);
        if (mReprocessGoing) {
            MY_LOGI("ReprocessGoing wait");
            mReprocessCond.wait(mReprocessLock);
            MY_LOGI("ReprocessGoing wait OK");
        }
    }
    if (mInImage_opaque != NULL) {
        Mutex::Autolock _l(mRedoQueueLock);
        if (!mRedoQueue.empty()) {
            Mutex::Autolock _ll(mTransferLock);
            if (mRequestQueue.empty() && mProcessingQueue.empty()) {
                onRedoFrame(0, MFALSE, MFALSE);
            }
        }
    }
    //
    if (mInImage_yuv != NULL) {
        Mutex::Autolock _l(mYuvQueueLock);
        if (!mYuvQueue.empty()) {
            Mutex::Autolock _ll(mTransferLock);
            if (mRequestQueue.empty() && mProcessingQueue.empty()) {
                onYuvFrame(0, MFALSE, MFALSE);
            }
        }
    }


    PUBLIC_API_OUT

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
flush(android::sp<IPipelineFrame> const &pFrame)
{
    return BaseNode::flush(pFrame);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
flush()
{
    PUBLIC_API_IN

    CAM_TRACE_CALL();

    Mutex::Autolock _l(mPublicLock);
    {
        Mutex::Autolock _lReproc(mReprocLock);
    }
    onHandleFlush(MFALSE);

    //[TODO]
    //wait until deque thread going back to waiting state;
    //in case next node receives queue() after flush()

    PUBLIC_API_OUT

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
void
P1NodeImp::
requestExit()
{
    FUNCTION_IN

    //let deque thread back
    Thread::requestExit();
    {
        Mutex::Autolock _l(mThreadLock);
        mThreadCond.broadcast();
    }
    join();

    //let enque thread back
    Mutex::Autolock _l(mControls3AListLock);
    mControls3AListCond.broadcast();

    FUNCTION_OUT
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
P1NodeImp::
readyToRun()
{
    // set name
    ::prctl(PR_SET_NAME, (unsigned long)"Cam@P1NodeImp", 0, 0, 0);

    // set normal
    struct sched_param sched_p;
    sched_p.sched_priority = 0;
    ::sched_setscheduler(0, P1THREAD_POLICY, &sched_p);
    ::setpriority(PRIO_PROCESS, 0, P1THREAD_PRIORITY);   //  Note: "priority" is nice value.
    //
    ::sched_getparam(0, &sched_p);
    MY_LOGD(
        "Tid: %d, policy: %d, priority: %d"
        , ::gettid(), ::sched_getscheduler(0)
        , sched_p.sched_priority
    );

    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
P1NodeImp::
threadLoop()
{
    // check if going to leave thread
    CAM_TRACE_CALL();
    {
        Mutex::Autolock _l(mThreadLock);

        if (!mActive) {
            MY_LOGD("wait+");
            mThreadCond.wait(mThreadLock);
            MY_LOGD("wait-");
        }

        if (exitPending()) {
            MY_LOGD("leaving");
            return true;
        }
    }

    // deque buffer, and handle frame and metadata
    onProcessDequedFrame();


    // trigger point for the first time
    {
        RWLock::AutoWLock _l(mConfigRWLock);
        if (mInit) {
            onProcessFrame(MTRUE);
            mInit = MFALSE;
        }
    }

    onProcessDropFrame();
#if SUPPORT_HQC
    if (highQualityCapture_get() == P1_HQC_STATE_DEQ) {
        highQualityCapture_uninit();
    }
#endif
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
onProcessFrame(
    MBOOL initial,
    MUINT32 magicNum,
    MUINT32 sofIdx
)
{
    FUNCTION_IN

    size_t size = 0;

    #if SUPPORT_HQC
    if (highQualityCapture_get() > P1_HQC_STATE_SET) {
        MY_LOGI("HighQualityCapture is on processing bypass num:%d sof:%d",
            magicNum, sofIdx);
        return;
    }
    #endif

    //(1)
    if(!initial) { // [TODO] && VALID 3A PROCESSED NOTIFY
        QueNode_T node;
        bool exist = false;
        {
            Mutex::Autolock _l(mRequestQueueLock);
            Que_T::iterator it = mRequestQueue.begin();
            for(; it != mRequestQueue.end(); it++) {
                if ((*it).magicNum == magicNum) {
                    node = *it;
                    node.sofIdx = sofIdx;
                    mRequestQueue.erase(it);
                    exist = true;
                    break;
                }
            }
        }
        if (exist) {
            onProcessEnqueFrame(node);
            Mutex::Autolock _ll(mProcessingQueueLock);
            mProcessingQueue.push_back(node);
        } else {
            MY_LOGW_IF(magicNum!=0, "no: %d", magicNum);
            Mutex::Autolock _l(mRequestQueueLock);
            String8 str;
            str += String8::format("[req/size(%zu)]: ", mRequestQueue.size());
            Que_T::iterator it = mRequestQueue.begin();
            for(; it != mRequestQueue.end(); it++) {
                str += String8::format("%d ", (*it).magicNum);
            }
            MY_LOGD("%s", str.string());
            #if SUPPORT_HQC
            if (highQualityCapture_get() == P1_HQC_STATE_SET) {
                MY_LOGW("HighQualityCapture is on but not match: %d", magicNum);
                highQualityCapture_uninit();
            }
            #endif
        }
    }

    //(2)
    {
        Mutex::Autolock _l(mControls3AListLock);
        if (!mControls3AList.empty()) {
            mControls3AList.erase(mControls3AList.begin());
        }
        mControls3AListCond.broadcast();

        //dump
        String8 str("[3A]: ");
        List<MetaSet_T>::iterator it = mControls3AList.begin();
        for (; it != mControls3AList.end(); it++) {
            str += String8::format("%d ", it->halMeta.entryFor(MTK_P1NODE_PROCESSOR_MAGICNUM).itemAt(0, Type2Type< MINT32 >()));
        }
        MY_LOGD1("%s", str.string());

    }
    //(3)
    #if SUPPORT_3A
    if (mp3A) {
        #if SUPPORT_HQC
        if (highQualityCapture_get() == P1_HQC_STATE_REQ) {
            MBOOL isHighQualityCaptureSet = false;
            MetaSet_T controls3A;
            {
                Mutex::Autolock _l(mControls3AListLock);
                List<MetaSet_T>::iterator it = mControls3AList.begin();
                if (mControls3AList.size() >= (size_t)(m3AProcessedDepth - 1)) {
                    for(int i = 0; i < (m3AProcessedDepth - 1); i++) {
                        it ++;
                    }
                    MUINT32 setNum =
                        it->halMeta.entryFor(MTK_P1NODE_PROCESSOR_MAGICNUM).
                        itemAt(0, Type2Type< MINT32 >());
                    if (highQualityCapture_check(setNum)) {
                        highQualityCapture_set(P1_HQC_STATE_SET);
                        isHighQualityCaptureSet = true;
                    }
                }
            }
            MY_LOGD1("HighQualityCapture set(%d) current_num(%d)",
                isHighQualityCaptureSet, magicNum);
        }
        #endif
        {
            Mutex::Autolock _l(mControls3AListLock);
            size = mControls3AList.size();
        }
        mp3A->set(mControls3AList, size);
    }
    #endif
    FUNCTION_OUT
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
onProcessEnqueFrame(
    QueNode_T &pFrame
)
{
    FUNCTION_IN

    //(1)
    //pass request directly if it's a reprocessing one
    //[TODO]
    //if( mInHalMeta == NULL) {
    //    onDispatchFrame(pFrame);
    //    return;
    //}

    //(2)
    hardwareOps_enque(pFrame, MFALSE, MTRUE);

    FUNCTION_OUT
}


/******************************************************************************
 *
 ******************************************************************************/
P1NodeImp::QueNode_T
P1NodeImp::
getProcessingFrame_ByNumber(MUINT32 magicNum)
{
    FUNCTION_IN
    QueNode_T frame;

    Mutex::Autolock _l(mProcessingQueueLock);
    if (mProcessingQueue.empty()) {
        MY_LOGE("mProcessingQueue is empty");
        return frame;
    }

    #if 1
        Que_T::iterator it = mProcessingQueue.begin();
        for (; it != mProcessingQueue.end(); it++) {
            frame = *it;
            if (frame.magicNum == magicNum) {
                break;
            }
        }
        if (it == mProcessingQueue.end()) {
            MY_LOGE("cannot find the right node for num: %d", magicNum);
            return frame;
        }
        else {
            frame = *it;
            mProcessingQueue.erase(it);
            mProcessingQueueCond.broadcast();
        }
    #else
        frame = *mProcessingQueue.begin();
        mProcessingQueue.erase(mProcessingQueue.begin());
        mProcessingQueueCond.broadcast();
    #endif

    FUNCTION_OUT
    //
    return frame;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P1NodeImp::
getProcessingFrame_ByAddr(IImageBuffer* const imgBuffer,
                          MUINT32 magicNum,
                          QueNode_T &frame
)
{
    FUNCTION_IN

    MBOOL ret = MFALSE;
    if (imgBuffer == NULL) {
        MY_LOGE("imgBuffer == NULL");
        return ret;
    }

    // get the right node from mProcessingQueue
    Mutex::Autolock _l(mProcessingQueueLock);
    if (mProcessingQueue.empty()) {
        MY_LOGE("ProQ is empty");
        return ret;
    }

    Que_T::iterator it = mProcessingQueue.begin();
    for (; it != mProcessingQueue.end(); it++) {
        frame = *it;
        if (imgBuffer == (*it).img_full.get() ||
            imgBuffer == (*it).img_resizer.get() ||
            imgBuffer == (*it).img_opaque.get()) {
            if (frame.magicNum == magicNum) {
                ret = MTRUE;
            } else {
                #if SUPPORT_PERFRAME_CTRL
                MY_LOGE("magicNum from driver(%d), should(%d)",
                       magicNum, frame.magicNum);
                #else
                if((magicNum & 0x40000000) != 0) {
                    MY_LOGW("magicNum from driver(0x%x) is uncertain",
                          magicNum);
                    ret = MFALSE;
                } else {
                    ret = MTRUE;
                    MY_LOGW("magicNum from driver(%d), should(%d)",
                          magicNum, frame.magicNum);
                }
                #endif
            }
            break;
        } else {
            continue;
        }
    }

    if (it == mProcessingQueue.end()) {
        MY_LOGE("no node with imagebuf(%p), PA(0x%zx), num(%d)",
                 imgBuffer, imgBuffer->getBufPA(0), magicNum);
        for (Que_T::iterator it = mProcessingQueue.begin(); it != mProcessingQueue.end(); it++) {
            MY_LOGW("[proQ] num(%d)", (*it).magicNum);
            MY_LOGW_IF((*it).img_full!=NULL, "[proQ] imagebuf(%p), PA(0x%zx)",
                (*it).img_full.get(), (*it).img_full->getBufPA(0));
            MY_LOGW_IF((*it).img_resizer!=NULL, "[proQ] imagebuf(%p), PA(0x%zx)",
                (*it).img_resizer.get(), (*it).img_resizer->getBufPA(0));
        }
        for (Que_T::iterator it = mRequestQueue.begin(); it != mRequestQueue.end(); it++) {
            MY_LOGW("[reqQ] magic %d:", (*it).magicNum);
        }
    }
    else {

        if (mProcessingQueue.size() == 1) {
            if (mInImage_opaque != NULL) {
                Mutex::Autolock _l(mRedoQueueLock);
                if (!mRedoQueue.empty()) {
                    Mutex::Autolock _ll(mReprocessLock);
                    mReprocessGoing |= MTRUE;
                }
            }
            if (mInImage_yuv != NULL) {
                Mutex::Autolock _l(mYuvQueueLock);
                if (!mYuvQueue.empty()) {
                    Mutex::Autolock _ll(mReprocessLock);
                    mReprocessGoing |= MTRUE;
                }
            }
        };

        frame = *it;
        mProcessingQueue.erase(it);
        mProcessingQueueCond.broadcast();
        MY_LOGD1("magic: %d", magicNum);
    }

    FUNCTION_OUT
    //
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
onProcessDropFrame()
{
    Vector<QueNode_T > nodeQ;
    {
        Mutex::Autolock _l(mDropQueueLock);
        for(size_t i = 0; i < mDropQueue.size(); i++) {
            QueNode_T node = getProcessingFrame_ByNumber(mDropQueue[i]);
            nodeQ.push_back(node);
            MY_LOGD("drop: %d", mDropQueue[i]);
            #if SUPPORT_HQC
            if (highQualityCapture_get() == P1_HQC_STATE_ENQ &&
                highQualityCapture_check(mDropQueue[i])) {
                MY_LOGW("HighQualityCapture drop: %d", mDropQueue[i]);
                highQualityCapture_set(P1_HQC_STATE_DEQ);
            }
            #endif
        }
        mDropQueue.clear();
    }

    for(size_t i = 0; i < nodeQ.size(); i++) {
         onFlushProcessingFrame(nodeQ[i]);
    }

    return OK;
}


#if SUPPORT_HQC
/******************************************************************************
 *
 ******************************************************************************/
void
P1NodeImp::
highQualityCapture_init(MUINT32 setMagicNum)
{
    FUNCTION_IN;
    Mutex::Autolock _l(mHighQualityCaptureLock);
    mHighQualityCaptureState = P1_HQC_STATE_REQ;
    mHighQualityCaptureNum = setMagicNum;
    MY_LOGI("HighQualityCapture(%d) init", mHighQualityCaptureNum);
    FUNCTION_OUT;
}


/******************************************************************************
 *
 ******************************************************************************/
void
P1NodeImp::
highQualityCapture_set(P1_HQC_STATE state)
{
    Mutex::Autolock _l(mHighQualityCaptureLock);
    mHighQualityCaptureState = state;
    MY_LOGD1("HighQualityCapture(%d) set[%d]",
        mHighQualityCaptureNum,mHighQualityCaptureState);
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
void
P1NodeImp::
highQualityCapture_setAddr(MVOID* addr)
{
    Mutex::Autolock _l(mHighQualityCaptureLock);
    mHighQualityCaptureAddr = addr;
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
P1_HQC_STATE
P1NodeImp::
highQualityCapture_get(void)
{
    Mutex::Autolock _l(mHighQualityCaptureLock);
    return mHighQualityCaptureState;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID*
P1NodeImp::
highQualityCapture_getAddr()
{
    Mutex::Autolock _l(mHighQualityCaptureLock);
    return mHighQualityCaptureAddr;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P1NodeImp::
highQualityCapture_check(MUINT32 checkMagicNum)
{
    Mutex::Autolock _l(mHighQualityCaptureLock);
    return ((mHighQualityCaptureState > P1_HQC_STATE_NONE)
        && (mHighQualityCaptureNum == checkMagicNum));
}


/******************************************************************************
 *
 ******************************************************************************/
void
P1NodeImp::
highQualityCapture_uninit()
{
    FUNCTION_IN;

    MY_LOGI("HighQualityCapture(%d) uninit", mHighQualityCaptureNum);
    onHandleFlush(MFALSE);
    hwImageBuffer_unlock(true);

    Mutex::Autolock _l(mHighQualityCaptureLock);
    mHighQualityCaptureState = P1_HQC_STATE_NONE;
    mHighQualityCaptureNum = 0;
    mHighQualityCaptureCond.broadcast();

    FUNCTION_OUT;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
createPureBuf(QueNode_T const& node, QBufInfo &enBuf)
{
    if (mvOutImage_full.size() == 0) {
        return;
    }
    if (mOutImage_resizer == NULL) {
        return;
    }
    // by driver comment
    // use EPipe_PURE_RAW in RRZO, then
    // IMGO: got PURE_RAW
    // RRZO: got PROCESSED_RAW
    MY_LOGD("[CropInfo] (%d,%d-%dx%d)(%dx%d)",
        node.cropRect_resizer.p.x, node.cropRect_resizer.p.y,
        node.cropRect_resizer.s.w, node.cropRect_resizer.s.h,
        node.dstSize_resizer.w, node.dstSize_resizer.h);
    // for IMGO setting
    MSize dstSize_full = mvOutImage_full[0]->getImgSize();
    MRect cropRect_full = MRect(MPoint(0, 0), mSensorParams.size);
    NSCam::NSIoPipe::NSCamIOPipe::BufInfo rBufInfo_full (
        PORT_IMGO,
        node.img_full.get(),
        dstSize_full,
        cropRect_full,
        node.magicNum,
        node.sofIdx,
        (MUINT32)(EPipe_PURE_RAW));
    enBuf.mvOut.push_back(rBufInfo_full);
    // for RRZO setting
    MSize dstSize_resizer = node.dstSize_resizer;
    MRect cropRect_resizer = node.cropRect_resizer;
    NSCam::NSIoPipe::NSCamIOPipe::BufInfo rBufInfo_resizer (
        PORT_RRZO,
        node.img_resizer.get(),
        dstSize_resizer,
        cropRect_resizer,
        node.magicNum,
        node.sofIdx,
        (MUINT32)(EPipe_PURE_RAW));
    enBuf.mvOut.push_back(rBufInfo_resizer);
    //
    highQualityCapture_setAddr(node.img_full.get());
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
onReprocessRawFrame(
    QueNode_T &pFrame,
    QBufInfo &deqBuf,
    MetaSet_T &result3A
)
{
    FUNCTION_IN;
    sp<IImageStreamBuffer> pure_halFrame_full = pFrame.halFrame_full;
    sp<IImageBuffer> pure_img_full = pFrame.img_full;
    const char* initStr = "P1NodeReprocess";
    const NSCam::NSIoPipe::PortID IMGI( NSIoPipe::EPortType_Memory,
                                        NSImageio::NSIspio::EPortIndex_IMGI, 0);
    const NSCam::NSIoPipe::PortID IMGO( NSIoPipe::EPortType_Memory,
                                        NSImageio::NSIspio::EPortIndex_IMGO, 1);
    const NSCam::NSIoPipe::PortID RRZO( NSIoPipe::EPortType_Memory,
                                        NSImageio::NSIspio::EPortIndex_RRZO, 1);

    // init hardware instance
    NSCam::NSIoPipe::NSPostProc::IFeatureStream* pStream = NULL;
    pStream = NSCam::NSIoPipe::NSPostProc::IFeatureStream::createInstance(
        initStr,
        /*NSCam::NSIoPipe::NSPostProc::*/EFeatureStreamTag_PRaw_Stream,
        0xFFFF, true);
    pStream->init();
    QParams enqueParams;
    QParams dequeParams;
    //
    // prepare 3A tuning
    void* pTuning = NULL;
    unsigned int tuningsize;
    if(!pStream->deTuningQue(tuningsize, pTuning) || (pTuning == NULL)) {
        MY_LOGE("cannot get tunning buffer");
        return BAD_VALUE;
    }
    MetaSet_T controlMetaSet;
    if (pFrame.appFrame != NULL) {
        MetaSet_T inMetaSet;
        IMetadata result_NULL;
        result_NULL.clear();
        /*
        if (mInAppMeta != NULL) {
            lockMetadata(pFrame.appFrame, mInAppMeta->getStreamId(),
                inMetaSet.appMeta);
        }
        */
        if (mInHalMeta != NULL) {
            lockMetadata(pFrame.appFrame, mInHalMeta->getStreamId(),
                inMetaSet.halMeta);
        }
        generateAppMeta(pFrame.appFrame, result3A, deqBuf,
                                                        controlMetaSet.appMeta);
        generateHalMeta(result3A, deqBuf, result_NULL, inMetaSet.halMeta,
                                                        controlMetaSet.halMeta);
    }
    mp3A->setIsp(1, controlMetaSet, pTuning, &result3A);
    enqueParams.mvTuningData.push_back((MVOID*)pTuning);
    //
    // prepare parameter and src/dst buffers from input frame
    Input src;
    src.mPortID = IMGI;
    if (pure_img_full != NULL) {
        src.mBuffer = pure_img_full.get();
    } else {
        MY_LOGE("Buffer No Exist - src");
        return BAD_VALUE;
    }
    enqueParams.mvIn.push_back(src);
    //
    Output dstFull;
    dstFull.mPortID = IMGO;
    for (size_t i = 0; i < mvOutImage_full.size(); i++) {
        sp<IImageStreamBuffer>  pOutpImageStreamBuffer;
        sp<IImageBuffer> imageBuffer;
        if (OK != lockImageBuffer(pFrame.appFrame,
                                    mvOutImage_full[i]->getStreamId(),
                                    pOutpImageStreamBuffer, imageBuffer)) {
            continue;
        } else {
            dstFull.mBuffer = imageBuffer.get();
            pFrame.img_full = imageBuffer;
            pFrame.halFrame_full = NULL;

            if (dstFull.mBuffer == NULL) {
                MY_LOGE("Buffer No Exist - dst_full");
                return BAD_VALUE;
            } else {
                enqueParams.mvOut.push_back(dstFull);
                //
                // pass1 crop info
                MRect p1SrcCrop_Full;
                MSize p1Dst_Full;
                MRect p1DstCrop_Full;
                p1SrcCrop_Full = pFrame.cropRect_full;
                p1Dst_Full = pFrame.dstSize_full;
                p1DstCrop_Full.s = pFrame.dstSize_full;
                p1DstCrop_Full.p.x = 0;
                p1DstCrop_Full.p.y = 0;
                for (size_t i = 0; i < deqBuf.mvOut.size(); i++) {
                    if (deqBuf.mvOut[i].mPortID == PORT_IMGO) {
                        MY_LOGD1("[CropInfo][IMGO] mvOut[%zu] -"
                        " CropS(%d, %d, %dx%d) CropD(%d, %d, %dx%d) "
                        "DstSize(%dx%d)", i,
                        deqBuf.mvOut[i].mMetaData.mCrop_s.leftTop().x,
                        deqBuf.mvOut[i].mMetaData.mCrop_s.leftTop().y,
                        deqBuf.mvOut[i].mMetaData.mCrop_s.size().w,
                        deqBuf.mvOut[i].mMetaData.mCrop_s.size().h,
                        deqBuf.mvOut[i].mMetaData.mCrop_d.leftTop().x,
                        deqBuf.mvOut[i].mMetaData.mCrop_d.leftTop().y,
                        deqBuf.mvOut[i].mMetaData.mCrop_d.size().w,
                        deqBuf.mvOut[i].mMetaData.mCrop_d.size().h,
                        deqBuf.mvOut[i].mMetaData.mDstSize.w,
                        deqBuf.mvOut[i].mMetaData.mDstSize.h);

                        p1SrcCrop_Full = deqBuf.mvOut[i].mMetaData.mCrop_s;
                        p1Dst_Full = deqBuf.mvOut[i].mMetaData.mDstSize;
                        p1DstCrop_Full = deqBuf.mvOut[i].mMetaData.mCrop_d;
                    }
                }
                enqueParams.mvP1SrcCrop.push_back(p1SrcCrop_Full);
                enqueParams.mvP1Dst.push_back(p1Dst_Full);
                enqueParams.mvP1DstCrop.push_back(p1DstCrop_Full);
            }
        }
    }
    //
    enqueParams.mvPrivaData.push_back(NULL);
    enqueParams.mvMagicNo.push_back(pFrame.magicNum);
    //
    // suspend hardware
    hardwareOps_suspend();
#if 1
    // enqueue and dequeue to hardware
    //enque
    MY_LOGD1("ENQUE BEGIN");
    if (!pStream->enque(enqueParams)) {
        MY_LOGE("ENQUE FAIL");
    }
    MY_LOGD1("ENQUE DONE");
    //deque
    MY_LOGD1("DEQUE BEGIN");
    if (!pStream->deque(dequeParams)) {
        MY_LOGE("DEQUE FAIL");
    }
    MY_LOGD1("DEQUE DONE");
#endif
    //
    if ((pure_halFrame_full != 0) && (mpStreamPool_full != 0)) {
        returnLockedImageBuffer(pure_img_full, pure_halFrame_full,
                                                mpStreamPool_full);
    } else {
        hwImageBuffer_unlock(true);
    }
    // return the tuning buffer
    if(pTuning) {
        pStream->enTuningQue(pTuning);
        pTuning = NULL;
    }
    // uninit hardware instance
    pStream->uninit();
    pStream->destroyInstance(initStr);
    //
    FUNCTION_OUT;
    return OK;
}
#endif


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
onProcessDequedFrame()
{
    FUNCTION_IN

    CAM_TRACE_CALL();

    QBufInfo deqBuf;
    MetaSet_T result3A;
    if(hardwareOps_deque(deqBuf, result3A) != OK) {
        if(mlastNonReprocNum <= mlastProcessedNum)
            onRedoFrame(0, false, true);
        return BAD_VALUE;
    }

    QueNode_T pFrame;
    MBOOL match = getProcessingFrame_ByAddr(deqBuf.mvOut[0].mBuffer,
                                          deqBuf.mvOut[0].mMetaData.mMagicNum_hal,
                                          pFrame);
    MERROR ret;
    if (match == MFALSE) {
        onFlushProcessingFrame(pFrame);
        ret = BAD_VALUE;
    }
    else {
        IMetadata resultEIS;
        IMetadata inAPP;
#if SUPPORT_HQC
        MVOID* imgo_addr = NULL;
        for ( size_t i = 0; i < deqBuf.mvOut.size(); i++ )
        {
            if( deqBuf.mvOut[i].mPortID.index == PORT_IMGO.index ) {
                imgo_addr = deqBuf.mvOut[i].mBuffer;
                break;
            }
        }
        if (highQualityCapture_get() == P1_HQC_STATE_DEQ &&
            highQualityCapture_getAddr() == imgo_addr
            /*highQualityCapture_check(deqBuf.mvOut[0].mMetaData.mMagicNum_hal)*/) {
            onReprocessRawFrame(pFrame, deqBuf, result3A);
        }
        else
#endif
        if(OK == lockMetadata(pFrame.appFrame, mInAppMeta->getStreamId(), inAPP)){
            if (isEISOn(&inAPP) || is3DNROn(&inAPP)) {
                onProcessEIS(pFrame, resultEIS, deqBuf);
            }
        }
        if (mInImage_opaque != NULL) {
            onRedoFrame(pFrame.magicNum, MFALSE, MTRUE);
        }
        if (mInImage_yuv != NULL) {
            onYuvFrame(pFrame.magicNum, MFALSE, MTRUE);
        }
        onReturnProcessingFrame(pFrame, deqBuf, result3A, resultEIS);
        mlastProcessedNum = pFrame.magicNum;
        ret = OK;
    }

    {
        Mutex::Autolock _l(mReprocessLock);
        mReprocessGoing = MFALSE;
        mReprocessCond.broadcast();
    }

    if (mInImage_opaque != NULL) {
        Mutex::Autolock _l(mRedoQueueLock);
        if (!mRedoQueue.empty()) {
            Mutex::Autolock _ll(mTransferLock);
            if (mRequestQueue.empty() && mProcessingQueue.empty()) {
                onRedoFrame(0, MFALSE, MFALSE);
            }
        }
    }

    if (mInImage_yuv != NULL) {
        Mutex::Autolock _l(mYuvQueueLock);
        if (!mYuvQueue.empty()) {
            Mutex::Autolock _ll(mTransferLock);
            if (mRequestQueue.empty() && mProcessingQueue.empty()) {
                onYuvFrame(0, MFALSE, MFALSE);
            }
        }
    }
    FUNCTION_OUT

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
onHandleFlush(
    MBOOL wait
)
{
    FUNCTION_IN
    CAM_TRACE_CALL();

    //wake up queue thread.
    {
        Mutex::Autolock _l(mControls3AListLock);
        mControls3AListCond.broadcast();
    }

    //stop hardware
    if (!wait) {
        hardwareOps_stop(); //include hardware and 3A
    }

    //(1) clear request queue
    {
        Mutex::Autolock _l(mRequestQueueLock);
        while(!mRequestQueue.empty()) {
            QueNode_T pFrame = *mRequestQueue.begin();
            mRequestQueue.erase(mRequestQueue.begin());
            onFlushRequestFrame(pFrame);
        }
    }

    //(2) clear processing queue
    //     wait until processing frame coming out
    if (wait) {
        Mutex::Autolock _l(mProcessingQueueLock);
        while(!mProcessingQueue.empty()) {
            mProcessingQueueCond.wait(mProcessingQueueLock);
        }
    } else {
        // must guarantee hardware has been stopped.
        Mutex::Autolock _l(mProcessingQueueLock);
        while(!mProcessingQueue.empty()) {
            QueNode_T pFrame = *mProcessingQueue.begin();
            mProcessingQueue.erase(mProcessingQueue.begin());
            onFlushProcessingFrame(pFrame);
        }
    }

    //(3) clear dummy queue
    while(!mdummyQueue.empty()) {
        QueNode_T pFrame = *mdummyQueue.begin();
        onFlushProcessingFrame(pFrame);
        mdummyQueue.erase(mdummyQueue.begin());
        //
        hwDummyBuffer_return(mDummyBufferFull);
        hwDummyBuffer_return(mDummyBufferResizer);
    }


    //(4) clear drop frame queue
    onProcessDropFrame();
    if (mInImage_opaque != NULL) {
        Mutex::Autolock _lRedo(mRedoQueueLock);
        onRedoFrame(0, true, false);
        mRedoQueue.clear();
    }

    if (mInImage_yuv != NULL) {
        Mutex::Autolock _l(mYuvQueueLock);
        onYuvFrame(0, MTRUE, MFALSE);
        mYuvQueue.clear();
    }
    //(5) clear all
    mRequestQueue.clear(); //suppose already clear
    mProcessingQueue.clear(); //suppose already clear
    mControls3AList.clear();
    mImageStorage.uninit();
    m3AStorage.clear();
    mlastNum = 1;

    FUNCTION_OUT
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
onReturnProcessingFrame(
    QueNode_T const &pFrame,
    QBufInfo const &deqBuf,
    MetaSet_T const &result3A,
    IMetadata const &resultEIS
)
{
    FUNCTION_IN

    CAM_TRACE_CALL();


    if (pFrame.appFrame != 0) {

        // Out Image Stream
        if (pFrame.reqOut & REQ_OUT_RESIZER) {
            if (mOutImage_resizer != NULL) {
                returnLockedImageBuffer(pFrame.appFrame,
                        mOutImage_resizer->getStreamId(), pFrame.img_resizer, true);
            }
        }
        if (pFrame.reqOut & REQ_OUT_FULL) {
            for(size_t i = 0; i < mvOutImage_full.size(); i++) {
                returnLockedImageBuffer(pFrame.appFrame,
                        mvOutImage_full[i]->getStreamId(), pFrame.img_full, true);
            }
        }

        // APP in Meta Stream
        if (mInAppMeta != NULL) {
            StreamId_T const streamId_InAppMeta = mInAppMeta->getStreamId();
            returnLockedMetadata(pFrame.appFrame, streamId_InAppMeta);
        }

        // APP out Meta Stream
        if (mOutAppMeta != NULL){
            IMetadata appMetadata;
            StreamId_T const streamId_OutAppMeta = mOutAppMeta->getStreamId();
            generateAppMeta(pFrame.appFrame, result3A, deqBuf, appMetadata);
            if ((pFrame.reqOut & REQ_OUT_OPAQUE) && (pFrame.img_opaque != NULL)) {
                sp<IImageBufferHeap> pImageBufferHeap =
                                        pFrame.img_opaque->getImageBufferHeap();
                MERROR status = OpaqueReprocUtil::setAppMetadataToHeap(
                    pImageBufferHeap,
                    appMetadata);
            }
            m3AStorage.add(pFrame.magicNum, pFrame.appFrame, appMetadata);
            appMetadata = m3AStorage.valueFor(pFrame.magicNum).resultVal;
            if(m3AStorage.isCompleted(pFrame.magicNum)) {
                lock_and_returnMetadata(pFrame.appFrame, streamId_OutAppMeta, appMetadata);
                m3AStorage.removeItem(pFrame.magicNum);
            }
        }

        // HAL out Meta Stream
        if (mOutHalMeta != NULL){
            IMetadata inHalMetadata;
            IMetadata outHalMetadata;
            StreamId_T const streamId_OutHalMeta = mOutHalMeta->getStreamId();

            if (mInHalMeta != NULL) lockMetadata(pFrame.appFrame, mInHalMeta->getStreamId(), inHalMetadata);
            generateHalMeta(result3A, deqBuf, resultEIS, inHalMetadata, outHalMetadata);
            if ((pFrame.reqOut & REQ_OUT_OPAQUE) && (pFrame.img_opaque != NULL)) {
                sp<IImageBufferHeap> pImageBufferHeap =
                                        pFrame.img_opaque->getImageBufferHeap();
                MERROR status = OpaqueReprocUtil::setHalMetadataToHeap(
                    pImageBufferHeap,
                    outHalMetadata);
            }
            lock_and_returnMetadata(pFrame.appFrame, streamId_OutHalMeta, outHalMetadata);
        }

        // HAL in Meta Stream
        if (mInHalMeta != NULL) {
            StreamId_T const streamId_InHalMeta = mInHalMeta->getStreamId();
            returnLockedMetadata(pFrame.appFrame, streamId_InHalMeta);
        }
        if ((pFrame.reqOut & REQ_OUT_OPAQUE) && (mOutImage_opaque != NULL)) {
            returnLockedImageBuffer(pFrame.appFrame,
                                                mOutImage_opaque->getStreamId(),
                                                pFrame.img_opaque,
                                                true);
        }
        // Apply buffers to release
        IStreamBufferSet& rStreamBufferSet  = pFrame.appFrame->getStreamBufferSet();
        rStreamBufferSet.applyRelease(getNodeId());

        // dispatch to next node
        onDispatchFrame(pFrame.appFrame);
        MY_LOGD1("[return OK]: (%d, %d)", pFrame.appFrame->getFrameNo(), pFrame.magicNum);
        android_atomic_dec(&mInFlightRequestCnt);
        ATRACE_INT("P1_request_cnt", android_atomic_acquire_load(&mInFlightRequestCnt));

    }
    else {
        if (pFrame.halFrame_full != 0) {
            returnLockedImageBuffer(pFrame.img_full, pFrame.halFrame_full, mpStreamPool_full);
        } else if (pFrame.img_full != 0) {
            pFrame.img_full->unlockBuf(getNodeName());
            hwDummyBuffer_return(mDummyBufferFull);
            MY_LOGD("per-enq buffer (%d) unlockBuf - full", pFrame.magicNum);
        }
        if (pFrame.halFrame_resizer != 0) {
            returnLockedImageBuffer(pFrame.img_resizer, pFrame.halFrame_resizer, mpStreamPool_resizer);
        } else if (pFrame.img_resizer != 0) {
            pFrame.img_resizer->unlockBuf(getNodeName());
            hwDummyBuffer_return(mDummyBufferResizer);
            MY_LOGD("per-enq buffer (%d) unlockBuf - resizer", pFrame.magicNum);
        }
    }

    FUNCTION_OUT
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
onFlushRequestFrame(
    QueNode_T const& pFrame
)
{
    FUNCTION_IN
    CAM_TRACE_CALL();

    if (pFrame.appFrame != 0) {

        // Out Image Stream
        Vector<StreamId_T> streamId_Images;
        if (pFrame.reqOut & REQ_OUT_RESIZER) {
            if (mOutImage_resizer != NULL) {
                streamId_Images.push_back(mOutImage_resizer->getStreamId());
            }
        }
        if (pFrame.reqOut & REQ_OUT_FULL) {
            for(size_t i = 0; i < mvOutImage_full.size(); i++) {
                streamId_Images.push_back(mvOutImage_full[i]->getStreamId());
            }
        }
        for(size_t i = 0; i < streamId_Images.size(); i++) {
            StreamId_T const streamId_Image = streamId_Images[i];
            returnUnlockedImageBuffer(pFrame.appFrame, streamId_Image);
        }


        // APP in Meta Stream
        if (mInAppMeta != NULL) {
            StreamId_T const streamId_InAppMeta = mInAppMeta->getStreamId();
            returnUnlockedMetadata(pFrame.appFrame, streamId_InAppMeta);
        }

        // HAL in Meta Stream
        if (mInHalMeta != NULL) {
            StreamId_T const streamId_InHalMeta = mInHalMeta->getStreamId();
            returnUnlockedMetadata(pFrame.appFrame, streamId_InHalMeta);
        }

        // APP out Meta Stream
        if (mOutAppMeta != NULL) {
            StreamId_T const streamId_OutAppMeta = mOutAppMeta->getStreamId();
            returnUnlockedMetadata(pFrame.appFrame, streamId_OutAppMeta);
        }

        // HAL out Meta Stream
        if (mOutHalMeta != NULL) {
            StreamId_T const streamId_OutHalMeta = mOutHalMeta->getStreamId();
            returnUnlockedMetadata(pFrame.appFrame, streamId_OutHalMeta);
        }
        if ((pFrame.reqOut & REQ_OUT_OPAQUE) && (mOutImage_opaque != NULL)) {
            returnUnlockedImageBuffer(pFrame.appFrame,
                                                mOutImage_opaque->getStreamId());
        }
        if (mInImage_yuv != NULL) {
            returnUnlockedImageBuffer(pFrame.appFrame,
                                                mInImage_yuv->getStreamId());
        }
        // Apply buffers to release.
        IStreamBufferSet& rStreamBufferSet  = pFrame.appFrame->getStreamBufferSet();
        rStreamBufferSet.applyRelease(getNodeId());

        // dispatch to next node
        onDispatchFrame(pFrame.appFrame);
        MY_LOGD1("[return flush]: (%d, %d)", pFrame.appFrame->getFrameNo(), pFrame.magicNum);
        android_atomic_dec(&mInFlightRequestCnt);
        ATRACE_INT("P1_request_cnt", android_atomic_acquire_load(&mInFlightRequestCnt));
    }
    else {
        if (pFrame.halFrame_full != 0) {
            returnUnlockedImageBuffer(pFrame.halFrame_full, mpStreamPool_full);
        }
        if (pFrame.halFrame_resizer != 0) {
            returnUnlockedImageBuffer(pFrame.halFrame_resizer, mpStreamPool_resizer);
        }
    }


    FUNCTION_OUT
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
onFlushProcessingFrame(
    QueNode_T const& pFrame
)
{
    FUNCTION_IN
    CAM_TRACE_CALL();

    if (pFrame.appFrame != 0) {
        MBOOL success = MFALSE;

        // Out Image Stream
        if (pFrame.reqOut & REQ_OUT_RESIZER) {
            if (mOutImage_resizer != NULL) {
                returnLockedImageBuffer(pFrame.appFrame,
                        mOutImage_resizer->getStreamId(), pFrame.img_resizer, success);
            }
        }
        if (pFrame.reqOut & REQ_OUT_FULL) {
            for(size_t i = 0; i < mvOutImage_full.size(); i++) {
                returnLockedImageBuffer(pFrame.appFrame,
                        mvOutImage_full[i]->getStreamId(), pFrame.img_full, success);
            }
        }

        // APP in Meta Stream
        if (mInAppMeta != NULL) {
            StreamId_T const streamId_InAppMeta = mInAppMeta->getStreamId();
            returnLockedMetadata(pFrame.appFrame, streamId_InAppMeta, success);
        }

        // HAL in Meta Stream
        if (mInHalMeta != NULL) {
            StreamId_T const streamId_InHalMeta = mInHalMeta->getStreamId();
            returnLockedMetadata(pFrame.appFrame, streamId_InHalMeta, success);
        }

        // APP out Meta Stream
        if (mOutAppMeta != NULL) {
            StreamId_T const streamId_OutAppMeta = mOutAppMeta->getStreamId();
            returnLockedMetadata(pFrame.appFrame, streamId_OutAppMeta, success);
        }

        // HAL out Meta Stream
        if (mOutHalMeta != NULL) {
            StreamId_T const streamId_OutHalMeta  = mOutHalMeta->getStreamId();
            returnLockedMetadata(pFrame.appFrame, streamId_OutHalMeta, success);
        }
        if ((pFrame.reqOut & REQ_OUT_OPAQUE) && (mOutImage_opaque != NULL)) {
            returnLockedImageBuffer(pFrame.appFrame,
                                                mOutImage_opaque->getStreamId(),
                                                pFrame.img_opaque,
                                                success);
        }
        // Apply buffers to release
        IStreamBufferSet& rStreamBufferSet  = pFrame.appFrame->getStreamBufferSet();
        rStreamBufferSet.applyRelease(getNodeId());

        // dispatch to next node
        onDispatchFrame(pFrame.appFrame);
        MY_LOGD1("[return flush]: (%d, %d)", pFrame.appFrame->getFrameNo(), pFrame.magicNum);
        android_atomic_dec(&mInFlightRequestCnt);
        ATRACE_INT("P1_request_cnt", android_atomic_acquire_load(&mInFlightRequestCnt));
    }
    else {
        if (pFrame.halFrame_full != 0) {
            returnLockedImageBuffer(pFrame.img_full, pFrame.halFrame_full, mpStreamPool_full);
        }
        else if ((mDummyBufferFull != NULL) && (pFrame.img_full != 0)) {
            if (pFrame.img_full.get() ==
                (*mdummyQueue.begin()).img_full.get()) {
                pFrame.img_full->unlockBuf(getNodeName());
                //MY_LOGD("dummy buffer unlockBuf - full");
            }
        }
        if (pFrame.halFrame_resizer != 0) {
            returnLockedImageBuffer(pFrame.img_resizer, pFrame.halFrame_resizer, mpStreamPool_resizer);
        }
        else if ((mDummyBufferResizer != NULL) && (pFrame.img_resizer != 0)) {
            if (pFrame.img_resizer.get() ==
                (*mdummyQueue.begin()).img_resizer.get()) {
                pFrame.img_resizer->unlockBuf(getNodeName());
                //MY_LOGD("dummy buffer unlockBuf - resizer");
            }
        }
    }

    FUNCTION_OUT
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
onRedoFrame(
    MUINT32 currentNum,
    MBOOL onFlush,
    MBOOL onLock
)
{
    FUNCTION_IN;
    Mutex::Autolock _l(mReprocLock);

    Vector<QueNode_T> vRedoQ;
    {
        if (onLock) {
            mRedoQueueLock.lock();
        }

        Que_T::iterator it = mRedoQueue.begin();
        for(; it != mRedoQueue.end(); ) {
            if (currentNum == 0 || (*it).magicNum <= currentNum) {
                vRedoQ.push_back(*it);
                it = mRedoQueue.erase(it);
            }
            else {
                ++it;
            }
        }

        if (onLock) {
            mRedoQueueLock.unlock();
        }
    }

    if (vRedoQ.size() != 0 && mInImage_opaque == NULL) {
        MY_LOGE("Can not find in-opaque buffer info");
        return;
    }

    for (size_t i = 0; i < vRedoQ.size(); i++) {
        QueNode_T& pFrame = vRedoQ.editItemAt(i);
        MBOOL success = (onFlush) ? false : true;
        MERROR outAppMetaSuccess = false;
        MERROR outHalMetaSuccess = false;
        IMetadata appMeta;
        IMetadata halMeta;

        if (onFlush) {
            returnUnlockedImageBuffer(pFrame.appFrame, mInImage_opaque->getStreamId());
        } else {
            sp<IImageStreamBuffer>  pOutpImageStreamBuffer;
            sp<IImageBuffer> imageBuffer;
            if (OK != lockImageBuffer(pFrame.appFrame,
                                    mInImage_opaque->getStreamId(),
                                    pOutpImageStreamBuffer, imageBuffer)) {
                MY_LOGE("Can not lock in-opaque buffer from frame");
            } else {
                sp<IImageBufferHeap> pHeap = imageBuffer->getImageBufferHeap();
                outAppMetaSuccess = OpaqueReprocUtil::getAppMetadataFromHeap(
                                        pHeap,
                                        appMeta);
                outHalMetaSuccess = OpaqueReprocUtil::getHalMetadataFromHeap(
                                        pHeap,
                                        halMeta);

                IMetadata::IEntry entry(MTK_HAL_REQUEST_REQUIRE_EXIF);
                entry.push_back(1, Type2Type<MUINT8>());
                halMeta.update(entry.tag(), entry);
            }
            //
            returnLockedImageBuffer(pFrame.appFrame,
                                                mInImage_opaque->getStreamId(),
                                                pFrame.img_opaque,
                                                success);
        }

        // APP out Meta Stream
        if (mOutAppMeta != NULL) {
            StreamId_T const streamId_OutAppMeta = mOutAppMeta->getStreamId();
            if (outAppMetaSuccess == OK) {
                lock_and_returnMetadata(pFrame.appFrame,
                                                streamId_OutAppMeta, appMeta);
            } else {
                returnLockedMetadata(pFrame.appFrame,
                                                streamId_OutAppMeta, false);
            }
        }

        // HAL out Meta Stream
        if (mOutHalMeta != NULL) {
            StreamId_T const streamId_OutHalMeta  = mOutHalMeta->getStreamId();
            if (outAppMetaSuccess == OK) {
                lock_and_returnMetadata(pFrame.appFrame,
                                                streamId_OutHalMeta, halMeta);
            } else {
                returnLockedMetadata(pFrame.appFrame,
                                                streamId_OutHalMeta, false);
            }
        }

        // APP in Meta Stream
        if (mInAppMeta != NULL) {
            StreamId_T const streamId_InAppMeta = mInAppMeta->getStreamId();
            returnLockedMetadata(pFrame.appFrame, streamId_InAppMeta, success);
        }

        // HAL in Meta Stream
        if (mInHalMeta != NULL) {
            StreamId_T const streamId_InHalMeta = mInHalMeta->getStreamId();
            returnLockedMetadata(pFrame.appFrame, streamId_InHalMeta, success);
        }

        // Apply buffers to release
        IStreamBufferSet& rStreamBufferSet =
                                        pFrame.appFrame->getStreamBufferSet();
        rStreamBufferSet.applyRelease(getNodeId());

        // dispatch to next node
        onDispatchFrame(pFrame.appFrame);
    }

    FUNCTION_OUT;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
onYuvFrame(
    MUINT32 currentNum,
    MBOOL onFlush,
    MBOOL onLock
)
{
    FUNCTION_IN;

    if (mInImage_yuv == NULL) {
        return;
    }

    Vector<QueNode_T> vYuvQ;
    {
        if (onLock) {
            mYuvQueueLock.lock();
        }

        Vector<QueNode_T>::iterator it = mYuvQueue.begin();
        for(; it != mYuvQueue.end(); ) {
            if (currentNum == 0 || (*it).magicNum <= currentNum) {
                vYuvQ.push_back(*it);
                it = mYuvQueue.erase(it);
            }
            else {
                ++it;
            }
        }

        if (onLock) {
            mYuvQueueLock.unlock();
        }
    }

    if (vYuvQ.size() != 0 && mInImage_yuv == NULL) {
        MY_LOGE("Can not find in-yuv buffer info");
        return;
    }

    for (size_t i = 0; i < vYuvQ.size(); i++) {
        QueNode_T& node = vYuvQ.editItemAt(i);
        if (onFlush) {
            onFlushRequestFrame(node);
        } else {
            IMetadata inAppMetadata;
            IMetadata outAppMetadata;
            IMetadata inHalMetadata;
            IMetadata outHalMetadata;
            MINT64 timestamp = 0;
            // In Image Stream
            if (mInImage_yuv != NULL) {
                returnUnlockedImageBuffer(node.appFrame, mInImage_yuv->getStreamId());
            }
            // APP in Meta Stream
            if (mInAppMeta != NULL) {
                StreamId_T const streamId_InAppMeta = mInAppMeta->getStreamId();
                if (OK != lockMetadata(node.appFrame, mInAppMeta->getStreamId(), inAppMetadata)) {
                    MY_LOGW("cannot lock in-app-metadata");
                    returnUnlockedMetadata(node.appFrame, streamId_InAppMeta);
                } else {
                    if (tryGetMetadata< MINT64 >(
                        &inAppMetadata, MTK_SENSOR_TIMESTAMP, timestamp)) {
                        MY_LOGD1("timestamp from in-app %lld", timestamp);
                    } else {
                        MY_LOGI("cannot find timestamp from in-app");
                        timestamp = 0;
                    }
                    returnLockedMetadata(node.appFrame, streamId_InAppMeta);
                }
            }
            // APP out Meta Stream
            if (mOutAppMeta != NULL) {
                StreamId_T const streamId_OutAppMeta = mOutAppMeta->getStreamId();
                if (trySetMetadata< MINT64 > (
                    &outAppMetadata, MTK_SENSOR_TIMESTAMP, timestamp)) {
                    if (OK != lock_and_returnMetadata(node.appFrame, streamId_OutAppMeta, outAppMetadata)) {
                        MY_LOGW("cannot lock out-app-metadata");
                        returnUnlockedMetadata(node.appFrame, streamId_OutAppMeta);
                    }
                } else {
                    MY_LOGW("cannot set timestamp to out-app");
                    returnUnlockedMetadata(node.appFrame, streamId_OutAppMeta);
                }
            }
            // HAL in Meta Stream
            if (mInHalMeta != NULL) {
                StreamId_T const streamId_InHalMeta = mInHalMeta->getStreamId();
                if (OK != lockMetadata(node.appFrame, mInHalMeta->getStreamId(), inHalMetadata)) {
                    MY_LOGW("cannot lock in-hal-metadata");
                    returnUnlockedMetadata(node.appFrame, streamId_InHalMeta);
                } else {
                    returnLockedMetadata(node.appFrame, streamId_InHalMeta);
                }
            }
            // HAL out Meta Stream
            if (mOutHalMeta != NULL){
                StreamId_T const streamId_OutHalMeta = mOutHalMeta->getStreamId();
                outHalMetadata = inHalMetadata;
                if (!trySetMetadata< MINT32 > (
                    &outHalMetadata, MTK_P1NODE_SENSOR_MODE, mSensorParams.mode)) {
                    MY_LOGW("cannot update MTK_P1NODE_SENSOR_MODE to out-hal-metadata");
                }
                if (!trySetMetadata< MRect > (
                    &outHalMetadata, MTK_P1NODE_SCALAR_CROP_REGION, MRect(mSensorParams.size.w, mSensorParams.size.h))) {
                    MY_LOGW("cannot update MTK_P1NODE_SCALAR_CROP_REGION to out-hal-metadata");
                }
                if (!trySetMetadata< MRect > (
                    &outHalMetadata, MTK_P1NODE_DMA_CROP_REGION, MRect(mSensorParams.size.w, mSensorParams.size.h))) {
                    MY_LOGW("cannot update MTK_P1NODE_DMA_CROP_REGION to out-hal-metadata");
                }
                if (!trySetMetadata< MSize > (
                    &outHalMetadata, MTK_P1NODE_RESIZER_SIZE, mSensorParams.size)) {
                    MY_LOGW("cannot update MTK_P1NODE_RESIZER_SIZE to out-hal-metadata");
                }
                if (OK != lock_and_returnMetadata(node.appFrame, streamId_OutHalMeta, outHalMetadata)) {
                    MY_LOGW("cannot lock out-hal-metadata");
                    returnUnlockedMetadata(node.appFrame, streamId_OutHalMeta);
                }
            }
            // Apply buffers to release
            IStreamBufferSet& rStreamBufferSet =
                                            node.appFrame->getStreamBufferSet();
            rStreamBufferSet.applyRelease(getNodeId());
            // dispatch to next node
            onDispatchFrame(node.appFrame);
            MY_LOGD1("[Return %s ZslYUV]:(%d, %d, %d) indicate(%d) remain(%d)",
                (onFlush) ? "Flush" : "OK",node.magicNum,
                node.appFrame->getFrameNo(), node.appFrame->getRequestNo(),
                currentNum, mYuvQueue.size());
            android_atomic_dec(&mInFlightRequestCnt);
            ATRACE_INT("P1_request_cnt",
                android_atomic_acquire_load(&mInFlightRequestCnt));
        }
    }

    FUNCTION_OUT;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
onProcess3AResult(
    MUINT32 magicNum,
    MUINT32 key,
    MUINT32 val
)
{
    MY_LOGD2("%d", magicNum);

    if(magicNum == 0) return;

    m3AStorage.add(magicNum, key, val);
    if(m3AStorage.isCompleted(magicNum)) {
        sp<IPipelineFrame> spFrame = m3AStorage.valueFor(magicNum).spFrame;
        StreamId_T const streamId_OutAppMeta = mOutAppMeta->getStreamId();
        IMetadata appMetadata = m3AStorage.valueFor(magicNum).resultVal;
        lock_and_returnMetadata(spFrame, streamId_OutAppMeta, appMetadata);
        m3AStorage.removeItem(magicNum);

        IStreamBufferSet& rStreamBufferSet  = spFrame->getStreamBufferSet();
        rStreamBufferSet.applyRelease(getNodeId());
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
onProcessEIS(
    QueNode_T const &pFrame,
    IMetadata &resultEIS,
    QBufInfo const &deqBuf
)
{
    if(pFrame.appFrame ==NULL) {
        return;
    }
    #if SUPPORT_EIS
    if (deqBuf.mvOut.size() == 0) {
        MY_LOGE("deqBuf is empty");
        return;
    }
    MUINT64 timestamp = deqBuf.mvOut[0].mMetaData.mTimeStamp;
    EIS_HAL_CONFIG_DATA config;
    config.p1ImgW = mSensorParams.size.w; // [AWARE] need to revise by platform
    config.p1ImgH = mSensorParams.size.h; //
    #if 1 // use RRZO DstSize
    for (size_t i = 0; i < deqBuf.mvOut.size(); i++) {
        if (deqBuf.mvOut[i].mPortID == PORT_RRZO) {
            config.p1ImgW = deqBuf.mvOut[i].mMetaData.mDstSize.w;
            config.p1ImgH = deqBuf.mvOut[i].mMetaData.mDstSize.h;
            break;
        } else {
            continue;
        }
    }
    #endif
    mpEIS->DoEis(EIS_PASS_1, &config, timestamp);

    MUINT32 X_INT, Y_INT, X_FLOAT, Y_FLOAT, WIDTH, HEIGHT;
    mpEIS->GetEisResult(X_INT, X_FLOAT, Y_INT, Y_FLOAT, WIDTH, HEIGHT);
    MINT32 GMV_X, GMV_Y;
    mpEIS->GetEisGmv(GMV_X, GMV_Y);
    IMetadata::IEntry entry(MTK_EIS_REGION);
    entry.push_back(X_INT, Type2Type< MINT32 >());
    entry.push_back(X_FLOAT, Type2Type< MINT32 >());
    entry.push_back(Y_INT, Type2Type< MINT32 >());
    entry.push_back(Y_FLOAT, Type2Type< MINT32 >());
    entry.push_back(WIDTH, Type2Type< MINT32 >());
    entry.push_back(HEIGHT, Type2Type< MINT32 >());
    entry.push_back(GMV_X, Type2Type< MINT32 >());
    entry.push_back(GMV_Y, Type2Type< MINT32 >());
    resultEIS.update(MTK_EIS_REGION, entry);
    MY_LOGD1("(%dx%d) %d, %d, %d, %d, %d, %d, %d, %d",
        config.p1ImgW, config.p1ImgH,
        X_INT, X_FLOAT, Y_INT, Y_FLOAT, WIDTH, HEIGHT, GMV_X, GMV_Y);
    #endif
}


/******************************************************************************
 *
 ******************************************************************************/
void
P1NodeImp::
doNotifyCb(
    MINT32  _msgType,
    MINTPTR _ext1,
    MINTPTR _ext2,
    MINTPTR /*_ext3*/
)
{
    switch(_msgType)
    {
        case IHal3ACb::eID_NOTIFY_3APROC_FINISH:
            onProcessFrame(MFALSE, (MUINT32)_ext1, (MUINT32)_ext2);
            break;
        case IHal3ACb::eID_NOTIFY_CURR_RESULT:
            //onProcess3AResult((MUINT32)_ext1,(MUINT32)_ext2, (MUINT32)_ext3); //magic, key, val
            break;
        default:
            break;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
void
P1NodeImp::
doNotifyDropframe(MUINT magicNum, void* cookie)
{
   if (cookie == NULL) {
       MY_LOGE("return cookie is NULL");
       return;
   }

   Mutex::Autolock _l(reinterpret_cast<P1NodeImp*>(cookie)->mDropQueueLock);
   reinterpret_cast<P1NodeImp*>(cookie)->mDropQueue.push_back(magicNum);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
DummyBuffer::
init(sp<IImageStreamInfo> const& imgStreamInfo, int numHardwareBuffer)
{
    FUNCTION_IN;
    MERROR err = OK;
    MUINT32 imgStreamSize = 0;
    //
    if (imgStreamInfo == NULL) {
        MY_LOGE("imgStreamInfo is NULL");
        return BAD_VALUE;
    }
    for (size_t i = 0; i < imgStreamInfo->getBufPlanes().size(); i++) {
        imgStreamSize += imgStreamInfo->getBufPlanes()[i].sizeInBytes;
    }
    return init(imgStreamSize, numHardwareBuffer);
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
DummyBuffer::
init(MUINT32 const imgStreamSize, int numHardwareBuffer)
{
    FUNCTION_IN;
    MERROR err = OK;
    //
    if (imgStreamSize == 0) {
        MY_LOGE("Image Stream Size is 0");
        return BAD_VALUE;
    }
    mSize = imgStreamSize;
    mSlot = numHardwareBuffer;
    mTotal = mSize + (sizeof(MINTPTR) * (mSlot - 1));
    IImageBufferAllocator::ImgParam imgParam(mTotal, 0);
    sp<IIonImageBufferHeap> pHeap =
        IIonImageBufferHeap::create(mName.string(), imgParam);
    if (pHeap == NULL) {
        MY_LOGE("[%s] image buffer heap create fail", mName.string());
        return BAD_VALUE;
    }
    MUINT const usage = (GRALLOC_USAGE_SW_READ_OFTEN |
                        GRALLOC_USAGE_HW_CAMERA_READ |
                        GRALLOC_USAGE_HW_CAMERA_WRITE);
    sp<IImageBuffer> pImageBuffer = pHeap->createImageBuffer();
    if (pImageBuffer == NULL) {
        MY_LOGE("[%s] image buffer create fail", mName.string());
        return BAD_VALUE;
    }
    if (!(pImageBuffer->lockBuf(mName.string(), usage))) {
        MY_LOGE("[%s] image buffer lock fail", mName.string());
        return BAD_VALUE;
    }
    mVAddr = pHeap->getBufVA(0);
    mPAddr = pHeap->getBufPA(0);
    mHeapId = pHeap->getHeapID();
    mDummyBuffer = pImageBuffer;
    mUsingCount = 0;
    MY_LOGI("DummyBufferHeap[%s] Len(%d, %d, %d) VA(0x%zx) PA(0x%zx) ID(%d)",
        mName.string(), mSize, mSlot, mTotal, mVAddr, mPAddr, mHeapId);
    //
    FUNCTION_OUT;
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
DummyBuffer::
uninit(void)
{
    FUNCTION_IN;
    MERROR err = OK;
    //
    if (mUsingCount > 0) {
        MY_LOGE("dummy buffer [%s] is using (%d) - should not uninit",
            mName.string(), mUsingCount);
        return BAD_VALUE;
    }
    if (mDummyBuffer != NULL) {
        mDummyBuffer->unlockBuf(mName.string());
        mDummyBuffer = NULL;
        mVAddr = 0;
        mPAddr = 0;
        mSize = 0;
        mSlot = 0;
        mTotal = 0;
        mUsingCount = 0;
    }
    //
    FUNCTION_OUT;
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hwDummyBuffer_init(void)
{
    FUNCTION_IN;
    MERROR err = OK;
    //
    #if DISABLE_BLOB_DUMMY_BUF
    return err;
    #endif
    //
    if ((mDummyBufferFull != NULL) || (mDummyBufferResizer != NULL)) {
        err = hwDummyBuffer_uninit();
        if (err != OK) {
            MY_LOGE("init - release dummy buffer fail");
            return err;
        }
    }
    //
    if ((mvOutImage_full.size() != 0) || (mOutImage_opaque != NULL)) {
        sp<DummyBuffer> pDummyBuffer =
            new DummyBuffer("Hal:Image:P1:Fulldummy", mLogLevel);
        int num = mNumHardwareBuffer +
                ((mpStreamPool_full == NULL) ? (mNumHardwareBuffer) : (0));
        err = pDummyBuffer->init(mvOutImage_full[0], num);
        if (err != OK) {
            MY_LOGE("init - full dummy buffer fail");
            return err;
        }
        mDummyBufferFull = pDummyBuffer;
    }
    //
    if (mOutImage_resizer != NULL) {
        sp<DummyBuffer> pDummyBuffer =
            new DummyBuffer("Hal:Image:P1:Resizedummy", mLogLevel);
        int num = mNumHardwareBuffer +
                ((mpStreamPool_resizer == NULL) ? (mNumHardwareBuffer) : (0));
        err = pDummyBuffer->init(mOutImage_resizer, num);
        if (err != OK) {
            MY_LOGE("init - resizer dummy buffer fail");
            return err;
        }
        mDummyBufferResizer = pDummyBuffer;
    }
    //
    FUNCTION_OUT
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hwDummyBuffer_acquire(
    sp<DummyBuffer> const& dummyBuffer,
    char const* szName,
    sp<IImageBuffer> & imageBuffer
)
{
    FUNCTION_IN;
    MERROR err = OK;
    MUINT const usage = (GRALLOC_USAGE_SW_READ_OFTEN |
                        GRALLOC_USAGE_HW_CAMERA_READ |
                        GRALLOC_USAGE_HW_CAMERA_WRITE);
    //
    if (dummyBuffer == NULL) {
        MY_LOGE("acquire [%s] - dummy buffer NULL", szName);
        return BAD_VALUE;
    }
    //
    MINT32 index = dummyBuffer->query();
    if ((MUINT32)index >= dummyBuffer->mSlot || index < 0) {
        MY_LOGE("acquire [%s] - dummy buffer index out of range (%d >= %d)",
            szName, index, dummyBuffer->mSlot);
        return BAD_VALUE;
    }
    MY_LOGD1("acquire [%s] - index(%d/%d)", szName, index, dummyBuffer->mSlot);
    // use eImgFmt_BLOB for dummy buffer, plane count is 1
    size_t bufStridesInBytes[] = {dummyBuffer->mSize, 0, 0};
    size_t bufBoundaryInBytes[] = {0, 0, 0};
    MUINTPTR const cVAddr = dummyBuffer->mVAddr + (sizeof(MINTPTR) * (index));
    MUINTPTR const virtAddr[] = {cVAddr, 0, 0};
    MUINTPTR const cPAddr = dummyBuffer->mPAddr + (sizeof(MINTPTR) * (index));
    MUINTPTR const phyAddr[] = {cPAddr, 0, 0};
    IImageBufferAllocator::ImgParam imgParam =
                        IImageBufferAllocator::ImgParam(dummyBuffer->mSize, 0);
    MINT32 heapId[] = {dummyBuffer->mHeapId, 0, 0};
    PortBufInfo_dummy portBufInfo = PortBufInfo_dummy(
                                        heapId[0],
                                        virtAddr,
                                        phyAddr,
                                        1);
    sp<IImageBufferHeap> imgBufHeap = IDummyImageBufferHeap::create(
                                        szName,
                                        imgParam,
                                        portBufInfo,
                                        (1 <= mLogLevel) ? true : false);
    if (imgBufHeap == NULL) {
        MY_LOGE("acquire [%s] - image buffer heap create fail", szName);
        return UNKNOWN_ERROR;
    }
    sp<IImageBuffer> imgBuf = imgBufHeap->createImageBuffer();
    if (imgBuf == NULL) {
        MY_LOGE("acquire [%s] - image buffer create fail", szName);
        return BAD_VALUE;
    }
    if (!(imgBuf->lockBuf(szName, usage))) {
        MY_LOGE("acquire [%s] - image buffer lock fail", szName);
        return BAD_VALUE;
    }
    imageBuffer = imgBuf;
    dummyBuffer->count(true);
    //
    FUNCTION_OUT;
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hwDummyBuffer_return(sp<DummyBuffer>& dummyBuffer)
{
    FUNCTION_IN;
    MERROR err = OK;
    //
    if (dummyBuffer != NULL) {
        err = dummyBuffer->count(false);
        if (err != OK) {
            MY_LOGE("%s: set count fail", dummyBuffer->mName.string());
            return err;
        }
    }
    //
    FUNCTION_OUT;
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hwDummyBuffer_release(sp<DummyBuffer>& dummyBuffer)
{
    FUNCTION_IN;
    MERROR err = OK;
    //
    if (dummyBuffer != NULL) {
        err = dummyBuffer->uninit();
        if (err != OK) {
            MY_LOGE("%s: set count fail", dummyBuffer->mName.string());
            MY_LOGE("release - full dummy buffer fail");
            return err;
        }
        dummyBuffer = NULL;
    }
    //
    FUNCTION_OUT;
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hwDummyBuffer_uninit(void)
{
    FUNCTION_IN;
    MERROR err = OK;
    //
    if (mDummyBufferFull != NULL) {
        err = hwDummyBuffer_release(mDummyBufferFull);
        if (err != OK) {
            MY_LOGE("uninit - free full dummy buffer fail");
        }
    }
    //
    if (mDummyBufferResizer != NULL) {
        err = hwDummyBuffer_release(mDummyBufferResizer);
        if (err != OK) {
            MY_LOGE("uninit - free resizer dummy buffer fail");
        }
    }
    //
    FUNCTION_OUT;
    return err;
}


/******************************************************************************
 *
 *****************************************************************************/
MERROR
P1NodeImp::
hwImageBuffer_init(sp<IImageStreamInfo> const& imgStreamInfo)
{
    MERROR err = OK;
    MBOOL isFull = true;

    {
        Mutex::Autolock _l(mImageBufferLock);
        if (imgStreamInfo == NULL) {
            MY_LOGE("imgStreamInfo is NULL");
            return BAD_VALUE;
        }
        if (imgStreamInfo->getStreamId() == mOutImage_resizer->getStreamId()) {
            isFull = false;
            if (mImageBufferResizer != NULL) {
                MY_LOGD1("ImageBufferResizer is ready (%p)",
                    mImageBufferResizer.get());
                return OK;
            }
        } else if (mImageBufferFull != NULL) {
            MY_LOGD1("ImageBufferFull is ready (%p)",
            mImageBufferFull.get());
            return OK;
        }
    }

    MUINT32 stride = imgStreamInfo->getBufPlanes()[0].rowStrideInBytes;

    android::String8 imgBufName = android::String8(
                                    imgStreamInfo->getStreamName());
    imgBufName += ":Ex";

    return hwImageBuffer_init(isFull,
                                imgStreamInfo->getImgFormat(),
                                imgStreamInfo->getImgSize(),
                                stride, imgBufName.string());
}


/******************************************************************************
 *
 *****************************************************************************/
MERROR
P1NodeImp::
hwImageBuffer_init(MBOOL isFull, MINT32 format,
                            MSize size, MUINT32 stride, char const* szName)
{
    FUNCTION_IN;
    MERROR err = OK;
    Mutex::Autolock _l(mImageBufferLock);

    if (!isFull) {
        if (mImageBufferResizer != NULL) {
            MY_LOGD1("ImageBufferResizer is ready (%p)",
                mImageBufferResizer.get());
            return OK;
        }
    } else if (mImageBufferFull != NULL) {
        MY_LOGD1("ImageBufferFull is ready (%p)",
        mImageBufferFull.get());
        return OK;
    }

    android::String8 imgBufName = android::String8(szName);
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    MUINT32 bufStridesInBytes[3] = {stride, 0, 0};
    IImageBufferAllocator::ImgParam imgParam =
        IImageBufferAllocator::ImgParam((EImageFormat)format,
            size, bufStridesInBytes, bufBoundaryInBytes, 1);
    sp<IIonImageBufferHeap> pHeap =
        IIonImageBufferHeap::create(imgBufName.string(), imgParam);
    if (pHeap == NULL) {
        MY_LOGE("[%s] ImageBufferHeap create fail", imgBufName.string());
        return BAD_VALUE;
    }
    sp<IImageBuffer> pImgBuf = pHeap->createImageBuffer();
    if (pImgBuf == NULL) {
        MY_LOGE("[%s] ImageBuffer create fail", imgBufName.string());
        return BAD_VALUE;
    }
    if (isFull) {
        mImageBufferFull = pImgBuf;
        mImageBufferNameFull = imgBufName;
        mImageBufferLockedFull = false;
    } else {
        mImageBufferResizer = pImgBuf;
        mImageBufferNameResizer = imgBufName;
        mImageBufferLockedResizer = false;
    }

    FUNCTION_OUT;
    return err;
}


/******************************************************************************
 *
 *****************************************************************************/
MERROR
P1NodeImp::
hwImageBuffer_lock(MBOOL isFull, sp<IImageBuffer> & imageBuffer)
{
    FUNCTION_IN;
    MERROR err = OK;
    Mutex::Autolock _l(mImageBufferLock);
    MUINT const usage = (GRALLOC_USAGE_SW_READ_OFTEN |
                        GRALLOC_USAGE_HW_CAMERA_READ |
                        GRALLOC_USAGE_HW_CAMERA_WRITE);
    sp<IImageBuffer> imgBuf;
    android::String8 imgBufferName;
    MBOOL* pImgBufferLocked = NULL;
    imageBuffer = NULL;
    if (isFull) {
        imgBuf = mImageBufferFull;
        imgBufferName = mImageBufferNameFull;
        pImgBufferLocked = &mImageBufferLockedFull;
    } else {
        imgBuf = mImageBufferResizer;
        imgBufferName = mImageBufferNameResizer;
        pImgBufferLocked = &mImageBufferLockedResizer;
    }
    //
    if (imgBuf == NULL) {
        MY_LOGW("[%s] ImageBuffer not init", (isFull) ? "full" : "resizer");
        return BAD_VALUE;
    }
    if ((*pImgBufferLocked) == true) {
        MY_LOGW("[%s] ImageBuffer was locked", (isFull) ? "full" : "resizer");
        return BAD_VALUE;
    }
    if (!(imgBuf->lockBuf(imgBufferName, usage))) {
        MY_LOGE("[%s] ImageBuffer lock fail", (isFull) ? "full" : "resizer");
        return BAD_VALUE;
    }
    (*pImgBufferLocked) = true;
    imageBuffer = imgBuf;

    FUNCTION_OUT;
    return err;
}


/******************************************************************************
 *
 *****************************************************************************/
MERROR
P1NodeImp::
hwImageBuffer_unlock(MBOOL isFull)
{
    FUNCTION_IN;
    MERROR err = OK;
    Mutex::Autolock _l(mImageBufferLock);
    sp<IImageBuffer> imgBuf;
    android::String8 imgBufferName;
    MBOOL* pImgBufferLocked = NULL;
    if (isFull) {
        imgBuf = mImageBufferFull;
        imgBufferName = mImageBufferNameFull;
        pImgBufferLocked = &mImageBufferLockedFull;
    } else {
        imgBuf = mImageBufferResizer;
        imgBufferName = mImageBufferNameResizer;
        pImgBufferLocked = &mImageBufferLockedResizer;
    }
    //
    if (imgBuf == NULL) {
        MY_LOGD2("[%s] ImageBuffer not init", (isFull) ? "full" : "resizer");
    } else if ((*pImgBufferLocked) == false) {
        MY_LOGD2("[%s] ImageBuffer not locked", (isFull) ? "full" : "resizer");
    } else {
        imgBuf->unlockBuf(imgBufferName);
        (*pImgBufferLocked) = false;
    }

    FUNCTION_OUT;
    return err;
}


/******************************************************************************
 *
 *****************************************************************************/
MERROR
P1NodeImp::
hwImageBuffer_uninit(void)
{
    FUNCTION_IN;
    MERROR err = OK;
    Mutex::Autolock _l(mImageBufferLock);
    if (mImageBufferFull != NULL) {
        //mImageBufferFull->unlockBuf(mImageBufferNameFull);
        mImageBufferFull = NULL;
        mImageBufferNameFull = "";
        mImageBufferLockedFull = false;
    }
    if (mImageBufferResizer != NULL) {
        //mImageBufferResizer->unlockBuf(mImageBufferNameResizer);
        mImageBufferResizer = NULL;
        mImageBufferNameResizer = "";
        mImageBufferLockedResizer = false;
    }

    FUNCTION_OUT;
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hardwareOps_start()
{
#if SUPPORT_ISP
    FUNCTION_IN
    CAM_TRACE_CALL();

    Mutex::Autolock _l(mHardwareLock);

    mActive = MTRUE;
    mInit   = MTRUE;

    mDequeThreadProfile.reset();
    mImageStorage.init(mLogLevel);

    //
    CAM_TRACE_BEGIN("isp init");
    mpCamIO = INormalPipe_FrmB::createInstance(getOpenId(), getNodeName(), 1); // burstQnum --> for slow motion
    if(!mpCamIO || !mpCamIO->init())
    {
        MY_LOGE("hardware init fail");
        return DEAD_OBJECT;
    }
    CAM_TRACE_END();

    //
    IHalSensor::ConfigParam sensorCfg =
    {
        (MUINT)getOpenId(),                 /* index            */
        mSensorParams.size,                 /* crop */
        mSensorParams.mode,                 /* scenarioId       */
        0,                                  /* isBypassScenario */
        1,                                  /* isContinuous     */
        MFALSE,                             /* iHDROn           */
        mSensorParams.fps,                  /* framerate        */
        0,                                  /* two pixel on     */
        0,                                  /* debugmode        */
    };

    vector<IHalSensor::ConfigParam> vSensorCfg;
    vSensorCfg.push_back(sensorCfg);

    //
    vector<portInfo> vPortInfo;
    if (mvOutImage_full.size() != 0)
    {
        portInfo OutPort(
                PORT_IMGO,
                (EImageFormat)mvOutImage_full[0]->getImgFormat(),
                mvOutImage_full[0]->getImgSize(),
                MRect(MPoint(0,0), mSensorParams.size),
                mvOutImage_full[0]->getBufPlanes().itemAt(0).rowStrideInBytes,
                0, //pPortCfg->mStrideInByte[1],
                0, //pPortCfg->mStrideInByte[2],
                0, // pureraw
                MTRUE);              //packed

        vPortInfo.push_back(OutPort);
    }


    if (mOutImage_resizer != NULL)
    {
        portInfo OutPort(
                PORT_RRZO,
                (EImageFormat)mOutImage_resizer->getImgFormat(),
                mOutImage_resizer->getImgSize(),
                MRect(MPoint(0,0), mSensorParams.size),
                mOutImage_resizer->getBufPlanes().itemAt(0).rowStrideInBytes,
                0, //pPortCfg->mStrideInByte[1],
                0, //pPortCfg->mStrideInByte[2],
                0, // pureraw
                MTRUE);              //packed

        vPortInfo.push_back(OutPort);
    }

    QInitParam halCamIOinitParam(
               0,                           /*sensor test pattern */
               10,                          /* bit depth*/
               vSensorCfg,
               vPortInfo);

    halCamIOinitParam.m_DropCB = doNotifyDropframe;
    halCamIOinitParam.m_returnCookie = this;


    #if SUPPORT_3A
    CAM_TRACE_BEGIN("3a createinstance");
    mp3A = IHal3A::createInstance(IHal3A::E_Camera_3, getOpenId(), getNodeName());
    mp3A->setSensorMode(mSensorParams.mode);
    CAM_TRACE_END();
    #endif

    //
    CAM_TRACE_BEGIN("isp config");
    if(!mpCamIO->configPipe(halCamIOinitParam))
    {
        MY_LOGE("hardware config pipe fail");
        return BAD_VALUE;
    }
    CAM_TRACE_END();

    #if SUPPORT_EIS
    CAM_TRACE_BEGIN("eis config");
    mpEIS = EisHal_R::CreateInstance(LOG_TAG, getOpenId());
    mpEIS->Init();
    EIS_HAL_CONFIG_DATA  config;
    config.sensorType = IHalSensorList::get()->queryType(getOpenId());
    //config.memAlignment //[AWARE] may need to modify by platform
    config.configSce = EIS_SCE_EIS;

    mpEIS->ConfigEis(EIS_PASS_1, config);
    mEisActive = MTRUE;
    CAM_TRACE_END();
    #endif


    #if SUPPORT_3A
    CAM_TRACE_BEGIN("3A start");
    if (mp3A) {
        mp3A->start();
        mp3A->attachCb(IHal3ACb::eID_NOTIFY_3APROC_FINISH, this);
        mp3A->attachCb(IHal3ACb::eID_NOTIFY_CURR_RESULT, this);
        //m3AProcessedDepth = mp3A->getCapacity();
    }
    m3AActive = MTRUE;
    CAM_TRACE_END();
    #endif


    //register 3 real frames and 3 dummy frames
    //[TODO] in case that delay frame is above 3 but memeory has only 3, pending aquirefromPool
    CAM_TRACE_BEGIN("create node");
    {
        for (int i = 0; i < mNumHardwareBuffer; i++) {
            createNode(NULL, &mProcessingQueue, &mProcessingQueueLock,
                             &mControls3AList, &mControls3AListLock);
            hardwareOps_enque(mProcessingQueue.editItemAt(mProcessingQueue.size()-1), MFALSE, MFALSE);
        }
        for (int i = 0; i < mNumHardwareBuffer; i++) {
            createNode(mdummyQueue);
            hardwareOps_enque(mdummyQueue.editItemAt(mdummyQueue.size()-1), MTRUE, MFALSE);
        }
        // Due to pipeline latency, delay frame should be above 3
        // if delay frame is more than 3, add node to mProcessingQueue here.
        for (int i = 0; i < mDelayframe - mNumHardwareBuffer; i++) {
            createNode(NULL, &mProcessingQueue, &mProcessingQueueLock,
                             &mControls3AList, &mControls3AListLock);
        }
    }
    CAM_TRACE_END();
    //
    CAM_TRACE_BEGIN("isp start");
    if(!mpCamIO->start()) {
        MY_LOGE("hardware start fail");
        return BAD_VALUE;
    }
    CAM_TRACE_END();

    mCamActive = MTRUE;


    MINT32 tgInfo = HwEventIrq::E_NONE;
    IHalSensorList* const pIHalSensorList = IHalSensorList::get();
    IHalSensor* pHalSensor = pIHalSensorList->createSensor(LOG_TAG, getOpenId());
    MUINT32 sensorDev = (MUINT32)pIHalSensorList->querySensorDevIdx(getOpenId());
    SensorDynamicInfo senInfo;
    MINT32 i4SensorDevId = pHalSensor->querySensorDynamicInfo(sensorDev, &senInfo);
    pHalSensor->destroyInstance(LOG_TAG);
    MY_LOGD("sensorDev = %d, senInfo.TgInfo = %d", sensorDev, senInfo.TgInfo);
    switch (senInfo.TgInfo)
    {
        case CAM_TG_1:
            tgInfo = HwEventIrq::E_TG1;
            break;
        case CAM_TG_2:
            tgInfo = HwEventIrq::E_TG2;
            break;
        default:
            break;
    }

    HwEventIrq::ConfigParam irqConfig(sensorDev, tgInfo, HwEventIrq::E_Event_Vsync);
    mpIEventIrq = HwEventIrq::createInstance(irqConfig, "P1Node");


    {
        Mutex::Autolock _l(mThreadLock);
        mThreadCond.broadcast();
    }

    FUNCTION_OUT

    return OK;
#else
    return OK;
#endif

}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
createFullBuf(sp<IImageBuffer> const &pImageBuffer, MUINT32 num, MUINT32 idx, QBufInfo &enBuf)
{
    if (mvOutImage_full.size() == 0) {
        return;
    }

    NSCam::NSIoPipe::NSCamIOPipe::BufInfo rBufInfo (
        PORT_IMGO,
        pImageBuffer.get(),
        mvOutImage_full[0]->getImgSize(),
        MRect(MPoint(0, 0), mSensorParams.size),
        num,
        idx);

    enBuf.mvOut.push_back(rBufInfo);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
createResizerBuf(sp<IImageBuffer> const &pImageBuffer, MUINT32 num, MUINT32 idx, QBufInfo &enBuf)
{
    if (mOutImage_resizer == NULL) {
        return;
    }

    NSCam::NSIoPipe::NSCamIOPipe::BufInfo rBufInfo(
        PORT_RRZO,
        pImageBuffer.get(),
        mOutImage_resizer->getImgSize(),
        MRect(MPoint(0, 0), mSensorParams.size),
        num,
        idx);

    enBuf.mvOut.push_back(rBufInfo);
}

#if (SUPPORT_SCALING_CROP)
/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
createFullBuf(QueNode_T const& node, QBufInfo &enBuf)
{
    if (mvOutImage_full.size() == 0) {
        return;
    }

    MSize dstSize = node.dstSize_full;
    MRect cropRect = node.cropRect_full;

    if (mInit) {
        MY_LOGD1("createFullBuf mInit : (%d)", mInit);
        dstSize = mvOutImage_full[0]->getImgSize();
        cropRect = MRect(MPoint(0, 0), mSensorParams.size);
    }
    /*
    MY_LOGD("[CropInfo] (%d,%d-%dx%d)(%dx%d)", cropRect.p.x, cropRect.p.y,
        cropRect.s.w, cropRect.s.h, dstSize.w, dstSize.h);
    */

    NSCam::NSIoPipe::NSCamIOPipe::BufInfo rBufInfo (
        PORT_IMGO,
        node.img_full.get(),
        dstSize,
        cropRect,
        node.magicNum,
        node.sofIdx);

    enBuf.mvOut.push_back(rBufInfo);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
createResizerBuf(QueNode_T const& node, QBufInfo &enBuf)
{
    if (mOutImage_resizer == NULL) {
        return;
    }

    MSize dstSize = node.dstSize_resizer;
    MRect cropRect = node.cropRect_resizer;

    if (mInit) {
        MY_LOGD1("createResizerBuf mInit : (%d)", mInit);
        dstSize = mOutImage_resizer->getImgSize();
        cropRect = MRect(MPoint(0, 0), mSensorParams.size);
    }
    /*
    MY_LOGD("[CropInfo] (%d,%d-%dx%d)(%dx%d)", cropRect.p.x, cropRect.p.y,
        cropRect.s.w, cropRect.s.h, dstSize.w, dstSize.h);
    */

    NSCam::NSIoPipe::NSCamIOPipe::BufInfo rBufInfo (
        PORT_RRZO,
        node.img_resizer.get(),
        dstSize,
        cropRect,
        node.magicNum,
        node.sofIdx);

    enBuf.mvOut.push_back(rBufInfo);
}
#endif

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
generateAppMeta(sp<IPipelineFrame> const &request, MetaSet_T const &result3A,
                QBufInfo const &deqBuf, IMetadata &appMetadata)
{
    //[3A/Flash/sensor section]
    appMetadata = result3A.appMeta;

    //[request section]
    // android.request.frameCount
    {
        IMetadata::IEntry entry(MTK_REQUEST_FRAME_COUNT);
        entry.push_back( request->getFrameNo(), Type2Type< MINT32 >());
        appMetadata.update(MTK_REQUEST_FRAME_COUNT, entry);
    }
    // android.request.metadataMode
    {
        IMetadata::IEntry entry(MTK_REQUEST_METADATA_MODE);
        entry.push_back(MTK_REQUEST_METADATA_MODE_FULL, Type2Type< MUINT8 >());
        appMetadata.update(MTK_REQUEST_METADATA_MODE, entry);
    }

    //[sensor section]
    // android.sensor.timestamp
    {
        MINT64 frame_duration = 0; //IMetadata::IEntry entry(MTK_SENSOR_FRAME_DURATION);
                                   //should get from control.
        MINT64 timestamp = deqBuf.mvOut[0].mMetaData.mTimeStamp - frame_duration;
        IMetadata::IEntry entry(MTK_SENSOR_TIMESTAMP);
        entry.push_back(timestamp, Type2Type< MINT64 >());
        appMetadata.update(MTK_SENSOR_TIMESTAMP, entry);
    }

    //[sensor section]
    // android.sensor.rollingshutterskew
    // [TODO] should query from sensor
    {
        IMetadata::IEntry entry(MTK_SENSOR_ROLLING_SHUTTER_SKEW);
        entry.push_back(33000000, Type2Type< MINT64 >());
        appMetadata.update(MTK_SENSOR_ROLLING_SHUTTER_SKEW, entry);
    }


}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
generateHalMeta(MetaSet_T const &result3A, QBufInfo const &deqBuf, IMetadata const &resultEIS, IMetadata const &inHalMetadata, IMetadata &halMetadata)
{
    if (deqBuf.mvOut.size() == 0) {
        MY_LOGE("deqBuf is empty");
        return;
    }

    //3a tuning
    halMetadata = result3A.halMeta;

    //eis
    halMetadata += resultEIS;

    // in hal meta
    halMetadata += inHalMetadata;

    //rrzo
    for (size_t i = 0; i < deqBuf.mvOut.size(); i++) {
        if (deqBuf.mvOut[i].mPortID == PORT_RRZO) {
            //crop region
            {
                IMetadata::IEntry entry(MTK_P1NODE_SCALAR_CROP_REGION);
                entry.push_back(deqBuf.mvOut[i].mMetaData.mCrop_s, Type2Type< MRect >());
                halMetadata.update(MTK_P1NODE_SCALAR_CROP_REGION, entry);
            }
            {
                IMetadata::IEntry entry(MTK_P1NODE_DMA_CROP_REGION);
                entry.push_back(deqBuf.mvOut[i].mMetaData.mCrop_d, Type2Type< MRect >());
                halMetadata.update(MTK_P1NODE_DMA_CROP_REGION, entry);
            }

            {
                IMetadata::IEntry entry(MTK_P1NODE_RESIZER_SIZE);
                entry.push_back(deqBuf.mvOut[i].mMetaData.mDstSize, Type2Type< MSize >());
                halMetadata.update(MTK_P1NODE_RESIZER_SIZE, entry);
            }
            /*
            MY_LOGD("[CropInfo] CropS(%d, %d, %dx%d) "
                "CropD(%d, %d, %dx%d) DstSize(%dx%d)",
                deqBuf.mvOut[i].mMetaData.mCrop_s.leftTop().x,
                deqBuf.mvOut[i].mMetaData.mCrop_s.leftTop().y,
                deqBuf.mvOut[i].mMetaData.mCrop_s.size().w,
                deqBuf.mvOut[i].mMetaData.mCrop_s.size().h,
                deqBuf.mvOut[i].mMetaData.mCrop_d.leftTop().x,
                deqBuf.mvOut[i].mMetaData.mCrop_d.leftTop().y,
                deqBuf.mvOut[i].mMetaData.mCrop_d.size().w,
                deqBuf.mvOut[i].mMetaData.mCrop_d.size().h,
                deqBuf.mvOut[i].mMetaData.mDstSize.w,
                deqBuf.mvOut[i].mMetaData.mDstSize.h);
            */
        } else {
            continue;
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hardwareOps_enque(
    QueNode_T &node,
    MBOOL dummy,
    MBOOL block
)
{
#if SUPPORT_ISP

    FUNCTION_IN

    QBufInfo enBuf;

    //(1) prepare buffer
    if (node.appFrame == NULL) {
        {
            sp<IImageStreamBuffer> streamBuffer;
            sp<IImageBuffer> imageBuffer;
            MBOOL useDummy = (dummy | (mpStreamPool_full == NULL));
            if (useDummy && (mDummyBufferFull != NULL))
            {
                if (OK != hwDummyBuffer_acquire(mDummyBufferFull,
                                        getNodeName(),
                                        imageBuffer)
                )
                {
                    MY_LOGE("full image buffer heap fail");
                    return BAD_VALUE;
                }
                node.halFrame_full = NULL;
                node.img_full = imageBuffer;
            }
            else if (mpStreamPool_full != 0)
            {
                MERROR err = mpStreamPool_full->acquireFromPool(getNodeName(),streamBuffer, ::s2ns(300));
                if( err != OK )
                {
                    if( err == TIMED_OUT )
                        MY_LOGW("acquire timeout");
                    else
                        MY_LOGE("acquire failed");

                    mpStreamPool_full->dumpPool();
                    return BAD_VALUE;
                }

                lockImageBuffer(streamBuffer, imageBuffer);
                node.halFrame_full = streamBuffer;
                node.img_full = imageBuffer;
            }
            //
            #if (SUPPORT_SCALING_CROP_IMGO)
            createFullBuf(node, enBuf);
            #else
            createFullBuf(imageBuffer, node.magicNum, node.sofIdx, enBuf);
            #endif
        }
        {
            sp<IImageStreamBuffer> streamBuffer;
            sp<IImageBuffer> imageBuffer;
            MBOOL useDummy = (dummy | (mpStreamPool_resizer == NULL));
            if (useDummy && (mDummyBufferResizer != NULL))
            {
                if (OK != hwDummyBuffer_acquire(mDummyBufferResizer,
                                        getNodeName(),
                                        imageBuffer)
                )
                {
                    MY_LOGE("resizer image buffer heap fail");
                    return BAD_VALUE;
                }
                node.halFrame_resizer = NULL;
                node.img_resizer = imageBuffer;
            }
            else if (mpStreamPool_resizer != 0)
            {
                MERROR err = mpStreamPool_resizer->acquireFromPool(getNodeName(), streamBuffer, ::s2ns(300));
                if( err != OK )
                {
                    if( err == TIMED_OUT )
                        MY_LOGW("acquire timeout");
                    else
                        MY_LOGE("acquire failed");

                    mpStreamPool_resizer->dumpPool();
                    return BAD_VALUE;
                }
                lockImageBuffer(streamBuffer, imageBuffer);
                node.halFrame_resizer = streamBuffer;
                node.img_resizer = imageBuffer;
            }
            //
            #if (SUPPORT_SCALING_CROP_RRZO)
            createResizerBuf(node, enBuf);
            #else
            createResizerBuf(imageBuffer, node.magicNum, node.sofIdx, enBuf);
            #endif
        }
    }
#if SUPPORT_HQC
    else if ((highQualityCapture_get() == P1_HQC_STATE_SET) &&
        (highQualityCapture_check(node.magicNum))) {
        // for IMGO
        if (mpStreamPool_full != 0) {
            sp<IImageStreamBuffer> streamBuffer;
            sp<IImageBuffer> imageBuffer;
            MERROR err = mpStreamPool_full->acquireFromPool(getNodeName(),
                                                    streamBuffer, ::s2ns(300));
            if( err != OK )
            {
                if( err == TIMED_OUT )
                    MY_LOGW("acquire timeout");
                else
                    MY_LOGE("acquire failed");

                mpStreamPool_full->dumpPool();
                MY_LOGE("HighQualityCapture full buffer fail");
                highQualityCapture_uninit();
                return BAD_VALUE;
            }
            lockImageBuffer(streamBuffer, imageBuffer);
            node.halFrame_full = streamBuffer;
            node.img_full = imageBuffer;
        } else if (mvOutImage_full.size() > 0) {
            if (OK != hwImageBuffer_init(mvOutImage_full[0])) {
                MY_LOGE("HighQualityCapture full image buffer init fail");
                highQualityCapture_uninit();
                return BAD_VALUE;
            }
            sp<IImageBuffer> imageBuffer;
            MERROR err = hwImageBuffer_lock(true, imageBuffer);
            if( err != OK )
            {
                MY_LOGE("HighQualityCapture full image buffer lock fail");
                highQualityCapture_uninit();
                return BAD_VALUE;
            }
            node.halFrame_full = NULL;
            node.img_full = imageBuffer;
        } else {
            MY_LOGE("HighQualityCapture no full raw stream info");
            highQualityCapture_uninit();
            return BAD_VALUE;
        }
        // for RRZO
        if (mOutImage_resizer != NULL) {
            sp<IImageStreamBuffer>  pOutpImageStreamBuffer;
            sp<IImageBuffer> imageBuffer;
            if (OK == lockImageBuffer(node.appFrame,
                                        mOutImage_resizer->getStreamId(),
                                        pOutpImageStreamBuffer, imageBuffer)) {
                node.halFrame_resizer = NULL;
                node.img_resizer = imageBuffer;
            } else {
                MY_LOGE("HighQualityCapture resizer buffer fail");
                highQualityCapture_uninit();
                return BAD_VALUE;
            }
        }
        createPureBuf(node, enBuf);
    }
#endif
    else {
        if(node.reqOut & REQ_OUT_RESIZER) {
            if (mOutImage_resizer != NULL) {
                sp<IImageStreamBuffer>  pOutpImageStreamBuffer;
                sp<IImageBuffer> imageBuffer;
                if (OK == lockImageBuffer(node.appFrame, mOutImage_resizer->getStreamId(),
                            pOutpImageStreamBuffer, imageBuffer)) {
                    node.img_resizer = imageBuffer;
#if (SUPPORT_SCALING_CROP_RRZO)
                    createResizerBuf(node, enBuf);
#else
                    createResizerBuf(imageBuffer, node.magicNum, node.sofIdx, enBuf);
#endif
                }
            }
        }
        if(node.reqOut & REQ_OUT_FULL) {
            for (size_t i = 0; i < mvOutImage_full.size(); i++) {
                sp<IImageStreamBuffer>  pOutpImageStreamBuffer;
                sp<IImageBuffer> imageBuffer;
                if (OK != lockImageBuffer(node.appFrame, mvOutImage_full[i]->getStreamId(),
                            pOutpImageStreamBuffer, imageBuffer)) {
                    continue;
                }
                node.img_full = imageBuffer;
#if (SUPPORT_SCALING_CROP_IMGO)
                createFullBuf(node, enBuf);
#else
                createFullBuf(imageBuffer, node.magicNum, node.sofIdx, enBuf);
#endif
            }
        }

        if (node.reqOut & REQ_OUT_OPAQUE) {
            if (mOutImage_opaque != NULL) {
                sp<IImageStreamBuffer>  pOutpImageStreamBuffer;
                sp<IImageBuffer> imageBuffer;
                if (OK == lockImageBuffer(node.appFrame,
                            mOutImage_opaque->getStreamId(),
                            pOutpImageStreamBuffer, imageBuffer)) {
                    MY_LOGD("lock mOutImage_opaque");
                    node.img_opaque = imageBuffer;
                    createFullBuf(imageBuffer, node.magicNum, node.sofIdx, enBuf);
                }
                else {
                    MY_LOGE("lockImageBuffer failed");
                }
            }
        }

    }


    //(2)
    //very hardware-dependent
    //to ensure that status is not in drop status.
    MINT32 status = _normal_status;
    while(block) {
        if (!mActive) break;

        if (MTRUE != mpCamIO->sendCommand(EPIPECmd_GET_CUR_FRM_STATUS, (MINTPTR)&status, 0, 0)) {
            MY_LOGE("error");
            break;
        }
        if (status == _drop_frame_status || status == _1st_enqloop_status) {
            HwEventIrq::Duration duration;
            if (mpIEventIrq) {
                MY_LOGW("wait for status: %d", status);
                mpIEventIrq->wait(duration);
                MY_LOGD("wait-");
            }
        } else {
            break;
        }
    }

    //enque
    if (!dummy) {
        CAM_TRACE_FMT_BEGIN("enq #(%d/%d)",
            node.appFrame != NULL ? node.appFrame->getFrameNo() : 0,
            node.magicNum);
        if(!mpCamIO->enque(enBuf)) {
            MY_LOGE("enque fail");
            CAM_TRACE_FMT_END();
            #if SUPPORT_HQC
            if (highQualityCapture_get() == P1_HQC_STATE_SET &&
                highQualityCapture_check(node.magicNum)) {
                MY_LOGE("HighQualityCapture enque fail");
                highQualityCapture_uninit();
            }
            #endif
            return BAD_VALUE;
        }
        CAM_TRACE_FMT_END();
        #if SUPPORT_HQC
        if (highQualityCapture_get() == P1_HQC_STATE_SET &&
            highQualityCapture_check(node.magicNum)) {
            highQualityCapture_set(P1_HQC_STATE_ENQ);
        }
        #endif
    } else {
        if(!mpCamIO->DummyFrame(enBuf)) {
            MY_LOGE("enque dummy fail");
            return BAD_VALUE;
        }
    }

    FUNCTION_OUT
    return OK;

#else
    return OK;
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hardwareOps_deque(QBufInfo &deqBuf, MetaSet_T &result3A)
{

#if SUPPORT_ISP

    FUNCTION_IN

    if (!mActive) {
        return BAD_VALUE;
    }

    Mutex::Autolock _l(mHardwareLock);
    if (!mActive) {
        return BAD_VALUE;
    }


    {
        // deque buffer, and handle frame and metadata
        if ((mvOutImage_full.size() > 0) || (mOutImage_opaque != NULL))/*(mpStreamPool_full!=NULL)*/ {
            NSCam::NSIoPipe::NSCamIOPipe::BufInfo rBufInfo;
            rBufInfo.mPortID = PORT_IMGO;
            deqBuf.mvOut.push_back(rBufInfo);
        }
        if (mOutImage_resizer != NULL)/*(mpStreamPool_resizer!=NULL)*/ {
            NSCam::NSIoPipe::NSCamIOPipe::BufInfo rBufInfo;
            rBufInfo.mPortID = PORT_RRZO;
            deqBuf.mvOut.push_back(rBufInfo);
        }

        mDequeThreadProfile.pulse_down();
        MY_LOGD1("%" PRId64 ", %f", mDequeThreadProfile.getAvgDuration(), mDequeThreadProfile.getFps());
        if(!mpCamIO->deque(deqBuf)) {
            if (deqBuf.mvOut.at(0).mMetaData.m_bDummyFrame) {
                MY_LOGW("dummy");
                return BAD_VALUE;
            } else {
                MY_LOGE("deque fail");
                return BAD_VALUE;
            }
        }
        mDequeThreadProfile.pulse_up();
        #if SUPPORT_HQC

        MVOID* imgo_addr = NULL;
        for ( size_t i = 0; i < deqBuf.mvOut.size(); i++ )
        {
            if( deqBuf.mvOut[i].mPortID.index == PORT_IMGO.index ) {
                imgo_addr = deqBuf.mvOut[i].mBuffer;
                break;
            }
        }
        if (highQualityCapture_get() == P1_HQC_STATE_ENQ &&
            highQualityCapture_getAddr() == imgo_addr
            /*highQualityCapture_check(deqBuf.mvOut[0].mMetaData.mMagicNum_hal)*/) {
            highQualityCapture_set(P1_HQC_STATE_DEQ);
        }
        #endif
    }


    //

    static bool shouldPrint = false;
    if (shouldPrint) {
        for(size_t i = 0; i < deqBuf.mvOut.size(); i++) {
            char filename[256];
            sprintf(filename, "/data/P1_%d_%d_%d.raw", deqBuf.mvOut.at(i).mMetaData.mMagicNum_hal,
                deqBuf.mvOut.at(i).mBuffer->getImgSize().w,
                deqBuf.mvOut.at(i).mBuffer->getImgSize().h);
            NSCam::Utils::saveBufToFile(filename, (unsigned char*)deqBuf.mvOut.at(i).mBuffer->getBufVA(0), deqBuf.mvOut.at(i).mBuffer->getBufSizeInBytes(0));
            shouldPrint = false;
        }
    }



#if SUPPORT_3A
    if (mActive && mp3A) {
        mp3A->notifyP1Done(deqBuf.mvOut[0].mMetaData.mMagicNum_hal);
        mp3A->get(deqBuf.mvOut[0].mMetaData.mMagicNum_hal, result3A);
    }
#endif


    FUNCTION_OUT

    return OK;
#else
    return OK;
#endif

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hardwareOps_stop()
{
#if SUPPORT_ISP
    CAM_TRACE_CALL();

    FUNCTION_IN

    //(1) handle active flag
    if (!mActive) {
        MY_LOGD("active=%d", mActive);
        return OK;
    }
    mActive = MFALSE;

    //(2.1) stop EIS thread
    #if SUPPORT_EIS
    CAM_TRACE_BEGIN("eis thread stop");
    if(mpEIS && mEisActive) {
        mpEIS->EisThreadStop();
    }
    mEisActive = MFALSE;
    CAM_TRACE_END();
    #endif

    //(2.2) stop 3A
    #if SUPPORT_3A
    CAM_TRACE_BEGIN("3A stop");
    if (mp3A) {
        if(m3AActive) {
            mp3A->detachCb(IHal3ACb::eID_NOTIFY_3APROC_FINISH, this);
            mp3A->detachCb(IHal3ACb::eID_NOTIFY_CURR_RESULT, this);
            mp3A->stop();
        }
    }
    m3AActive = MFALSE;
    CAM_TRACE_END();
    #endif

    //(2.3) stop isp
    {
        Mutex::Autolock _l(mHardwareLock);
        CAM_TRACE_BEGIN("isp stop");
        if (mpIEventIrq) {
            mpIEventIrq->destroyInstance("P1Node");
            mpIEventIrq = NULL;
        }
        if(mpCamIO) {
            if(mCamActive && !mpCamIO->stop()) {
                MY_LOGE("hardware stop fail");
                return BAD_VALUE;
            }
        }
        mCamActive = MFALSE;
        CAM_TRACE_END();
    }

    //(3.1) destroy 3A
    #if SUPPORT_3A
    CAM_TRACE_BEGIN("3A destroy");
    if (mp3A) {
        mp3A->destroyInstance(getNodeName());
        mp3A = NULL;
    }
    CAM_TRACE_END();
    #endif

    //(3.2) destroy isp
    {
        Mutex::Autolock _l(mHardwareLock);
        #if SUPPORT_EIS
        if(mpEIS) {
            mpEIS->Uninit();
            mpEIS->DestroyInstance(LOG_TAG);
            mpEIS = NULL;
        }
        #endif
        //
        CAM_TRACE_BEGIN("isp destroy");
        if(mpCamIO) {
            if(!mpCamIO->uninit() )
            {
                MY_LOGE("hardware uninit fail");
                return BAD_VALUE;
            }
            mpCamIO->destroyInstance(getNodeName());
            mpCamIO = NULL;
        }
        CAM_TRACE_END();
    }
    //
    FUNCTION_OUT

    return OK;

#else
    return OK;
#endif

}


#if SUPPORT_HQC
/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hardwareOps_suspend()
{
    FUNCTION_IN;

    if (mActive) {
        hardwareOps_stop();
    }

    FUNCTION_OUT;

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hardwareOps_resume()
{
    FUNCTION_IN;

    if (!mActive) {
        hardwareOps_start();
    }

    FUNCTION_OUT;

    return OK;
}
#endif


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
lockMetadata(sp<IPipelineFrame> const& pFrame, StreamId_T const streamId, IMetadata &rMetadata)
{
    if (pFrame == NULL) {
        //MY_LOGE("nput is NULL");
        return INVALID_OPERATION;
    }

    //  Input Meta Stream: Request
    IStreamBufferSet& rStreamBufferSet  = pFrame->getStreamBufferSet();
    sp<IMetaStreamBuffer> pMetaStreamBuffer = NULL;
    MERROR const err = ensureMetaBufferAvailable_(
        pFrame->getFrameNo(),
        streamId,
        rStreamBufferSet,
        pMetaStreamBuffer
    );
    if (err != OK) {
        return err;
    }
    rMetadata = *pMetaStreamBuffer->tryReadLock(getNodeName());

    return err;

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
returnLockedMetadata(sp<IPipelineFrame> const& pFrame, StreamId_T const streamId, MBOOL success)
{
    if (pFrame == NULL) {
        //MY_LOGE("nput is  NULL");
        return INVALID_OPERATION;
    }

    //  Input Meta Stream: Request
    IStreamBufferSet& rStreamBufferSet  = pFrame->getStreamBufferSet();
    sp<IMetaStreamBuffer> pMetaStreamBuffer = NULL;
    MERROR const err = ensureMetaBufferAvailable_(
        pFrame->getFrameNo(),
        streamId,
        rStreamBufferSet,
        pMetaStreamBuffer,
        MFALSE
    );
    if (err != OK) {
        return err;
    }

    if  ( 0 > mInStreamIds.indexOf(streamId) ) {
        if (success) {
            pMetaStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_OK);
        } else {
            pMetaStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_ERROR);
        }
    }

    //
    //  Mark this buffer as USED by this user.
    //  Mark this buffer as RELEASE by this user.
    rStreamBufferSet.markUserStatus(
        streamId, getNodeId(),
        IUsersManager::UserStatus::USED |
        IUsersManager::UserStatus::RELEASE
    );

    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
returnUnlockedMetadata(sp<IPipelineFrame> const& pFrame, StreamId_T const streamId)
{
    if (pFrame == NULL) {
        //MY_LOGE("nput is NULL");
        return INVALID_OPERATION;
    }

    //  Input Meta Stream: Request
    IStreamBufferSet& rStreamBufferSet  = pFrame->getStreamBufferSet();
    //
    //  Mark this buffer as RELEASE by this user.
    rStreamBufferSet.markUserStatus(
        streamId, getNodeId(),
        IUsersManager::UserStatus::RELEASE
    );

    return OK;

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
lock_and_returnMetadata(sp<IPipelineFrame> const& pFrame, StreamId_T const streamId, IMetadata &metadata)
{
    if (pFrame == NULL) {
        //MY_LOGE("input is NULL");
        return INVALID_OPERATION;
    }

    IStreamBufferSet& rStreamBufferSet  = pFrame->getStreamBufferSet();

    //
    sp<IMetaStreamBuffer>   pMetaStreamBuffer  = NULL;
    MERROR err = ensureMetaBufferAvailable_(
        pFrame->getFrameNo(),
        streamId,
        rStreamBufferSet,
        pMetaStreamBuffer
    );
    if (err != OK) {
        return err;
    }
    IMetadata* pMetadata = pMetaStreamBuffer->tryWriteLock(getNodeName());
    if (pMetadata == NULL) {
        MY_LOGE("pMetadata == NULL");
        return BAD_VALUE;
    }

    *pMetadata = metadata;

    pMetaStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_OK);
    pMetaStreamBuffer->unlock(getNodeName(), pMetadata);
    //
    //  Mark this buffer as USED by this user.
    //  Mark this buffer as RELEASE by this user.
    rStreamBufferSet.markUserStatus(
        streamId, getNodeId(),
        IUsersManager::UserStatus::USED |
        IUsersManager::UserStatus::RELEASE
    );

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
lockImageBuffer(sp<IPipelineFrame> const& pFrame, StreamId_T const streamId,
                  sp<IImageStreamBuffer>  &pOutpImageStreamBuffer, sp<IImageBuffer> &rImageBuffer)
{
    if (pFrame == NULL) {
        //MY_LOGE("input is NULL");
        return INVALID_OPERATION;
    }

    IStreamBufferSet& rStreamBufferSet  = pFrame->getStreamBufferSet();

    MERROR const err = ensureImageBufferAvailable_(
        pFrame->getFrameNo(),
        streamId,
        rStreamBufferSet,
        pOutpImageStreamBuffer
    );
    if (err != OK) {
        return err;
    }

    MUINT const groupUsage = pOutpImageStreamBuffer->queryGroupUsage(getNodeId());
    sp<IImageBufferHeap>  pOutpImageBufferHeap = pOutpImageStreamBuffer->tryWriteLock(getNodeName());
    if (pOutpImageBufferHeap == NULL) {
        MY_LOGE("pOutpImageBufferHeap == NULL");
        return BAD_VALUE;
    }

    if ((mOutImage_opaque != NULL) &&
        (mOutImage_opaque->getStreamId() == streamId)) {

        pOutpImageBufferHeap->lockBuf(getNodeName());
        OpaqueReprocUtil::setOpaqueInfoToHeap(
            pOutpImageBufferHeap,
            mRawParams.size,
            mRawParams.format,
            mRawParams.stride,
            mRawParams.length);
        pOutpImageBufferHeap->unlockBuf(getNodeName());
	}

    if (((mOutImage_opaque != NULL) &&
        (mOutImage_opaque->getStreamId() == streamId)) ||
        ((mInImage_opaque != NULL) &&
        (mInImage_opaque->getStreamId() == streamId))) {

        pOutpImageBufferHeap->lockBuf(getNodeName());
        MERROR status = OpaqueReprocUtil::getImageBufferFromHeap(
                pOutpImageBufferHeap,
                rImageBuffer);
        pOutpImageBufferHeap->unlockBuf(getNodeName());
        if ( status != OK) {
            MY_LOGE("Can not get ImageBuffer from opaque ImageBufferHeap");
            return BAD_VALUE;
        }
    }

    rImageBuffer = pOutpImageBufferHeap->createImageBuffer();
    rImageBuffer->lockBuf(getNodeName(), groupUsage);

    //mImageStorage.enque(pOutpImageStreamBuffer, rImageBuffer);


    MY_LOGD1("stream buffer: %p, heap: %p, buffer: %p, usage: 0x%x",
        pOutpImageStreamBuffer.get(), pOutpImageBufferHeap.get(), rImageBuffer.get(), groupUsage);

    return err;

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
returnLockedImageBuffer(sp<IPipelineFrame> const& pFrame, StreamId_T const streamId, sp<IImageBuffer> const& pImageBuffer, MBOOL success)
{
    if (pFrame == NULL) {
        //MY_LOGE("input is NULL");
        return INVALID_OPERATION;
    }

    IStreamBufferSet& rStreamBufferSet  = pFrame->getStreamBufferSet();

    sp<IImageStreamBuffer>  pOutpImageStreamBuffer = NULL;
    MERROR const err = ensureImageBufferAvailable_(
        pFrame->getFrameNo(),
        streamId,
        rStreamBufferSet,
        pOutpImageStreamBuffer,
        MFALSE
    );
    if (err != OK) {
        return err;
    }

    sp<IImageBuffer> pOutpImageBuffer = pImageBuffer;// mImageStorage.deque(reinterpret_cast<MINTPTR>(pOutpImageStreamBuffer.get()));
    if (pOutpImageBuffer == NULL) {
        MY_LOGE("pImageBuffer == NULL");
        return BAD_VALUE;
    }

    if  ( 0 > mInStreamIds.indexOf(streamId) ) {
        if (success) {
            pOutpImageStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_OK);
        } else {
            pOutpImageStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_ERROR);
        }
    }

    pOutpImageBuffer->unlockBuf(getNodeName());
    pOutpImageStreamBuffer->unlock(getNodeName(), pOutpImageBuffer->getImageBufferHeap());
    //
    //  Mark this buffer as USED by this user.
    //  Mark this buffer as RELEASE by this user.
    rStreamBufferSet.markUserStatus(
        streamId, getNodeId(),
        IUsersManager::UserStatus::USED |
        IUsersManager::UserStatus::RELEASE
    );


    return OK;

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
returnUnlockedImageBuffer(sp<IPipelineFrame> const& pFrame, StreamId_T const streamId)
{
    if (pFrame == NULL) {
        //MY_LOGE("nput is  NULL");
        return INVALID_OPERATION;
    }

    IStreamBufferSet& rStreamBufferSet  = pFrame->getStreamBufferSet();
    //  Mark this buffer as USED by this user.
    //  Mark this buffer as RELEASE by this user.
    rStreamBufferSet.markUserStatus(
        streamId, getNodeId(),
        IUsersManager::UserStatus::RELEASE
    );

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
lockImageBuffer(sp<IImageStreamBuffer> const& pStreamBuffer, sp<IImageBuffer> &pImageBuffer)
{
    if (pStreamBuffer == NULL) {
        return BAD_VALUE;
    }
    MUINT const usage = GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_HW_CAMERA_READ | GRALLOC_USAGE_HW_CAMERA_WRITE;
    sp<IImageBufferHeap>  pOutpImageBufferHeap = pStreamBuffer->tryWriteLock(getNodeName());
    if (pOutpImageBufferHeap == NULL) {
        MY_LOGE("pOutpImageBufferHeap == NULL");
        return BAD_VALUE;
    }
    pImageBuffer = pOutpImageBufferHeap->createImageBuffer();
    pImageBuffer->lockBuf(getNodeName(), usage);

    //mImageStorage.enque(pStreamBuffer, pImageBuffer);

    MY_LOGD1("streambuffer: %p, heap: %p, buffer: %p, usage: 0x%x",
        pStreamBuffer.get(), pOutpImageBufferHeap.get(), pImageBuffer.get(), usage);

    return OK;

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
returnLockedImageBuffer(sp<IImageBuffer> const &pImageBuffer,
                        sp<IImageStreamBuffer> const &pStreamBuffer,
                        sp<IImageStreamBufferPoolT> const &pStreamPool)
{
    if (pImageBuffer == NULL || pStreamBuffer == NULL || pStreamPool == NULL) {
        MY_LOGE_IF(pImageBuffer == NULL,  "pImageBuffer == NULL");
        MY_LOGE_IF(pStreamBuffer == NULL, "pStreamBuffer == NULL");
        MY_LOGE_IF(pStreamPool == NULL,   "pStreamPool == NULL");
        return BAD_VALUE;
    }

    pImageBuffer->unlockBuf(getNodeName());
    pStreamBuffer->unlock(getNodeName(), pImageBuffer->getImageBufferHeap());

    if(pStreamPool != NULL) {
        pStreamPool->releaseToPool(getNodeName(), pStreamBuffer);
    }

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
returnUnlockedImageBuffer(sp<IImageStreamBuffer> const &pStreamBuffer,
                          sp<IImageStreamBufferPoolT> const &pStreamPool)
{
    if (pStreamBuffer == NULL || pStreamPool == NULL ) {
        MY_LOGE_IF(pStreamBuffer == NULL, "pStreamBuffer == NULL");
        MY_LOGE_IF(pStreamPool == NULL,   "pStreamPool == NULL");
        return BAD_VALUE;
    }

    pStreamPool->releaseToPool(getNodeName(), pStreamBuffer);

    return OK;
}

#if (SUPPORT_SCALING_CROP)
/******************************************************************************
 *
 *****************************************************************************/
MVOID
P1NodeImp::
prepareCropInfo(IMetadata* pMetadata,
           QueNode_T& node)
{
    if (mvOutImage_full.size() > 0) {
        node.dstSize_full = mvOutImage_full[0]->getImgSize();
        node.cropRect_full = MRect(MPoint(0, 0), mSensorParams.size);
    } else {
        node.dstSize_full = MSize(0, 0);
        node.cropRect_full = MRect(MPoint(0, 0), MSize(0, 0));
    }
    if (mOutImage_resizer != NULL) {
        node.dstSize_resizer = mOutImage_resizer->getImgSize();
        node.cropRect_resizer = MRect(MPoint(0, 0), mSensorParams.size);
    } else {
        node.dstSize_resizer= MSize(0, 0);
        node.cropRect_resizer = MRect(MPoint(0, 0), MSize(0, 0));
    }
    //
    if (pMetadata != NULL) {
        MRect cropRect_metadata;    // get from metadata
        MRect cropRect_control;     // set to node

        if( !tryGetMetadata<MRect>(pMetadata, MTK_SCALER_CROP_REGION,
            cropRect_metadata) ) {
            MY_LOGI("Metadata exist - no MTK_SCALER_CROP_REGION, "
                "crop size set to full(%dx%d) resizer(%dx%d)",
                node.dstSize_full.w, node.dstSize_full.h,
                node.dstSize_resizer.w, node.dstSize_resizer.h);
        } else {
            simpleTransform tranActive2Sensor = simpleTransform(
                    MPoint(0,0), mActiveArray.size(), mSensorParams.size);
            cropRect_control.p = transform(tranActive2Sensor,
                                            cropRect_metadata.leftTop());
            cropRect_control.s = transform(tranActive2Sensor,
                                            cropRect_metadata.size());

            #if SUPPORT_EIS
            if (mpEIS)
            {
                MBOOL isEisOn = false;
                MRect const requestRect = MRect(cropRect_control);
                MSize const sensorSize = MSize(mSensorParams.size);
                MPoint const requestCenter=
                    MPoint((requestRect.p.x + (requestRect.s.w >> 1)),
                            (requestRect.p.y + (requestRect.s.h >> 1)));
                isEisOn = isEISOn(pMetadata);
                cropRect_control.s = mpEIS->QueryMinSize(isEisOn, sensorSize,
                                                        requestRect.size());

                if (cropRect_control.s.w != requestRect.size().w)
                {
                    MSize::value_type half_len =
                        ((cropRect_control.s.w + 1) >> 1);
                    if (requestCenter.x < half_len) {
                        cropRect_control.p.x = 0;
                    } else if ((requestCenter.x + half_len) > sensorSize.w) {
                        cropRect_control.p.x = sensorSize.w -
                                                cropRect_control.s.w;
                    } else {
                        cropRect_control.p.x = requestCenter.x - half_len;
                    }
                }
                if (cropRect_control.s.w != requestRect.size().w)
                {
                    MSize::value_type half_len =
                        ((cropRect_control.s.h + 1) >> 1);
                    if (requestCenter.y < half_len) {
                        cropRect_control.p.y = 0;
                    } else if ((requestCenter.y + half_len) > sensorSize.h) {
                        cropRect_control.p.y = sensorSize.h -
                                                cropRect_control.s.h;
                    } else {
                        cropRect_control.p.y = requestCenter.y - half_len;
                    }
                }
            }
            #endif
            /*
            MY_LOGD("[CropInfo] metadata(%d, %d, %dx%d) "
                "control(%d, %d, %dx%d) "
                "active(%d, %d, %dx%d) "
                "sensor(%dx%d)",
                cropRect_metadata.leftTop().x,
                cropRect_metadata.leftTop().y,
                cropRect_metadata.size().w, cropRect_metadata.size().h,
                cropRect_control.leftTop().x,
                cropRect_control.leftTop().y,
                cropRect_control.size().w, cropRect_control.size().h,
                mActiveArray.leftTop().x,
                mActiveArray.leftTop().y,
                mActiveArray.size().w, mActiveArray.size().h,
                mSensorParams.size.w, mSensorParams.size.h);
            */
            // TODO: check more case about crop region
            if ((cropRect_control.size().w < 0) ||
                (cropRect_control.size().h < 0) ||
                (cropRect_control.leftTop().x < 0) ||
                (cropRect_control.leftTop().y < 0) ||
                (cropRect_control.leftTop().x >= mSensorParams.size.w) ||
                (cropRect_control.leftTop().y >= mSensorParams.size.h)) {
                MY_LOGW("Metadata exist - invalid cropRect_control"
                    "(%d, %d, %dx%d) sensor(%dx%d)",
                    cropRect_control.leftTop().x,
                    cropRect_control.leftTop().y,
                    cropRect_control.size().w, cropRect_control.size().h,
                    mSensorParams.size.w, mSensorParams.size.h);
                return;
            }
            if ((cropRect_control.p.x + cropRect_control.s.w) >
                mSensorParams.size.w) {
                cropRect_control.s.w = mSensorParams.size.w -
                                        cropRect_control.p.x;
            }
            if ((cropRect_control.p.y + cropRect_control.s.h) >
                mSensorParams.size.h) {
                cropRect_control.s.h = mSensorParams.size.h -
                                        cropRect_control.p.y;
            }
            // calculate the crop region validity
            calculateCropInfoFull(mSensorParams.pixelMode,
                                    mSensorParams.size,
                                    (mvOutImage_full.size() > 0) ?
                                    (mvOutImage_full[0]->getImgSize()) :
                                    (MSize(0, 0)),
                                    cropRect_control,
                                    node.cropRect_full,
                                    node.dstSize_full,
                                    mLogLevel);
            calculateCropInfoResizer(mSensorParams.pixelMode,
                                    mSensorParams.size,
                                    (mOutImage_resizer != NULL) ?
                                    (mOutImage_resizer->getImgSize()) :
                                    (MSize(0, 0)),
                                    cropRect_control,
                                    node.cropRect_resizer,
                                    node.dstSize_resizer,
                                    mLogLevel);
        }
    }
}
#endif

/******************************************************************************
 *
 *****************************************************************************/
MVOID
P1NodeImp::
createNode(sp<IPipelineFrame> appframe,
           Que_T *Queue,
           Mutex *QueLock,
           List<MetaSet_T> *list,
           Mutex *listLock)
{
    //create queue node
    MUINT32 newNum = get_and_increase_magicnum();
    MetaSet_T metaInfo;
    bool gotMetaInfo = false;
    MUINT8 isHQC = 0;

    MUINT32 nodeOut = REQ_OUT_NONE;
    MUINT32 nodeType = REQ_TYPE_UNKNOWN;
    MBOOL isRedoRequest = false;
    MBOOL isYuvRequest = false;
    IPipelineFrame::InfoIOMapSet rIOMapSet;

    if (appframe != NULL) {
        if(OK != appframe->queryInfoIOMapSet(getNodeId(), rIOMapSet)) {
            MY_LOGE("queryInfoIOMap failed");
            return;
        }
        //
        // do some check
        IPipelineFrame::ImageInfoIOMapSet& imageIOMapSet =
                                                rIOMapSet.mImageInfoIOMapSet;
        if(!imageIOMapSet.size()) {
            MY_LOGW("no imageIOMap in frame");
            return;
        }
        //
        for (size_t i = 0; i < imageIOMapSet.size(); i++) {
            IPipelineFrame::ImageInfoIOMap const& imageIOMap = imageIOMapSet[i];
            if (imageIOMap.vIn.size() > 0) {
                for (size_t j = 0; j < imageIOMap.vIn.size(); j++) {
                    StreamId_T const streamId = imageIOMap.vIn.keyAt(j);
                    if (mInImage_opaque != NULL &&
                        streamId == mInImage_opaque->getStreamId()) {
                        isRedoRequest = MTRUE;
                        break;
                    }
                    if (mInImage_yuv != NULL &&
                        streamId == mInImage_yuv->getStreamId()) {
                        isYuvRequest = MTRUE;
                        break;
                    }
                }
                break;
            } else {
                for (size_t j = 0; j < imageIOMap.vOut.size(); j++) {
                    StreamId_T const streamId = imageIOMap.vOut.keyAt(j);
                    if (mOutImage_resizer != NULL) {
                        if (streamId == mOutImage_resizer->getStreamId()) {
                            nodeOut |= REQ_OUT_RESIZER;
                        }
                    }
                    for (size_t k = 0; k < mvOutImage_full.size(); k++) {
                        if (streamId == mvOutImage_full[k]->getStreamId()) {
                            nodeOut |= REQ_OUT_FULL;
                        }
                    }
                    if (mOutImage_opaque != NULL) {
                        if (streamId == mOutImage_opaque->getStreamId()) {
                            nodeOut |= REQ_OUT_OPAQUE;
                        }
                    }
                }
            }
        }
    }

    if (isRedoRequest) {
        nodeType = REQ_TYPE_REDO;
        nodeOut = REQ_OUT_NONE;
    } else if(isYuvRequest) {
        nodeType = REQ_TYPE_YUV;
        nodeOut = REQ_OUT_NONE;
        // add the frame stall
        {
            //fill in hal metadata
            IMetadata::IEntry entry1(MTK_P1NODE_PROCESSOR_MAGICNUM);
            entry1.push_back(newNum, Type2Type< MINT32 >());
            metaInfo.halMeta.update(MTK_P1NODE_PROCESSOR_MAGICNUM, entry1);
            //
            IMetadata::IEntry entry3(MTK_HAL_REQUEST_DUMMY);
            entry3.push_back(1, Type2Type< MUINT8 >());
            metaInfo.halMeta.update(MTK_HAL_REQUEST_DUMMY, entry3);

            if(listLock != NULL) {
                Mutex::Autolock _l(*listLock);
                (*list).push_back(metaInfo);
            } else {
                (*list).push_back(metaInfo);
            }
        }

    } else if (list!=NULL) { // create list for 3A
        //fill in App metadata
        nodeType = (appframe == NULL) ? REQ_TYPE_PADDING : REQ_TYPE_NORMAL;
        if (appframe != NULL) {
            if (mInAppMeta != NULL) {
                lockMetadata(appframe, mInAppMeta->getStreamId(),
                    metaInfo.appMeta);
                gotMetaInfo = true;
            }
            if (mInHalMeta != NULL) {
                lockMetadata(appframe, mInHalMeta->getStreamId(),
                    metaInfo.halMeta);
            }
            #if SUPPORT_HQC
            if (// for property command force to disable
                (!mHighQualityCaptureForceDisable) &&
                // it should not be applied in PIP mode
                ((mpInstanceInfo != NULL) && (mpInstanceInfo->getCnt() < 2)) &&
                // the full raw path must be presented
                (mvOutImage_full.size() > 0) &&
                // it needs the app metadata to judge the condition
                ((mInAppMeta != NULL) &&
                isHighQualityCaptureOn(&(metaInfo.appMeta), mp3A))) {
                highQualityCapture_init(newNum);
                isHQC = 1;
            }
            #endif
        }
        //fill in hal metadata
        IMetadata::IEntry entry1(MTK_P1NODE_PROCESSOR_MAGICNUM );
        entry1.push_back(newNum, Type2Type< MINT32 >());
        metaInfo.halMeta.update(MTK_P1NODE_PROCESSOR_MAGICNUM, entry1);

        IMetadata::IEntry entry2(MTK_HAL_REQUEST_REPEAT);
        entry2.push_back(0, Type2Type< MUINT8 >());
        metaInfo.halMeta.update(MTK_HAL_REQUEST_REPEAT, entry2);


        MUINT8 isdummy =  appframe == NULL ? 1 : 0;
        IMetadata::IEntry entry3(MTK_HAL_REQUEST_DUMMY);
        entry3.push_back(isdummy, Type2Type< MUINT8 >());
        metaInfo.halMeta.update(MTK_HAL_REQUEST_DUMMY, entry3);

        #if SUPPORT_HQC
        if (isHQC > 0) {
            IMetadata::IEntry entryHQC(MTK_HAL_REQUEST_HIGH_QUALITY_CAP);
            entryHQC.push_back(isHQC, Type2Type< MUINT8 >());
            metaInfo.halMeta.update(MTK_HAL_REQUEST_HIGH_QUALITY_CAP, entryHQC);
        }
        #endif

        if(listLock != NULL) {
            Mutex::Autolock _l(*listLock);
            (*list).push_back(metaInfo);
        } else {
            (*list).push_back(metaInfo);
        }

    }

    MY_LOGD("node type(0x%X) out(0x%X)", nodeType, nodeOut);
    if (nodeType == REQ_TYPE_UNKNOWN) {
        MY_LOGW("request type UNKNOWN");
    } else if (nodeType == REQ_TYPE_REDO) {
        MY_LOGD("request type REDO");
        Mutex::Autolock _l(mRedoQueueLock);
        QueNode_T node;
        node.magicNum = newNum;
        node.sofIdx = P1SOFIDX_INIT_VAL;
        node.appFrame = appframe;
        node.reqOut = nodeOut;
        node.reqType = nodeType;
        mRedoQueue.push_back(node);
    } else if (nodeType == REQ_TYPE_YUV) {
       MY_LOGD("request type YUV");
       Mutex::Autolock _l(mYuvQueueLock);
       QueNode_T node;
       node.magicNum = newNum;
       node.sofIdx = P1SOFIDX_INIT_VAL;
       node.appFrame = appframe;
       node.reqType = nodeType;
       node.reqOut = nodeOut;
       mYuvQueue.push_back(node);
    } else if ((nodeType == REQ_TYPE_NORMAL) && (nodeOut == REQ_OUT_NONE)) {
        MY_LOGW("request out NONE");
    } else // node type is REQ_TYPE_NORMAL-with-output or REQ_TYPE_PADDING

    if(Queue!=NULL){
        Mutex::Autolock _l(*QueLock);
        QueNode_T node;
        node.magicNum = newNum;
        node.sofIdx = P1SOFIDX_INIT_VAL;
        node.appFrame = appframe;
        #if (SUPPORT_SCALING_CROP)
        prepareCropInfo((gotMetaInfo) ? (&metaInfo.appMeta) : (NULL), node);
        #endif
        node.reqOut = nodeOut;
        node.reqType = nodeType;
        (*Queue).push_back(node);
        mlastNonReprocNum = newNum;
    }

    if (appframe != NULL) {
        MY_LOGD1("[New Request] frameNo: %u, magic Num: %d", appframe->getFrameNo(), newNum);
    } else {
        MY_LOGD1("[New Request: dummy] magic Num: %d", newNum);
    }
}


/******************************************************************************
 *
 *****************************************************************************/
MVOID
P1NodeImp::
createNode(Que_T &Queue)
{
    MUINT32 newNum = get_and_increase_magicnum();
    {
        QueNode_T node;
        node.magicNum = newNum;
        node.sofIdx = P1SOFIDX_INIT_VAL;
        node.appFrame = NULL;
        node.reqOut = REQ_OUT_NONE;
        node.reqType = REQ_TYPE_DUMMY;
        Queue.push_back(node);
    }

    MY_LOGD1("[New Request: dummy] magic Num: %d", newNum);
}


/******************************************************************************
 *
 ******************************************************************************/
sp<P1Node>
P1Node::
createInstance()
{
    static InstanceInfo sInstanceInfo;
    return new P1NodeImp(&sInstanceInfo);

}

