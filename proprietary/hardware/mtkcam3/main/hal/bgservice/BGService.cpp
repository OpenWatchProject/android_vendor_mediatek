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

#define LOG_TAG "MtkCam/BGService"
#include "BGService.h"
#include <mtkcam/utils/std/Log.h>
#include <mtkcam3/feature/eventCallback/EventCallbackMgr.h>
#include <log/log.h>
#include <mtkcam/utils/std/ULog.h>
#include <utils/Errors.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_PRERELEASE);

using namespace std;
using namespace android;


namespace vendor {
namespace mediatek {
namespace hardware {
namespace camera {
namespace bgservice {
namespace V1_1 {
namespace implementation {

using vendor::mediatek::hardware::camera::bgservice::V1_1::IBGService;

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

using namespace std;
using namespace NSCam;
using namespace android;

// Methods from ::vendor::mediatek::hardware::camera::bgservice::V1_0::IBGService follow.
Return<int32_t> BGService::setEventCallback(int32_t ImgReaderId, const sp<::vendor::mediatek::hardware::camera::bgservice::V1_0::IEventCallback>& callback) {
    FUNC_START;

    //unlink to death notification for existed callback
    sp<::vendor::mediatek::hardware::camera::bgservice::V1_0::IEventCallback> oldCallback = nullptr;
    BGServiceWrap::getInstance()->getCallback(oldCallback);
    if (oldCallback != nullptr){
        oldCallback->unlinkToDeath(this);
        BGServiceWrap::getInstance()->resetCallback();
    }
    //link to death notification for callback
    if ( callback != nullptr ) {
        ::android::hardware::Return<bool> linked = callback->linkToDeath(this, (uint64_t)this);
        if (!linked.isOk())
        {
            MY_LOGE("Transaction error in linking to mEventCallback death: %s, ImgReaderId(%d)", linked.description().c_str(), ImgReaderId);
        }
        else if (!linked)
        {
            MY_LOGE("Unable to link to mEventCallback death notifications");
        }
    }
    else
    {
        MY_LOGE("App HIDL EventCallback function is empty, callback=%p", callback.get());
    }
    std::string version = "1_0";
    // bool emitError = false;
    // sp<IEventCallback> callback_v1_1 = IEventCallback::castFrom(callback, emitError);

    BGServiceWrap::getInstance()->setEventCallback(callback, version);
    FUNC_END;

    return int32_t {};
}

// Methods from ::vendor::mediatek::hardware::camera::bgservice::V1_0::IBGService follow.
Return<int32_t> BGService::setEventCallback_1_1(int32_t ImgReaderId, const sp<::vendor::mediatek::hardware::camera::bgservice::V1_1::IEventCallback>& callback) {
    FUNC_START;

    //unlink to death notification for existed callback
    sp<IEventCallback> oldCallback = nullptr;
    BGServiceWrap::getInstance()->getCallback(oldCallback);
    if (oldCallback != nullptr){
        oldCallback->unlinkToDeath(this);
        BGServiceWrap::getInstance()->resetCallback();
    }
    //link to death notification for callback
    if ( callback != nullptr ) {
        ::android::hardware::Return<bool> linked = callback->linkToDeath(this, (uint64_t)this);
        if (!linked.isOk())
        {
            MY_LOGE("Transaction error in linking to mEventCallback death: %s, ImgReaderId(%d)", linked.description().c_str(), ImgReaderId);
        }
        else if (!linked)
        {
            MY_LOGE("Unable to link to mEventCallback death notifications");
        }
    }

    std::string version = "1_1";
    BGServiceWrap::getInstance()->setEventCallback(IEventCallback::castFrom(callback), version);
    FUNC_END;

    return int32_t {};
}


/******************************************************************************
 *
 ******************************************************************************/
void BGService::serviceDied(uint64_t cookie __unused, const wp<hidl::base::V1_0::IBase>& who __unused)
{
    if (cookie != (uint64_t)this) {
        MY_LOGE("Unexpected IEventCallback serviceDied cookie 0x%" PRIx64 ", expected %p", cookie, this);
    }
    CAM_ULOGME("%s: BGService IEventCallback serviceDied!!",__FUNCTION__);

    BGServiceWrap::getInstance()->resetCallback();
}

// Methods from ::android::hidl::base::V1_0::IBase follow.

IBGService* HIDL_FETCH_IBGService(const char* /* name */) {
    CAM_ULOGMI("IBGService  into HIDL_FETCH_IBGService");
    return new BGService();
}



/* BGServiceWrap */
/*******************************************************************************
* Get Instance
********************************************************************************/
BGServiceWrap*
BGServiceWrap::getInstance()
{
    static BGServiceWrap inst;
    return &inst;
}


/******************************************************************************
 *
 ******************************************************************************/
Return<int32_t>
BGServiceWrap::setEventCallback(const sp<IEventCallback>& callback, std::string& version) {

    //unlink to death notification for existed callback
    // if ( mEventCallback != nullptr ) {
    //     MY_LOGD("1.2");
    //     mEventCallback->unlinkToDeath(this);
    //     mEventCallback = nullptr;
    // }

    {
        Mutex::Autolock _l(mLock);
        mv11EventCallback = callback;
        mVersion = version;
        if (mv11EventCallback == nullptr)
        {
            MY_LOGE("App HIDL EventCallback function is empty, mv11EventCallback=%p", mv11EventCallback.get());
        }
    }
    EventCallbackMgr::getInstance()->registerRequestCB(requestCallback);
    return int32_t {};
}

/******************************************************************************
 *
 ******************************************************************************/
Return<int32_t>
BGServiceWrap::setEventCallback(const sp<::vendor::mediatek::hardware::camera::bgservice::V1_0::IEventCallback>& callback, std::string& version) {
    {
        Mutex::Autolock _l(mLock);
        mEventCallback = callback;
        mVersion = version;
        if (mEventCallback == nullptr)
        {
            MY_LOGE("App HIDL EventCallback function is empty, mEventCallback=%p", mEventCallback.get());
        }
    }
    EventCallbackMgr::getInstance()->registerRequestCB(requestCallback);
    return int32_t {};
}


/******************************************************************************
 *
 ******************************************************************************/
void
BGServiceWrap::resetCallback() {
    Mutex::Autolock _l(mLock);
    mEventCallback = nullptr;
    mv11EventCallback = nullptr;
}

/******************************************************************************
 *
 ******************************************************************************/
void
BGServiceWrap::getCallback(sp<IEventCallback>& callback) {
    FUNC_START;
    Mutex::Autolock _l(mLock);
    callback = mv11EventCallback;
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
void
BGServiceWrap::getCallback(sp<::vendor::mediatek::hardware::camera::bgservice::V1_0::IEventCallback>& callback) {
    FUNC_START;
    Mutex::Autolock _l(mLock);
    callback = mEventCallback;
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
BGServiceWrap::
onRequestCallback( int32_t const ImgReaderId,
                   int32_t  const frameNumber,
                   uint32_t           status,
                   uint64_t const timestamp)
{
    FUNC_START;
    Mutex::Autolock _l(mLock);

    if (((mVersion == "1_0") && mEventCallback == nullptr) || ((mVersion == "1_1") && mv11EventCallback == nullptr))
    {
        MY_LOGE("App HIDL ver %s EventCallback function is empty! frame(%d) status(%d) cannot callback!", mVersion.c_str(), frameNumber, status);
        return false;
    }

    //always trigger v1.1 function. if need to support v1.0 function, it should add hint from BGServiceWrap::setEventCallback
    //auto ret = mEventCallback->onCompleted(ImgReaderId, frameNumber, status);
    Return<bool> ret = false;
    MY_LOGI("Callback via %s interface, ImgR(%d), FN(%d), Status(%d), TS(%" PRId64 ") ",
        mVersion.c_str(), ImgReaderId, frameNumber, status, timestamp);

    if (mVersion == "1_1")
        ret = mv11EventCallback->onCompleted_1_1(ImgReaderId, frameNumber, status, timestamp, 0, 0);
    else
        ret = mEventCallback->onCompleted(ImgReaderId, frameNumber, status);

    if (!ret.isOk()) {
        MY_LOGE("Transaction error in IEventCallback::onCompleted: %s", ret.description().c_str());
        return false;
    }

    FUNC_END;
    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
BGServiceWrap::
requestCallback( int32_t const ImgReaderId,
                int32_t  const frameNumber,
                uint32_t            status,
                uint64_t const timestamp)
{
    FUNC_START;
    if (ATRACE_ENABLED())
    {
        String8 const str = String8::format(
            "BgsCb:ID:%d:frame:%d:status:%d:timestamp:%" PRId64,
            ImgReaderId, frameNumber, status, timestamp);
        CAM_TRACE_BEGIN(str.string());
    }
    bool ret = false;
    ret = BGServiceWrap::getInstance()->onRequestCallback( ImgReaderId,
                                                            frameNumber,
                                                            status,
                                                            timestamp);
    if (ATRACE_ENABLED())
    {
        CAM_TRACE_END();
    }
    FUNC_END;
    return ret;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace bgservice
}  // namespace camera
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
