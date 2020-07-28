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
#define LOG_TAG "mmsdk/FeatureManager"
#include <dlfcn.h>
#include <cutils/log.h>
#include <utils/Errors.h>
#include <utils/String8.h>
#include <utils/String16.h>
#include <mmsdk/IFeatureManager.h>
#include <mmsdk/IEffectHal.h>
using namespace NSCam;
using namespace android;


/******************************************************************************
 *
 ******************************************************************************/
#define FUNCTION_LOG_START      ALOGD("[%s] - E.", __FUNCTION__)
#define FUNCTION_LOG_END        ALOGD("[%s] - X.", __FUNCTION__)



FeatureManager::
FeatureManager()
{
    FUNCTION_LOG_START;
    //@todo implement this - use dlopen
    //mpEffectFactory = new EffectFactory();
    FUNCTION_LOG_END;
}


FeatureManager::
~FeatureManager()
{
    FUNCTION_LOG_START;
    mpEffectFactory = NULL;
    FUNCTION_LOG_END;
}


status_t
FeatureManager::
setParameter(android::String8 key, android::String8 value)
{
    FUNCTION_LOG_START;
    (void)key;
    (void)value;
    FUNCTION_LOG_END;
    return OK;
}


android::String8
FeatureManager::
getParameter(android::String8 key)
{
    FUNCTION_LOG_START;
    (void)key;
    FUNCTION_LOG_END;
    return String8("@todo implement this");
}


status_t
FeatureManager::
setUp(EffectHalVersion const &nameVersion)
{
    FUNCTION_LOG_START;
    (void)nameVersion;
    //@todo implement this
    FUNCTION_LOG_END;
    return OK;
}


status_t
FeatureManager::
tearDown(EffectHalVersion const &nameVersion)
{
    FUNCTION_LOG_START;
    (void)nameVersion;
    //@todo implement this
    FUNCTION_LOG_END;
    return OK;
}


status_t
FeatureManager::
//getEffectFactory(sp<IEffectFactory> & effectFactory)
getEffectFactory(sp<IInterface> & effectFactory)
{
    FUNCTION_LOG_START;
    status_t result = OK;

    //@todo implement this
    result = createEffectFactory();
    effectFactory = mpEffectFactory;

    FUNCTION_LOG_END;
    return result;
}

status_t
FeatureManager::
createEffectFactory()
{
    FUNCTION_LOG_START;
    if(mpEffectFactory != NULL)
    {
        FUNCTION_LOG_END;
        return  OK;
    }
    mpEffectFactory = new EffectFactory();
    FUNCTION_LOG_END;
    return OK;
}

