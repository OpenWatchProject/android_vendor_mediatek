// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>

// Module header file

// Local header file
#include "BMBufferPool.h"

using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace NSCam::NSCamFeature::NSFeaturePipe::NSBMDN;
/*******************************************************************************
* Global Define
********************************************************************************/
#define PIPE_MODULE_TAG "BMDeNoise"
#define PIPE_CLASS_TAG "BufferPool"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG

#define MY_LOGD(fmt, arg...)                  CAM_LOGD("[%s][%s] " fmt, mName, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)                  CAM_LOGE("[%s][%s] " fmt, mName, __FUNCTION__, ##arg)

#define SCOPE_LOGGER          auto __scope_logger__ = create_scope_logger(__FUNCTION__, mName)
static std::shared_ptr<char> create_scope_logger(const char* functionName, const char* callerName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD("[%s][%s] + ", callerName, pText);
    return std::shared_ptr<char>(pText, [](char* p){ CAM_LOGD("[%s] -", p); });
}

#define SCOPE_TIMER(VAR, TEXT)  auto VAR = create_scope_timer(TEXT)
#include <memory>
#include <functional>
#include <utils/SystemClock.h> // elapsedRealtime(), uptimeMillis()
static std::unique_ptr<const char, std::function<void(const char*)> >
create_scope_timer(const char* text)
{
    auto t1 = android::elapsedRealtimeNano();
    return std::unique_ptr<const char, std::function<void(const char*)> >
        (
            text,
            [t1](const char* t)->void
            {
                auto t2 = android::elapsedRealtimeNano();
                CAM_LOGD("BMDeNoise_Profile %s --> duration(ms): %" PRId64 "", t, (t2 -t1)/1000000);
            }
        );
}
/*******************************************************************************
* External Function
********************************************************************************/



/*******************************************************************************
* Enum Define
********************************************************************************/




/*******************************************************************************
* Structure Define
********************************************************************************/






//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
BMBufferPool::
BMBufferPool(const char *name)
  : mName(name)
{
}

BMBufferPool::
~BMBufferPool()
{
}
/*******************************************************************************
* Public Functions
********************************************************************************/
MBOOL
BMBufferPool::
init(Vector<BufferConfig> bufConfig, MUINT32 tuningBufferCount)
{
    SCOPE_LOGGER;
    Mutex::Autolock _l(mPoolLock);

    if(mbInint){
        MY_LOGE("do init when it's already init!");
        return MFALSE;
    }

    mvImageBufSets.clear();
    mvGraphicBufSets.clear();

    for(auto e:bufConfig){

        SCOPE_TIMER(__t0, e.name);

        MY_LOGD("[%s(%d)] s:%dx%d f:%d u:%d single:%d g:%d max:%d",
            e.name, e.bufID,
            e.width, e.height, e.format, e.usage,
            e.isSingle, e.isGraphic,
            e.MaximumCount
        );

        if(mvImageBufSets.indexOfKey(e.bufID) >= 0){
            MY_LOGE("re-init the same image buf ID, should not happend!");
            return MFALSE;
        }

         if(mvGraphicBufSets.indexOfKey(e.bufID) >= 0){
            MY_LOGE("re-init the same graphic buf ID, should not happend!");
            return MFALSE;
        }

        if(!e.isGraphic)
        {
            android::sp<ImageBufferPool> pImagePool = ImageBufferPool::create(
            e.name,
            e.width, e.height,
            e.format, e.usage, e.isSingle
            );
            if(pImagePool == nullptr){
                MY_LOGE("create [%s] failed!", e.name);
                return MFALSE;
            }
            ImageBufferSet bufSet = {e, pImagePool, 0};
            mvImageBufSets.add(e.bufID, bufSet);
        }else
        {
            android::sp<GraphicBufferPool> pGraphicPool = GraphicBufferPool::create(
            e.name,
            e.width, e.height,
            getPixelFormat(e.format), e.usage
            );
            if(pGraphicPool == nullptr){
                MY_LOGE("create [%s] failed!", e.name);
                return MFALSE;
            }
            GraphicBufferSet bufSet = {e, pGraphicPool, 0};
            mvGraphicBufSets.add(e.bufID, bufSet);
        }
    }

    MY_LOGD("[%s] tuningBufferCount:%d", mName, tuningBufferCount);
    mpTuningBufferPool = TuningBufferPool::create(mName, NSIoPipe::NSPostProc::INormalStream::getRegTableSize());
    mpTuningBufferPool->allocate(tuningBufferCount);

    mbInint = MTRUE;
    return MTRUE;
}

MBOOL
BMBufferPool::
uninit()
{
    SCOPE_LOGGER;
    Mutex::Autolock _l(mPoolLock);

    SCOPE_TIMER(__t0, __FUNCTION__);

    if(!mbInint){
        MY_LOGE("do uninit when it's not init yet!");
        return MFALSE;
    }

    // image buffers release
    for(size_t i = 0 ; i < mvImageBufSets.size(); i ++){
        ImageBufferPool::destroy(mvImageBufSets.editValueAt(i).mImagePool);
    }
    mvImageBufSets.clear();

    // graphic buffers release
    for(size_t i = 0 ; i < mvGraphicBufSets.size(); i ++){
        GraphicBufferPool::destroy(mvGraphicBufSets.editValueAt(i).mGraphicPool);
    }
    mvGraphicBufSets.clear();

    // tuning buffers
    TuningBufferPool::destroy(mpTuningBufferPool);

    mbInint = MFALSE;
    return MTRUE;
}

