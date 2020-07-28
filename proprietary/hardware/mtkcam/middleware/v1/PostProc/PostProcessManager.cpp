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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#define LOG_TAG "MtkCam/PostProcessManager"
//
#include <sys/prctl.h>
#include <sys/resource.h>
//
#include <sys/time.h>
//
#include <utils/List.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>
#include <utils/Thread.h>
//
#include <mtkcam/def/BuiltinTypes.h>
#include <mtkcam/def/Errors.h>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Misc.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/PostProc/IPostProc.h>
//
#include <mtkcam/feature/effectHalBase/ICallbackClientMgr.h>
// rlim
#include <dirent.h>
#include <sys/time.h>
#include <sys/resource.h>
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define FUNCTION_LOG_START          MY_LOGD_IF(1<=1, " +");
#define FUNCTION_LOG_END            MY_LOGD_IF(1<=1, " -");

#define POSTPROCESSOR_POLICY     (SCHED_OTHER)
#define POSTPROCESSOR_PRIORITY   (0)

#define TOLERANCE_COUNT (50)

using namespace std;
using namespace NSCam;
using namespace android::NSPostProc;
using namespace android;

class PostProcImageCallback
      : public IPostProcImageCallback
{
public:
    PostProcImageCallback(IImagePostProcessManager *p) : mpPostProcManager(p) {}
    virtual ~PostProcImageCallback(){}

public:

    bool onCB_CompressedImage_packed(
                    PostProcessorType  processType,
                    int64_t const      i8Timestamp,
                    uint32_t const     u4BitstreamSize,
                    uint8_t const*     puBitstreamBuf,
                    uint32_t const     u4CallbackIndex,
                    bool     const     fgIsFinalImage,
                    uint32_t const     msgType
                )
    {
        return (mpPostProcManager != NULL) ? mpPostProcManager->onCB_CompressedImage_packed(
                                                                processType,
                                                                i8Timestamp,
                                                                u4BitstreamSize,
                                                                puBitstreamBuf,
                                                                u4CallbackIndex,
                                                                fgIsFinalImage,
                                                                msgType) : false;
    }

private:
  IImagePostProcessManager *mpPostProcManager;
};

class ImagePostProcessManager : public IImagePostProcessManager
{
static const int MAX_POSTPROC_REQUSET_CNT = 10;

struct PostProcRequestInfo
{
  MINT32                         requestNo;
  MINT64                         timeStamp;
  wp<IPostProcRequestCB>         pCb;
  MBOOL                          isFinalImage;
};

public:
    ImagePostProcessManager();
    virtual ~ImagePostProcessManager();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief add processor to post process manager.
     * @return The process result
     */
    virtual android::status_t                  addProcesssor(PostProcessorType type, android::sp<IImagePostProcessor> pProcessor) override;
    /**
     * @brief remove processor to post process manager.
     * @return The process result
     */
    virtual android::status_t                  clearProcesssor(PostProcessorType type = PostProcessorType::NONE) override;
    /**
     * @brief enque to post processor.
     * @return The process result
     */
    virtual android::status_t                  enque(android::sp<ImagePostProcessData> data) override;
    /**
     * @brief flush post processor queue.
     * @return The process result
     */
    virtual android::status_t                  flush() override;
    /**
     * @brief wait all processing done.
     * @return The process result
     */
    virtual android::status_t                  waitUntilDrained() override;
    /**
     * @brief destroy all member field.
     * @return The process result
     */
    virtual android::status_t                  destroy() override;
    /**
     * @brief get queue size
     * @return return size
     */
    virtual MINT32                             size() override;
    /**
     * @brief check background service is available or not.
     * @return The check result
     */
     virtual MBOOL                             isAvailable() override;
    /**
     * @brief set IShotCallback while turn-off fast s2s
     * @return The process result
     */
    virtual android::status_t                  setShotCallback(sp<IShotCallback> pShotCallback) override;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  IPostProcImageCallback Interface.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    virtual bool                               onCB_CompressedImage_packed(
                                                    PostProcessorType processType,
                                                    int64_t const     i8Timestamp,
                                                    uint32_t const    u4BitstreamSize,
                                                    uint8_t const*    puBitstreamBuf,
                                                    uint32_t const    u4CallbackIndex = 0,
                                                    bool              fgIsFinalImage = true,
                                                    uint32_t const    msgType = MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE
                                               ) override;
public:
    android::status_t                  onDequeData(android::sp<ImagePostProcessData>& data);
    android::status_t                  onProcessData(android::sp<ImagePostProcessData> const& data);
    MVOID                              dump();
private:
    int                                get_fd_count();
private:
    wp<IShotCallback>                 mpShotCallback;
    sp<IPostProcImageCallback>        mpPostProcImageCallback;
    MBOOL                             mbExit = MFALSE;
    MBOOL                             mbDisableFastS2S = MFALSE;

