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

#define LOG_TAG "mtkcam-ControlMetaBufferGenerator"

#include <impl/ControlMetaBufferGenerator.h>
#include <mtkcam/utils/std/ULog.h>

//
#include "MyUtils.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_MODEL);

/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using HalMetaStreamBuffer = NSCam::v3::Utils::HalMetaStreamBuffer;
using HalMetaStreamBufferAllocatorT = HalMetaStreamBuffer::Allocator;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace model {


/******************************************************************************
 *
 ******************************************************************************/
auto
generateControlAppMetaBuffer(
    std::vector<android::sp<IMetaStreamBuffer>> *out,
    android::sp<IMetaStreamBuffer> const pMetaStreamBuffer,
    IMetadata const* pOriginalAppControl,
    IMetadata const* pAdditionalApp,
    IMetaStreamInfo const* pInfo
) -> int
{
    //  Append addition data to App Control IMetaStreamBuffer if it exists;
    //  otherwise allocate a new one with original + additional.

    android::sp<IMetaStreamBuffer> metaBuf;
    if (pMetaStreamBuffer != nullptr)
    {
        metaBuf = pMetaStreamBuffer;
        if  ( pAdditionalApp != nullptr ) {
            IMetadata *appMeta = pMetaStreamBuffer->tryWriteLock(LOG_TAG);
            if (CC_UNLIKELY(appMeta == nullptr))
            {
                MY_LOGE("Cannot get app control, somebody didn't release lock?");
                return -ENODEV;
            }
            *appMeta += *pAdditionalApp;
            pMetaStreamBuffer->unlock(LOG_TAG, appMeta);
        }
    }
    else
    {
        // sub/dummy frame app control : use original app control + additional
        IMetadata subMeta(*pOriginalAppControl);
        if  ( pAdditionalApp != nullptr ) {
            subMeta += *pAdditionalApp;
        }
        metaBuf = HalMetaStreamBufferAllocatorT(const_cast<IMetaStreamInfo*>(pInfo))(subMeta);
    }
    (*out).push_back(metaBuf);
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
generateControlHalMetaBuffer(
    std::vector<android::sp<IMetaStreamBuffer>> *out,
    IMetadata const* pAdditionalHal,
    IMetaStreamInfo const* pInfo
) -> int
{
    if  ( pAdditionalHal != nullptr ) {
        sp<HalMetaStreamBuffer> pBuffer =
            HalMetaStreamBufferAllocatorT(const_cast<IMetaStreamInfo*>(pInfo))(*pAdditionalHal);
        (*out).push_back(pBuffer);
    }
    return OK;
}


};  //namespace model
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam

