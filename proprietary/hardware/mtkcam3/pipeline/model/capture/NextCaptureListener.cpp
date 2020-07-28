/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "mtkcam-NextCaptureListener"
//
#include "NextCaptureListener.h"
//
#include "MyUtils.h"
//
#include <impl/ResultUpdateHelper.h>
//
#include <mtkcam3/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam3/pipeline/utils/streambuf/StreamBuffers.h>
#include <mtkcam3/pipeline/hwnode/StreamId.h>
//
#include <mtkcam3/pipeline/prerelease/IPreReleaseRequest.h>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/sys/MemoryInfo.h>
//
/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::v3::pipeline::model;
using namespace NSCam::v3::Utils;
using namespace NSCam::v3::pipeline::prerelease;
typedef NSCam::v3::Utils::HalMetaStreamBuffer::Allocator  HalMetaStreamBufferAllocatorT;

#define TOLERANCE_COUNT (50)
#define MIN_FREE_MEM    (300000000)
#define TOLERANCE_IMGO_COUNT (0)
#define MAX_IN_FLIGHT_JPEG   (5)
/******************************************************************************
 *
 ******************************************************************************/
auto
INextCaptureListener::
createInstance(
    int32_t openId,
    std::string const& name,
    CtorParams  const& rCtorParams
) -> android::sp<INextCaptureListener>
{
    sp<NextCaptureListener> pInstance = new NextCaptureListener(openId, name, rCtorParams);
    if  ( CC_UNLIKELY(pInstance==nullptr) ) {
        MY_LOGE("create instance fail");
        return nullptr;
    }

    return pInstance;
}


/******************************************************************************
 *
 ******************************************************************************/
NextCaptureListener::
NextCaptureListener(int32_t openId, std::string const& name, INextCaptureListener::CtorParams const& rCtorParams)
    : mOpenId(openId)
    , mUserName(name)
    , mLogLevel(0)
    , mMaxJpegNum(rCtorParams.maxJpegNum)
    , mMaxImgoNum(rCtorParams.maxImgoNum)
    , mInFlightJpeg(0)
    , mpPipelineModelCallback(rCtorParams.pCallback)
{

      mpStreamInfo = new MetaStreamInfo(
                            "Meta:App:Callback",
                            eSTREAMID_META_APP_DYNAMIC_CALLBACK,
                            eSTREAMTYPE_META_OUT,
                            0
                        );
//    mLogLevel = ::property_get_int32("vendor.debug.camera.log", 0);
    // get current process limit count
    struct rlimit rlim;
    getrlimit(RLIMIT_NOFILE, &rlim);
    mFDLimitCount = rlim.rlim_cur;
    MY_LOGD("max fd count is: %lu, maxJpegNum %d, maxImgoNum %d", mFDLimitCount, mMaxJpegNum, mMaxImgoNum);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
NextCaptureListener::
onCaptureInFlightUpdated(
    CaptureInFlightUpdated const& params
) -> void
{
    Mutex::Autolock _l(mMutex);
    mInFlightJpeg = params.inFlightJpegCount;
    if (mlRequestNo.size() > 0)
    {
        int32_t nextCaptureCnt = MAX_IN_FLIGHT_JPEG - mInFlightJpeg;
        if (nextCaptureCnt > (int32_t)(mlRequestNo.size())) nextCaptureCnt = (int32_t)mlRequestNo.size();

        List<uint32_t>::iterator item = mlRequestNo.begin();
        uint32_t requestNo;
        while ( item != mlRequestNo.end() && nextCaptureCnt > 0) {
            requestNo = *item;
            onNextCaptureUpdated(requestNo);
            nextCaptureCnt--;
            item = mlRequestNo.erase(item);
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
NextCaptureListener::
get_fd_count()
-> int
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
auto
NextCaptureListener::
nextCaptureDecision(
    MUINT32 requestNo,
    MUINT32 requestCnt
) -> MBOOL
{
    uint32_t preReleaseCount = IPreReleaseRequestMgr::getInstance()->getPreleaseRequestCnt();
    int      fdCount         = get_fd_count();
    int64_t  freeMemorySize  = NSCam::NSMemoryInfo::getFreeMemorySize();

    MY_LOGD("(in-flight Capture, in-flight request at P2C, preReleaseCount, maxJpegNum, maxImgoNum) = (%d,%d,%d,%d,%d), requestNo: %d",
              mInFlightJpeg, requestCnt, preReleaseCount, mMaxJpegNum, mMaxImgoNum, requestNo);

    MBOOL ret = (  (mlRequestNo.size() > 0)                           // exist pending requests
               ||  (mInFlightJpeg >= MAX_IN_FLIGHT_JPEG)              // max in-flight jpegs
               ||  (requestCnt + TOLERANCE_IMGO_COUNT >= mMaxImgoNum) // check imgo buffer size
               || ((mInFlightJpeg + preReleaseCount) >= mMaxJpegNum)  // out of max jpeg buffer
               || ((fdCount + TOLERANCE_COUNT) >= mFDLimitCount)      // out of fd
               ||  (freeMemorySize < MIN_FREE_MEM) )                  // out of memory
               ? MFALSE : MTRUE;

    if (ret == MFALSE) {
        MY_LOGD("pending callback (in-flight Capture, in-flight request at P2C, preReleaseCount, maxJpegNum, maxImgoNum) = (%d,%d,%d,%d,%d), requestNo: %d",
              mInFlightJpeg, requestCnt, preReleaseCount, mMaxJpegNum, mMaxImgoNum, requestNo);

        MY_LOGD("cur_fd(%d), max(%lu), count(%d) free memory(%" PRId64 " bytes)",
                  fdCount, mFDLimitCount, fdCount + TOLERANCE_COUNT, freeMemorySize);
    }
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
NextCaptureListener::
onNextCaptureCallBack(
    MUINT32   requestNo,
    MUINT32   requestCnt,
    MBOOL     /*bSkipCheck*/
)
{
    Mutex::Autolock _l(mMutex);
    if (nextCaptureDecision(requestNo, requestCnt)) {
        onNextCaptureUpdated(requestNo);
    } else {
        mlRequestNo.push_back(requestNo);
    }
}

auto
NextCaptureListener::
onNextCaptureUpdated(uint32_t requestNo)
-> void
{
    MY_TRACE_FUNC_LIFE();
    MY_LOGD("NextCapture requestNo %d", requestNo);
    sp<IPipelineModelCallback> pCallback;
    pCallback = mpPipelineModelCallback.promote();
    if ( CC_UNLIKELY(! pCallback.get()) ) {
        MY_LOGE("can not promote pCallback for NextCapture");
        return;
    }

    // generate sp<IMetaStreamBuffer> with only MTK_CONTROL_CAPTURE_NEXT_READY
    sp<HalMetaStreamBuffer> pNextCaptureMetaBuffer =
        HalMetaStreamBufferAllocatorT(mpStreamInfo.get())();

    IMetadata* meta = pNextCaptureMetaBuffer->tryWriteLock(LOG_TAG);
    IMetadata::setEntry<MINT32>( meta, MTK_CONTROL_CAPTURE_NEXT_READY, 1);
    pNextCaptureMetaBuffer->unlock(LOG_TAG,meta);
    pNextCaptureMetaBuffer->finishUserSetup();
    ResultUpdateHelper(mpPipelineModelCallback, requestNo, pNextCaptureMetaBuffer, false, true);
}
