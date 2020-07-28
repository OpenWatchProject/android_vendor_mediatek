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

#define LOG_TAG "JpgPack"
//
#include <sys/prctl.h>
#include <sys/resource.h>
#include <cutils/properties.h>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
//
#include "JpgPack.h"

#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_JPEG_NODE);

#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_ULOGM_ASSERT((1), "[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_ULOGM_FATAL("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace stereo;

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
JpgPack::
JpgPack()
{
}


/******************************************************************************
 *
 ******************************************************************************/
JpgPack::
~JpgPack()
{
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpgPack::
init()
{
    FUNC_START;
    //
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpgPack::
config(ConfigJpgParams const& rParams)
{
    FUNC_START;
    //
    mpIn_BokehJpg       = rParams.pIn_BokehJpg;
    mpIn_CleanJpg       = rParams.pIn_CleanJpg;
    mpIn_DepthmapYuv    = rParams.pIn_DepthmapYuv;
    mpOut_ResultJpg     = rParams.pOut_ResultJpg;
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
JpgPack::
process()
{
    FUNC_START;
    //
    // StereoInfoAccessor library
    sp<StereoInfoAccessor> pAccessor = new StereoInfoAccessor();
    //
    // convert IImageBuffer to StereoBuffer_t
    StereoBuffer_t bokehBuffer;
    StereoBuffer_t cleanBuffer;
    StereoBuffer_t depthmapBuffer;

    auto copyData = [] (sp<IImageBuffer>& imageBuf, StereoBuffer_t& stereoBuf) {
        MUINT32 uBufSize = (imageBuf->getBitstreamSize() > 0) ?
            imageBuf->getBitstreamSize() : imageBuf->getBufSizeInBytes(0);
        unsigned char *pInVa = (unsigned char *) (imageBuf->getBufVA(0));
        stereoBuf = {pInVa, uBufSize};
    };
    // Bokeh image
    copyData(mpIn_BokehJpg, bokehBuffer);
    // Clean image
    copyData(mpIn_CleanJpg, cleanBuffer);
    // Depthmap
    copyData(mpIn_DepthmapYuv, depthmapBuffer);

    // prepare StereoCaptureInfo
    StereoCaptureInfo captureInfo;
    captureInfo.jpgBuffer = bokehBuffer;
    captureInfo.clearImage = cleanBuffer;
    captureInfo.depthBuffer = depthmapBuffer;

    StereoBuffer_t packedJpgBuffer;
    MUINT32 finalSize = 0;
    // call pack API to pack bokeh image, clean image and depthmap
    pAccessor->writeStereoCaptureInfo(captureInfo, packedJpgBuffer);
    // convert StereoBuffer_t to IImageBuffer
    if(packedJpgBuffer.isValid())
    {
        unsigned char *pOutVa = (unsigned char *) (mpOut_ResultJpg->getBufVA(0));
        memcpy(pOutVa, packedJpgBuffer.data, packedJpgBuffer.size);
        finalSize = packedJpgBuffer.size;
    }
    else
    {
        // copy bokeh image to result buffer
        unsigned char *pInVa = (unsigned char *) (mpIn_BokehJpg->getBufVA(0));
        unsigned char *pOutVa = (unsigned char *) (mpOut_ResultJpg->getBufVA(0));
        MUINT32 uBufSize = (mpIn_BokehJpg->getBitstreamSize() > 0) ?
            mpIn_BokehJpg->getBitstreamSize() : mpIn_BokehJpg->getBufSizeInBytes(0);
        memcpy(pOutVa, pInVa, uBufSize);
        finalSize = uBufSize;
    }
    //
    FUNC_END;
    return finalSize;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpgPack::
uninit()
{
    FUNC_START;
    //

    //
    FUNC_END;
    return OK;
}