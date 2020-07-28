/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

#define LOG_TAG "MtkCam/ISPCallback"

#include "ISPCallback.h"
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/Trace.h>
#include <log/log.h>
#include <utils/Errors.h>
#include <system/camera_metadata.h>
#include <mtkcam/utils/metadata/IMetadata.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_ISP_HAL_SERVER);

namespace NSCam {
namespace ISPHal {

#define MY_LOGV(fmt, arg...)        CAM_ULOGMV(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)

#if 1
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif

//using vendor::mediatek::hardware::camera::isphal::V1_0::Status;
using namespace vendor::mediatek::hardware::camera::isphal::V1_0;
using namespace NSCam::v3;
using namespace NSCam::ISPHal;

/******************************************************************************
 *
 ******************************************************************************/
ISPCallback::
~ISPCallback()
{
    MY_LOGI("destroy ISP callback");
}

/******************************************************************************
 *
 ******************************************************************************/
ISPCallback::
ISPCallback(android::sp<ICallback> cb, android::sp<ISPMetadataConverter> converter)
    : mISPCallback(cb)
    , mMetadataConverter(converter)
{
    MY_LOGI("create ISP callback");
}

/******************************************************************************
 *
 ******************************************************************************/
auto ISPCallback::onFrameUpdated(UserOnFrameUpdated const& params) -> void
{
    FUNC_START;
    if (params.bFrameEnd)
    {
        MY_LOGD("request[%d] complete", params.requestNo);
        handleCallbacktoHidl(params.requestNo);
    }
    else if (params.bisEarlyCB)
    {
        MY_LOGD("request[%d] early callback", params.requestNo);
        handleEarlyCallback(params.requestNo, params.pMetadata);
    }
    else
    {
        MY_LOGD("request[%d] partial done", params.requestNo);
        handlePartialResult(params.requestNo, params.vOutMeta);
    }
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
auto ISPCallback::registerReq(uint32_t const& req) -> void
{
    FUNC_START;
    ::android::Mutex::Autolock _l(mLock);
    std::shared_ptr<IMetadata> result;
    auto search = mResultMetaMap.find(req);
    if (search != mResultMetaMap.end())
    {
        MY_LOGE("error!!, the request(%d) already exist!!!", req);
    }
    else
    {
        result = std::make_shared<IMetadata>();
        mResultMetaMap.emplace(req, CallbackResult(result));
    }
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
auto ISPCallback::waitForReqDrained(int32_t const& operation __unused) -> void
{
    FUNC_START;
    ::android::Mutex::Autolock _l(mLock);
    while( !mResultMetaMap.empty() )
    {
        mWaitCond.waitRelative(mLock,1000000000);
    }
    FUNC_END;
}

// internal
/******************************************************************************
 *
 ******************************************************************************/
auto ISPCallback::handleCallbacktoHidl(uint32_t reqNo) -> void
{
    CAM_TRACE_CALL();
    FUNC_START;
    ::android::Mutex::Autolock _l(mLock);
    auto search = mResultMetaMap.find(reqNo);
    if (search != mResultMetaMap.end())
    {
        void *p_isp_metadata = nullptr;
        size_t isp_metadata_size = 0;
        IMetadata* pMetadata = search->second.data.get();
        if (CC_UNLIKELY(
            ! mMetadataConverter->convertToISPMetadata(*pMetadata, p_isp_metadata, &isp_metadata_size)
        ||  ! p_isp_metadata
        ||  0 == isp_metadata_size ))
        {
            MY_LOGE("fail: IMetadata -> camera_metadata(%p) size:%zu", p_isp_metadata, isp_metadata_size);
        }
        hidl_vec<uint8_t> temp;
        temp.setToExternal((uint8_t *)p_isp_metadata, isp_metadata_size);
        ISPResult HidlResult{
            .reqNumber          = reqNo,
            .result             = std::move(temp),
            .status             = search->second.hasError ?
                                  (Status::INTERNAL_ERROR) :
                                  (Status::OK),
        };

        auto ret = mISPCallback->processResult(HidlResult);
        if (CC_UNLIKELY(!ret.isOk())) {
            MY_LOGE("Transaction error in ISPCallback::processResult: %s", ret.description().c_str());
        }
        mResultMetaMap.erase(search);
        mMetadataConverter->freeISPMetaBuffer(p_isp_metadata);
    }
    else
    {
        MY_LOGW("Cannot find result for request(%d)", reqNo);
    }
    if ( mResultMetaMap.empty() )
    {
        mWaitCond.signal();
    }
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
auto ISPCallback::handleEarlyCallback(uint32_t reqNo, const IMetadata* pMetadata) -> void
{
    CAM_TRACE_CALL();
    FUNC_START;
    ::android::Mutex::Autolock _l(mLock);
    {
        void *p_isp_metadata = nullptr;
        size_t isp_metadata_size = 0;
        if (CC_UNLIKELY(
            ! mMetadataConverter->convertToISPMetadata(*pMetadata, p_isp_metadata, &isp_metadata_size)
        ||  ! p_isp_metadata
        ||  0 == isp_metadata_size ))
        {
            MY_LOGE("fail: IMetadata -> camera_metadata(%p) size:%zu", p_isp_metadata, isp_metadata_size);
        }
        hidl_vec<uint8_t> temp;
        temp.setToExternal((uint8_t *)p_isp_metadata, isp_metadata_size);
        ISPResult HidlResult{
            .reqNumber          = reqNo,
            .result             = std::move(temp),
            .status             = (Status::OK),
        };

        auto ret = mISPCallback->processEarlyResult(HidlResult);
        if (CC_UNLIKELY(!ret.isOk())) {
            MY_LOGE("Transaction error in ISPCallback::processEarlyResult: %s", ret.description().c_str());
        }
        mMetadataConverter->freeISPMetaBuffer(p_isp_metadata);
    }
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
auto ISPCallback::handlePartialResult(uint32_t reqNo, std::vector<android::sp<IMetaStreamBuffer>> vOutMeta) -> bool
{
    CAM_TRACE_CALL();
    FUNC_START;
    ::android::Mutex::Autolock _l(mLock);
    std::shared_ptr<IMetadata> result;
    auto search = mResultMetaMap.find(reqNo);
    if (search != mResultMetaMap.end())
    {
        if (search->second.hasError)
        {
            return false;
        }
        result = search->second.data;
    }
    else
    {
        result = std::make_shared<IMetadata>();
        mResultMetaMap.emplace(reqNo, CallbackResult(result));
    }

    for(auto const& OutMetaBuf : vOutMeta)
    {
        if (OutMetaBuf->hasStatus(STREAM_BUFFER_STATUS::ERROR))
        {
            search->second.hasError = true;
            return false;
        }
        auto OutMeta = OutMetaBuf->tryReadLock(LOG_TAG);
        if (OutMeta != nullptr)
        {
            *(result.get()) += *OutMeta;
            OutMetaBuf->unlock(LOG_TAG, OutMeta);
        }
    }
    FUNC_END;
    return true;
}


}  // namespace ISPHal
}  // namespace NSCam