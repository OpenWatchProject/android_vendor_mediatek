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

#define LOG_TAG "TPBokehPlugin"
//
#include <sys/prctl.h>
#include <sys/resource.h>
#include <cutils/properties.h>
//
#include <mtkcam/utils/std/Log.h>
//
#include "../TPNodePlugin.h"
#include "TPBokehPlugin.h"

#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)

#define WRITE_PERMISSION 0660
#define DUALCAM 1

#define TP_PLUGIN_DEFAULT_ID  (MTK_PLUGIN_MODE_BOKEH_3RD_PARTY)

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;

// mount the implementation of third party
REGISTER_TPNODE_PLUGIN(TP_PLUGIN_DEFAULT_ID, TPBokehPlugin);

/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}

/******************************************************************************
 *
 ******************************************************************************/
TPBokehPlugin::
TPBokehPlugin()
{
}


/******************************************************************************
 *
 ******************************************************************************/
TPBokehPlugin::
~TPBokehPlugin()
{
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
TPBokehPlugin::
init()
{
    MY_LOGD("tpbokeh plugin:init +");
    usleep(300 * 1000);
    MY_LOGD("tpbokeh plugin:init -");
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
TPBokehPlugin::
uninit()
{
}

/******************************************************************************
 *
 ******************************************************************************/
TPNodePlugin::PluginProfile&
TPBokehPlugin::
profile()
{
    static PluginProfile pf = {
                    .id = TP_PLUGIN_DEFAULT_ID,
                    .type = BLENDING
                };

    return pf;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
TPBokehPlugin::
process(RequestParams const& params)
{
    MY_LOGD("default plugin:request[%u] +", params.uRequestNo);

    // dump param info
    for (size_t i = 0; i < params.uInCount; i++) {
        MY_LOGD("[input-%zd] AppMeta:%p HalMeta:%p FullYuv:%p RszYuv:%p",
                i,
                params.in[i].pAppMeta,
                params.in[i].pHalMeta,
                params.in[i].pFullYuv,
                params.in[i].pResizedYuv);
    }

    MY_LOGD("[output] AppMeta:%p HalMeta:%p FullYuv:%p Depth:%p Clean:%p",
            params.out.pAppMeta,
            params.out.pHalMeta,
            params.out.pFullYuv,
            params.out.pDepth,
            params.out.pClean);

    // use first frame to make a gray image
    IMetadata* pHalMeta = params.in[0].pHalMeta;
    IMetadata* pAppMeta = params.in[0].pAppMeta;
    IImageBuffer* pInputYuv = params.in[0].pFullYuv;
    IImageBuffer* pResizeYuv = params.in[0].pResizedYuv;
    IImageBuffer* pOutputYuv = params.out.pFullYuv;
    IImageBuffer* pOutClean = params.out.pClean;

    PluginId_T pluginId = 0;
    IMetadata::getEntry<MINT32>(pHalMeta, MTK_PLUGIN_MODE, pluginId);
    MY_LOGD("process request with plugin(%d)", pluginId);

    // MINT32 bokehLevel = 0;
    // IMetadata::getEntry<MINT32>(pAppMeta, MTK_STEREO_FEATURE_DOF_LEVEL, bokehLevel);
    // MY_LOGD("MTK_STEREO_FEATURE_DOF_LEVEL:%d", bokehLevel);
    // if( ! tryGetMetadata<MINT32>(const_cast<IMetadata*>(pAppMeta), MTK_STEREO_FEATURE_DOF_LEVEL, bokehLevel) ){
    //     MY_LOGE("cannot get MTK_STEREO_FEATURE_DOF_LEVEL after updating request");
    // }
    // else{
    //     MY_LOGD("MTK_STEREO_FEATURE_DOF_LEVEL:%d", bokehLevel);
    // }

    for (size_t i = 0; i < pInputYuv->getPlaneCount(); i++) {
        //MY_LOGD("1");
        void *pInVa = (void *) (pInputYuv->getBufVA(i));
        //MY_LOGD("2");
        void *pOutVa = (void *) (pOutputYuv->getBufVA(i));
        //MY_LOGD("3");
        void *pOutCleanVa = (void *) (pOutClean->getBufVA(i));
        //MY_LOGD("4");
        MUINT32 uBufSize = pInputYuv->getBufSizeInBytes(i);

        memcpy(pOutVa, pInVa, uBufSize);
        memcpy(pOutCleanVa, pInVa, uBufSize);
    }

    pOutputYuv->syncCache(eCACHECTRL_FLUSH);
    pOutClean->syncCache(eCACHECTRL_FLUSH);

#if DUALCAM
    IImageBuffer* pOutputDepth = params.out.pDepth;
    void *pDepthVa = (void *) (pOutputDepth->getBufVA(0));
    MUINT32 uDepthSize = pOutputDepth->getBufSizeInBytes(0);
    MY_LOGD("process depth va(%p) size(%u)", pDepthVa, uDepthSize);
    memset(pDepthVa, 100, uDepthSize);

    pOutputDepth->syncCache(eCACHECTRL_FLUSH);
#endif

    // dump data by using property: debug.stereo.dumpcapturedata
    char cLogLevel[PROPERTY_VALUE_MAX];
    ::property_get("debug.stereo.dumpcapturedata", cLogLevel, "0");
    MINT32 value = ::atoi(cLogLevel);
    if(value > 0)
    {
        // regenerate filename if need.
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        auto msFilename = std::string("/sdcard/vsdof/tpnode_ut/")+std::to_string(millis);
        MY_LOGD("filename(%s)", msFilename.c_str());

        bool isSuccess = false;
        isSuccess = NSCam::Utils::makePath(msFilename.c_str(), WRITE_PERMISSION);

        if(isSuccess){
            pInputYuv->saveToFile((msFilename+std::string("/")+std::string("inputYUV")).c_str());
            pOutputYuv->saveToFile((msFilename+std::string("/")+std::string("resultYUV")).c_str());
            pOutClean->saveToFile((msFilename+std::string("/")+std::string("cleanImg")).c_str());
            pOutputDepth->saveToFile((msFilename+std::string("/")+std::string("DepthMap")).c_str());
            pResizeYuv->saveToFile((msFilename+std::string("/")+std::string("resizeYUV")).c_str());
        }
        else{
            MY_LOGE("makePath failed!");
        }
    }

    response(params, android::OK);

    MY_LOGD("default plugin:request[%u] -", params.uRequestNo);
}

