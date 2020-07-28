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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include "P2_VRGralloc.h"

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    VRGralloc
#define P2_TRACE        TRACE_VR_GRALLOC
#include "P2_LogHeader.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_STR_PROC);

#define KEY_VR_GRALLOC_DEBUG  "vendor.debug.p2s.vr.gralloc.debug"
#define KEY_VR_GRALLOC_SKIP   "vendor.debug.p2s.vr.gralloc.skip"
#define KEY_VR_GRALLOC_DIV    "vendor.debug.p2s.vr.gralloc.div"

using NSCam::Feature::ILog;
using NSCam::IImageBuffer;

namespace P2
{

static MVOID get_debug_level(MBOOL &debug, MBOOL &skip, MUINT32 &div)
{
    static MBOOL sDebug = property_get_bool(KEY_VR_GRALLOC_DEBUG, false);
    static MBOOL sSkip = property_get_bool(KEY_VR_GRALLOC_SKIP, false);
    static MUINT32 sDiv_ = property_get_int32(KEY_VR_GRALLOC_DIV, 0);
    static MUINT32 sDiv = (sDiv_ == 1000 || sDiv_ == 1000000) ? sDiv_ : 1;

    debug = sDebug;
    skip = sSkip;
    div = sDiv;
}

MBOOL updateGralloc_VRInfo(const ILog &log, const buffer_handle_t &handle, const std::vector<IImageBuffer*> buffers, MUINT32 count, const MINT64 &cam2FwTs)
{
    TRACE_S_FUNC_ENTER(log);
    MBOOL ret = MTRUE;
    ge_smvr_info_t info;
    info.timestamp[0] = 0;
    info.timestamp_cam2fw = (uint64_t)cam2FwTs;
    const MUINT32 MAX = sizeof(info.timestamp)/sizeof(info.timestamp[0]);
    int result = GRALLOC_EXTRA_OK;

    MBOOL debug = MFALSE, skip = MFALSE;
    MUINT32 div = 1;
    get_debug_level(debug, skip, div);
    if(cam2FwTs == 0)
    {
        MY_S_LOGW(log, "camera send to FW timestamp == 0!!!! Something Wrong.");
    }

    if( debug || (log.getLogLevel() >= 1))
    {
        MY_S_LOGD(log, "skip=%d, div=%d, img[]=%zu, count=%d, cam2FwTs=%" PRId64,
                       skip, div, buffers.size(), count, info.timestamp_cam2fw);
        for( MUINT32 i = 0, n = buffers.size(); i < n && i < count; ++i )
        {
            MINT64 ts = buffers[i] ? buffers[i]->getTimestamp() : 0;
            MY_S_LOGD(log, "img[%d](%p) timestamp=%" PRId64 " ns (%" PRId64 " ms)", i, buffers[i], ts, ts/1000000);
        }
    }

    info.frame_count = 0;
    if( count > buffers.size() )
    {
        MY_S_LOGE(log, "finish count(%d) > buffer count(%zu)", count, buffers.size());
        ret = MFALSE;
    }
    else if( count > MAX )
    {
        MY_S_LOGE(log, "count(%d) > ge_smvr_info_t size(%d)", count, MAX);
        ret = MFALSE;
    }
    else
    {
        info.frame_count = count;
        for( MUINT32 i = 0; i < info.frame_count; ++i )
        {
            info.timestamp[i] = buffers[i] ? buffers[i]->getTimestamp() : 0;
        }
    }

    if( debug )
    {
        div = (div > 0) ? div : 1;
        MY_S_LOGD(log, "smvr_info.frame_count=%d divide base=%d", info.frame_count, div);
        for( MUINT32 i = 0; i < info.frame_count; ++i )
        {
            info.timestamp[i] /= div;
            MY_S_LOGD(log, "smvr_info.timestamp[%d]=%" PRId64 " (%" PRId64 " ms)", i, info.timestamp[i], info.timestamp[i]/(1000000/div));
        }
    }

    if( !skip )
    {
        result = gralloc_extra_perform(handle, GRALLOC_EXTRA_SET_SMVR_INFO, &info);
    }

    if( result != GRALLOC_EXTRA_OK )
    {
        MY_S_LOGE(log, "gralloc_extra_perform(%d) != OK", result);
        ret = MFALSE;
    }

    if(info.timestamp[0] != info.timestamp_cam2fw)
    {
        MY_S_LOGI(log, " cam2Fw(%" PRId64 ") / rec buf[0] timestamp(%" PRId64 "), count=%d", info.timestamp_cam2fw, info.timestamp[0], info.frame_count);
    }

    TRACE_S_FUNC_EXIT(log);
    return ret;
}

} // namespace P2
