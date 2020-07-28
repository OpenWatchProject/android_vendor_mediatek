/* MediaTek Inc. (C) 2018. All rights reserved.
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

#include "SecureElement.h"

#define LOG_TAG "SERadioRspHandler"
#include <log/log.h>

#include "selog.h"

#define DEBUG_QUEUE 0
#define DEBUG_RESPONSE 1

namespace android {
namespace hardware {
namespace secure_element {
namespace V1_0 {
namespace implementation {

SERadioResponseHandler::SERadioResponseHandler(sp<SecureElement> se, const hidl_string& name) {
    mSE = se;
    mSlotName = name;

    mQueueMutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&mQueueMutex);
    mQueueNextIndex = 0;
    for (uint8_t i = 0; i < RESPONSE_QUEUE_SIZE; ++i)
        mQueue[i].handled = true;
    pthread_mutex_unlock(&mQueueMutex);

    SELOGD("Created");
}

SERadioResponseHandler::~SERadioResponseHandler() {
    pthread_mutex_unlock(&mQueueMutex);
    pthread_mutex_destroy(&mQueueMutex);

    mSE = nullptr;
}

bool SERadioResponseHandler::removeResponse(struct SERadioResponseData& data, enum SERadioResponseType type, int32_t serial) {

    bool result = false;
    pthread_mutex_lock(&mQueueMutex);

    for (uint8_t i = 0; i < RESPONSE_QUEUE_SIZE; ++i) {
        if (mQueue[i].handled == true)
            continue;

        int32_t serialInQueue;
        serialInQueue = mQueue[i].data.info.serial;

        if (mQueue[i].type == type && serialInQueue == serial) {
            data = mQueue[i].data;
            mQueue[i].handled = true;
            result = true;
            break;
        }
    }

    pthread_mutex_unlock(&mQueueMutex);

#if DEBUG_QUEUE
    SELOGD("Remove response [%d: #%d] result=%d", type, serial, result);
#endif

    return result;
}


// Methods for response
Return<void> SERadioResponseHandler::getIccCardStatusResponse(const AOSP_V1_0::RadioResponseInfo& info, const AOSP_V1_0::CardStatus& cardStatus) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif

    SERadioResponseData data;
    data.info = info;
    data.cardStatus = cardStatus;

    addToQueue(SE_ICC_CARD_STATUS, data);

    return Void();
}

Return<void> SERadioResponseHandler::getATRResponse(const AOSP_V1_0::RadioResponseInfo& info, const hidl_string& response) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif

    SERadioResponseData data;
    data.info = info;
    data.response = response;

    addToQueue(SE_ICC_GET_ATR, data);

    return Void();
}

Return<void> SERadioResponseHandler::iccTransmitApduBasicChannelResponse(const AOSP_V1_0::RadioResponseInfo& info, const AOSP_V1_0::IccIoResult& result) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif

    SERadioResponseData data;
    data.info = info;
    data.result = result;

    addToQueue(SE_ICC_TRANSMIT_APDU_BASIC, data);

    return Void();
}

Return<void> SERadioResponseHandler::iccOpenLogicalChannelResponse(const AOSP_V1_0::RadioResponseInfo& info, int32_t channelId, const hidl_vec<int8_t>& selectResponse) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif

    SERadioResponseData data;
    data.info = info;
    data.channelId = channelId;
    data.selectResponse = selectResponse;

    addToQueue(SE_ICC_OPEN_CHANNEL, data);

    return Void();
}

Return<void> SERadioResponseHandler::iccCloseLogicalChannelResponse(const AOSP_V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif

    SERadioResponseData data;
    data.info = info;

    addToQueue(SE_ICC_CLOSE_CHANNEL, data);

    return Void();
}

Return<void> SERadioResponseHandler::iccTransmitApduLogicalChannelResponse(const AOSP_V1_0::RadioResponseInfo& info, const AOSP_V1_0::IccIoResult& result) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif

    SERadioResponseData data;
    data.info = info;
    data.result = result;

    addToQueue(SE_ICC_TRANSMIT_APDU_LOGICAL, data);

    return Void();
}


// Private methods follow.
void SERadioResponseHandler::addToQueue(enum SERadioResponseType type, const SERadioResponseData& data) {
    pthread_mutex_lock(&mQueueMutex);

    if (mQueue[mQueueNextIndex].handled == false) {
        SELOGE("Warning: response [%d: #%d] didn't be handled and it will be dropped",
            mQueue[mQueueNextIndex].type, mQueue[mQueueNextIndex].data.info.serial);
    }

    mQueue[mQueueNextIndex].type = type;
    mQueue[mQueueNextIndex].handled = false;
    mQueue[mQueueNextIndex].data = data;
    ++mQueueNextIndex;
    if (mQueueNextIndex == RESPONSE_QUEUE_SIZE)
        mQueueNextIndex = 0;

    pthread_mutex_unlock(&mQueueMutex);

#if DEBUG_QUEUE
    SELOGD("Add response [%d: #%d]", type, data.info.serial);
#endif

    mSE->notifyResponse();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace secure_element
}  // namespace hardware
}  // namespace android
