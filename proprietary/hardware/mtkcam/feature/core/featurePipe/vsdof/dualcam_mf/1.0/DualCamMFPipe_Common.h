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

#ifndef _MTK_CAMERA_FEATURE_PIPE_DUALCAM_MF_PIPE_COMMON_H_
#define _MTK_CAMERA_FEATURE_PIPE_DUALCAM_MF_PIPE_COMMON_H_

// Standard C header file
#include <ctime>
#include <chrono>
#include <cmath>
#include <map>

// Android system/core header file
#include <utils/RefBase.h>
#include <utils/KeyedVector.h>

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/def/common.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <isp_tuning/isp_tuning.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/aaa/aaa_hal_common.h>
#include <mtkcam/feature/stereo/pipe/vsdof_common.h>
#include <mtkcam/feature/stereo/pipe/IDualCamMultiFramePipe.h>
#include <mtkcam/drv/iopipe/PortMap.h>

// Module header file
#include <featurePipe/core/include/ImageBufferPool.h>
#include <featurePipe/core/include/GraphicBufferPool.h>
#include <featurePipe/vsdof/util/TuningBufferPool.h>
#include <featurePipe/vsdof/util/P2Operator.h>
#include <vsdof/hal/ProfileUtil.h>

#include <mtkcam/feature/mfnr/MfllTypes.h>
#include <mtkcam/feature/mfnr/IMfllCore.h>

// Local header file