    mutable Mutex                     mProcessorLock;
    android::DefaultKeyedVector<PostProcessorType, android::sp<IImagePostProcessor>> mvImagePostProcessor;


    mutable Mutex                     mQueueLock;
    mutable Condition                 mQueueCond;
    android::List<android::sp<ImagePostProcessData> > mQueue;

    // check if request done
    mutable Mutex                     mRequestLock;
    DefaultKeyedVector<MINT64, PostProcRequestInfo> mvPostProcRequests;
//    DefaultKeyedVector<MINT64, PostProcRequestInfo> mvRemoveRequests;
    rlim_t                            mFDLimitCount = 0;

/*******************************************************************************
* Class Define
********************************************************************************/
protected:
    class ProcessThread : public Thread
    {
        public:

                                        ProcessThread(ImagePostProcessManager* pProcessImp)
                                            : mpProcessImp(pProcessImp)
                                        {}

                                        ~ProcessThread()
                                        {}

        public:

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  Thread Interface.
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

        public:
                        // Ask this object's thread to exit. This function is asynchronous, when the
                        // function returns the thread might still be running. Of course, this
                        // function can be called from a different thread.
                        void        requestExit() override;

                        // Good place to do one-time initializations
                        status_t    readyToRun() override;

        private:
                        // Derived class must implement threadLoop(). The thread starts its life
                        // here. There are two ways of using the Thread object:
                        // 1) loop: if threadLoop() returns true, it will be called again if
                        //          requestExit() wasn't called.
                        // 2) once: if threadLoop() returns false, the thread will exit upon return.
                        bool        threadLoop() override;

        private:

                        ImagePostProcessManager*       mpProcessImp = nullptr;
    };
    sp<ProcessThread>             mpProcessThread = nullptr;
};

/******************************************************************************
 *
 ******************************************************************************/
IImagePostProcessManager *
IImagePostProcessManager::
getInstance()
{
    static ImagePostProcessManager gImgPostProcMgr;
    return &gImgPostProcMgr;
}
/******************************************************************************
 *
 ******************************************************************************/
void
IImagePostProcessManager::
destroyPostProcessor()
{
    // clear all processor
    IImagePostProcessManager::getInstance()->destroy();
}
/******************************************************************************
 *
 ******************************************************************************/
ImagePostProcessManager::
ImagePostProcessManager()
{
    if(mpProcessThread == nullptr)
    {
      mpProcessThread = new ProcessThread(this);
      if(mpProcessThread->run(LOG_TAG) != OK)
      {
          MY_LOGE("create post process thread fail.");
          return;
      }
    }
    ICallbackClientMgr::getInstance()->registerCB(destroyPostProcessor);

    mpPostProcImageCallback = new PostProcImageCallback(this);
    // get current process limit count
    struct rlimit rlim;
    getrlimit(RLIMIT_NOFILE, &rlim);
    mFDLimitCount = rlim.rlim_cur;
    MY_LOGD("max fd count is: %d", mFDLimitCount);
}
/******************************************************************************
 *
 ******************************************************************************/
