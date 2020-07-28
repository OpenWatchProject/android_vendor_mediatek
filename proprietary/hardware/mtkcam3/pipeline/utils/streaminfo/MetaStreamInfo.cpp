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

#define LOG_TAG "mtkcam-streaminfo"
//
#include <inttypes.h>
//
#include <cutils/compiler.h>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam3/pipeline/utils/streaminfo/MetaStreamInfo.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_UTILITY);
//
using namespace android;
using namespace NSCam::v3;
using namespace NSCam::v3::Utils;


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
//
#define MY_LOGV_IF(cond, ...)       do { if (            (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if (            (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if (            (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
auto
MetaStreamInfoBuilder::
build() const -> android::sp<IMetaStreamInfo>
{
    android::sp<MetaStreamInfo> pStreamInfo = new MetaStreamInfo();
    if (CC_UNLIKELY(pStreamInfo==nullptr)) {
        MY_LOGE(
            "Fail on build a new MetaStreamInfo: %#" PRIx64 "(%s)",
            mData.mStreamId, mData.mStreamName.c_str()
        );
        return nullptr;
    }

    pStreamInfo->mData = mData;

    return pStreamInfo;
}


/******************************************************************************
 *
 ******************************************************************************/
MetaStreamInfo::
MetaStreamInfo(
    char const* streamName,
    StreamId_T  streamId,
    MUINT32     streamType,
    size_t      maxBufNum,
    size_t      minInitBufNum,
    MINT        physicalId
)
    : IMetaStreamInfo()
{
    mData.mStreamName = streamName;
    mData.mStreamId = streamId;
    mData.mStreamType = streamType;
    mData.mMaxBufNum = maxBufNum;
    mData.mMinInitBufNum = minInitBufNum;
    mData.mPhysicalCameraId = physicalId;
}


/******************************************************************************
 *
 ******************************************************************************/
char const*
MetaStreamInfo::
getStreamName() const
{
    return  mData.mStreamName.c_str();
}


/******************************************************************************
 *
 ******************************************************************************/
IStreamInfo::StreamId_T
MetaStreamInfo::
getStreamId() const
{
    return  mData.mStreamId;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
MetaStreamInfo::
getStreamType() const
{
    return  mData.mStreamType;
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
MetaStreamInfo::
getMaxBufNum() const
{
    return  mData.mMaxBufNum;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
MetaStreamInfo::
setMaxBufNum(size_t count)
{
    mData.mMaxBufNum = count;
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
MetaStreamInfo::
getMinInitBufNum() const
{
    return  mData.mMinInitBufNum;
}


/******************************************************************************
 *
 ******************************************************************************/
android::String8
MetaStreamInfo::
toString() const
{
    android::String8 os = android::String8::format(
        "%#" PRIx64 " "
        "maxBuffers:%zu minInitBufNum:%zu "
        "%s phy:%d",
        getStreamId(),
        getMaxBufNum(), getMinInitBufNum(),
        getStreamName(), getPhysicalCameraId()
    );
    return os;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
MetaStreamInfo::
getPhysicalCameraId() const
{
    return mData.mPhysicalCameraId;
}