using android::String8;
using android::KeyedVector;
using namespace android;
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSDCMF {

// using NSCam::NSIoPipe::PORT_DEPI;
using NSCam::NSIoPipe::PORT_IMGCI;
using NSCam::NSIoPipe::PORT_DMGI;
using NSCam::NSIoPipe::PORT_IMGI;
using NSCam::NSIoPipe::PORT_IMG2O;
using NSCam::NSIoPipe::PORT_IMG3O;
using NSCam::NSIoPipe::PORT_WDMAO;
using NSCam::NSIoPipe::PORT_WROTO;
using NSCam::NSIoPipe::PORT_FEO;
using NSCam::NSIoPipe::PORT_MFBO;

/*******************************************************************************
* String Define
********************************************************************************/
static constexpr const char* STRING_FASTS2S         = "vendor.fc.dualcamMF.fastS2S";
static constexpr const char* STRING_QUICK_POST      = "vendor.fc.daulcamMF.quickPost";
static constexpr const char* STRING_DUMP_START      = "vendor.di.daulcamMF.dump.start";
static constexpr const char* STRING_DUMP_SIZE       = "vendor.di.daulcamMF.dump.size";
static constexpr const char* STRING_DO_PORTER       = "vendor.di.daulcamMF.porter.dump";
static constexpr const char* STRING_TUNING_DUMP     = "vendor.di.daulcamMF.tuning.dump";
static constexpr const char* STRING_SENSOR_FMT      = "vendor.dg.daulcamMF.sensorfmt";
static constexpr const char* STRING_DUMP_PATH       = "/sdcard/dualcam_mf";
static constexpr const char* STRING_DRAW_TIMESTAMP  = "vendor.di.daulcamMF.drawtimestamp";
static constexpr const char* STRING_CUSTOMER_TUNING = "vendor.di.daulcamMF.tuning.cust";

/*******************************************************************************
* Enum Define
********************************************************************************/

/*******************************************************************************
* Structure Define
********************************************************************************/

/*******************************************************************************
* ManualStopWatch
********************************************************************************/
class ManualStopWatch : public IStopWatch
{
public:
    virtual void Start() override
    {
        mProfile->beginProfile();
    }

    virtual void Stop() override
    {
        mProfile->endProfile(false);
    }

    ManualStopWatch& operator=(const ManualStopWatch&) = delete;
    ManualStopWatch(const ManualStopWatch&) = delete;
private:
    ManualStopWatch(android::sp<ProfileUtil> profile)
    : mProfile(profile)
    {

    }

    android::sp<ProfileUtil> mProfile;

    friend class StopWatchCollection;
};
/*******************************************************************************
* AutoStopWatch
********************************************************************************/
class AutoStopWatch : public IStopWatch
{
public:
    virtual void Start() override
    {
        CAM_LOGE("Not implemented.");
    }

    virtual void Stop() override
    {
        CAM_LOGE("Not implemented.");
    }

    virtual ~AutoStopWatch() override
    {
        mProfile->endProfile(false);
    }

    AutoStopWatch& operator=(const AutoStopWatch&) = delete;
    AutoStopWatch(const AutoStopWatch&) = delete;

private:
    AutoStopWatch(android::sp<ProfileUtil> profile)
    : mProfile(profile)
    {
        mProfile->beginProfile();
    }

    android::sp<ProfileUtil> mProfile;

    friend class StopWatchCollection;
};
/*******************************************************************************
* StopWatchCollection
********************************************************************************/
class StopWatchCollection : public IStopWatchCollection
{
public:
    StopWatchCollection(std::string name, MUINT32 requestID)
    : mName(name),
      mRequestID(requestID),
      mIsEnableLog(StereoSettingProvider::isProfileLogEnabled()) { }

    virtual ~StopWatchCollection();

    virtual std::string GetName() const override {return mName;}

    virtual android::sp<IStopWatch> GetStopWatch(eStopWatchType type, const std::string name) override;
    virtual void BeginStopWatch(const std::string name, const void* cookie) override;
    virtual void EndStopWatch(const void* cookie) override;
    virtual void PrintReport() override;

    StopWatchCollection& operator=(const StopWatchCollection& other)    = delete;
    StopWatchCollection& operator=(StopWatchCollection&& other)         = delete;
    StopWatchCollection(const StopWatchCollection& other)               = delete;
    StopWatchCollection(StopWatchCollection&& other)                    = delete;

private:
    const std::string mName;
    const MUINT32 mRequestID;
    const MBOOL mIsEnableLog;

    mutable std::mutex mMutex;

    std::list<android::sp<ProfileUtil>> mStopWatches;
    std::map<const void*, android::sp<ProfileUtil>> mProfileUtilTable;
};

class ImageBufInfoMap;
typedef android::sp<ImageBufInfoMap> ImgInfoMapPtr;
typedef std::map< DualCamMFBufferID, sp<IImageBuffer> >                IImageBufferSet;
typedef KeyedVector<DualCamMFBufferID, SmartImageBuffer>               SmartImageBufferSet;
typedef KeyedVector<DualCamMFBufferID, SmartGraphicBuffer>             GraphicImageBufferSet;
typedef std::map< DualCamMFBufferID, sp<mfll::IMfllImageBuffer> >      MfllImageBufferSet;
typedef KeyedVector<DualCamMFBufferID, IMetadata* > MetadataPtrSet;
typedef KeyedVector<DualCamMFBufferID, IMetadata > MetadataSet;
typedef KeyedVector<DualCamMFBufferID, string > ExtBufNameSet;

class ImageBufInfoMap: public LightRefBase<ImageBufInfoMap>
{
public:
    ImageBufInfoMap(sp<PipeRequest> ptr);
    virtual ~ImageBufInfoMap() {clear();}
    // IImageBuffer
    ssize_t addIImageBuffer(const DualCamMFBufferID& key, const sp<IImageBuffer>& value);
    ssize_t delIImageBuffer(const DualCamMFBufferID& key);
    const sp<IImageBuffer> getIImageBuffer(const DualCamMFBufferID& key);
    const IImageBufferSet& getIImageBufferSet(){return mvIImageBufData;};
    // SmartImageBuffer
    ssize_t addSmartBuffer(const DualCamMFBufferID& key, const SmartImageBuffer& value);
    ssize_t delSmartBuffer(const DualCamMFBufferID& key);
    const SmartImageBuffer getSmartBuffer(const DualCamMFBufferID& key);
    const SmartImageBufferSet& getSmartBufferSet(){return mvSmartImgBufData;};
    // GraphicBuffer
    ssize_t addGraphicBuffer(const DualCamMFBufferID& key, const SmartGraphicBuffer& value);
    ssize_t delGraphicBuffer(const DualCamMFBufferID& key);
    const SmartGraphicBuffer getGraphicBuffer(const DualCamMFBufferID& key);
    const GraphicImageBufferSet& getGraphicBufferSet(){return mvGraImgBufData;};
    // MfllBuffer
    ssize_t addMfllBuffer(const DualCamMFBufferID& key, const sp<mfll::IMfllImageBuffer>& value);
    ssize_t delMfllBuffer(const DualCamMFBufferID& key);
    const sp<mfll::IMfllImageBuffer> getMfllBuffer(const DualCamMFBufferID& key);
    const MfllImageBufferSet& getMfllBufferSet(){return mvMfllImgBufData;};
    // Metadata pointer
    ssize_t addMetadata(const DualCamMFBufferID& key, IMetadata* const value);
    ssize_t delMetadata(const DualCamMFBufferID& key);
    IMetadata* getMetadata(const DualCamMFBufferID& key);
    // Metadata
    ssize_t addSolidMetadata(const DualCamMFBufferID& key, IMetadata const value);
    ssize_t delSolidMetadata(const DualCamMFBufferID& key);
    IMetadata getSolidMetadata(const DualCamMFBufferID& key);
    // ExtBufName
    ssize_t addExtBufName(const DualCamMFBufferID& key, const string& value);
    bool getExtBufName(const DualCamMFBufferID& key, string& rString);

    MVOID clear() {
        mvIImageBufData.clear();
        mvSmartImgBufData.clear();
        mvGraImgBufData.clear();
        mvMetaPtrData.clear();
        mvMetaData.clear();
        mvExtNameSet.clear();
    }
    sp<PipeRequest> getRequestPtr() { return mpReqPtr;}

    MINT32 getRequestNo(){return mpReqPtr->getRequestNo();}

    void setDump(MBOOL _enableDump){mEnableDump = _enableDump;};
    MBOOL shouldDump(){return mEnableDump;};

private:
    IImageBufferSet mvIImageBufData;
    SmartImageBufferSet mvSmartImgBufData;
    GraphicImageBufferSet mvGraImgBufData;
    MfllImageBufferSet mvMfllImgBufData;
    MetadataPtrSet mvMetaPtrData;
    MetadataSet mvMetaData;
    ExtBufNameSet mvExtNameSet;

    sp<PipeRequest> mpReqPtr;

    MBOOL mEnableDump = MTRUE;
};

class RequestDumpHelper
{
public:
    RequestDumpHelper()
    {
        mDumpStartIdx = ::property_get_int32(STRING_DUMP_START, 0);
        mDumpBufSize =  ::property_get_int32(STRING_DUMP_SIZE, 0);
        mIsDoPorter =   ::property_get_int32(STRING_DO_PORTER, 0);

        CAM_LOGD("startIndex: %d, dumpBufSize: %d, isDoPorter: %d", mDumpStartIdx, mDumpBufSize, mIsDoPorter);
    }

    bool isDoDump(MUINT requestID)
    {
        return (requestID >= mDumpStartIdx) && (requestID < (mDumpStartIdx + mDumpBufSize));
    }

    bool isDoDump(const PipeRequest& pipeRequest)
    {
        return isDoDump(pipeRequest.getRequestNo());
    }

    bool isDoPorter()
    {
        return mIsDoPorter;
    }

private:
    MUINT mDumpStartIdx;
    MUINT mDumpBufSize;
    MUINT mIsDoPorter;
};

class ScopeLogger
{
public:
    ScopeLogger(const char* text1, const char* text2 = nullptr, int idx = -1);
    ~ScopeLogger();
private:
    const char* mText1 = nullptr;
    const char* mText2 = nullptr;
    int mIdx = -1;
};

struct SimpleTimer
{
public:
    SimpleTimer();
    SimpleTimer(bool bStart);
    /* start timer */
    MBOOL startTimer();
    /* cpunt the timer, return elaspsed time from start timer. */
    float countTimer();
public:
    std::chrono::time_point<std::chrono::system_clock> start;
};

/*******************************************************************************
* utility macro definition
********************************************************************************/
#define RETRIEVE_OFRAMEINFO(request, rFrameInfo, BufID) \
    if (request->vOutputFrameInfo.indexOfKey(BufID) >= 0) \
    {\
        rFrameInfo = request->vOutputFrameInfo.valueFor(BufID);\
    }
#define RETRIEVE_OFRAMEINFO_IMGBUF_WITH_ERROR(request, rFrameInfo, BufID, rImgBuf) \
    if (request->vOutputFrameInfo.indexOfKey(BufID) >= 0) \
    { \
        rFrameInfo = request->vOutputFrameInfo.valueFor(BufID);\
        rFrameInfo->getFrameBuffer(rImgBuf); \
    }\
    else\
    {\
        CAM_LOGE("Cannot find the frameBuffer in the effect request, frameID=%d!", BufID); \
    }

/*******************************************************************************
*
********************************************************************************/
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
    IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        CAM_LOGW("pMetadata == NULL");
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
        CAM_LOGW("pMetadata == NULL");
        return;
    }
    //
    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}

template <typename T>
inline MVOID
updateEntry(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        CAM_LOGW("pMetadata == NULL");
        return;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}
/*******************************************************************************
* Namespace end.
********************************************************************************/
};
};
};
};

#endif