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

#include "CaptureFeaturePlugin.h"
#include "CaptureFeatureRequest.h"

// Logging
#define PIPE_CLASS_TAG "PluginHandle"
#define PIPE_TRACE TRACE_BOKEH_NODE
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_CAPTURE);

//#define __DEBUG

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSCapture {


/*******************************************************************************
* Class Define
*******************************************************************************/

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PluginHelper Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
NSPipelinePlugin::BufferHandle::Ptr
PluginHelper::
CreateBuffer(sp<CaptureFeatureNodeRequest> pNodeRequest, TypeID_T typeId, Direction dir)
{
    BufferID_T bufId = pNodeRequest->mapBufferID(typeId, dir);
    return (bufId == NULL_BUFFER)
            ? nullptr
            : make_shared<PluginBufferHandle>(pNodeRequest, bufId);
}

NSPipelinePlugin::BufferHandle::Ptr
PluginHelper::
CreateBuffer(sp<CaptureFeatureNodeRequest> pNodeRequest, BufferID_T bufId)
{
    return (bufId == NULL_BUFFER)
            ? nullptr
            : make_shared<PluginBufferHandle>(pNodeRequest, bufId);
}

NSPipelinePlugin::MetadataHandle::Ptr
PluginHelper::
CreateMetadata(sp<CaptureFeatureNodeRequest> pNodeRequest, MetadataID_T metaId)
{
    if (!pNodeRequest->hasMetadata(metaId))
        return nullptr;
    return make_shared<PluginMetadataHandle>(pNodeRequest, metaId);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PluginBufferHandle Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PluginBufferHandle::
PluginBufferHandle(sp<CaptureFeatureNodeRequest> pNodeRequest, BufferID_T bufId)
    : mpNodeRequest(pNodeRequest)
    , mpImageBuffer(NULL)
    , mBufferId(bufId)
{

}

PluginBufferHandle::
~PluginBufferHandle()
{
    if (mpImageBuffer != NULL) {
        MY_LOGW("did NOT release plugin buffer:%d", mBufferId);
        release();
    }
}

IImageBuffer*
PluginBufferHandle::acquire(MINT usage)
{
    (void) usage;
    if(mpImageBuffer == NULL) {
        auto pNodeRequest = mpNodeRequest.promote();
        if (pNodeRequest == NULL)
            return NULL;

        mpImageBuffer = pNodeRequest->acquireBuffer(mBufferId);
        if (mpImageBuffer == NULL) {
            MY_LOGW("acquire an empty buffer, bufId:%d", mBufferId);
        }
#ifdef __DEBUG
        MY_LOGI("Fmt(0x%x) ImgSize(%dx%d) BufSize(%d) Stride(%d) VA/PA(%#" PRIxPTR "/%#" PRIxPTR ")",
               mpImageBuffer->getImgFormat(),
               mpImageBuffer->getImgSize().w,
               mpImageBuffer->getImgSize().h,
               mpImageBuffer->getBufSizeInBytes(0),
               mpImageBuffer->getBufStridesInBytes(0),
               mpImageBuffer->getBufVA(0),
               mpImageBuffer->getBufPA(0));
#endif
    }
    return mpImageBuffer;
}

MVOID
PluginBufferHandle::
release()
{
    if(mpImageBuffer != NULL) {
        mpImageBuffer = NULL;

        auto pNodeRequest = mpNodeRequest.promote();
        if (pNodeRequest == NULL)
            return;

        pNodeRequest->releaseBuffer(mBufferId);
    }
}


MVOID
PluginBufferHandle::
dump(std::ostream& os) const
{
    if (mpImageBuffer == NULL)
        os << "{ null }" << std::endl;
    else
        os << "{address: " << mpImageBuffer << "}" << std::endl;;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PluginSmartBufferHandle Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
std::shared_ptr<PluginSmartBufferHandle>
PluginSmartBufferHandle::
createInstance(const char* name, MINT fmt, MINT32 sensorIdx, const IImageBuffer* pSrcImgBuf, const sp<CaptureBufferPool>& bufPool)
{
    const MINT srcImgFmt = pSrcImgBuf->getImgFormat();
    const MSize srcImgSize = pSrcImgBuf->getImgSize();
    const MSize alignment = HwStrideAlignment::queryFormatAlignment(fmt);
    //
    android::sp<IIBuffer> dstImgBuf = bufPool->getImageBuffer(srcImgSize, fmt, alignment);
//    const MINT dstImgFmt = dstImgBuf->getImageBuffer()->getImgFormat();
    const MSize dstImgSize = dstImgBuf->getImageBuffer()->getImgSize();
    if (!formatConverter(sensorIdx, pSrcImgBuf, dstImgBuf->getImageBufferPtr()))
    {
        MY_LOGE("failed to execute format convert, name:%s src:%p, dst:%p", name, pSrcImgBuf, dstImgBuf->getImageBufferPtr());
        return NULL;
    }

    MY_LOGD("create instance, name:%s, sensorIdx:%d, dstAddr:%p, dstFmt:%s, dstSize:%dx%d, srcAddr:%p, srcFmt:%s, srcSize:%dx%d, poolAddr:%p",
        name, sensorIdx,
        dstImgBuf->getImageBufferPtr(), ImgFmt2Name(fmt), dstImgSize.w, dstImgSize.h,
        pSrcImgBuf, ImgFmt2Name(srcImgFmt), srcImgSize.w, srcImgSize.h,
        bufPool.get());

    return PluginSmartBufferHandle::createInstance(name, dstImgBuf);
}

std::shared_ptr<PluginSmartBufferHandle>
PluginSmartBufferHandle::
createInstance(const char* name, MINT fmt, MSize size, const sp<CaptureBufferPool>& bufPool)
{
    MSize alignment = HwStrideAlignment::queryFormatAlignment(fmt);
    android::sp<IIBuffer> imgBuffer = bufPool->getImageBuffer(size, fmt, alignment);
    const MSize imgSize = imgBuffer->getImageBuffer()->getImgSize();

    MY_LOGD("create instance, name:%s, addr:%p, fmt:%s, size:%dx%d, alignmentSize:%dx%d, poolAddr:%p",
        name, imgBuffer->getImageBufferPtr(), ImgFmt2Name(fmt), size.w, size.h, imgSize.w, imgSize.h, bufPool.get());

    return createInstance(name, imgBuffer);
}

std::shared_ptr<PluginSmartBufferHandle>
PluginSmartBufferHandle::
createInstance(const char* name, const android::sp<IIBuffer>& imgBuffer)
{
    return std::shared_ptr<PluginSmartBufferHandle>(new PluginSmartBufferHandle(name, imgBuffer));
}

PluginSmartBufferHandle::
PluginSmartBufferHandle(const char* name, const android::sp<IIBuffer>& imgBuffer)
    : mName(name)
    , mImgBuffer(imgBuffer)
{
    if (mImgBuffer == NULL) {
        MY_LOGW("construct an invalid handle, name:%s", mName.c_str());
    }
}

PluginSmartBufferHandle::
~PluginSmartBufferHandle()
{
    if (mImgBuffer != NULL) {
        MY_LOGW("did NOT release plugin buffer, name:%s", mName.c_str());
        release();
    }
}

IImageBuffer*
PluginSmartBufferHandle::
acquire(MINT usage)
{
    (void) usage;
    if (mImgBuffer == NULL)
        return NULL;

    return mImgBuffer->getImageBufferPtr();
}

MVOID
PluginSmartBufferHandle::
release()
{
    if (mImgBuffer != NULL) {
        if (mOnReleasingCallback != NULL) {
            mOnReleasingCallback(acquire(eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN));
        }
        mImgBuffer = NULL;
    }
}

MVOID
PluginSmartBufferHandle::
dump(std::ostream& os) const
{
    os << "{ not implementation }" << std::endl;
}

android::sp<IIBuffer>
PluginSmartBufferHandle::
getSmartImageBuffer()
{
    return mImgBuffer;
}

MVOID
PluginSmartBufferHandle::
setOnReleasingCallback(OnReleasingCallback onReleasingCB)
{
    mOnReleasingCallback = onReleasingCB;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PluginMetadataHandle Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PluginMetadataHandle::
PluginMetadataHandle(sp<CaptureFeatureNodeRequest> pNodeRequest, MetadataID_T metaId)
    : mpNodeRequest(pNodeRequest)
    , mpMetadata(NULL)
    , mMetaId(metaId)
{

}

PluginMetadataHandle::
~PluginMetadataHandle()
{
    if (mpMetadata != NULL) {
        MY_LOGW("did NOT release plugin metadata:%d", mMetaId);
        release();
    }
}

IMetadata*
PluginMetadataHandle::
acquire()
{
    if (mpMetadata == NULL) {
        auto pNodeRequest = mpNodeRequest.promote();
        if (pNodeRequest == NULL)
            return NULL;
        mpMetadata = pNodeRequest->acquireMetadata(mMetaId);
    }
    return mpMetadata;
}

MVOID
PluginMetadataHandle::
release()
{
    if(mpMetadata != NULL) {
        mpMetadata = NULL;

        auto pNodeRequest = mpNodeRequest.promote();
        if (pNodeRequest == NULL)
            return;
        pNodeRequest->releaseMetadata(mMetaId);
    }
}

MVOID
PluginMetadataHandle::
dump(std::ostream& os) const
{
    if (mpMetadata == NULL)
        os << "{ null }" << std::endl;
    else
        os << "{count: " << mpMetadata->count() << "}" << std::endl;;
}


} // NSCapture
} // NSFeaturePipe
} // NSCamFeature
} // NSCam
