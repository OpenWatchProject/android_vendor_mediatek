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

#define LOG_TAG "mtkcam-RequestMetadataPolicy"

#include "RequestMetadataPolicy.h"
//
#include "MyUtils.h"
#include <algorithm>
//
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_POLICY);
/******************************************************************************
 *
 ******************************************************************************/
using namespace android;


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

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define PrintCrop(crop) MY_LOGI("%s = (%d, %d), %dx%d", #crop, crop.p.x, crop.p.y, crop.s.w, crop.s.h)

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {
namespace requestmetadata {


/******************************************************************************
 *
 ******************************************************************************/
auto
RequestMetadataPolicy_Default::
evaluateP1YuvCrop(
    EvaluateRequestParams const& params
) -> int
{
    // if support P1 direct yuv, need calculate P1 FOV crop
    if (mPolicyParams.pPipelineUserConfiguration->pParsedAppConfiguration->useP1DirectFDYUV && params.FDYuvSize.size() != 0)
    {
        int Id = (*(params.pSensorId))[0];
        MRect ActiveArray = mvActiveArray[Id];
        MSize SensorSize = (*params.pSensorSize)[0];
        MSize rrzoBufSize = params.RrzoSize[0];
        MRect FDCrop;
        if (ActiveArray.s.size() == 0)
        {
            MY_LOGE("cannot get active array size, cam id : %d", Id);
            return -1;
        }
        IMetadata::IEntry P1Crop = (*params.pAdditionalHal)[0]->entryFor(MTK_P1NODE_SENSOR_CROP_REGION);
        if(P1Crop.isEmpty())
        {
            // if there is no p1 crop in hal metadata, p1node always use app crop and won't use FD scaler crop
            //MY_LOGD("have no p1 crop region");
            return OK;
        }
        MRect P1CropRegion = P1Crop.itemAt(0, Type2Type<MRect>());
        IMetadata::IEntry AppCrop = params.pRequest_AppControl->entryFor(MTK_SCALER_CROP_REGION);
        if (AppCrop.isEmpty())
        {
            MY_LOGW("cannot get scaler crop region");
            return OK;
        }

        if (rrzoBufSize.w != P1CropRegion.s.w || rrzoBufSize.h != P1CropRegion.s.h)
        {
            // current FD crop only be support whilde rrzo crop is same as rrzo dst size
            return OK;
        }

        /*
            Calculate P1 FD YUV crop. Because P1 FD crop is on sensor domain,
            calcluate sensor domain crop for P1 and active domain crop for FD node.
            The FD crop is inside P1 crop region
        */
        MRect AppCropRegion = AppCrop.itemAt(0, Type2Type<MRect>());
        int ratio_1024x = (AppCropRegion.s.w << 10) / ActiveArray.s.w;
        MSize FDYuvSize = params.FDYuvSize;
        // add p1 hw margin
        FDYuvSize.w = FDYuvSize.w*12/10;
        FDYuvSize.h = FDYuvSize.h*12/10;
        MY_LOGD("FDYuvSize : %dx%d", FDYuvSize.w, FDYuvSize.h);
        FDCrop.s.w = MAX(MIN((SensorSize.w * ratio_1024x) >> 10, P1CropRegion.s.w), FDYuvSize.w);
        FDCrop.s.h = MAX(MIN((SensorSize.h * ratio_1024x) >> 10, P1CropRegion.s.h), FDYuvSize.h);
        FDCrop.p.x = ((P1CropRegion.s.w - FDCrop.s.w) >> 1);//P1CropRegion.p.x + ((P1CropRegion.s.w - FDCrop.s.w) >> 1);
        FDCrop.p.y = ((P1CropRegion.s.h - FDCrop.s.h) >> 1);//P1CropRegion.p.y + ((P1CropRegion.s.h - FDCrop.s.h) >> 1);
        PrintCrop(P1CropRegion);
        PrintCrop(ActiveArray);
        PrintCrop(FDCrop);
        IMetadata::setEntry<MRect>((*params.pAdditionalHal)[0].get(), MTK_P1NODE_YUV_RESIZER1_CROP_REGION, FDCrop);


    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
RequestMetadataPolicy_Default::
updateRawImageInfo(
    EvaluateRequestParams const& params
) -> int
{
    for(size_t i=0;i<params.pAdditionalHal->size();i++)
    {
        IMetadata::IEntry entry(MTK_HAL_REQUEST_RAW_IMAGE_INFO);
        MSize imgoSize;
        MINT32 fmt = 0;
        MINT32 stride = 0;
        std::shared_ptr<IMetadata> meta = (*params.pAdditionalHal)[i];
        if(meta == nullptr) continue;
        if(params.additionalRawInfo != nullptr &&
           (params.additionalRawInfo->size()-1) >= i)
        {
            auto& data = (*params.additionalRawInfo)[i];
            imgoSize = data.imgSize;
            fmt = data.format;
            stride = data.stride;
            entry.push_back(fmt, Type2Type<MINT32>());
            entry.push_back(stride, Type2Type<MINT32>());
            entry.push_back(imgoSize.w, Type2Type<MINT32>());
            entry.push_back(imgoSize.h, Type2Type<MINT32>());
        }
        else
        {
            // update raw info from request image stream.
            if(params.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.size() != 0)
            {
                // logical raw stream
                auto iter =
                            params.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.find(
                                                mPolicyParams.pPipelineStaticInfo->openId);
                if(iter != params.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.end())
                {
                    auto imgoStreamInfo = iter->second;
                    imgoSize = imgoStreamInfo->getImgSize();
                    fmt = imgoStreamInfo->getImgFormat();

                    IMetadata::IEntry packedEntry = (params.pRequest_AppControl)->entryFor(MTK_CONTROL_CAPTURE_PACKED_RAW_ENABLE);
                    if( !packedEntry.isEmpty() && packedEntry.itemAt(0, Type2Type<MINT32>()) > 0){
                        stride = imgoStreamInfo->getImageBufferInfo().bufPlanes.planes[0].rowStrideInBytes;
                    }
                    else {
                        stride = imgoStreamInfo->getBufPlanes().planes[0].rowStrideInBytes;
                    }
                    entry.push_back(fmt, Type2Type<MINT32>());
                    entry.push_back(stride, Type2Type<MINT32>());
                    entry.push_back(imgoSize.w, Type2Type<MINT32>());
                    entry.push_back(imgoSize.h, Type2Type<MINT32>());
                }
            }
            if(params.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical.size() != 0)
            {
                if(params.pSensorId->size() != params.pAdditionalHal->size())
                {
                    MY_LOGW("params.pSensorId.size(%d) != params.pAdditionalHal->size(%d)",
                                params.pSensorId->size(),
                                params.pAdditionalHal->size());
                    continue;
                }
                auto sensorId = (*params.pSensorId)[i];
                auto iter =
                            params.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical.find(sensorId);
                if(iter != params.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical.end())
                {
                    imgoSize = iter->second[0]->getImgSize();
                    fmt = iter->second[0]->getImgFormat();
                    stride = iter->second[0]->getBufPlanes().planes[0].rowStrideInBytes;
                    entry.push_back(fmt, Type2Type<MINT32>());
                    entry.push_back(stride, Type2Type<MINT32>());
                    entry.push_back(imgoSize.w, Type2Type<MINT32>());
                    entry.push_back(imgoSize.h, Type2Type<MINT32>());
                }
                else
                {
                    MY_LOGE("cannot find id(%d) in vAppImage_Output_RAW16_Physical", sensorId);
                    continue;
                }
            }
        }
        if(meta->count() > 0)
        {
            meta->update(entry.tag(), entry);
            MY_LOGI("raw output: s(%dx%d) f(%x) stride(%d)",
                            imgoSize.w,
                            imgoSize.h,
                            fmt,
                            stride);
        }
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
RequestMetadataPolicy_Default::
RequestMetadataPolicy_Default(
    CreationParams const& params
)
    :mPolicyParams(params)
{
    for (size_t i = 0; i < params.pPipelineStaticInfo->sensorId.size(); i++)
    {
        mvTargetRrzoSize.push_back(MSize(0, 0));
        int Id = params.pPipelineStaticInfo->sensorId[i];
        MRect ActiveArray;
        sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(Id);
        if( ! pMetadataProvider.get() ) {
            CAM_ULOGME(" ! pMetadataProvider.get(), id : %d ", Id);
            mvActiveArray.emplace(Id, ActiveArray);
            continue;
        }

        IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();
        {
            IMetadata::IEntry active_array_entry = static_meta.entryFor(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION);
            if( !active_array_entry.isEmpty() ) {
                ActiveArray = active_array_entry.itemAt(0, Type2Type<MRect>());
            } else {
                CAM_ULOGME("no static info: MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION");
            }
            mvActiveArray.emplace(Id, ActiveArray);
        }
    }
    mUniqueKey = NSCam::Utils::TimeTool::getReadableTime();
    mvAppStreamId.clear();
    mForceExif = property_get_int32("vendor.debug.camera.dump.isp.preview", 0);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
RequestMetadataPolicy_Default::
evaluateRequest(
    EvaluateRequestParams const& params
) -> int
{
    // update request unique key
    if (params.pAdditionalHal->size() > 0)
    {
        // check any entry contain MTK_PIPELINE_UNIQUE_KEY or not.
        IMetadata::IEntry entry(MTK_PIPELINE_UNIQUE_KEY);
        IMetadata::IEntry entry1;
        for(size_t i=0;i<params.pAdditionalHal->size();i++)
        {
            if((*params.pAdditionalHal)[i] != nullptr)
            {
                entry1 = (*params.pAdditionalHal)[i]->entryFor(MTK_PIPELINE_UNIQUE_KEY);
                break;
            }
        }
        if(entry1.isEmpty())
        {
            entry.push_back(mUniqueKey, Type2Type<MINT32>());
        }
        else
        {
            entry = entry1;
        }
        for(size_t i=0;i<params.pAdditionalHal->size();++i){
            if((*params.pAdditionalHal)[i] != nullptr) {
                (*params.pAdditionalHal)[i]->update(entry.tag(), entry);
            }
        }
    }

    // update request id
    {
        IMetadata::IEntry entry(MTK_PIPELINE_REQUEST_NUMBER);
        entry.push_back(params.requestNo, Type2Type<MINT32>());
        for(size_t i=0;i<params.pAdditionalHal->size();++i){
            if((*params.pAdditionalHal)[i] != nullptr) {
            (*params.pAdditionalHal)[i]->update(entry.tag(), entry);
            }
        }
    }

    // common metadata
    {
        MINT64 iMinFrmDuration = 0;
        std::vector<int64_t>  vStreamId;
        vStreamId.clear();
        for (auto const& it : params.pRequest_AppImageStreamInfo->vAppImage_Output_Proc)
        {
            StreamId_T const streamId = it.first;
            auto minFrameDuration = mPolicyParams.pPipelineUserConfiguration->vMinFrameDuration.find(streamId);
            vStreamId.push_back(streamId);
            if ( minFrameDuration == mPolicyParams.pPipelineUserConfiguration->vMinFrameDuration.end() )
            {
                MY_LOGD("Request App stream %#" PRIx64 "may be customize stream", streamId);
                continue;
            }
            if ( std::find(mvAppStreamId.begin(), mvAppStreamId.end(), streamId) == mvAppStreamId.end())
            {
                continue;
            }
            if ( CC_UNLIKELY(minFrameDuration->second < 0) ) {
                MY_LOGE("Request App stream %#" PRIx64 "have not configured yet", streamId);
                continue;
            }
            iMinFrmDuration = ( minFrameDuration->second > iMinFrmDuration ) ?
                                minFrameDuration->second : iMinFrmDuration;
        }
        mvAppStreamId = vStreamId;
        //MY_LOGD( "The min frame duration is %" PRId64, iMinFrmDuration);
        IMetadata::IEntry entry(MTK_P1NODE_MIN_FRM_DURATION);
        entry.push_back(iMinFrmDuration, Type2Type<MINT64>());
        for(size_t i=0;i<params.pAdditionalHal->size();++i){
            if((*params.pAdditionalHal)[i] != nullptr) {
                (*params.pAdditionalHal)[i]->update(entry.tag(), entry);
            }
        }
    }
    MUINT8 bRepeating = (MUINT8)(params.pRequest_ParsedAppMetaControl ? params.pRequest_ParsedAppMetaControl->repeating : false);
    {
        IMetadata::IEntry entry(MTK_HAL_REQUEST_REPEAT);
        entry.push_back(bRepeating, Type2Type< MUINT8 >());
        for(size_t i=0;i<params.pAdditionalHal->size();++i){
            if((*params.pAdditionalHal)[i] != nullptr) {
                (*params.pAdditionalHal)[i]->update(entry.tag(), entry);
            }
        }
        //MY_LOGD("Control AppMetadata is repeating(%d)", bRepeating);
    }
    {
        if ( params.isZSLMode
          || params.pRequest_AppImageStreamInfo->pAppImage_Jpeg != nullptr
          || params.pRequest_AppImageStreamInfo->pAppImage_Output_Priv != nullptr
          || params.needExif
          || mForceExif
           )
        {
            //MY_LOGD("set MTK_HAL_REQUEST_REQUIRE_EXIF = 1");
            IMetadata::IEntry entry2(MTK_HAL_REQUEST_REQUIRE_EXIF);
            entry2.push_back(1, Type2Type<MUINT8>());
            for(size_t i=0;i<params.pAdditionalHal->size();++i){
                if((*params.pAdditionalHal)[i] != nullptr) {
                    (*params.pAdditionalHal)[i]->update(entry2.tag(), entry2);
                }
            }
        }

        for(size_t i=0;i<params.pAdditionalHal->size();++i)
        {
            if((*params.pAdditionalHal)[i] != nullptr) {
                IMetadata::IEntry entry3(MTK_HAL_REQUEST_SENSOR_SIZE);
                entry3.push_back((*params.pSensorSize)[i], Type2Type<MSize>());
                (*params.pAdditionalHal)[i]->update(entry3.tag(), entry3);

                IMetadata::IEntry entrySenId(MTK_HAL_REQUEST_SENSOR_ID);
                entrySenId.push_back(mPolicyParams.pPipelineStaticInfo->sensorId[i], Type2Type<MINT32>());
                (*params.pAdditionalHal)[i]->update(entrySenId.tag(), entrySenId);

                IMetadata::IEntry entryDevId(MTK_HAL_REQUEST_DEVICE_ID);
                entryDevId.push_back(mPolicyParams.pPipelineStaticInfo->openId, Type2Type<MINT32>());
                (*params.pAdditionalHal)[i]->update(entryDevId.tag(), entryDevId);
            }
        }
    }
    if (params.pRequest_AppImageStreamInfo->hasVideoConsumer)
    {
        IMetadata::IEntry entry(MTK_PIPELINE_VIDEO_RECORD);
        entry.push_back(1, Type2Type< MINT32 >());
        for(size_t i=0;i<params.pAdditionalHal->size();++i){
            if((*params.pAdditionalHal)[i] != nullptr) {
                (*params.pAdditionalHal)[i]->update(entry.tag(), entry);
            }
        }
        //MY_LOGD("Control AppMetadata is repeating(%d)", bRepeating);
    }

    if (params.pRequest_AppImageStreamInfo->hasVideoConsumer)
    {
        IMetadata::IEntry entry(MTK_PIPELINE_VIDEO_RECORD);
        entry.push_back(1, Type2Type< MINT32 >());
        for(size_t i=0;i<params.pAdditionalHal->size();++i){
            if((*params.pAdditionalHal)[i] != nullptr) {
                (*params.pAdditionalHal)[i]->update(entry.tag(), entry);
            }
        }
        //MY_LOGD("Control AppMetadata is repeating(%d)", bRepeating);
    }

    // for process raw
    int force_process = ::property_get_int32("vendor.debug.camera.app.process.raw", 0);
    if ( (params.pRequest_ParsedAppMetaControl->control_processRaw == 1 || force_process == 1) && !(mPolicyParams.pPipelineStaticInfo->isNeedP2ProcessRaw))
    {
        IMetadata::IEntry entry(MTK_P1NODE_RAW_TYPE);
        entry.push_back(NSIoPipe::NSCamIOPipe::EPipe_PROCESSED_RAW, Type2Type< MINT32 >());
        for(size_t i=0;i<params.pAdditionalHal->size();++i) {
            if (CC_LIKELY((*params.pAdditionalHal)[i] != nullptr))
            {
                if (((*params.pAdditionalHal)[i]->entryFor(MTK_P1NODE_RAW_TYPE)).isEmpty())
                {
                    (*params.pAdditionalHal)[i]->update(entry.tag(), entry);
                }
                else
                {
                    MY_LOGI("MTK_P1NODE_RAW_TYPE already existed, index(%d)", i);
                }
            }
        }
    }

    #if 0
    {
        for(size_t i = 0; i < params.pAdditionalHal->size(); ++i)
        {
            if (!params.needReconfigure)
            {
                if (!bRepeating || mvTargetRrzoSize[i].size() == 0)
                {
                    IMetadata::IEntry Crop = (*params.pAdditionalHal)[i]->entryFor(MTK_P1NODE_SENSOR_CROP_REGION);
                    if( Crop.isEmpty() ) {
                        Crop = params.pRequest_AppControl->entryFor(MTK_SCALER_CROP_REGION);
                        if (Crop.isEmpty())
                        {
                            MY_LOGW("cannot get scaler crop region, index : %zu", i);
                            continue;
                        }
                    }
                    MSize rrzoBufSize = params.RrzoSize[i];
                    MRect cropRegion = Crop.itemAt(0, Type2Type<MRect>());
                    #define ALIGN16(x) (((x) + 15) & ~(15))
                    if((cropRegion.s.w * rrzoBufSize.h) > (cropRegion.s.h * rrzoBufSize.w))
                    {
                        MINT32 temp = rrzoBufSize.h;
                        rrzoBufSize.h = ALIGN16(rrzoBufSize.w * cropRegion.s.h / cropRegion.s.w);
                        if (rrzoBufSize.h > temp)
                        {
                            rrzoBufSize.h = temp;
                        }
                    }
                    else
                    {
                        MINT32 temp = rrzoBufSize.w;
                        rrzoBufSize.w = ALIGN16(rrzoBufSize.h * cropRegion.s.w / cropRegion.s.h);
                        if (rrzoBufSize.w > temp)
                        {
                            rrzoBufSize.w = temp;
                        }
                    }
                    #undef ALIGN16
                    //mvTargetRrzoSize.push_back(rrzoBufSize);
                    mvTargetRrzoSize[i].w = rrzoBufSize.w;
                    mvTargetRrzoSize[i].h = rrzoBufSize.h;
                }
                IMetadata::IEntry Rrzotag(MTK_P1NODE_RESIZER_SET_SIZE);
                Rrzotag.push_back(mvTargetRrzoSize[i], Type2Type<MSize>());
                (*params.pAdditionalHal)[i]->update(MTK_P1NODE_RESIZER_SET_SIZE, Rrzotag);
            }
            else
            {
                mvTargetRrzoSize[i].w = 0;
                mvTargetRrzoSize[i].h = 0;
            }
        }
    }
    #endif

    if (params.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.size() == 0 &&
        params.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical.size() == 0 &&
        params.pSensorId != nullptr && !params.needReconfigure)
    {
        do
        {
            auto sizeCheck =
                    (params.pAdditionalHal->size() == params.pSensorId->size()) &&
                    (params.pAdditionalHal->size() == params.pSensorSize->size());
            if(!sizeCheck)
            {
                MY_LOGE("additionHal.size(%zu) sensorId.size(%zu) sensorSize.size(%zu)",
                        params.pAdditionalHal->size(),
                        params.pSensorId->size(),
                        params.pSensorSize->size());
            }
            else
            {
                for(size_t i=0;i<params.pAdditionalHal->size();i++)
                {
                    if((*params.pAdditionalHal)[i] != nullptr)
                    {
                        IMetadata::IEntry Crop = (*params.pAdditionalHal)[i]->entryFor(MTK_P1NODE_SENSOR_CROP_REGION);
                        if( !(Crop.isEmpty()) )
                        {
                            break;
                        }
                        int Id = (*(params.pSensorId))[i];
                        MRect ActiveArray = mvActiveArray[Id];
                        if (ActiveArray.s.size() == 0)
                        {
                            MY_LOGE("cannot get active array size, cam id : %d", Id);
                            break;
                        }

                        Crop = params.pRequest_AppControl->entryFor(MTK_SCALER_CROP_REGION);
                        if (Crop.isEmpty())
                        {
                            MY_LOGW("cannot get scaler crop region, index : %zu", i);
                            break;
                        }
                        //MSize sensorSize = (*params.pSensorSize)[i];
                        MRect cropRegion = Crop.itemAt(0, Type2Type<MRect>());
                        MSize toSensorSize;
                        MSize FixedRRZOSize;
                        toSensorSize.w = (*params.pSensorSize)[i].w * cropRegion.s.w / ActiveArray.s.w;
                        toSensorSize.h = (*params.pSensorSize)[i].h * cropRegion.s.h / ActiveArray.s.h;

                        if (params.fixedRRZOSize.w != 0 && params.fixedRRZOSize.h != 0)
                        {
                            FixedRRZOSize = params.fixedRRZOSize;
                        }
                        else
                        {
                            FixedRRZOSize = params.RrzoSize[i];
                        }

                        if (toSensorSize.w > FixedRRZOSize.w || toSensorSize.h > FixedRRZOSize.h)
                        {
                            #if 0
                            if (cropRegion.p.x == 0 && cropRegion.p.y == 0 && cropRegion.s.w == ActiveArray.s.w && cropRegion.s.h == ActiveArray.s.h)
                            {
                                cropRegion.p.x = 2;
                                cropRegion.p.y = 2;
                                cropRegion.s.w = (*params.pSensorSize)[i].w - 4;
                                cropRegion.s.h = (*params.pSensorSize)[i].h - 4;
                                IMetadata::setEntry<MRect>((*params.pAdditionalHal)[i].get(), MTK_P1NODE_SENSOR_CROP_REGION, cropRegion);
                            }
                            #endif
                        }
                        else
                        {
                            MY_LOGD("fix rrzo size : %dx%d, toSensorSize : %dx%d", FixedRRZOSize.w, FixedRRZOSize.h
                                                                                 , toSensorSize.w, toSensorSize.h);
                            if (FixedRRZOSize.w < (*params.pSensorSize)[i].w && FixedRRZOSize.h < (*params.pSensorSize)[i].h)
                            {
                                cropRegion.s.w = FixedRRZOSize.w;
                                cropRegion.s.h = FixedRRZOSize.h;
                                cropRegion.p.x = ((*params.pSensorSize)[i].w - FixedRRZOSize.w) >> 1;
                                cropRegion.p.y = ((*params.pSensorSize)[i].h - FixedRRZOSize.h) >> 1;
                                IMetadata::setEntry<MRect>((*params.pAdditionalHal)[i].get(), MTK_P1NODE_SENSOR_CROP_REGION, cropRegion);
                            }
                        }
                    }
                }
            }
        } while(0);

    }

    evaluateP1YuvCrop(params);

    if(params.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.size() > 0 ||
       params.pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical.size() > 0)
    {
        updateRawImageInfo(params);
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto makePolicy_RequestMetadata_Default(
    CreationParams const& params
) -> std::shared_ptr<IRequestMetadataPolicy>
{
    return std::make_shared<RequestMetadataPolicy_Default>(params);
}


/******************************************************************************
 *
 ******************************************************************************/
RequestMetadataPolicy_DebugDump::
RequestMetadataPolicy_DebugDump(
    CreationParams const& params
)
    :mPolicyParams(params)
{
}


/******************************************************************************
 *
 ******************************************************************************/
auto
RequestMetadataPolicy_DebugDump::
evaluateRequest(
    EvaluateRequestParams const& params
) -> int
{
    if ( CC_LIKELY(mPolicyParams.pRequestMetadataPolicy != nullptr) )
    {
        mPolicyParams.pRequestMetadataPolicy->evaluateRequest(params);
    }

    //
    int debugRayType = property_get_int32("vendor.debug.camera.raw.type", -1);
    if(debugRayType >= 0 && !(mPolicyParams.pPipelineStaticInfo->isNeedP2ProcessRaw))
    {
        MY_LOGD("set vendor.debug.camera.raw.type(%d) => MTK_P1NODE_RAW_TYPE(%d)  0:processed-raw 1:pure-raw",debugRayType,debugRayType);
        IMetadata::IEntry entry(MTK_P1NODE_RAW_TYPE);
        entry.push_back(debugRayType, Type2Type< int >());
        for(size_t i=0;i<params.pAdditionalHal->size();++i) {
            if (CC_LIKELY((*params.pAdditionalHal)[i] != nullptr))
            {
                (*params.pAdditionalHal)[i]->update(entry.tag(), entry);
            }
        }
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
auto makePolicy_RequestMetadata_DebugDump(
    CreationParams const& params
) -> std::shared_ptr<IRequestMetadataPolicy>
{
    return std::make_shared<RequestMetadataPolicy_DebugDump>(params);
}
};  //namespace requestmetadata
};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam

