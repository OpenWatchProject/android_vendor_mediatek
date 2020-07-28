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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#define LOG_TAG "MtkCam/TimestampSyncDataUpdater"
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#include "TimestampSyncDataUpdater.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_P1_SYNCHELPER);

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("(%4d) [%s] " fmt, __LINE__, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("(%4d) [%s] " fmt, __LINE__, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("(%4d) [%s] " fmt, __LINE__, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("(%4d) [%s] " fmt, __LINE__, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("(%4d) [%s] " fmt, __LINE__, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%4d) [%s] " fmt, __LINE__, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%4d) [%s] " fmt, __LINE__, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

using namespace NSCam::v3::Utils::Imp;
/******************************************************************************
 *
 ******************************************************************************/
bool
TimestampSyncDataUpdater::
update(
    SyncData &data
)
{
    MY_LOGA_IF((data.mSyncTarget.size() == 0), "Invalid operation.");
    //
    if(data.mSyncTarget.size() == 1) {
        // single cam frame, ignore.
        return true;
    }
    //
    MINT64 syncTolerance;
    MINT32 syncFailBehavior;
    MINT64 baseTimestamp;
    MINT64 timestamp;
    bool checkresult = true;
    int first_cam_id = data.mSyncTarget[0];
    auto *inputParams_first = data.mvInputParams[first_cam_id];
    auto *outputPamras_first = data.mvOutputParams[first_cam_id];
    auto timestamp_result_check = [&syncTolerance](MINT64 base, MINT64 target) {
        int timeStamp_ms_base = base/1000000;
        int timeStamp_ms_target = target/1000000;
        int tolerance_ms = syncTolerance/1000;
        int64_t diff = abs(timeStamp_ms_base - timeStamp_ms_target);
        if (diff<=tolerance_ms)
            return true;
        else
            return false;
    };
    auto updateCheckResult = [&data](MINT64 result) {
        for(auto&& item:data.mvOutputParams) {
            IMetadata::setEntry<MINT64>(
                                        item.second->HalDynamic,
                                        MTK_FRAMESYNC_RESULT,
                                        result);
        }
    };
    if(!IMetadata::getEntry<MINT64>(inputParams_first->HalControl, MTK_FRAMESYNC_TOLERANCE, syncTolerance)) {
        goto lbExit;
    }
    if(!IMetadata::getEntry<MINT32>(inputParams_first->HalControl, MTK_FRAMESYNC_FAILHANDLE, syncFailBehavior)) {
        goto lbExit;
    }
    // select base timestamp
    if(!IMetadata::getEntry<MINT64>(outputPamras_first->HalDynamic, MTK_P1NODE_FRAME_START_TIMESTAMP, baseTimestamp)) {
        goto lbExit;
    }
    // base element is first item.
    for(auto&& item:data.mvOutputParams) {
        if(item.first == first_cam_id) continue;
        if(!IMetadata::getEntry<MINT64>(
                                        item.second->HalDynamic,
                                        MTK_P1NODE_FRAME_START_TIMESTAMP,
                                        timestamp)) {
            MY_LOGA("timestamp not exist! should not happened");
            goto lbExit;
        }
        checkresult &= timestamp_result_check(baseTimestamp, timestamp);
        MY_LOGD("[r%d:f%d]CamId = %d, time1=%" PRId64 "ns, ret = %d, synID = %d, time2=%" PRId64 "ns,\
            tolerance=%" PRId64 "us\n", data.mRequestNo, data.mFrameNo, first_cam_id, baseTimestamp, checkresult,
            item.first, timestamp, syncTolerance);
    }
    // update result
    if(checkresult) {
        updateCheckResult(MTK_FRAMESYNC_RESULT_PASS);
    }
    else {
        if(MTK_FRAMESYNC_FAILHANDLE_DROP == syncFailBehavior) {
            updateCheckResult(MTK_FRAMESYNC_RESULT_FAIL_DROP);
        }
        else {
            updateCheckResult(MTK_FRAMESYNC_RESULT_FAIL_CONTINUE);
        }
    }
lbExit:
    return true;
}