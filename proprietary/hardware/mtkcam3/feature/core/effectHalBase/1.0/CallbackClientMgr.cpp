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

#define LOG_TAG "mmsdk/CallbackClientMgr"

#include <log/log.h>
#include <utils/Errors.h>
#include <mtkcam3/feature/effectHalBase/ICallbackClientMgr.h>
#include "common.h"

#include <cutils/compiler.h> // CC_LIKELY, CC_UNLIKELY
//use property_get
#include <cutils/properties.h>

#include <utils/List.h>
#include <mtkcam/def/PriorityDefs.h>
#include <mtkcam/utils/std/Format.h>
#include <sys/prctl.h>
#include <time.h>

#include <mutex>
#include <list>
#include <condition_variable> // std::condition_variable
#include <chrono>
// custom
#include <camera_custom_stereo.h>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_UTILITY);
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV(LOG_TAG "[%s] " fmt "\n", __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD(LOG_TAG "[%s] " fmt "\n", __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI(LOG_TAG "[%s] " fmt "\n", __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW(LOG_TAG "[%s] " fmt "\n", __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME(LOG_TAG "[%s] " fmt "\n", __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_ULOGM_ASSERT(0, LOG_TAG "[%s] " fmt "\n", __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_ULOGM_FATAL(LOG_TAG "[%s] " fmt "\n", __FUNCTION__, ##arg)
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

#define CALLBACK_RETRY_COUNT        0
#define WAIT_RETRY_TIMEOUT_MS       3000   // ms
using namespace std;
using namespace NSCam;
using namespace android;

/******************************************************************************
 *
 ******************************************************************************/
class CallbackClientMgr : public ICallbackClientMgr
{

public:
                             CallbackClientMgr() {mRetryCount = CALLBACK_RETRY_COUNT;}
    virtual                 ~CallbackClientMgr() {mlTimestamp.clear(); mlCbFunc.clear();}

    virtual MINT32           size();
    virtual MVOID            createCallbackClient(MINT64 timestamp);
    virtual MVOID            removeCallbackClient(MINT64 timestamp, MBOOL bCB, uint32_t const u4CbId);
    virtual MBOOL            canSupportBGService();
    virtual MBOOL            registerCB(CallbackFunc func, uint32_t const u4CbId);
    virtual MBOOL            registerBufferCB(BufferCBFunc func);
    virtual MBOOL            registerBufferCB(BufferPostviewCBFunc func);


// Calback API
    virtual bool              onCB_CompressedImage_packed(
                                  int64_t const   i8Timestamp                   __attribute__((unused)),
                                  uint32_t const  u4BitstreamSize               __attribute__((unused)),
                                  uint8_t const*  puBitstreamBuf                __attribute__((unused)),
                                  uint32_t const  u4CallbackIndex = 0,
                                  bool            fgIsFinalImage = true,
                                  uint32_t const  msgType = MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE,
                                  uint32_t const  u4CbId  = MTK_CAMERA_CALLBACK_CLIENT_ID_SINGLE
                              );
    virtual bool              onCB_PostviewClient(
                                  int64_t const             i8Timestamp,
                                  NSCam::IImageBuffer const *pPostviewBuf,
                                  uint32_t const  u4CbId  = MTK_CAMERA_CALLBACK_CLIENT_ID_SINGLE

                              );

    virtual bool              onCB_PostviewClient(
                                  int64_t  const i8Timestamp,
                                  uint32_t const planeCount,
                                  uint32_t const width,
                                  uint32_t const height,
                                  uint32_t const fmt,
                                  uint8_t* const u4Bitstream[],
                                  uint32_t const widthInBytes[],
                                  uint32_t const strideInBytes[],
                                  uint32_t const scanlines[],
                                  uint32_t const  u4CbId  = MTK_CAMERA_CALLBACK_CLIENT_ID_SINGLE

                              );

private:
    std::list<MINT64>                 mlTimestamp;        // timestamp to setCallbackClient
    std::mutex                        mMutex;             // mutex for callbackclient list
    std::condition_variable           mCondVar;
    std::list<std::pair<CallbackFunc, uint32_t>>  mlCbFunc; // <callback function, callback id>

    std::mutex                        mCBMutex;           // mutex for cb function
    std::mutex                        mbufferCBMutex;     // mutex for onCB_CompressedImage_packed
    BufferCBFunc                      mBufferCbFunc = nullptr;
    BufferPostviewCBFunc              mBufferPostviewCbFunc = nullptr;

    uint32_t                          mRetryCount;
};

/******************************************************************************
 *
 ******************************************************************************/
ICallbackClientMgr *
ICallbackClientMgr::
getInstance()
{
    static CallbackClientMgr gCbClientMgr;
    return &gCbClientMgr;
}

/******************************************************************************
 *
 ******************************************************************************/
int64_t
ICallbackClientMgr::
generateTimestamp()
{
    struct timespec t;
    ::memset(&t, 0x00, sizeof( struct timespec ));
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_nsec + t.tv_sec * 1000000000LL;
}

/******************************************************************************
 *
 ******************************************************************************/
size_t
ICallbackClientMgr::
getCustomizedBufOffset(uint32_t const  msgType)
{
    switch (msgType) {
        case CAMERA_MSG_COMPRESSED_IMAGE:
        case CAMERA_MSG_POSTVIEW_FRAME:
            return 0; // this value is a customized value.
        default:
            return 0;
    }
    return 0;
}

// Get formatted image format data
uint32_t
ICallbackClientMgr::
getFormatedImageFormat(EImageFormat const f)
{
    #define _MAKE_FORMAT_(a,b,c,d) ((char)(a) | ((char)(b) << 8) | ((char)(c) << 16) | ((char)(d) << 24))

    switch (f) {
    case eImgFmt_YUY2:      return _MAKE_FORMAT_('Y','U','Y','2');
    case eImgFmt_YV12:      return _MAKE_FORMAT_('Y','V','1','2');
    case eImgFmt_NV21:      return _MAKE_FORMAT_('N','V','2','1');
    case eImgFmt_NV12:      return _MAKE_FORMAT_('N','V','1','2');
    default:                ;
    }

    return 0;
    #undef  _MAKE_FORMAT_
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
CallbackClientMgr::
createCallbackClient(MINT64 timestamp)
{
    FUNCTION_LOG_START
    std::unique_lock <std::mutex> l(mMutex);
    mlTimestamp.push_front(timestamp);
    MY_LOGD("TimeStamp %" PRId64 " CallbackClientList size %zd", timestamp, mlTimestamp.size());
    mRetryCount = CALLBACK_RETRY_COUNT;
    mCondVar.notify_one();
    FUNCTION_LOG_END
    return;
}
/******************************************************************************
 *
 ******************************************************************************/
MINT32
CallbackClientMgr::
size()
{
    FUNCTION_LOG_START
    std::unique_lock <std::mutex> l(mMutex);
    MY_LOGD("CallbackClientList size %zd", mlTimestamp.size());
    FUNCTION_LOG_END
    return mlTimestamp.size();
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
CallbackClientMgr::
canSupportBGService()
{
    if (size() <= 0)
        return false;

    if ( ! ::canSupportBGService()) {
        return false;
    }

    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
CallbackClientMgr::
registerCB(CallbackFunc func, uint32_t const u4CbId)
{
    FUNCTION_LOG_START
    std::lock_guard <std::mutex> l(mCBMutex);
    if (!mlCbFunc.empty())
    {
        std::list<std::pair<CallbackFunc, uint32_t>>::iterator iterCbFunc = mlCbFunc.begin();
        for(;iterCbFunc != mlCbFunc.end();++iterCbFunc)
        {
            if ((*iterCbFunc).first == func)
            {
                MY_LOGD("func existed");
                return MTRUE;
            }
        }
    }
    mlCbFunc.push_front({func, u4CbId});
    MY_LOGD("mlCbFunc.size() %zd", mlCbFunc.size());
    FUNCTION_LOG_END
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
CallbackClientMgr::
registerBufferCB(BufferCBFunc func)
{
    FUNCTION_LOG_START
//    std::unique_lock <std::mutex> l(mCBMutex);
    mBufferCbFunc = func;
    FUNCTION_LOG_END
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
CallbackClientMgr::
registerBufferCB(BufferPostviewCBFunc func)
{
    FUNCTION_LOG_START
    mBufferPostviewCbFunc = func;
    FUNCTION_LOG_END
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
CallbackClientMgr::
removeCallbackClient(MINT64 timestamp, MBOOL bCB, uint32_t const u4CbId)
{
    FUNCTION_LOG_START
    {
        std::lock_guard <std::mutex> l(mMutex);
        if (!mlTimestamp.empty())
        {
            // erase elapsed CallbackClient
            std::list<MINT64>::iterator iter = mlTimestamp.begin();
            for(;iter != mlTimestamp.end();)
            {
                if (*iter == timestamp)
                {
                    iter   = mlTimestamp.erase(iter);
                    MY_LOGD("remove CallbackClient with timestamp = %" PRId64 ", size %zd", timestamp, mlTimestamp.size());
                    break;
                }
                else
                {
                    ++iter;
                }
            }
        }
        MY_LOGI("callbacbclientList %zd, bCB %d, CbId %d", mlTimestamp.size(), bCB, u4CbId);
    }

    if (bCB)
    {
        // destroy PostProc after CameraAP be destroyed
        std::lock_guard <std::mutex> l(mCBMutex);
        MY_LOGD("mlCbFunc.size() %zd", mlCbFunc.size());
        if (!mlCbFunc.empty())
        {
            std::list<std::pair<CallbackFunc, uint32_t>>::iterator iterCbFunc = mlCbFunc.begin();
            for(;iterCbFunc != mlCbFunc.end();++iterCbFunc)
            {
                if ((*iterCbFunc).second == u4CbId)
                {
                    MY_LOGD("Callback to remove PostProc");
                    (*iterCbFunc).first();
                }
            }
        }
    }
    FUNCTION_LOG_END
}
/******************************************************************************
 *
 ******************************************************************************/
bool
CallbackClientMgr::
onCB_CompressedImage_packed(
                            int64_t const   i8Timestamp,
                            uint32_t const  u4BitstreamSize,
                            uint8_t const*  puBitstreamBuf,
                            uint32_t const  u4CallbackIndex,
                            bool            fgIsFinalImage,
                            uint32_t const  msgType,
                            uint32_t const  u4CbId
                           )
{
    FUNCTION_LOG_START
    bool ret = false;
    std::lock_guard <std::mutex> l(mbufferCBMutex);

    MY_LOGD(
        "timestamp(%" PRId64 "), bitstream:size/buf=%d/%p, index(%d), IsFinalImage(%d), u4CbId(%d)",
        i8Timestamp, u4BitstreamSize, puBitstreamBuf, u4CallbackIndex, fgIsFinalImage, u4CbId
    );

    {
        std::lock_guard <std::mutex> lk(mMutex);
        if (mlTimestamp.empty() || mBufferCbFunc == nullptr)
        {
            MY_LOGE("CallbackClient list is empty!!!");
            return false;
        }
    }
    {
        // mapping MTK message to AOSP
        uint32_t _msgType =
            (msgType == MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE) ? CAMERA_MSG_COMPRESSED_IMAGE : msgType;

        // some message may need leading offset for store timestamp
        const uint32_t leadingOffset = getCustomizedBufOffset(_msgType);

//        uint32_t retryCount = mRetryCount;
        while(1)
        {
            ret = mBufferCbFunc(
                                  i8Timestamp,
                                  u4BitstreamSize,
                                  puBitstreamBuf,
                                  u4CallbackIndex,
                                  fgIsFinalImage,
                                  _msgType,
                                  leadingOffset,
                                  u4CbId
                               );
#if 0
            if (ret != true && retryCount > 0)
            {
                mRetryCount = 0;
                auto status = mCondVar.wait_for(lk, std::chrono::milliseconds(WAIT_RETRY_TIMEOUT_MS));
                if (CC_UNLIKELY( status == std::cv_status::timeout ))
                {
                    MY_LOGW("wait timeout");
                }
                retryCount--;
                MY_LOGD("Retry...");
                continue;
            }
#endif
            break;
        }
    }

    FUNCTION_LOG_END
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
CallbackClientMgr::
onCB_PostviewClient(
        int64_t const               i8Timestamp,
        NSCam::IImageBuffer const   *pPostviewBuf,
        uint32_t const              u4CbId
)
{
    FUNCTION_LOG_START
    bool ret = false;
    std::lock_guard <std::mutex> l(mbufferCBMutex);

    MY_LOGD(
        "timestamp(%" PRId64 "), imgfmt=0x%08x, WxH=%dx%d va=%" PRIxPTR ", u4CbId=%d",
        i8Timestamp, pPostviewBuf->getImgFormat(),
        pPostviewBuf->getImgSize().w, pPostviewBuf->getImgSize().h,
        pPostviewBuf->getBufVA(0), u4CbId
        );

    {
        std::lock_guard <std::mutex> lk(mMutex);
        if (mlTimestamp.empty() || mBufferPostviewCbFunc == nullptr)
        {
            MY_LOGE("CallbackClient list is empty!!!");
            return false;
        }
    }

    {
        const uint32_t leadingOffset =
            getCustomizedBufOffset(CAMERA_MSG_POSTVIEW_FRAME);

        uint32_t planeCount = (uint32_t)pPostviewBuf->getPlaneCount();
        uint32_t width      = (uint32_t)pPostviewBuf->getImgSize().w;
        uint32_t height     = (uint32_t)pPostviewBuf->getImgSize().h;
        uint8_t* buf[3]{0};
        uint32_t widthInBytes[3]{0};
        uint32_t strideInBytes[3]{0};
        uint32_t scanlines[3]{0};

        // prepare image information
        auto fmt = pPostviewBuf->getImgFormat();
        auto w   = pPostviewBuf->getImgSize().w;
        auto h   = pPostviewBuf->getImgSize().h;

        // retrieve information of image buffer
        for (size_t i = 0; i < planeCount; ++i)
        {
            using namespace NSCam::Utils::Format;

            buf[i]           = (uint8_t*)(void*)pPostviewBuf->getBufVA(i);
            widthInBytes[i]  = (queryPlaneWidthInPixels(fmt, i, w) * queryPlaneBitsPerPixel(fmt, i)) / 8;
            strideInBytes[i] = pPostviewBuf->getBufStridesInBytes(i);
            scanlines[i]     = queryPlaneHeightInPixels(fmt, i, h);
        }

        ret = mBufferPostviewCbFunc(
                i8Timestamp,
                planeCount,
                width,
                height,
                ICallbackClientMgr::getFormatedImageFormat((EImageFormat)fmt),
                buf,
                widthInBytes,
                strideInBytes,
                scanlines,
                leadingOffset,
                u4CbId
                );
    }

    FUNCTION_LOG_END
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
CallbackClientMgr::
onCB_PostviewClient(
    int64_t const  i8Timestamp,
    uint32_t const planeCount,
    uint32_t const width,
    uint32_t const height,
    uint32_t const fmt,
    uint8_t* const u4Bitstream[],
    uint32_t const widthInBytes[],
    uint32_t const strideInBytes[],
    uint32_t const scanlines[],
    uint32_t const u4CbId
)
{
    FUNCTION_LOG_START
    bool ret = false;
    std::lock_guard <std::mutex> l(mbufferCBMutex);

    MY_LOGD("timestamp(%" PRId64 ") planeCount=%u", i8Timestamp, planeCount);
    for (size_t i = 0; i < planeCount; i++)
    {
        MY_LOGD("plane:%zu,addr=%p,widthInByte=%u,strideInBytes=%u,scanlines=%u",
            i, u4Bitstream[i], widthInBytes[i], strideInBytes[i], scanlines[i]);
    }

    {
        std::lock_guard <std::mutex> lk(mMutex);
        if (mlTimestamp.empty() || mBufferPostviewCbFunc == nullptr)
        {
            MY_LOGE("CallbackClient list is empty!!!");
            return false;
        }
    }

    {
        uint32_t leadingOffset =
            getCustomizedBufOffset(CAMERA_MSG_POSTVIEW_FRAME);

        ret = mBufferPostviewCbFunc(
                i8Timestamp,
                planeCount,
                width,
                height,
                fmt,
                u4Bitstream,
                widthInBytes,
                strideInBytes,
                scanlines,
                leadingOffset,
                u4CbId
                           );
    }

    FUNCTION_LOG_END
    return ret;
}