MBOOL
BMBufferPool::
doAllocate(MINT32 count)
{
    SCOPE_LOGGER;

    {
        Mutex::Autolock _l(mPoolLock);
        if(!mbInint){
           MY_LOGE("doAllocate when it's not init yet!");
            return MFALSE;
        }
    }

    MBOOL ret = MFALSE;

    // image buffers
    for(size_t i = 0 ; i < mvImageBufSets.size(); i ++){
        if(mvImageBufSets.editValueAt(i).currentAllocateCount < mvImageBufSets.editValueAt(i).mBufConfig.MaximumCount){
            SCOPE_TIMER(__t0,mvImageBufSets.editValueAt(i).mBufConfig.name);
            mvImageBufSets.editValueAt(i).mImagePool->allocate(count);
            mvImageBufSets.editValueAt(i).currentAllocateCount ++;
        }else{
            MY_LOGD("already reach MaximumCount (%d)", mvImageBufSets.editValueAt(i).mBufConfig.MaximumCount);
        }

        MY_LOGD("[%s] current/max (%d/%d)",
            mvImageBufSets.editValueAt(i).mBufConfig.name,
            mvImageBufSets.editValueAt(i).currentAllocateCount,
            mvImageBufSets.editValueAt(i).mBufConfig.MaximumCount
        );

        // There is at least one buffer pool have not yet reached maximun count, return true.
        if(mvImageBufSets.editValueAt(i).currentAllocateCount < mvImageBufSets.editValueAt(i).mBufConfig.MaximumCount){
            ret = MTRUE;
        }
    }

    // Graphic buffers
    for(size_t i = 0 ; i < mvGraphicBufSets.size(); i ++){
        if(mvGraphicBufSets.editValueAt(i).currentAllocateCount < mvGraphicBufSets.editValueAt(i).mBufConfig.MaximumCount){
            SCOPE_TIMER(__t0,mvGraphicBufSets.editValueAt(i).mBufConfig.name);
            mvGraphicBufSets.editValueAt(i).mGraphicPool->allocate(count);
            mvGraphicBufSets.editValueAt(i).currentAllocateCount ++;
        }else{
            MY_LOGD("already reach MaximumCount (%d)", mvGraphicBufSets.editValueAt(i).mBufConfig.MaximumCount);
        }

        MY_LOGD("[%s] current/max (%d/%d)",
            mvGraphicBufSets.editValueAt(i).mBufConfig.name,
            mvGraphicBufSets.editValueAt(i).currentAllocateCount,
            mvGraphicBufSets.editValueAt(i).mBufConfig.MaximumCount
        );

        // There is at least one buffer pool have not yet reached maximun count, return true.
        if(mvGraphicBufSets.editValueAt(i).currentAllocateCount < mvGraphicBufSets.editValueAt(i).mBufConfig.MaximumCount){
            ret = MTRUE;
        }
    }

    MY_LOGD("ret:%d", ret);
    return ret;
}

android::sp<ImageBufferPool>
BMBufferPool::
getBufPool(BMDeNoiseBufferID bufID)
{
    // SCOPE_LOGGER;
    Mutex::Autolock _l(mPoolLock);

    if(mvImageBufSets.indexOfKey(bufID) >= 0){
        return mvImageBufSets.valueFor(bufID).mImagePool;
    }else{
        MY_LOGE("cant find bufferPool with id:%d!", bufID);
        return nullptr;
    }
}

android::sp<GraphicBufferPool>
BMBufferPool::
getGraphicBufPool(BMDeNoiseBufferID bufID)
{
    // SCOPE_LOGGER;
    Mutex::Autolock _l(mPoolLock);

    if(mvGraphicBufSets.indexOfKey(bufID) >= 0){
        return mvGraphicBufSets.valueFor(bufID).mGraphicPool;
    }else{
        MY_LOGE("cant find Graphic bufferPool with id:%d!", bufID);
        return nullptr;
    }
}

android::sp<TuningBufferPool>
BMBufferPool::
getTuningBufPool()
{
    // SCOPE_LOGGER;
    Mutex::Autolock _l(mPoolLock);

    if(mbInint){
        return mpTuningBufferPool;
    }else{
        MY_LOGE("cant getTuningBufPool before init!");
        return nullptr;
    }
}

android::PixelFormat
BMBufferPool::
getPixelFormat(EImageFormat eFmt)
{
    switch(eFmt)
    {
        case eImgFmt_RGBA8888:    return HAL_PIXEL_FORMAT_RGBA_8888;
        case eImgFmt_YV12:        return HAL_PIXEL_FORMAT_YV12;
        case eImgFmt_RAW16:       return HAL_PIXEL_FORMAT_RAW16;
        case eImgFmt_RAW_OPAQUE:  return HAL_PIXEL_FORMAT_RAW_OPAQUE;
        case eImgFmt_BLOB:        return HAL_PIXEL_FORMAT_BLOB;
        case eImgFmt_RGBX8888:    return HAL_PIXEL_FORMAT_RGBX_8888;
        case eImgFmt_RGB888:      return HAL_PIXEL_FORMAT_RGB_888;
        case eImgFmt_RGB565:      return HAL_PIXEL_FORMAT_RGB_565;
        case eImgFmt_BGRA8888:    return HAL_PIXEL_FORMAT_BGRA_8888;
        case eImgFmt_YUY2:        return HAL_PIXEL_FORMAT_YCbCr_422_I;
        case eImgFmt_I422:
        case eImgFmt_NV16:        return HAL_PIXEL_FORMAT_YCbCr_422_SP;
        case eImgFmt_NV21:        return HAL_PIXEL_FORMAT_YCrCb_420_SP;
        case eImgFmt_Y8:          return HAL_PIXEL_FORMAT_Y8;
        case eImgFmt_Y16:         return HAL_PIXEL_FORMAT_Y16;
        default:
            CAM_LOGE("unknow format:%d", eFmt);
            return HAL_PIXEL_FORMAT_Y8;
    };
}