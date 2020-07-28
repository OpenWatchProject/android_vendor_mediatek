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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#define LOG_TAG "MtkCamCustom/PlugFasteningS4Cell"
//
#include <cstdint>
#include "PlugFasteningS4Cell.h"
#include "remosaic_itf.h"

#define XTALK_LEN               2048
#define ARR_PATTERN             BAYER_GRBG
using namespace android;
using namespace NSCamCustom;

/******************************************************************************
 *
 ******************************************************************************/
PLUGFASTENING_CLASSNAME::
PLUGFASTENING_CLASSNAME(OPEN_VENDOR_ID id)
{
    FUNC_BGN;
    mOpenID = id; // must set OpenID
    //
    // add implementation here
    //
    FUNC_END;
};


/******************************************************************************
 *
 ******************************************************************************/
PLUGFASTENING_CLASSNAME::
~PLUGFASTENING_CLASSNAME()
{
    FUNC_BGN;
    //
    // add implementation here
    //
    FUNC_END;
};


/******************************************************************************
 * init function implementation
 ******************************************************************************/
MBOOL
PLUGFASTENING_CLASSNAME::
initImp(NSCam::PlugInitParam initParam)
{
    FUNC_BGN;
    //
    MBOOL res = true;
    MY_LOGD_IF(mLogLevel, "wxh: %dx%d eeprom: %p size: %zu", initParam.img_w, initParam.img_h, initParam.eep_buf_addr, initParam.eep_buf_size);
    remosaic_init(initParam.img_w, initParam.img_h, ARR_PATTERN/*flip*/, 64);
    if(remosaic_gainmap_gen(initParam.eep_buf_addr, initParam.eep_buf_size) != RET_OK) {
        MY_LOGE("remosaic gainmap fail!");
        res = false;
    }
    //
    FUNC_END;
    return res;
};


/******************************************************************************
 * uninit function implementation
 ******************************************************************************/
MBOOL
PLUGFASTENING_CLASSNAME::
uninitImp(void)
{
    FUNC_BGN;
    MBOOL res = MTRUE;
    //
    remosaic_deinit();
    //
    FUNC_END;
    return res;
};


/******************************************************************************
 * send command function implementation
 ******************************************************************************/
MBOOL
PLUGFASTENING_CLASSNAME::
sendCommandImp(MINT32 const cmd,
    MINTPTR arg1, MINTPTR arg2, MINTPTR arg3, MINTPTR arg4)
{
    FUNC_BGN;
    MBOOL res = MTRUE;
    MY_LOGI_IF(mLogLevel > 0, "CMD[%d]"
        "(%" PRIxPTR ", %" PRIxPTR ", %" PRIxPTR ", %" PRIxPTR ")",
        cmd, arg1, arg2, arg3, arg4);
    //
    switch (cmd)
    {
        case NSCam::NSCamPlug::GET_PARAM:
        {
            NSCam::PlugStaticInfo info;
            info.eep_buf_size = XTALK_LEN;
            *(NSCam::PlugStaticInfo*)arg1 = info;
            *(MINT32*) arg2 = ARR_PATTERN;
            break;
        }
        case NSCam::NSCamPlug::PROCESS:
        {
            NSCam::PlugProcessingParam _param = *(NSCam::PlugProcessingParam*)arg1;
            st_remosaic_param remosaic_param;
            remosaic_param.wb_r_gain = 1024;
            remosaic_param.wb_gr_gain = 1024;
            remosaic_param.wb_gb_gain = 1024;
            remosaic_param.wb_b_gain = 1024;
            remosaic_process_param_set(&remosaic_param);
            //
            uint8_t* src_buf = (uint8_t*)_param.src_buf;
            uint8_t* dst_buf = (uint8_t*)_param.dst_buf;
            size_t src_buf_size = _param.src_buf_size;
            size_t dst_buf_size = _param.dst_buf_size;
            //Step 3: Process Fcell.
            MY_LOGD_IF(mLogLevel, "src:%p dst:%p srcBufSize:%zu dstBufSize:%zu arg1:%p", src_buf, dst_buf, src_buf_size, dst_buf_size, (void*)arg1);
            if(remosaic_process(src_buf, src_buf_size, dst_buf, dst_buf_size) != RET_OK)
                res = false;
            break;
        }

    }
    //
    FUNC_END;
    return res;
};


