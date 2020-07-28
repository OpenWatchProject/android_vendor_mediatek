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

/**
 * @file BMDeNoise_common.h
 * @brief This is the common header for bayer+white de-noise feature pipe
*/

#ifndef _MTK_CAMERA_FEATURE_PIPE_BMDeNoise_PIPE_BMDeNoise_COMMON_H_
#define _MTK_CAMERA_FEATURE_PIPE_BMDeNoise_PIPE_BMDeNoise_COMMON_H_

// Standard C header file
#include <ctime>
#include <chrono>

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
#include <mtkcam/feature/stereo/pipe/IBMDeNoisePipe.h>
#include <mtkcam/drv/iopipe/PortMap.h>

// Module header file
#define PIPE_MODULE_TAG "BMDeNoisePipe"
#define PIPE_CLASS_TAG "Common"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG

#include <featurePipe/core/include/PipeLog.h>
#include <featurePipe/core/include/ImageBufferPool.h>
#include <featurePipe/core/include/GraphicBufferPool.h>
#include <featurePipe/vsdof/util/TuningBufferPool.h>
#include <featurePipe/vsdof/util/P2Operator.h>

#include <vsdof/hal/ProfileUtil.h>

// Local header file

// ==========================  logging  section ============================= //
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define VSDOF_LOGD(fmt, arg...)     do { if(this->mbDebugLog) {MY_LOGD("%d: " fmt, __LINE__, ##arg);} } while(0)

using android::String8;
using android::KeyedVector;
using namespace android;
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

using NSCam::NSIoPipe::PORT_DEPI;
using NSCam::NSIoPipe::PORT_DMGI;
using NSCam::NSIoPipe::PORT_IMGI;
using NSCam::NSIoPipe::PORT_IMG2O;
using NSCam::NSIoPipe::PORT_IMG3O;
using NSCam::NSIoPipe::PORT_WDMAO;
using NSCam::NSIoPipe::PORT_WROTO;
using NSCam::NSIoPipe::PORT_FEO;
using NSCam::NSIoPipe::PORT_MFBO;

/*******************************************************************************
* Enum Define
********************************************************************************/
enum eFrameIOType
{
    FRAME_INPUT,
    FRAME_OUTPUT
};
/*******************************************************************************
* Structure Define
********************************************************************************/
class ImageBufInfoMap;
typedef android::sp<ImageBufInfoMap> ImgInfoMapPtr;
typedef KeyedVector<BMDeNoiseBufferID, sp<IImageBuffer> > IImageBufferSet;
typedef KeyedVector<BMDeNoiseBufferID, SmartImageBuffer> SmartImageBufferSet;
typedef KeyedVector<BMDeNoiseBufferID, SmartGraphicBuffer> GraphicImageBufferSet;
typedef KeyedVector<BMDeNoiseBufferID, IMetadata* > MetadataSet;

class ImageBufInfoMap: public LightRefBase<ImageBufInfoMap>
{
public:
    ImageBufInfoMap(sp<PipeRequest> ptr);
    virtual ~ImageBufInfoMap() {clear();}
    // IImageBuffer
    ssize_t addIImageBuffer(const BMDeNoiseBufferID& key, const sp<IImageBuffer>& value);
    ssize_t delIImageBuffer(const BMDeNoiseBufferID& key);
    const sp<IImageBuffer>& getIImageBuffer(const BMDeNoiseBufferID& key);
    const IImageBufferSet& getIImageBufferSet(){return mvIImageBufData;};
    // SmartImageBuffer
    ssize_t addSmartBuffer(const BMDeNoiseBufferID& key, const SmartImageBuffer& value);
    ssize_t delSmartBuffer(const BMDeNoiseBufferID& key);
    const SmartImageBuffer& getSmartBuffer(const BMDeNoiseBufferID& key);
    const SmartImageBufferSet& getSmartBufferSet(){return mvSmartImgBufData;};
    // GraphicBuffer
    ssize_t addGraphicBuffer(const BMDeNoiseBufferID& key, const SmartGraphicBuffer& value);
    ssize_t delGraphicBuffer(const BMDeNoiseBufferID& key);
    const SmartGraphicBuffer& getGraphicBuffer(const BMDeNoiseBufferID& key);
    const GraphicImageBufferSet& getGraphicBufferSet(){return mvGraImgBufData;};
    // Metadata
    ssize_t addMetadata(const BMDeNoiseBufferID& key, IMetadata* const value);
    ssize_t delMetadata(const BMDeNoiseBufferID& key);
    IMetadata* getMetadata(const BMDeNoiseBufferID& key);

    MVOID clear() {mvIImageBufData.clear(); mvSmartImgBufData.clear(); mvGraImgBufData.clear();mvMetaData.clear();}
    sp<PipeRequest> getRequestPtr() { return mpReqPtr;}

    MINT32 getRequestNo(){return mpReqPtr->getRequestNo();}

private:
    IImageBufferSet mvIImageBufData;
    SmartImageBufferSet mvSmartImgBufData;
    GraphicImageBufferSet mvGraImgBufData;
    MetadataSet mvMetaData;

    sp<PipeRequest> mpReqPtr;
};

class ScopeLogger
{
public:
    ScopeLogger(const char* text1, const char* text2 = nullptr);
    ~ScopeLogger();
private:
    const char* mText1 = nullptr;
    const char* mText2 = nullptr;
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
        MY_LOGE("Cannot find the frameBuffer in the effect request, frameID=%d!", BufID); \
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
        MY_LOGW("pMetadata == NULL");
        return;
    }
    //
    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}

template <typename T>
MBOOL
tryGetMetadataInRequest(
    sp<PipeRequest> pPipeReq,
    eFrameIOType type,
    BMDeNoiseBufferID bid,
    MUINT32 const tag,
    T & rVal)
{
    IMetadata* pMeta = pPipeReq->getMetadata(bid);

    if(pMeta == nullptr){
        MY_LOGE("no such metadata!");
        return MFALSE;
    }

    if( !tryGetMetadata<T>(pMeta, tag, rVal) ) {
        MY_LOGE("no such metadata tag!");
        return MFALSE;
    }

    return MTRUE;
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
        MY_LOGW("pMetadata == NULL");
        return;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}

enum tuning_tag
{
    tuning_tag_G2G = 0,
    tuning_tag_G2C,
    tuning_tag_GGM,
    tuning_tag_UDM,
};
/*******************************************************************************
* Namespace end.
********************************************************************************/
};
};
};


#endif