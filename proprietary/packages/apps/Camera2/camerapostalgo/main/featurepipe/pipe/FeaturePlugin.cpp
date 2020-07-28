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

#include <pipe/FeaturePlugin.h>
#include <pipe/FeatureRequest.h>

// Logging
#define PIPE_CLASS_TAG "PluginHandle"
#define PIPE_TRACE 0
#include <core/PipeLog.h>

using namespace std;

//#define __DEBUG

/*******************************************************************************
* Namespace start.
********************************************************************************/

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {



/*******************************************************************************
* Class Define
*******************************************************************************/

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PluginHelper Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
NSCam::NSPipelinePlugin::BufferHandle::Ptr
PluginHelper::
CreateBuffer(sp<FeatureNodeRequest> pNodeRequest, TypeID_T typeId, Direction dir)
{
    BufferID_T bufId = pNodeRequest->mapBufferID(typeId, dir);
    return (bufId == NULL_BUFFER)
            ? nullptr
            : make_shared<PluginBufferHandle>(pNodeRequest, bufId);
}

NSCam::NSPipelinePlugin::BufferHandle::Ptr
PluginHelper::
CreateBuffer(sp<FeatureNodeRequest> pNodeRequest, BufferID_T bufId)
{
    return (bufId == NULL_BUFFER)
            ? nullptr
            : make_shared<PluginBufferHandle>(pNodeRequest, bufId);
}

NSCam::NSPipelinePlugin::MetadataHandle::Ptr
PluginHelper::
CreateMetadata(sp<FeatureNodeRequest> pNodeRequest, MetadataID_T metaId)
{
    if (!pNodeRequest->hasMetadata(metaId))
        return nullptr;
    return make_shared<PluginMetadataHandle>(pNodeRequest, metaId);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PluginBufferHandle Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PluginBufferHandle::
PluginBufferHandle(sp<FeatureNodeRequest> pNodeRequest, BufferID_T bufId)
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
//  PluginMetadataHandle Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PluginMetadataHandle::
PluginMetadataHandle(sp<FeatureNodeRequest> pNodeRequest, MetadataID_T metaId)
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


} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com


