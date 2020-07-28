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

#define LOG_TAG "MtkCam/PostProcess"

#include <mtkcam/def/BuiltinTypes.h>
#include <mtkcam/def/Errors.h>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Misc.h>
//
#include <mtkcam/middleware/v1/PostProc/IPostProc.h>
//
#include "processor/DummyPostProcessor.h"
#include "processor/ThirdPartyPostProcessor.h"

/******************************************************************************
 *
 ******************************************************************************/
using namespace std;
using namespace android;
using namespace android::NSPostProc;
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

/******************************************************************************
 *
 ******************************************************************************/
sp<IImagePostProcessor>
IImagePostProcessor::
createInstance(
    PostProcessorType type
)
{
    auto typeToString = [&type]()
    {
        switch(type)
        {
            case PostProcessorType::NONE:
                return "NONE";
            case PostProcessorType::DUMMY:
                return "DUMMY";
            case PostProcessorType::BOKEH:
                return "BOKEH";
            case PostProcessorType::THIRDPARTY_MFNR:
                return "THIRDPARTY_MFNR";
            case PostProcessorType::THIRDPARTY_HDR:
                return "THIRDPARTY_HDR";
            case PostProcessorType::THIRDPARTY_BOKEH:
                return "THIRDPARTY_BOKEH";
            case PostProcessorType::THIRDPARTY_DCMF:
                return "THIRDPARTY_DCMF";
            default:
                return "NO_SET";
        }
    };
    MY_LOGD("create post processor(%s)", typeToString());

    if (PostProcessorType::DUMMY == type)
    {
        return new DummyPostProcessor(type);
    }
    else if(  PostProcessorType::THIRDPARTY_MFNR  == type
           || PostProcessorType::THIRDPARTY_HDR   == type
           || PostProcessorType::THIRDPARTY_BOKEH == type
           || PostProcessorType::THIRDPARTY_DCMF == type)
    {
        return new ThirdPartyPostProcessor(type);
    }
    else
    {
        MY_LOGD("processor not support");
    }
    return nullptr;
}

status_t
ImagePostProcessorBase::
flush()
{
  MY_LOGD("+");
  status_t ret = (mpPipeline != nullptr) ? mpPipeline->flush() : OK;
  MY_LOGD("-");
  return ret;
}

status_t
ImagePostProcessorBase::
waitUntilDrained()
{
  MY_LOGD("+");
  status_t ret = (mpPipeline != nullptr) ? mpPipeline->waitUntilDrained() : OK;
  MY_LOGD("-");
  return ret;
}
