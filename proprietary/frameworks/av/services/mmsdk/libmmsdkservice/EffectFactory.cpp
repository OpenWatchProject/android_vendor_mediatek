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

#define LOG_TAG "mmsdk/EffectFactory"

#include <cutils/log.h>
#include <utils/Errors.h>
#include <mmsdk/IEffectHal.h>
#include <sys/time.h>

#include <vendor/mediatek/hardware/camera/callbackclient/1.1/types.h>
/******************************************************************************
 *
 ******************************************************************************/
#define FUNCTION_LOG_START      ALOGD("[%s] - E.", __FUNCTION__)
#define FUNCTION_LOG_END        ALOGD("[%s] - X.", __FUNCTION__)


using namespace NSCam;
using vendor::mediatek::hardware::camera::callbackclient::V1_1::MtkCBClientID;
//using namespace android;

std::mutex                   EffectFactory::msCBClientMutex;
android::wp<ICallbackClient> EffectFactory::mwpCBClient;
android::wp<ICallbackClient> EffectFactory::mwpCBClientDual;

EffectFactory::
EffectFactory()
{
    FUNCTION_LOG_START;
    //@todo implement this
    FUNCTION_LOG_END;
};


EffectFactory::
~EffectFactory()
{
    FUNCTION_LOG_START;
    //@todo implement this
    FUNCTION_LOG_END;
}

status_t
EffectFactory::
createCallbackClient(EffectHalVersion nameVersion, sp<ICallbackClient> &callbackClient)
{
    FUNCTION_LOG_START;
    struct timespec t;
    MINT64 timestamp;
    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    timestamp = (t.tv_sec) * 1000000000LL + t.tv_nsec; // nano second
    ALOGD("[%s] create name=%s, timestamp=%" PRId64 "", __FUNCTION__, nameVersion.effectName.string(), timestamp);

    callbackClient = makeCallbackClient(nameVersion.effectName, timestamp);

    // while making new CallbackClient instance,
    // we clear the resource of the previous one avoiding memory leakage
    {
        std::lock_guard<std::mutex> lk( msCBClientMutex );
        bool bDual = callbackClient->isDualCB() ;
        sp<ICallbackClient> pCB = bDual ? mwpCBClientDual.promote() : mwpCBClient.promote();
        if (pCB != nullptr)
        {
            ALOGD("[%s] clear old CallbackClient(refcnt=%d)", __FUNCTION__, pCB->getStrongCount() );
            pCB->stop();
        }
        else
        {
            ALOGD("[%s] previous CallbackClient doesn't exist, don't clear surfaces", __FUNCTION__);
        }

        // update the current CallbackClient to container
        if (callbackClient != nullptr)
        {
            if (bDual)
            {
                mwpCBClientDual = callbackClient;
            }
            else
            {
                mwpCBClient     = callbackClient;
            }
        }
        else
        {
            ALOGW("[%s] Fail to create CallbackClient!", __FUNCTION__);
        }
    }

    FUNCTION_LOG_END;
    return OK;
}