ImagePostProcessManager::
~ImagePostProcessManager()
{
    FUNCTION_LOG_START
    destroy();
    if(mpProcessThread != nullptr)
    {
        mpProcessThread->requestExit();
        mQueueCond.signal();
        mpProcessThread->join();
    }
    mvPostProcRequests.clear();
//    mvRemoveRequests.clear();
    FUNCTION_LOG_END
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ImagePostProcessManager::
addProcesssor(
    PostProcessorType type,
    android::sp<IImagePostProcessor> pProcessor
)
{
    FUNCTION_LOG_START
    MY_LOGD("add type %d", type);
    Mutex::Autolock _l(mProcessorLock);
    ssize_t idx = mvImagePostProcessor.indexOfKey(type);
    if (idx < 0)
    {
        mvImagePostProcessor.add(type, pProcessor);
    }
    else
    {
        MY_LOGD("type %d is inside", type);
    }
    FUNCTION_LOG_END
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ImagePostProcessManager::
clearProcesssor(PostProcessorType type)
{
    FUNCTION_LOG_START
    MY_LOGD("clear type %d", type);
    Mutex::Autolock _l(mProcessorLock);
    if (type == PostProcessorType::NONE)
    {
        mvImagePostProcessor.clear();
        MY_LOGD("clear all processor");
    }
    else
    {
        ssize_t idx = mvImagePostProcessor.indexOfKey(type);
        if (idx >= 0)
        {
            mvImagePostProcessor.removeItem(type);
        }
        else
        {
            MY_LOGW("not found type %d", type);
        }
    }
    MY_LOGD("size %zu", mvImagePostProcessor.size());
    FUNCTION_LOG_END
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ImagePostProcessManager::
enque(android::sp<ImagePostProcessData> data)
{
    Mutex::Autolock _l(mQueueLock);
    mQueue.push_back(data);
    mQueueCond.signal();
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ImagePostProcessManager::
flush()
{
    FUNCTION_LOG_START
    {
        Mutex::Autolock _l(mQueueLock);
        mQueue.clear();
    }
    {
        Mutex::Autolock _l(mProcessorLock);

        for(MUINT32 i = 0 ; i < mvImagePostProcessor.size() ; i++)
        {
            sp<IImagePostProcessor> pImagePostProc = mvImagePostProcessor.valueAt(i);
            pImagePostProc->flush();
        }
    }
    FUNCTION_LOG_END
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ImagePostProcessManager::
waitUntilDrained()
{
    FUNCTION_LOG_START
    Mutex::Autolock _l(mProcessorLock);

    for(MUINT32 i = 0 ; i < mvImagePostProcessor.size() ; i++)
    {
        sp<IImagePostProcessor> pImagePostProc = mvImagePostProcessor.valueAt(i);
        pImagePostProc->waitUntilDrained();
    }
    FUNCTION_LOG_END
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ImagePostProcessManager::
destroy()
{
    FUNCTION_LOG_START
    flush();
    waitUntilDrained();
    clearProcesssor();

    {
        Mutex::Autolock _l(mRequestLock);
        mvPostProcRequests.clear();
//        mvRemoveRequests.clear();
    }
    FUNCTION_LOG_END
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ImagePostProcessManager::
size()
{
    FUNCTION_LOG_START
    MINT32 size1, size2;
    {
        Mutex::Autolock _l(mQueueLock);
        size1 = mQueue.size();
    }
    {
        Mutex::Autolock _l(mRequestLock);
        size2 = mvPostProcRequests.size();
    }
    MY_LOGD("size: Queue(%d), OnProcess(%d)", size1, size2);
    return (size1+size2);
    FUNCTION_LOG_END
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ImagePostProcessManager::
isAvailable()
{
    MY_LOGD("cur_fd(%d), max(%d), count(%d)", get_fd_count(), mFDLimitCount, get_fd_count()+TOLERANCE_COUNT);
    if((get_fd_count() + TOLERANCE_COUNT) >= mFDLimitCount)
    {
        return MFALSE;
    }
    else
    {
        return MTRUE;
    }
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ImagePostProcessManager::
setShotCallback(
    sp<IShotCallback> pShotCallback
)
{
    FUNCTION_LOG_START
    mpShotCallback = pShotCallback;
    mbDisableFastS2S = MTRUE;
    FUNCTION_LOG_END
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
ImagePostProcessManager::
onCB_CompressedImage_packed(
    PostProcessorType processType,
    int64_t const     i8Timestamp,
    uint32_t const    u4BitstreamSize,
    uint8_t const*    puBitstreamBuf,
    uint32_t const    u4CallbackIndex,
    bool              fgIsFinalImage,
    uint32_t const    msgType
)
{
    FUNCTION_LOG_START
    if (puBitstreamBuf != NULL)
    {
        if (mbDisableFastS2S)
        {
            sp<IShotCallback> pCB = mpShotCallback.promote();
            if (pCB != nullptr)
            {
                pCB->onCB_CompressedImage_packed(
                                i8Timestamp,
                                u4BitstreamSize,
                                puBitstreamBuf,
                                u4CallbackIndex,
                                fgIsFinalImage,
                                msgType);
            }
            else
            {
                MY_LOGW("mpShotCallback is nullptr!!");
            }
        }
        else
        {
            ICallbackClientMgr::getInstance()->onCB_CompressedImage_packed(
                                                    i8Timestamp,
                                                    u4BitstreamSize,
                                                    puBitstreamBuf,
                                                    u4CallbackIndex,
                                                    fgIsFinalImage,
                                                    msgType,
                                                    MTK_CAMERA_CALLBACK_CLIENT_ID_DUAL);
        }
    }
    else
    {
        MY_LOGW("type %d, receive NULL Image!!", processType);
    }

    // callback to middleware if needed
    if (fgIsFinalImage)
    {
        PostProcRequestInfo info;
        ssize_t idx;
        {
            Mutex::Autolock _l(mRequestLock);
            MY_LOGD("mvPostProcRequests size %zu", mvPostProcRequests.size());
            idx = mvPostProcRequests.indexOfKey(i8Timestamp);
            if (__builtin_expect( idx < 0, false) ) {
                MY_LOGE("mvPostProcRequests.indexOfKey(%" PRId64 ") returns %zd", i8Timestamp, idx);
            }
            else {
                info = mvPostProcRequests.valueAt(idx);
            }
            mvPostProcRequests.removeItem(i8Timestamp);
        }
        if (idx >= 0)
        {
            sp<IPostProcRequestCB> pCB = info.pCb.promote();
            if (pCB != nullptr)
            {
                pCB->onPostProcEvent(info.requestNo, PostProcRequestCB::POSTPROC_DONE, 0, 0);
            }
            mQueueCond.signal();
        }
        else
        {
            MY_LOGW("can't find timestamp %" PRId64 " in mvPostProcRequests", i8Timestamp);
            dump();
        }
    }
    FUNCTION_LOG_END
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ImagePostProcessManager::
onDequeData(
    android::sp<ImagePostProcessData>& data
)
{
    MBOOL bPostProcAvail;
    {
        Mutex::Autolock _l(mRequestLock);
        bPostProcAvail = mvPostProcRequests.size() < MAX_POSTPROC_REQUSET_CNT;
    }
    Mutex::Autolock _l(mQueueLock);
    //  Wait until the queue is not empty or not going exit
    while ((mQueue.empty() || !bPostProcAvail) && !mbExit)
    {
        status_t status = mQueueCond.wait(mQueueLock);
        if  ( OK != status ) {
            MY_LOGW(
                "wait status:%d:%s, mQueue.size:%zu",
                status, ::strerror(-status), mQueue.size()
            );
        }
        {
            Mutex::Autolock _l(mRequestLock);
            bPostProcAvail = mvPostProcRequests.size() < MAX_POSTPROC_REQUSET_CNT;
            if (!bPostProcAvail && !mQueue.empty())
            {
                MY_LOGD("Wait PostProc size(%zu), Queue size(%zu)", mvPostProcRequests.size(), mQueue.size());
            }
        }
    }
    //
    if  ( mbExit ) {
        MY_LOGW_IF(!mQueue.empty(), "[flush] mQueue.size:%zu", mQueue.size());
        return DEAD_OBJECT;
    }

    if (!mQueue.empty() && bPostProcAvail)
    {
        data = *mQueue.begin();
        mQueue.erase(mQueue.begin());
        MY_LOGD("Queue size %zu", mQueue.size());
    }
    else
    {
        MY_LOGE("shouldn't go here!!");
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ImagePostProcessManager::
onProcessData(
    android::sp<ImagePostProcessData> const& data
)
{
    FUNCTION_LOG_START
    Mutex::Autolock _l(mProcessorLock);
    // thirdParty uses the same processor
    MBOOL bThirdParty = data->mProcessType == PostProcessorType::THIRDPARTY_MFNR
                     || data->mProcessType == PostProcessorType::THIRDPARTY_HDR
                     || data->mProcessType == PostProcessorType::THIRDPARTY_BOKEH;
    PostProcessorType type = bThirdParty ? PostProcessorType::THIRDPARTY_MFNR : data->mProcessType;
    sp<IImagePostProcessor> pPostProcessor = mvImagePostProcessor.valueFor(type);
    if(pPostProcessor == nullptr)
    {
        pPostProcessor = IImagePostProcessor::createInstance(data->mProcessType);
        if (pPostProcessor == nullptr)
        {
            MY_LOGE("can't create processor: type(%d)!!", data->mProcessType);
            return BAD_TYPE;
        }

        mvImagePostProcessor.add(type, pPostProcessor);
    }
    sp<PostProcRequestSetting> setting = new PostProcRequestSetting();
    setting->mProcessType       = data->mProcessType;
    setting->mRequestNo         = data->mRequestNo;
    setting->miTimeStamp        = data->miTimeStamp;
    setting->mvInputData        = data->mvInputData;
    setting->mShotParam         = data->mShotParam;
    setting->mJpegParam         = data->mJpegParam;
    setting->mpImageCallback    = mpPostProcImageCallback;
    setting->mIsFinalData       = data->mIsFinalData;

    MY_LOGD("---- before dump ----");
    setting->dump();
    MY_LOGD("---- after dump ----");

    // assign current timestamp
    if (setting->miTimeStamp == -1)
    {
        struct timespec t;
        t.tv_sec = t.tv_nsec = 0;
        clock_gettime(CLOCK_MONOTONIC, &t);
        setting->miTimeStamp = (t.tv_sec) * 1000000000LL + t.tv_nsec; // nano second
        MY_LOGD("assign current timestamp %" PRId64 " to requestNO %d", setting->miTimeStamp, setting->mRequestNo);
    }

    PostProcRequestInfo info;
    info.requestNo    = setting->mRequestNo;
    info.timeStamp    = setting->miTimeStamp;
    info.pCb          = data->mpCb;
    info.isFinalImage = MFALSE;
    {
        Mutex::Autolock _l(mRequestLock);
        mvPostProcRequests.add(setting->miTimeStamp, info);
    }
    DefaultKeyedVector<MINT32, android::sp<IImageBuffer>> vImage;
    size_t size = 0;
    auto it = setting->mvInputData.begin()->mImageQueue.begin();
    if (it != setting->mvInputData.begin()->mImageQueue.end())
    {
        vImage = *it;
        size = vImage.size();
    }
    MY_LOGD("Type[%d] RequestNO[%d] timestamp %" PRId64 " InputData[%zu], Img_Reqs[%zu] Img_Req_List[%zu]",
                setting->mProcessType,
                setting->mRequestNo,
                setting->miTimeStamp,
                setting->mvInputData.size(),
                setting->mvInputData.begin()->mImageQueue.size(),
                size);

    // callback N3D and LDC for internal vsdof
    if (setting->mProcessType == PostProcessorType::BOKEH)
    {
      #define CALLBACK_BUFFER(streamId, msgType) \
          do\
          {\
              ssize_t index = vImage.indexOfKey(streamId);\
              if(index >= 0)\
              {\
              sp<IImageBuffer> pBuffer = vImage.valueAt(index);\
              pBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);\
              MUINT32 u4Size = pBuffer->getBufSizeInBytes(0);\
              MY_LOGD("msgType %d, u4Size %d", msgType, u4Size);\
              uint8_t const* puBuf = (uint8_t const*)pBuffer->getBufVA(0);\
              ICallbackClientMgr::getInstance()->onCB_CompressedImage_packed(\
                                                      setting->miTimeStamp,\
                                                      u4Size,\
                                                      puBuf,\
                                                      msgType,\
                                                      0,\
                                                      msgType,\
                                                      MTK_CAMERA_CALLBACK_CLIENT_ID_DUAL);\
              pBuffer->unlockBuf(LOG_TAG);\
              }\
              else\
              {\
                  MY_LOGE("Should not happened. miss -" #streamId);\
              }\
          }while(0);
          CALLBACK_BUFFER(eSTREAMID_IMAGE_PIPE_STEREO_DBG_LDC, MTK_CAMERA_MSG_EXT_DATA_STEREO_LDC);
          CALLBACK_BUFFER(eSTREAMID_IMAGE_PIPE_STEREO_N3D_DEBUG, MTK_CAMERA_MSG_EXT_DATA_STEREO_N3D_DEBUG);
    }
    pPostProcessor->doPostProc(setting);
    FUNCTION_LOG_END
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
ImagePostProcessManager::
dump()
{
    //TODO
}
/******************************************************************************
 *
 ******************************************************************************/
int
ImagePostProcessManager::
get_fd_count()
{
    int fd_count = -1;
    char buf[64];
    struct dirent *dp = nullptr;

    snprintf(buf, 64, "/proc/%i/fd/", getpid());

    DIR *dir = opendir(buf);
    while ((dp = readdir(dir)) != NULL)
    {
          fd_count++;
    }
    closedir(dir);
    return fd_count;
}

/******************************************************************************
 *
 ******************************************************************************/
void
ImagePostProcessManager::
ProcessThread::
requestExit()
{
    FUNCTION_LOG_START;
    Thread::requestExit();
    mpProcessImp->mbExit = MTRUE;
    FUNCTION_LOG_END
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
ImagePostProcessManager::
ProcessThread::
readyToRun()
{
    //
    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    if (POSTPROCESSOR_POLICY == SCHED_OTHER) {
        sched_p.sched_priority = 0;
        ::sched_setscheduler(0, POSTPROCESSOR_POLICY, &sched_p);
        ::setpriority(PRIO_PROCESS, 0, POSTPROCESSOR_PRIORITY);   //  Note: "priority" is nice value.
    } else {
        sched_p.sched_priority = POSTPROCESSOR_PRIORITY;          //  Note: "priority" is real-time priority.
        ::sched_setscheduler(0, POSTPROCESSOR_POLICY, &sched_p);
    }

    MY_LOGD("tid(%d) policy(%d) priority(%d)", ::gettid(), POSTPROCESSOR_POLICY, POSTPROCESSOR_PRIORITY);

    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
ImagePostProcessManager::
ProcessThread::
threadLoop()
{
    android::sp<ImagePostProcessData> pIPPData = nullptr;
    if  (
            !exitPending()
        &&  OK == mpProcessImp->onDequeData(pIPPData)
        &&  pIPPData != 0
        )
    {
        mpProcessImp->onProcessData(pIPPData);
        return true;
    }

    MY_LOGD("exit unpack thread");
    return  false;
}
