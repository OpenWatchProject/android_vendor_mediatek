/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "MtkCam/ISPStreamController"
//
#include <log/log.h>
#include <utils/Errors.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
//
#include <mtkcam3/pipeline/hwnode/StreamId.h>
#include <mtkcam3/pipeline/stream/IStreamInfo.h>
#include <mtkcam/aaa/IIspMgr.h>
#include <mtkcam/aaa/IHalISP.h>
#include <mtkcam3/pipeline/utils/packutils/PackUtils.h>
// for rrzo temp patch
#include <mtkcam3/feature/bsscore/IBssCore.h>
//
#include "ISPStreamController.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_ISP_HAL_SERVER);

namespace NSCam {
namespace ISPHal {

#define MY_LOGV(fmt, arg...)        CAM_ULOGMV(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)

#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#if 1
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif
//
#if 1
#define MY_LOG_DEBUG(fmt, arg...)   CAM_LOGD(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#else
#define MY_LOG_DEBUG(fmt, arg...)   ((void)0)
#endif
//

using MetaStreamInfo = NSCam::v3::Utils::MetaStreamInfo;
using ImageStreamInfoBuilder = NSCam::v3::Utils::ImageStreamInfoBuilder;
using IImageStreamInfo = NSCam::v3::IImageStreamInfo;
using ISPStreamBufferHandle = NSCam::v3::ISPImageStreamBuffer::ISPStreamBufferHandle;

using android::String8;
using ::android::hardware::graphics::common::V1_0::PixelFormat;
using ::android::hardware::graphics::common::V1_0::Dataspace;
using ::android::hardware::hidl_handle;

using namespace vendor::mediatek::hardware::camera::isphal::V1_0;
using namespace NSCam::v3;
using namespace NSCam::ISPHal;

template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata const* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if (pMetadata == NULL) {
        CAM_LOGW("pMetadata == NULL");
        return MFALSE;
    }
    //
    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if(!entry.isEmpty()) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    //
    return MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
ISPStreamController::
~ISPStreamController()
{
    MY_LOGI("destroy ISP Stream Controller: mId(%d)", mId);
    mMetaConverter = nullptr;
}

/******************************************************************************
 *
 ******************************************************************************/
ISPStreamController::
ISPStreamController(int32_t id, android::sp<ISPMetadataConverter> converter)
    : mId(id)
    , mMetaConverter(converter)
{
    mLogLevel = property_get_int32("vendor.debug.camera.log.ispstreamctrl", 0);
    MY_LOGI("create ISP Stream Controller: mId(%d), mMetaConverter(%p)", id, converter.get());
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ISPStreamController::
configure(
    const V1_0::ISPStreamConfiguration& config,
    std::shared_ptr<UserConfigurationParams>& params
) -> int
{
    FUNC_START;
    //dump the input ISPStreamConfiguration
    {
        MY_LOG_DEBUG("Dump the input ISPStreamConfiguration");
        //input
        for (size_t i = 0; i < config.InputStreams.size(); i++){
            const auto& stream = config.InputStreams[i];
            MY_LOG_DEBUG("Input: id(%" PRIu64 "), size(%u), w(%u), h(%u), format(0x%x), usage(%#" PRIx64 ")",
                stream.id, stream.size, stream.width, stream.height, stream.format.format, stream.usage);
        }
        //output
        for (size_t i = 0; i < config.OutputStreams.size(); i++){
            const auto& stream = config.OutputStreams[i];
            MY_LOG_DEBUG("Output: id(%" PRIu64 "), size(%u), w(%u), h(%u), format(0x%x), usage(%#" PRIx64 ")",
                stream.id, stream.size, stream.width, stream.height, stream.format.format, stream.usage);
        }
    }
    //
    params = std::make_shared<UserConfigurationParams>();

    //convert the configParams to IMetadata
    if ( ! mMetaConverter->convertToIMetadata(config.ConfigureParams.data(), params->sessionParams) )
    {
        MY_LOGE("convert config params failed");
        return -1;
    }

    //create image stream info
    {
        MBOOL needOutTuning = false;
        //input
        for (size_t i = 0; i < config.InputStreams.size(); i++){
            auto pImageStreamInfo = createImageStreamInfo(config.InputStreams[i], eSTREAMTYPE_IMAGE_IN);
            if ( pImageStreamInfo != nullptr ){
                params->vImageStreams.insert({pImageStreamInfo->getStreamId(), pImageStreamInfo});
            }
            else{
                return -1;
            }
        }

        //output
        for (size_t i = 0; i < config.OutputStreams.size(); i++){
            auto pImageStreamInfo = createImageStreamInfo(config.OutputStreams[i], eSTREAMTYPE_IMAGE_OUT);
            if ( pImageStreamInfo != nullptr ){
                params->vImageStreams.insert({pImageStreamInfo->getStreamId(), pImageStreamInfo});
            }
            else{
                return -1;
            }
            if (pImageStreamInfo->getImgFormat() != eImgFmt_BLOB && pImageStreamInfo->getImgFormat() != eImgFmt_JPEG)
            {
                needOutTuning = true;
            }
        }
        //HAL stream
        {
            auto pImageStreamInfo = createImageStreamInfo(eSTREAMID_IMAGE_ISP_DEVICE_INPUT_LCSO, eSTREAMTYPE_IMAGE_IN);
            if ( pImageStreamInfo != nullptr ){
                params->vImageStreams.insert({pImageStreamInfo->getStreamId(), pImageStreamInfo});
            }
            else{
                return -1;
            }
        }
        {
            auto pImageStreamInfo = createImageStreamInfo(eSTREAMID_IMAGE_ISP_DEVICE_INPUT_RESIZED, eSTREAMTYPE_IMAGE_IN);
            if ( pImageStreamInfo != nullptr ){
                params->vImageStreams.insert({pImageStreamInfo->getStreamId(), pImageStreamInfo});
            }
            else{
                return -1;
            }
        }
        //tuning stream
        if ( needOutTuning ){
            auto pImageStreamInfo = createImageStreamInfo(eSTREAMID_IMAGE_ISP_DEVICE_OUTPUT_TUNING, eSTREAMTYPE_IMAGE_OUT);
            if ( pImageStreamInfo != nullptr ){
                params->vImageStreams.insert({pImageStreamInfo->getStreamId(), pImageStreamInfo});
            }
            else{
                return -1;
            }
        }
    }


    //create meta stream info
    {
#define SET_METASTREAM(_id_, _name_, _type_, _maxBuf_, _initBufNum_)    \
        {                                                               \
            auto pMetaStreamInfo = new MetaStreamInfo(                  \
                                        _name_,                         \
                                        _id_,                           \
                                        _type_,                         \
                                        _maxBuf_,                       \
                                        _initBufNum_                    \
                                    );                                  \
            params->vMetaStreams.insert({_id_, pMetaStreamInfo});       \
            {                                                           \
                android::Mutex::Autolock _l(mMetaConfigMapLock);        \
                mMetaConfigMap.add(_id_, pMetaStreamInfo);              \
            }                                                           \
        }                                                               \
        //
        //input
        SET_METASTREAM(eSTREAMID_META_APP_CONTROL, "Meta:App:Control", eSTREAMTYPE_META_IN, 0, 0);
        SET_METASTREAM(eSTREAMID_META_PIPE_DYNAMIC_01, "Hal:Meta:P1:Dynamic", eSTREAMTYPE_META_IN, 10, 1);

        //output
        SET_METASTREAM(eSTREAMID_META_APP_DYNAMIC_01, "App:Meta:DynamicP1", eSTREAMTYPE_META_OUT, 10, 1);
    }

    //dump the output UserConfigurationParams
    {
        MY_LOG_DEBUG("Dump the output UserConfigurationParams");
        {
            android::Mutex::Autolock _l(mMetaConfigMapLock);
            for (size_t i = 0; i < mMetaConfigMap.size(); i++){
                const auto& metaStream = mMetaConfigMap.valueAt(i);
                MY_LOG_DEBUG("name(%s), FWK:id(%#" PRIx64 "), HAL:id(%#" PRIx64 "), type(%u)",
                    metaStream->getStreamName(), mMetaConfigMap.keyAt(i), metaStream->getStreamId(), metaStream->getStreamType());
            }
        }
        {
            android::Mutex::Autolock _l(mStreamConfigMapLock);
            for (size_t i = 0; i < mStreamConfigMap.size(); i++){
                const auto& imgStream = mStreamConfigMap.valueAt(i);
                MY_LOG_DEBUG("name(%s), FWK:id(%#" PRIx64 "), HAL:id(%#" PRIx64 "), type(%u), usage(%#" PRIx64 "), format(0x%x), w(%u), h(%u)",
                    imgStream->getStreamName(), mStreamConfigMap.keyAt(i), imgStream->getStreamId(),
                    imgStream->getStreamType(), imgStream->getUsageForAllocator(), imgStream->getImgFormat(),
                    imgStream->getImgSize().w, imgStream->getImgSize().h
                );
            }
        }
    }
    //
    FUNC_END;
    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ISPStreamController::
parseRequests(
    const hidl_vec<V1_0::ISPRequest>& hidl_reqs,
    std::vector<std::shared_ptr<UserRequestParams>>& parsed_reqs,
    std::shared_ptr<UserConfigurationParams>& params
) -> int
{
    FUNC_START;
    //dump the input ISPRequest
    {
        MY_LOG_DEBUG("Dump the input ISPRequest");
        for (size_t r = 0; r < hidl_reqs.size(); r++){
            auto& req = hidl_reqs[r];
            MY_LOG_DEBUG("Req %u", req.reqNumber);
            //input
            for (size_t in = 0; in < req.InputBuffers.size(); in++){
                const auto& inBuf = req.InputBuffers[in];
                MY_LOG_DEBUG("Input Image: id(%" PRIu64 "), handle(%p), tuningData(%p)",
                    inBuf.id, inBuf.buffer.getNativeHandle(), inBuf.tuningData.getNativeHandle());
            }
            //output
            for (size_t out = 0; out < req.OutputBuffers.size(); out++){
                const auto& outBuf = req.OutputBuffers[out];
                MY_LOG_DEBUG("Output Image: id(%" PRIu64 "), handle(%p), tuningData(%p)",
                    outBuf.id, outBuf.buffer.getNativeHandle(), outBuf.tuningData.getNativeHandle());
            }
        }
    }
#define ADD_METASTREAMBUF(_id_, _metadata_, _request_)                                                                         \
            {                                                                                                                  \
                android::Mutex::Autolock _l(mMetaConfigMapLock);                                                               \
                ssize_t indexOfMetaInfo = mMetaConfigMap.indexOfKey(_id_);                                                      \
                if  ( 0 > indexOfMetaInfo ) {                                                                                  \
                    MY_LOGE("Cannot find meta streamID%" PRIu64 " from mMetaConfigMap", _id_);                                 \
                    return -1;                                                                                                 \
                }                                                                                                              \
                else {                                                                                                         \
                    const android::sp<IMetaStreamInfo>& pMetaInfo = mMetaConfigMap.valueAt(indexOfMetaInfo);                   \
                    android::sp<ISPMetaStreamBuffer> pMetaStreamBuffer = createMetaStreamBuffer(pMetaInfo, _metadata_);        \
                    _request_->vIMetaBuffers.insert({pMetaStreamBuffer->getStreamInfo()->getStreamId(), pMetaStreamBuffer});   \
                }                                                                                                              \
            }                                                                                                                  \

    //
    if  ( hidl_reqs.size() == 0 ) {
        MY_LOGE("empty requests list");
        return  -1;
    }
    //TODO: check if the number of stream buffers is valid or not
    //
    for ( size_t req = 0; req < hidl_reqs.size(); ++req ) {
        const auto& request = hidl_reqs[req];
        ProprietaryRawInfo rawInfo;
        //
        // create multi requests
        for (size_t i = 0; i < request.InputBuffers.size(); i++){
            //create multi frames
            auto pItem = std::make_shared<UserRequestParams>();
                if ( !pItem ) {
                MY_LOGE("Bad UserRequestParams");
                return -1;
            }
            pItem->requestNo = request.reqNumber;
            parsed_reqs.push_back(pItem);
        }
        //
        //  parsed_reqs[0]->vOImageBuffers <- ISPRequest.OutputBuffers
        for (size_t i = 0; i < request.OutputBuffers.size(); i++){
            android::sp<ISPImageStreamBuffer> pStreamBuffer;
            if ( 0 > convertImageStreamBuffer("r"+std::to_string(request.reqNumber), request.OutputBuffers[i].buffer, request.OutputBuffers[i].id, pStreamBuffer, rawInfo) ){
                return -1;
            }
            parsed_reqs[0]->vOImageBuffers.insert({pStreamBuffer->getStreamInfo()->getStreamId(), pStreamBuffer});
            //
            //  parsed_reqs[0]->vOImageBuffers <- ISPRequest.OutputBuffers.tuningData
            if (request.OutputBuffers[i].tuningData != nullptr){
                android::sp<ISPImageStreamBuffer> pTunningStreamBuffer;
                if ( 0 > convertImageStreamBuffer("r"+std::to_string(request.reqNumber), request.OutputBuffers[i].tuningData, eSTREAMID_IMAGE_ISP_DEVICE_OUTPUT_TUNING, pTunningStreamBuffer, rawInfo) ){
                    MY_LOG_DEBUG("convert tuning image stream buffer failed: %u", request.reqNumber);
                    return -1;
                }
                parsed_reqs[0]->vOImageBuffers.insert({pTunningStreamBuffer->getStreamInfo()->getStreamId(), pTunningStreamBuffer});
            }
        }
        //
        //  parsed_reqs[0]->vIMetaBuffers <- ISPRequest.settings
        IMetadata ISPmeta = IMetadata();
        {
            //convert the configParams to IMetadata
            if ( ! mMetaConverter->convertToIMetadata(request.settings.data(), ISPmeta) )
            {
                MY_LOGE("convert config params failed");
                return -1;
            }
            //
            if ( mLogLevel >= 2 ){
                MY_LOG_DEBUG("Dump ISPMetadata settings +");
                ISPmeta.dump();
                MY_LOG_DEBUG("Dump ISPMetadata settings -");
            }
        }

        //  HAL MetaBuffers <- ISPRequest.tuningData
        for (size_t it = 0; it < parsed_reqs.size(); it++){
            auto& pItem = parsed_reqs[it];
            buffer_handle_t importedTuningBufferHandle = nullptr;
            android::sp<IImageBufferHeap> pImageBufferHeap;
            if ( 0 > convertTuningMetaBuffer("r"+std::to_string(request.reqNumber), request.InputBuffers[it], pImageBufferHeap, importedTuningBufferHandle) ){
                return -1;
            }
            std::shared_ptr<ISPStreamBufferHandle> ispBufferHandle = std::make_shared<ISPStreamBufferHandle>(importedTuningBufferHandle);
            IspTuningDataPackUtil::UnpackOutputParam unpackParam;
            pImageBufferHeap->lockBuf(LOG_TAG, (GRALLOC_USAGE_SW_READ_OFTEN|GRALLOC_USAGE_SW_WRITE_OFTEN) | GRALLOC1_PRODUCER_USAGE_CAMERA);
            IspTuningDataPackUtil PackUtils;
            MERROR ret = PackUtils.unpackIspTuningDataFromHeap(pImageBufferHeap, unpackParam);
            pItem->pFDData = unpackParam.pFDData;
            pItem->FDDataTimestamp = unpackParam.haltimestamp;
            pImageBufferHeap->unlockBuf(LOG_TAG);
            MY_LOGD("Unpack ret:%d",ret);
            MY_LOGD("tuning : importedTuningBufferHandle : %p", importedTuningBufferHandle);
            //
            //App control
            {
                //append to the metdata from ISPRequest.settings
                unpackParam.appControlMeta += ISPmeta;
                //
                if ( mLogLevel >= 2 ){
                    MY_LOG_DEBUG("Dump appended ISPMetadata settings +");
                    unpackParam.appControlMeta.dump();
                    MY_LOG_DEBUG("Dump appended ISPMetadata settings -");
                }
                //
                ADD_METASTREAMBUF(eSTREAMID_META_APP_CONTROL, unpackParam.appControlMeta, pItem);
            }

            //
            //P1 App Result
            ADD_METASTREAMBUF(eSTREAMID_META_APP_DYNAMIC_01, unpackParam.appResultMeta, pItem);

            //
            //P1 HAL Result
            if( mLogLevel >= 2 ){
                MY_LOGD("HalResult dump +");
                unpackParam.halResultMeta.dump();
                MY_LOGD("HalResult dump -");
            }

            ADD_METASTREAMBUF(eSTREAMID_META_PIPE_DYNAMIC_01, unpackParam.halResultMeta, pItem);
            {
                IMetadata::IEntry const& e1 = unpackParam.halResultMeta.entryFor(MTK_HAL_REQUEST_RAW_IMAGE_INFO);
                if ( ! e1.isEmpty() ) {
                    rawInfo.format = e1.itemAt(0, Type2Type<MINT32>());
                    rawInfo.stride = e1.itemAt(1, Type2Type<MINT32>());
                    rawInfo.width  = e1.itemAt(2, Type2Type<MINT32>());
                    rawInfo.height = e1.itemAt(3, Type2Type<MINT32>());
                    MY_LOGD("format: 0x%X, stride : %d, wxh : %dx%d", rawInfo.format, rawInfo.stride, rawInfo.width, rawInfo.height);
                }
            }

            //RRZO input
            {
                android::Mutex::Autolock _l(mStreamConfigMapLock);
                ssize_t index = mStreamConfigMap.indexOfKey(eSTREAMID_IMAGE_ISP_DEVICE_INPUT_RESIZED);
                android::sp<IImageStreamInfo> streamInfo = nullptr;
                sp<IImageBufferHeap> pImageBufferHeap = nullptr;
                if  ( 0 > index )
                {
                    MY_LOGE("Cannot find RRZO image from mStreamConfigMap");
                }
                else
                {
                    IImageStreamInfo::BufPlanes_t bufPlanes;
                    IMetadata::Memory dataInfo;
                    if (unpackParam.pRrzoImageBuffer != nullptr)
                    {
                        pImageBufferHeap = unpackParam.pRrzoImageBuffer->getImageBufferHeap();
                        IImageStreamInfo::BufPlanes_t bufPlanes;
                        bufPlanes.count = unpackParam.pRrzoImageBuffer->getPlaneCount();
                        for (int idx = 0; idx < bufPlanes.count; idx++)
                        {
                            bufPlanes.planes[idx].sizeInBytes      = unpackParam.pRrzoImageBuffer->getBufSizeInBytes(idx);
                            bufPlanes.planes[idx].rowStrideInBytes = unpackParam.pRrzoImageBuffer->getBufStridesInBytes(idx);
                        }
                        streamInfo = ImageStreamInfoBuilder((mStreamConfigMap.valueAt(index)).get())
                                .setAllocImgFormat(unpackParam.pRrzoImageBuffer->getImgFormat())
                                .setAllocBufPlanes(bufPlanes)
                                .setImgFormat(unpackParam.pRrzoImageBuffer->getImgFormat())
                                .setBufPlanes(bufPlanes)
                                .setImgSize(unpackParam.pRrzoImageBuffer->getImgSize())
                                .setBufStep({0})
                                .setStartOffset(0)
                                .build();
                    }
                    else
                    {
                        MY_LOGD("unpackParam.pRrzoImageBuffer is nullptr");
                    }
                    if (pImageBufferHeap != nullptr)
                    {
                        android::sp<ISPImageStreamBuffer> pRrzImageStreamBuffer =
                            ISPImageStreamBuffer::Allocator(streamInfo.get())(pImageBufferHeap.get(), importedTuningBufferHandle, ispBufferHandle);
                        pItem->vIImageBuffers.insert({eSTREAMID_IMAGE_ISP_DEVICE_INPUT_RESIZED, pRrzImageStreamBuffer});
                    }
                }
            }

            //
            //LCSO input
            {
                android::Mutex::Autolock _l(mStreamConfigMapLock);
                ssize_t index = mStreamConfigMap.indexOfKey(eSTREAMID_IMAGE_ISP_DEVICE_INPUT_LCSO);
                if  ( 0 > index ) {
                    MY_LOGE("Cannot find LCSO image from mStreamConfigMap");
                    return -1;
                }
                const android::sp<IImageStreamInfo>& streamInfo = mStreamConfigMap.valueAt(index);

                android::sp<ISPImageStreamBuffer> pLcsoImageStreamBuffer =
                    ISPImageStreamBuffer::Allocator(streamInfo.get())(unpackParam.pSttoImageBuffer->getImageBufferHeap(), importedTuningBufferHandle, ispBufferHandle);
                pItem->vIImageBuffers.insert({eSTREAMID_IMAGE_ISP_DEVICE_INPUT_LCSO, pLcsoImageStreamBuffer});
            }
        }
        //
        //  vIImageBuffers <- ISPRequest.InputBuffer
        //  InputBuffers.size() > 0 => multi-frame request
        for (size_t i = 0; i < request.InputBuffers.size(); i++){
            //
            android::sp<ISPImageStreamBuffer> pStreamBuffer;
            if ( 0 > convertImageStreamBuffer("r"+std::to_string(request.reqNumber), request.InputBuffers[i].buffer, request.InputBuffers[i].id, pStreamBuffer, rawInfo) ){
                return -1;
            }
            parsed_reqs[i]->vIImageBuffers.insert({pStreamBuffer->getStreamInfo()->getStreamId(), pStreamBuffer});
        }
    }
    //
    //dump the inoutput UserRequestParams
    {
        MY_LOG_DEBUG("Dump the output UserRequestParams");
        for (size_t r = 0; r < parsed_reqs.size(); r++){
            auto& req = parsed_reqs[r];
            MY_LOG_DEBUG("Req %u", req->requestNo);
            //input image
            for (auto it = req->vIImageBuffers.begin(); it != req->vIImageBuffers.end(); ++it){
                MY_LOG_DEBUG("name(%s), id(%#" PRIx64 ")",
                    it->second->getStreamInfo()->getStreamName(), it->second->getStreamInfo()->getStreamId());
            }
            //output image
            for (auto it = req->vOImageBuffers.begin(); it != req->vOImageBuffers.end(); ++it){
                MY_LOG_DEBUG("name(%s), id(%#" PRIx64 ")",
                    it->second->getStreamInfo()->getStreamName(), it->second->getStreamInfo()->getStreamId());
            }
            //metadata
            for (auto it = req->vIMetaBuffers.begin(); it != req->vIMetaBuffers.end(); ++it){
                MY_LOG_DEBUG("name(%s), id(%#" PRIx64 ")",
                    it->second->getStreamInfo()->getStreamName(), it->second->getStreamInfo()->getStreamId());
            }
        }
    }
    FUNC_END;
    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ISPStreamController::
createImageStreamInfo(
    const V1_0::ISPStream& srcStream,
    const MUINT32& srcStreamType
) -> android::sp<IImageStreamInfo>
{
    FUNC_START;
    //
    String8 streamName = String8::format("s%#" PRIx64 ":d%d:App:", srcStream.id, mId);
    StreamId_T streamId = 0;
    MINT imgFormat;
    MSize imgSize = MSize(0, 0);
    //
    if ( srcStreamType == eSTREAMTYPE_IMAGE_IN ){
        switch( srcStream.format.format ){
            case PixelFormat::YCBCR_420_888: //default set as NV21
            case PixelFormat::YCRCB_420_SP:
                streamName += String8("YCRCB_420_SP_input");
                streamId = eSTREAMID_IMAGE_ISP_DEVICE_INPUT_YUV;
                imgFormat = HAL_PIXEL_FORMAT_YCRCB_420_SP;
                break;

            case PixelFormat::RAW16:
            case PixelFormat::IMPLEMENTATION_DEFINED:
                streamName += String8("RAW_input");
                streamId = eSTREAMID_IMAGE_ISP_DEVICE_INPUT_RAW;
                imgFormat = (srcStream.format.format == PixelFormat::RAW16) ? eImgFmt_BAYER12_UNPAK : eImgFmt_BAYER10;
                break;

            default:
                MY_LOGE("Unsupported input image format:0x%x", srcStream.format.format);
                return nullptr;
        }
    }
    else if ( srcStreamType == eSTREAMTYPE_IMAGE_OUT ){
        switch( srcStream.format.format ){
            case PixelFormat::YCBCR_420_888:
            case PixelFormat::YCRCB_420_SP:
                streamName += String8("YCRCB_420_SP_output");
                streamId = (StreamId_T)srcStream.id;
                imgFormat = HAL_PIXEL_FORMAT_YCRCB_420_SP;
                break;
            case PixelFormat::RAW16:
                streamName += String8("RAW_Output");
                streamId = srcStream.id;
                imgFormat = eImgFmt_BAYER12_UNPAK;
                break;

            case PixelFormat::BLOB:
                streamName += String8("BLOB_output");
                streamId = (StreamId_T)srcStream.id;
                imgFormat = (srcStream.format.dataspace == static_cast<int32_t>(Dataspace::V0_JFIF)) ?
                            eImgFmt_JPEG : static_cast<MINT>(srcStream.format.format);
                break;

            default:
                MY_LOGE("Unsupported input image format:0x%x", srcStream.format.format);
                return nullptr;
        }
    }
    else{
        MY_LOGE("Unsupported stream type:%u", srcStreamType);
        return nullptr;
    }
    //
    IImageStreamInfo::BufPlanes_t bufPlanes;
    if ( queryPlanesInfo( srcStream, bufPlanes) < 0){
        MY_LOGE("%s query planes info failed", streamName.string());
        return nullptr;
    }
    imgSize.w = srcStream.width;
    imgSize.h = srcStream.height;
    //
    MUINT64 const usageForHal = (GRALLOC_USAGE_SW_READ_OFTEN|GRALLOC_USAGE_SW_WRITE_OFTEN) |
                          GRALLOC1_PRODUCER_USAGE_CAMERA ;
    MUINT64 usageForAllocator = usageForHal | srcStream.usage;
    //
    usageForAllocator = (srcStreamType == eSTREAMTYPE_IMAGE_OUT) ? usageForAllocator : usageForAllocator | GRALLOC_USAGE_HW_CAMERA_ZSL;
    //
    IImageStreamInfo::BufPlanes_t allocBufPlanes = bufPlanes;
    MINT allocImgFormat = imgFormat;
    // Alloc Format: Raw16 -> Blob
    if (srcStream.format.format == PixelFormat::RAW16) {
        allocBufPlanes.count = 1;
        allocImgFormat = static_cast<MINT>(eImgFmt_BLOB);
        allocBufPlanes.planes[0].sizeInBytes      = bufPlanes.planes[0].sizeInBytes;
        allocBufPlanes.planes[0].rowStrideInBytes = bufPlanes.planes[0].sizeInBytes;
        MY_LOGD("Raw16: (sizeInBytes, rowStrideInBytes)=(%d,%d)", bufPlanes.planes[0].sizeInBytes, allocBufPlanes.planes[0].rowStrideInBytes);
    }

    auto pImageStreamInfo = ImageStreamInfoBuilder()
                            .setStreamName(streamName.c_str())
                            .setStreamId(streamId)
                            .setStreamType(srcStreamType)
                            .setMaxBufNum(10)
                            .setMinInitBufNum(1)
                            .setUsageForAllocator(usageForAllocator)
                            .setAllocImgFormat(allocImgFormat)
                            .setAllocBufPlanes(allocBufPlanes)
                            .setImgFormat(imgFormat)
                            .setBufPlanes(bufPlanes)
                            .setImgSize(imgSize)
                            .setBufCount(1)
                            .setBufStep(0)
                            .setStartOffset(0)
                            .setTransform(srcStream.transform)
                            .build();

    {
        android::Mutex::Autolock _l(mStreamConfigMapLock);
        mStreamConfigMap.add(srcStream.id, pImageStreamInfo);
    }
    FUNC_END;
    return pImageStreamInfo;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ISPStreamController::
createImageStreamInfo(
    const uint64_t& srcStreamId,
    const MUINT32& srcStreamType
) -> android::sp<IImageStreamInfo>
{
    FUNC_START;
    //
    String8 streamName = String8::format("s%#" PRIx64 ":d%d:Hal:", srcStreamId, mId);
    StreamId_T streamId = 0;
    MINT imgFormat;
    MINT allocImgFormat;
    MSize imgSize = MSize(0, 0);
    IImageStreamInfo::BufPlanes_t bufPlanes;
    //
    switch ( srcStreamId ){
        case eSTREAMID_IMAGE_ISP_DEVICE_INPUT_LCSO:
        {
            NS3Av3::LCSO_Param lcsoParam;
            NS3Av3::Buffer_Info lcsoInfo;
            lcsoParam.bSupport = false;
            auto ispHal = MAKE_HalISP(0 /*sensorId*/, LOG_TAG);
            if ( CC_UNLIKELY(ispHal==nullptr) ) {
                MY_LOGE("get HalISP fail!");
                return nullptr;
            }
            if (ispHal->queryISPBufferInfo(lcsoInfo))
            {
                MY_LOGD("queryISPBufferInfo, support: %d, format: 0x%x, size: %dx%d",
                        lcsoInfo.LCESO_Param.bSupport, lcsoInfo.LCESO_Param.format,
                        lcsoInfo.LCESO_Param.size.w, lcsoInfo.LCESO_Param.size.h);
                lcsoParam.format = lcsoInfo.LCESO_Param.format;
                lcsoParam.size = lcsoInfo.LCESO_Param.size;
                lcsoParam.stride = lcsoInfo.LCESO_Param.stride;
            }
            else if ( auto ispMgr = MAKE_IspMgr() )
            {
                ispMgr->queryLCSOParams(lcsoParam);
            }
            else {
                MY_LOGE("Query IIspMgr FAILED!");
            }
            ispHal->destroyInstance(LOG_TAG);
            //
            streamName += String8("Image:LCSraw_input");
            streamId = eSTREAMID_IMAGE_ISP_DEVICE_INPUT_LCSO;
            allocImgFormat = imgFormat = lcsoParam.format;
            imgSize.w = lcsoInfo.LCESO_Param.size.w;
            imgSize.h = lcsoInfo.LCESO_Param.size.h;
            //
            switch (lcsoParam.format){
                case eImgFmt_STA_BYTE:
                case eImgFmt_STA_2BYTE:
                {
                    {

                        //IImageStreamInfo::BufPlane& plane = bufPlanes[0];
                        bufPlanes.count = 1;
                        bufPlanes.planes[0].sizeInBytes      = imgSize.h * lcsoParam.stride;
                        bufPlanes.planes[0].rowStrideInBytes = lcsoParam.stride;
                    }
                }break;
                default:
                    MY_LOGE("Unsupported LCSraw image format:0x%x", lcsoParam.format);
                    return nullptr;
            }
        }
        break;
        case eSTREAMID_IMAGE_ISP_DEVICE_INPUT_RESIZED:
        {
            //
            streamName += String8("Image:ResizeBuf_input");
            streamId = eSTREAMID_IMAGE_ISP_DEVICE_INPUT_RESIZED;
            allocImgFormat = imgFormat = MTK_CAM_PREFER_RRZO_USE_YUVFMT ? eImgFmt_MTK_YUV_P010 : eImgFmt_FG_BAYER10;
            imgSize.w = 1920; // temporary value, it is decided in request stage
            imgSize.h = 1440; // temporary value, it is decided in request stage
            //
            if (imgFormat == eImgFmt_FG_BAYER10)
            {
                bufPlanes.count = 1;
                bufPlanes.planes[0].sizeInBytes      = imgSize.w * imgSize.h * 10 / 8;
                bufPlanes.planes[0].rowStrideInBytes = imgSize.w * 10 / 8;
            }
            else
            {
                bufPlanes.count = 2;
                bufPlanes.planes[0].sizeInBytes      = imgSize.w * imgSize.h * 10 / 8;
                bufPlanes.planes[0].rowStrideInBytes = imgSize.w * 10 / 8;
                bufPlanes.planes[1].sizeInBytes      = bufPlanes.planes[0].sizeInBytes >> 1;
                bufPlanes.planes[1].rowStrideInBytes = imgSize.w * 10 / 8;
            }

        }
        break;
        case eSTREAMID_IMAGE_ISP_DEVICE_OUTPUT_TUNING:
        {
            //
            streamName += String8("Image:ISPTuning_output");
            streamId = eSTREAMID_IMAGE_ISP_DEVICE_OUTPUT_TUNING;
            imgFormat = eImgFmt_ISP_TUING;
            allocImgFormat = eImgFmt_BLOB;
            imgSize.w = kISP_TUNING_DATA_SIZE;
            imgSize.h = 1;
            //
            {
                IImageStreamInfo::BufPlane& plane = bufPlanes.planes[0];
                plane.sizeInBytes      = imgSize.h * imgSize.w;
                plane.rowStrideInBytes = imgSize.w;
                bufPlanes.count = 1;
            }

        }
        break;
        //
        default:
            MY_LOGE("Unsupported image streamId:%" PRIu64 "", srcStreamId);
            return nullptr;
    }
    MUINT64 const usageForHal = (GRALLOC_USAGE_SW_READ_OFTEN|GRALLOC_USAGE_SW_WRITE_OFTEN) |
                          GRALLOC1_PRODUCER_USAGE_CAMERA ;
    MUINT64 usageForAllocator = usageForHal;
    usageForAllocator = (srcStreamType == eSTREAMTYPE_IMAGE_OUT) ? usageForAllocator : usageForAllocator | GRALLOC_USAGE_HW_CAMERA_ZSL;
    auto pImageStreamInfo = ImageStreamInfoBuilder()
                            .setStreamName(streamName.c_str())
                            .setStreamId(streamId)
                            .setStreamType(srcStreamType)
                            .setMaxBufNum(10)
                            .setMinInitBufNum(1)
                            .setUsageForAllocator(usageForAllocator)
                            .setAllocImgFormat(allocImgFormat)
                            .setAllocBufPlanes(bufPlanes)
                            .setImgFormat(imgFormat)
                            .setBufPlanes(bufPlanes)
                            .setImgSize(imgSize)
                            .setBufCount(1)
                            .setBufStep(0)
                            .setStartOffset(0)
                            .build();

    {
        android::Mutex::Autolock _l(mStreamConfigMapLock);
        mStreamConfigMap.add(srcStreamId, pImageStreamInfo);
    }
    FUNC_END;
    return pImageStreamInfo;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ISPStreamController::
queryPlanesInfo(
    const ISPStream& stream,
    IImageStreamInfo::BufPlanes_t& planes
) -> int
{
    FUNC_START;
    switch ( stream.format.format ){
        case PixelFormat::BLOB:
        {
            {
                planes.count = 1;
                planes.planes[0].sizeInBytes      = stream.height*stream.width;
                planes.planes[0].rowStrideInBytes = stream.width;
                //checkPlaneSize(info, 0, (extra.rowNum));
            }
        }break;

        case PixelFormat::YCBCR_420_888: //default set as NV21
        case PixelFormat::YCRCB_420_SP:
        {
            {
                planes.count = 2;
                {// Y
                    planes.planes[0].sizeInBytes      = stream.stride[0] * stream.height;
                    planes.planes[0].rowStrideInBytes = stream.stride[0];
                }
                {// CrCb
                    planes.planes[1].sizeInBytes = stream.stride[1] * stream.height / 2;
                    planes.planes[1].rowStrideInBytes = stream.stride[1];
                }
            }
        }break;

        case PixelFormat::RAW16:
        {
            {
                planes.count = 1;
                planes.planes[0].sizeInBytes      = stream.size;
                planes.planes[0].rowStrideInBytes = stream.stride[0];
            }
        }break;

        case PixelFormat::IMPLEMENTATION_DEFINED :
        {
            const uint64_t u_camera_zsl = GRALLOC1_CONSUMER_USAGE_CAMERA | GRALLOC1_PRODUCER_USAGE_CAMERA;
            //if ((stream.usage & u_camera_zsl) == u_camera_zsl){
                //camera opaque
                {
                    planes.count = 1;
                    planes.planes[0].sizeInBytes      = stream.size;
                    planes.planes[0].rowStrideInBytes = stream.stride[0]*10/8; //1.25bpp
                }
            //}
        }break;

        default:
            MY_LOGE("Unsupported input image format:0x%x", stream.format.format);
            return -1;
    }
    //
    MY_LOG_DEBUG("Input: id(%" PRIu64 "), size(%u), w(%u), h(%u), format(0x%x)",
                stream.id, stream.size, stream.width, stream.height, stream.format.format);
    for (size_t i = 0; i < planes.count; i++){
        MY_LOG_DEBUG("Plane %d: sizeInBytes: %zu, rowStrideInBytes: %zu",
                    i, planes.planes[i].sizeInBytes, planes.planes[i].rowStrideInBytes);
    }
    FUNC_END;
    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ISPStreamController::
convertImageStreamBuffer(
    const std::string& bufferName,
    const hidl_handle& bufferHandle,
    const uint64_t streamId,
    android::sp<ISPImageStreamBuffer>& pStreamBuffer,
    const ProprietaryRawInfo& rawInfo
)   const -> int
{
    FUNC_START;
    MY_LOGD_IF(
        1,
        "%s: handle:%p",
        bufferName.c_str(), bufferHandle.getNativeHandle()
    );
    //check if the streamId is valid or not
    ssize_t index = 0;
    buffer_handle_t importedBufferHandle = bufferHandle;
    {
        {
            android::Mutex::Autolock _l(mStreamConfigMapLock);
            index = mStreamConfigMap.indexOfKey(streamId);
        }
        if  ( 0 > index ) {
            MY_LOGE("bad streamId:%#" PRIx64 "", streamId);
            return -1;
        }
        else {
            auto helper = IGrallocHelper::singleton();
            if  ( ! helper || importedBufferHandle == nullptr || ! helper->importBuffer(importedBufferHandle) )
            {
                importedBufferHandle = nullptr;
                MY_LOGE("GrallocHelper importBuffer failed: %s streamId:%#" PRIx64 " handle:%p",
                        bufferName.c_str(), streamId, bufferHandle.getNativeHandle());
                return -1;
            }
        }
    }
    {
        android::Mutex::Autolock _l(mStreamConfigMapLock);
        const auto& streamInfo = mStreamConfigMap.valueAt(index);
        std::shared_ptr<ISPStreamBufferHandle> ispBufferHandle = std::make_shared<ISPStreamBufferHandle>(importedBufferHandle);
        pStreamBuffer = createImageStreamBuffer(bufferName, streamInfo, importedBufferHandle, ispBufferHandle, -1/*acquire_fence*/, -1/*release_fence*/, rawInfo);
        if  ( pStreamBuffer == nullptr ) {
            MY_LOGE("fail to create ISPImageStreamBuffer - streamId:%#" PRIx64 " buffer handle:%p",  streamId, bufferHandle.getNativeHandle());
            return -ENODEV;
        }
    }
    FUNC_END;
    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ISPStreamController::
createImageStreamBuffer(
    const std::string& bufferName,
    const android::sp<IImageStreamInfo>& streamInfo,
    buffer_handle_t bufferHandle,
    std::shared_ptr<ISPStreamBufferHandle> ispBufferHandle,
    int const acquire_fence,
    int const release_fence,
    const ProprietaryRawInfo& rawInfo
)   const -> ISPImageStreamBuffer*
{
    sp<IImageStreamInfo> pStreamInfo = streamInfo;
    int heapFormat = pStreamInfo->getAllocImgFormat();
    int reqFormat  = pStreamInfo->getAllocImgFormat();
    int stride = 0;
    MSize size;
    // hack for customer, create packed raw image heap or change unpacked format by bpp
    if (heapFormat == static_cast<int>(eImgFmt_BLOB) && (rawInfo.stride != 0)) {
        reqFormat = rawInfo.format;
        stride = rawInfo.stride;
        size = MSize(rawInfo.width, rawInfo.height);

        IImageStreamInfo::BufPlanes_t bufPlanes = pStreamInfo->getBufPlanes();
        for (size_t i = 0 ; i < bufPlanes.count ; i++) {
            bufPlanes.planes[i].rowStrideInBytes = stride;
        }

        pStreamInfo = ImageStreamInfoBuilder(pStreamInfo.get())
                            .setImgFormat(reqFormat)
                            .setImgSize(size)
                            .setBufPlanes(bufPlanes)
                            .build();

        MY_LOGD("change raw16 to format:%x, allocFormat:%x", pStreamInfo->getImgFormat(), pStreamInfo->getAllocImgFormat());
    }

    if (heapFormat == static_cast<int>(eImgFmt_BLOB) && (pStreamInfo->getImgFormat() == static_cast<int>(eImgFmt_JPEG) || pStreamInfo->getImgFormat() == static_cast<int>(eImgFmt_ISP_TUING))) {
        heapFormat = pStreamInfo->getImgFormat();
    }

    android::sp<IGraphicImageBufferHeap> pImageBufferHeap =
        IGraphicImageBufferHeap::create(
            (bufferName + ":" + pStreamInfo->getStreamName()).c_str(),
            pStreamInfo->getUsageForAllocator(),
            pStreamInfo->getAllocImgFormat() == eImgFmt_BLOB ? MSize(pStreamInfo->getAllocBufPlanes().planes[0].rowStrideInBytes, 1): pStreamInfo->getImgSize(),
            heapFormat,
            &bufferHandle,
            acquire_fence,
            release_fence,
            stride
        );
    if ( pImageBufferHeap == nullptr ){
        android::LogPrinter logPrinter(LOG_TAG, ANDROID_LOG_DEBUG, "createIGraphicImageBufferHeap-Fail");
        MY_LOGF("IGraphicImageBufferHeap::create \"%s:%s\", handle: %p, fd: %d",
                 bufferName.c_str(), pStreamInfo->getStreamName(), bufferHandle, bufferHandle->data[0]);
    }

    /*if ( pStreamInfo->getAllocImgFormat() == HAL_PIXEL_FORMAT_RAW16
      && pStreamInfo->getStreamType() == eSTREAMTYPE_IMAGE_IN )
    {
        uint8_t colorFilterArrangement = 0;
        IMetadata const& staticMetadata = mCommonInfo->mMetadataProvider->getMtkStaticCharacteristics();
        bool ret = IMetadata::getEntry(&staticMetadata, MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT, colorFilterArrangement);
        MY_LOGF_IF(!ret, "no static info: MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT");
        auto sensorColorOrder = convertToSensorColorOrder(colorFilterArrangement);
        pImageBufferHeap->setColorArrangement(sensorColorOrder);
    }*/

    ISPImageStreamBuffer* pStreamBuffer =
    ISPImageStreamBuffer::Allocator(pStreamInfo.get())(pImageBufferHeap.get(), bufferHandle, ispBufferHandle);
    return pStreamBuffer;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ISPStreamController::
createMetaStreamBuffer(
    android::sp<IMetaStreamInfo> pStreamInfo,
    IMetadata const& rSettings
)   const -> ISPMetaStreamBuffer*
{
    ISPMetaStreamBuffer* pStreamBuffer =
    ISPMetaStreamBuffer::Allocator(pStreamInfo.get())(rSettings);
    return pStreamBuffer;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ISPStreamController::
convertTuningMetaBuffer(
    const std::string& bufferName,
    const ISPBuffer& streamBuffer,
    android::sp<IImageBufferHeap>& pImgBufHeap, /*output*/
    buffer_handle_t& importedTuningBufferHandle /*output*/
)   const -> int
{
    FUNC_START;
    MY_LOGD_IF(
            1,
            "StreamBuffer:%p streamId:%#" PRIx64 " tuningData:%p",
            &streamBuffer, streamBuffer.id, streamBuffer.tuningData.getNativeHandle()
        );
    importedTuningBufferHandle = streamBuffer.tuningData;
    {
        auto helper = IGrallocHelper::singleton();
        if  ( ! helper || ! helper->importBuffer(importedTuningBufferHandle) || importedTuningBufferHandle == nullptr )
        {
            importedTuningBufferHandle = nullptr;
            MY_LOGE("GrallocHelper importBuffer failed: StreamBuffer:%p streamId:%#" PRIx64 " handle:%p",
                    &streamBuffer, streamBuffer.id, streamBuffer.tuningData.getNativeHandle());
            return -1;
        }
    }
    pImgBufHeap = IGraphicImageBufferHeap::create(
            (bufferName + ":TuningData").c_str(),
            (GRALLOC_USAGE_SW_READ_OFTEN|GRALLOC_USAGE_SW_WRITE_OFTEN) | GRALLOC1_PRODUCER_USAGE_CAMERA /*usage*/,
            MSize(kISP_TUNING_DATA_SIZE, 1) /*size*/,
            static_cast<int>(eImgFmt_JPEG) /*format*/,
            &importedTuningBufferHandle,
            -1,
            -1
        );
    if ( pImgBufHeap == nullptr ){
        android::LogPrinter logPrinter(LOG_TAG, ANDROID_LOG_DEBUG, "createIGraphicImageBufferHeap-Fail");
        MY_LOGF("IGraphicImageBufferHeap::create \"%s\", handle: %p, fd: %d",
                 bufferName.c_str(), importedTuningBufferHandle, importedTuningBufferHandle->data[0]);
    }

    FUNC_END;
    return 0;
}


}  // namespace ISPHal
}  // namespace NSCam
