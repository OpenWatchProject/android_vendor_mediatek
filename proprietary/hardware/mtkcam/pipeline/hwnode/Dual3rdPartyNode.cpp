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
#define LOG_TAG "MtkCam/Dual3rdPartyNode"

//
#include "BaseNode.h"
//
#include "hwnode_utilities.h"
//
#include <mtkcam/feature/effectHalBase/EffectRequest.h>
#include <mtkcam/pipeline/stream/IStreamInfo.h>
#include <mtkcam/pipeline/stream/IStreamBuffer.h>
#include <mtkcam/pipeline/hwnode/Dual3rdPartyNode.h>
#include <mtkcam/feature/stereo/pipe/IIspPipe.h>
#include <mtkcam/feature/stereo/pipe/IIspPipeRequest.h>
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
//
#include <utils/RWLock.h>
#include <utils/KeyedVector.h>
#include <cutils/properties.h>
//
#include <sys/prctl.h>
#include <sys/resource.h>
//
#include <camera_custom_stereo.h>
#include <chrono>
//
#include "hwnode_utilities.h"

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils::Sync;
using namespace NSCam::NSCamFeature::NSFeaturePipe::DualCamThirdParty;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

//
#if 1
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif

class Dual3rdPartyNodeImp
: public BaseNode
, public Dual3rdPartyNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    Dual3rdPartyNodeImp();
    virtual ~Dual3rdPartyNodeImp();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNode Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MERROR init(InitParams_Base const& rParams);
    virtual MERROR init(InitParams const& rParams);
    virtual MERROR config(ConfigParams const& rParams);
    virtual MERROR uninit();
    virtual MERROR flush();
    virtual MERROR queue(sp<IPipelineFrame> pFrame);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Dual3rdPartyNodeImp private function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:

    /**
     * @brief callback launched when the request is finished
     * @param [in] tag Cookie pointer
     * @param [in] status Callback result status string
     * @param [in] pRequest Finished Efferct Request
     */
    static MVOID onEffectRequestFinished(
                                    MVOID* tag,
                                    PipeResultState state,
                                    sp<IIspPipeRequest>& request);
    /**
     * @brief internal callback launched when the request is finished
     * @param [in] status Callback result status string
     * @param [in] pRequest Finished Efferct Request
     * @return
     * - OK indicates success.
     * - others indicates failure.
     */
    MERROR onEffectReqDone(
                    PipeResultState state,
                    sp<IIspPipeRequest>& pRequest);
    /**
     * @brief Request success handle functino
     * @param [in] pFrame Pipeline frame
     * @param [in] pRequest Finished Efferct Request
     * @return
     * - OK indicates success.
     * - others indicates failure.
     */
    MERROR onEffectReqSucess(
                    sp<IPipelineFrame> const& pFrame,
                    sp<IIspPipeRequest>& pRequest);
    /**
     * @brief Request failure handle functino
     * @param [in] pFrame Pipeline frame
     * @param [in] pRequest Finished Efferct Request
     * @return
     * - OK indicates success.
     * - others indicates failure.
     */
    MERROR onEffectReqFailed(
                    PipeResultState state,
                    sp<IPipelineFrame> const& pFrame,
                    sp<IIspPipeRequest>& pRequest);

    /**
     * @brief YUV buffer ready handle function
     * @param [in] pRequest Finished Efferct Request
     * @return
     * - OK indicates success.
     * - others indicates failure.
     */
    MERROR onReqYUVDone(
                    sp<IIspPipeRequest>& pRequest);
    /**
     * @brief release in/out stream
     * @param [in] pFrame Pipeline frame
     * @param [in] pRequest Finished Efferct Request
     * @return
     * - OK indicates success.
     * - others indicates failure.
     */
    MERROR releaseAllInputStreams(
                        sp<IIspPipeRequest>& pRequest,
                        sp<IPipelineFrame> const& pFrame);
    MERROR releaseAllOutputStreams(
                        sp<IIspPipeRequest>& pRequest,
                        sp<IPipelineFrame> const& pFrame,
                        MBOOL bIsSuccess,
                        SortedVector<StreamId_T>* pExceptStreamId=NULL);
    /**
     * @brief retrieve imagea stream  inside frame
     * @param [in] pFrame Pipeline frame
     * @param [in] streamId Retrieving Stream ID
     * @param [in] eType StreamID IO Type
     * @param [out] rpImageBuffer output imagebuffer
     * @return
     * - OK indicates success.
     * - others indicates failure.
     */
    MERROR retrieveImageBuffer(
                        sp<IPipelineFrame> const& pFrame,
                        StreamId_T const streamId,
                        BufferIOType eType,
                        sp<IImageBuffer>& rpImageBuffer);
    /**
     * @brief retrieve metadata inside frame
     * @param [in] pFrame Pipeline frame
     * @param [in] streamId Retrieving Stream ID
     * @param [in] eType StreamID IO Type
     * @param [out] rpMetadata output metadata
     * @return
     * - OK indicates success.
     * - others indicates failure.
     */
    MERROR retrieveMetadata(
                        sp<IPipelineFrame> const& pFrame,
                        StreamId_T const streamId,
                        BufferIOType eType,
                        IMetadata*& rpMetadata);
    /**
     * @brief release image/metadata inside frame
     * @param [in] pFrame Pipeline frame
     * @param [in] streamId Retrieving Stream ID
     * @param [in] eType StreamID IO Type
     * @param [in] pImageBuffer buffer to release
     * @param [in] bIsSuccess the request result status
     * @return
     * - OK indicates success.
     * - others indicates failure.
     */
    MERROR releaseImageBuffer(
                    sp<IPipelineFrame> const& pFrame,
                    StreamId_T const streamId,
                    BufferIOType eType,
                    sp<IImageBuffer> const& pImageBuffer,
                    MBOOL bIsSuccess);

    MERROR releaseMetadata(
                    sp<IPipelineFrame> const& pFrame,
                    StreamId_T const streamId,
                    BufferIOType eType,
                    IMetadata* pMetadata,
                    MBOOL bIsSuccess);
    /**
     * @brief check the stream is allowed to be null
     * @param [in] streamId Retrieving Stream ID
     * @return
     * - MTRUE indicates yes
     * - MFALSE indicates no.
     */
    MBOOL validateTolerantStreamID(StreamId_T const streamId);

    /**
     * @brief Iterate the ioMapSet and add imagebuffer/metadata into request
     * @param [in] pFrame PipelineFrame
     * @param [in] rIOMapSet IOMapSet
     * @param [out] pEffectReq Output EffectRequest
     * @return
     * - OK indicates success.
     * - others indicates failure.
     */
    MERROR addEnqueImageBufferToRequest(
                                sp<IPipelineFrame>& pFrame,
                                IPipelineFrame::ImageInfoIOMapSet const& rIOMapSet,
                                sp<IIspPipeRequest>& pEffectReq);

    MERROR addEnqueMetaBufferToRequest(
                                sp<IPipelineFrame>& pFrame,
                                IPipelineFrame::MetaInfoIOMapSet const& rIOMapSet,
                                sp<IIspPipeRequest>& pEffectReq);

    /**
     * @brief add imagebuffer/metadata into request
     * @param [in] pFrame PipelineFrame
     * @return
     * - OK indicates success.
     * - others indicates failure.
     */
    MERROR addImageBufferFrameInfo(
                            const sp<IPipelineFrame>& pFrame,
                            const sp<IImageStreamInfo>& pImgStreamInfo,
                            const BufferSetting& setting,
                            sp<IIspPipeRequest>& pEffectReq);

    MERROR addMetaFrameInfo(
                        const sp<IPipelineFrame>& pFrame,
                        const sp<IMetaStreamInfo>& pMetaStreamInfo,
                        const BufferSetting& setting,
                        sp<IIspPipeRequest>& pEffectReq);

    /**
     * @brief Map stream ID to buffer ID
     * @param [in] streamId streamID
     * @param [out] rBufID bufferID
     * @return
     * - OK indicates success.
     * - others indicates failure.
     */
    MERROR mapToBufferID(StreamId_T streamId, IspPipeBufferID &rBufID);
    /**
     * @brief Map buffer id to stream id
     * @param [in] bufID buffer id
     * @param [out] rStreamId streamID
     * @return
     * - OK indicates success.
     * - others indicates failure.
     */
    MERROR mapToStreamID(IspPipeBufferID bufID, StreamId_T& rStreamId);

    /**
     * @brief Suspend frame operation
     * @param [in] pFrame PipelineFrame
     * @param [in] pRequest EffectRequest
     * @return
     * - OK indicates success.
     * - others indicates failure.
     */
    MERROR suspendThisFrame(
                    sp<IPipelineFrame> const& pFrame,
                    sp<IIspPipeRequest>& pRequest,
                    const char* reason="");
    /**
     * @brief unlock all stream and mark output stream error
     * @param [in] pFrame PipelineFrame
     * @param [in] pRequest EffectRequest
     * @return
     * - OK indicates success.
     * - others indicates failure.
     */
    MERROR unlockAndMarkOutStreamError(
                    sp<IPipelineFrame> const& pFrame,
                    sp<IIspPipeRequest> pRequest);
    /**
     * @brief generate bufferid-streamid map
     * @return
     * - OK indicates success.
     * - others indicates failure.
     */
    MERROR generateBIDMaps();
    /**
     * @brief update IsPPipe option
     * @return
     * - OK indicates success.
     * - others indicates failure.
     */
    MERROR updateIspPipeOption(IspPipeSetting& rPipeSetting);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Dual3rdPartyNodeImp protected member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    MINT32 mLogLevel;
    MINT32 mOpenId_Main2;
    // meta streaminfo
    sp<IMetaStreamInfo> mpInAppMeta = NULL;
    sp<IMetaStreamInfo> mpInHalMeta = NULL;
    sp<IMetaStreamInfo> mpInHalMeta_Main2 = NULL;
    sp<IMetaStreamInfo> mpOutAppMeta = NULL;
    sp<IMetaStreamInfo> mpOutHalMeta = NULL;
    // image streaminfo - input
    sp<IImageStreamInfo> mpInHalResizeRaw = NULL;
    sp<IImageStreamInfo> mpInHalResizeRaw_Main2 = NULL;
    sp<IImageStreamInfo> mpInHalFullRaw = NULL;
    sp<IImageStreamInfo> mpInHalFullRaw_Main2 = NULL;
    sp<IImageStreamInfo> mpInHalLCSO = NULL;
    sp<IImageStreamInfo> mpInHalLCSO_Main2 = NULL;
    sp<IImageStreamInfo> mpInHalFullYUV = NULL;
    sp<IImageStreamInfo> mpInHalFullYUV_Main2 = NULL;
    sp<IImageStreamInfo> mpInHalResizeYUV = NULL;
    sp<IImageStreamInfo> mpInHalResizeYUV_Main2 = NULL;
    // image streaminfo - output
    sp<IImageStreamInfo> mpOutAppImageDepthMap = NULL;
    sp<IImageStreamInfo> mpOutAppImagePreview = NULL;
    sp<IImageStreamInfo> mpOutAppImagePreviewCB = NULL;
    sp<IImageStreamInfo> mpOutAppImagePreviewFD = NULL;
    sp<IImageStreamInfo> mpOutHalImageCapBokeh = NULL;
    sp<IImageStreamInfo> mpOutHalImageBokehThumbnail = NULL;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Dual3rdPartyNodeImp private member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    // init flag
    bool mbInit;
    // ISP Pipe
    IIspPipe* mpIspPipe = NULL;
    mutable RWLock mConfigRWLock;
    // Pipeline frame container
    mutable RWLock mFrameMapRWLock;
    KeyedVector< MUINT32, sp<IPipelineFrame> > mvPipelineFrameMap;
    //BID To StreamID Map
    KeyedVector<IspPipeBufferID, StreamId_T> mPipeBIDToStreamIDMap;
    //StreamID to BID Map
    KeyedVector<StreamId_T, IspPipeBufferID> mStreamIDMapToIspBID;
    // error-tolerant stream id lsit
    KeyedVector<StreamId_T, MBOOL> mvTolerantStreamId;
};

/******************************************************************************
 *
 ******************************************************************************/
sp<Dual3rdPartyNode>
Dual3rdPartyNode::
createInstance()
{
    return new Dual3rdPartyNodeImp();
}


/******************************************************************************
 *
 ******************************************************************************/
Dual3rdPartyNodeImp::
Dual3rdPartyNodeImp()
: BaseNode()
, Dual3rdPartyNode()
, mbInit(MFALSE)
, mvPipelineFrameMap()
{
    mLogLevel = ::property_get_int32("debug.camera.log", 0);
    if ( mLogLevel == 0 ) {
        mLogLevel = ::property_get_int32("debug.camera.log.dual3rd", 0);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
Dual3rdPartyNodeImp::
~Dual3rdPartyNodeImp()
{
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
Dual3rdPartyNodeImp::
init(InitParams_Base const& rParams)
{
    MY_LOGE("Error: need to use the Dual3rdPartyNode::InitParams!");
    return INVALID_OPERATION;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
Dual3rdPartyNodeImp::
init(InitParams const& rParams)
{
    FUNC_START;
    if(mbInit)
    {
        MY_LOGW("Already perform init operations!");
        return INVALID_OPERATION;
    }

    RWLock::AutoWLock _l(mConfigRWLock);

    mbInit = MTRUE;
    mOpenId = rParams.openId;
    mOpenId_Main2 = rParams.openId_main2;
    mNodeId = rParams.nodeId;
    mNodeName = rParams.nodeName;

    //
    MY_LOGD("OpenId %d, nodeId %d, name %s, openID=%d openID_main2=%d",
            getOpenId(), getNodeId(), getNodeName(), mOpenId, mOpenId_Main2);

    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
Dual3rdPartyNodeImp::
config(ConfigParams const& rParams)
{
    FUNC_START;

    if(!mbInit)
    {
        MY_LOGW("Not perform init operations yet!");
        return INVALID_OPERATION;
    }
    #define SET_TOLERANT_STREAM_ID(target, source)\
        if(source != nullptr)\
        {\
            target = source;\
            mvTolerantStreamId.add(target->getStreamId(), MTRUE);\
        }
    // init stream info
    {
        RWLock::AutoWLock _l(mConfigRWLock);
        // metadata
        mpInAppMeta = rParams.pInAppMeta;
        mpOutAppMeta = rParams.pOutAppMeta;
        mpInHalMeta = rParams.pInHalMeta;
        mpInHalMeta_Main2 = rParams.pInHalMeta_Main2;
        mpOutHalMeta = rParams.pOutHalMeta;
        // hal input image
        mpInHalFullRaw = rParams.pInFullRaw;
        mpInHalFullRaw_Main2 = rParams.pInFullRaw_Main2;
        mpInHalResizeRaw = rParams.pInResizedRaw;
        mpInHalResizeRaw_Main2 = rParams.pInResizedRaw_Main2;
        mpInHalLCSO = rParams.pInLCSO;
        mpInHalLCSO_Main2 = rParams.pInLCSO_Main2;
        mpInHalFullYUV = rParams.pInFullSize_YUV;
        mpInHalFullYUV_Main2 = rParams.pInFullSize_YUV_Main2;
        mpInHalResizeYUV = rParams.pInResize_YUV;
        mpInHalResizeYUV_Main2 = rParams.pInResize_YUV_Main2;
        // hal output image
        mpOutAppImageDepthMap = rParams.pOutAppCapDepthResult;
        mpOutHalImageCapBokeh = rParams.pOutHalCapBokehResult;
        mpOutHalImageBokehThumbnail = rParams.pOutHalThumbnailImg_Cap;
        // tolerant streams
        SET_TOLERANT_STREAM_ID(mpOutAppImagePreviewCB, rParams.pOutAppPrvCB);
        SET_TOLERANT_STREAM_ID(mpOutAppImagePreview, rParams.pOutAppPrvImg);
        SET_TOLERANT_STREAM_ID(mpOutAppImagePreviewFD, rParams.pOutAppPrvFD);
    }
    #undef SET_TOLERANT_STREAM_ID
    // Generate the BID to StreamID map
    generateBIDMaps();
    // config Isp pipe option
    IspPipeOption pipeOption;
    pipeOption.mbEnableLCE = MFALSE;
    // config Isp pipe setting
    IspPipeSetting pipeSetting;
    MERROR ret = updateIspPipeOption(pipeSetting);
    if( ret == OK)
    {
        // create isp pipe
        MY_LOGD("Create Isp Pipe");
        mpIspPipe = IIspPipe::createInstance(pipeSetting, pipeOption);
        MY_LOGD("Init Isp Pipe");
        mpIspPipe->init();
        MY_LOGD("Sync Isp Pipe");
        // wait pipe ready
        mpIspPipe->sync();
    }
    else
        MY_LOGE("Failed to create IspPipe!");

    FUNC_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
Dual3rdPartyNodeImp::
updateIspPipeOption(IspPipeSetting& rPipeSetting)
{
    StereoSizeProvider* pStereoSizeProvider = StereoSizeProvider::getInstance();

    // get main1 rrzo/imgo size
    MBOOL bRet = pStereoSizeProvider->getcustomYUVSize(
                                    StereoHAL::eSTEREO_SENSOR_MAIN1,
                                    NSImageio::NSIspio::EPortIndex_IMGO,
                                    rPipeSetting.mszIMGO_Main1);

    bRet &= pStereoSizeProvider->getcustomYUVSize(
                                    StereoHAL::eSTEREO_SENSOR_MAIN1,
                                    NSImageio::NSIspio::EPortIndex_RRZO,
                                    rPipeSetting.mszRRZO_Main1);

    bRet &= pStereoSizeProvider->getcustomYUVSize(
                                    StereoHAL::eSTEREO_SENSOR_MAIN2,
                                    NSImageio::NSIspio::EPortIndex_IMGO,
                                    rPipeSetting.mszIMGO_Main2);

    bRet &= pStereoSizeProvider->getcustomYUVSize(
                                    StereoHAL::eSTEREO_SENSOR_MAIN2,
                                    NSImageio::NSIspio::EPortIndex_RRZO,
                                    rPipeSetting.mszRRZO_Main2);

    rPipeSetting.miSensorIdx_Main1 = mOpenId;
    rPipeSetting.miSensorIdx_Main2 = mOpenId_Main2;
    rPipeSetting.mszPreviewYUV = rPipeSetting.mszRRZO_Main1;

    if(bRet)
        return OK;
    else
        return BAD_VALUE;
}

/******************************************************************************
 *
 ******************************************************************************/
#define ADD_BID_STREAMID_MAP(streamInfo, BID)\
    if(streamInfo.get()) \
    { \
        mPipeBIDToStreamIDMap.add(BID, streamInfo->getStreamId()); \
        mStreamIDMapToIspBID.add(streamInfo->getStreamId(), BID); \
        MY_LOGD_IF(mLogLevel >= 1, "===========> BID=%d streamID=%#" PRIx64 "", BID, streamInfo->getStreamId());\
    }

/******************************************************************************
 *
 ******************************************************************************/
MERROR
Dual3rdPartyNodeImp::
generateBIDMaps()
{
    FUNC_START;
    // hal input image stream
    ADD_BID_STREAMID_MAP(mpInHalResizeRaw, PBID_IN_RESIZE_RAW_MAIN1);
    ADD_BID_STREAMID_MAP(mpInHalResizeRaw_Main2, PBID_IN_RESIZE_RAW_MAIN2);
    ADD_BID_STREAMID_MAP(mpInHalFullRaw, PBID_IN_FULL_RAW_MAIN1);
    ADD_BID_STREAMID_MAP(mpInHalFullRaw_Main2, PBID_IN_FULL_RAW_MAIN2);
    ADD_BID_STREAMID_MAP(mpInHalLCSO, PBID_IN_LCSO_MAIN1);
    ADD_BID_STREAMID_MAP(mpInHalLCSO_Main2, PBID_IN_LCSO_MAIN2);
    ADD_BID_STREAMID_MAP(mpInHalFullYUV, PBID_IN_FULLSIZE_YUV_MAIN1);
    ADD_BID_STREAMID_MAP(mpInHalFullYUV_Main2, PBID_IN_FULLSIZE_YUV_MAIN2);
    ADD_BID_STREAMID_MAP(mpInHalResizeYUV, PBID_IN_RESIZE_YUV_MAIN1);
    ADD_BID_STREAMID_MAP(mpInHalResizeYUV_Main2, PBID_IN_RESIZE_YUV_MAIN2);
    // output image
    ADD_BID_STREAMID_MAP(mpOutAppImageDepthMap, PBID_OUT_THIRDPARTY_DEPTH);
    ADD_BID_STREAMID_MAP(mpOutAppImagePreview, PBID_OUT_PV_YUV0);
    ADD_BID_STREAMID_MAP(mpOutAppImagePreviewCB, PBID_OUT_PV_YUV1);
    ADD_BID_STREAMID_MAP(mpOutAppImagePreviewFD, PBID_OUT_PV_FD);
    ADD_BID_STREAMID_MAP(mpOutHalImageCapBokeh, PBID_OUT_BOKEH_YUV);
    ADD_BID_STREAMID_MAP(mpOutHalImageBokehThumbnail, PBID_OUT_BOKEH_THUMBNAIL_YUV);
    // meta stream
    ADD_BID_STREAMID_MAP(mpInAppMeta, PBID_IN_APP_META);
    ADD_BID_STREAMID_MAP(mpInHalMeta, PBID_IN_HAL_META_MAIN1);
    ADD_BID_STREAMID_MAP(mpInHalMeta_Main2, PBID_IN_HAL_META_MAIN2);
    ADD_BID_STREAMID_MAP(mpOutAppMeta, PBID_OUT_APP_META);
    ADD_BID_STREAMID_MAP(mpOutHalMeta, PBID_OUT_HAL_META);
    FUNC_END;
    return OK;

}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
Dual3rdPartyNodeImp::
uninit()
{

    FUNC_START;

    if(!mbInit)
    {
        MY_LOGW("Not perform init operations yet!");
        return INVALID_OPERATION;
    }
    flush();
    {
        RWLock::AutoWLock _l(mConfigRWLock);
        mpIspPipe->uninit();
        delete mpIspPipe;
        mbInit = MFALSE;
    }
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
Dual3rdPartyNodeImp::
flush()
{
    FUNC_START;
    mpIspPipe->flush();
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
Dual3rdPartyNodeImp::
onEffectRequestFinished(
    MVOID* tag,
    PipeResultState state,
    sp<IIspPipeRequest>& pRequest
)
{
    Dual3rdPartyNodeImp *p3rdPtyNode = (Dual3rdPartyNodeImp *) tag;
    p3rdPtyNode->onEffectReqDone(state, pRequest);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
Dual3rdPartyNodeImp::
onEffectReqDone(
    PipeResultState state,
    sp<IIspPipeRequest>& pRequest
)
{
    MUINT32 reqID = pRequest->getRequestNo();
    sp<IPipelineFrame> pFrame;
    {

        RWLock::AutoWLock _l(mFrameMapRWLock);
        pFrame = mvPipelineFrameMap.valueFor(reqID);
        mvPipelineFrameMap.removeItem(reqID);
    }

    if(state == eRESULT_SUCCESS)
    {
        onEffectReqSucess(pFrame, pRequest);
    }
    else if(state == eRESULT_FLUSH)
    {
        suspendThisFrame(pFrame, pRequest, "request flushed");
    }
    else
    {
        onEffectReqFailed(state, pFrame, pRequest);
    }
    // stop timer adb log
    pRequest->stopTimer();
    MY_LOGD_IF(mLogLevel >= 1, "Request finished! reqID=%d, state=%s, Total cost time=%d ms",
                reqID, fnStateToName(state), pRequest->getElapsedTime());

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
Dual3rdPartyNodeImp::
onEffectReqFailed(
    PipeResultState state,
    sp<IPipelineFrame> const& pFrame,
    sp<IIspPipeRequest>& pRequest
)
{
    MY_LOGD_IF(mLogLevel >= 1, "+");
    MUINT32 reqID = pRequest->getRequestNo();

    if(state == eRESULT_FAILURE)
    {
        MY_LOGD_IF(mLogLevel >= 1, "state: %d", state);
        // release all input stream
        releaseAllInputStreams(pRequest, pFrame);
        // mark all output stream failed
        releaseAllOutputStreams(pRequest, pFrame, MFALSE);
        // dispatch
        MY_LOGD_IF(mLogLevel >= 1, "Dual3rdPartyNode: applyRelease reqID=%d", reqID);
        pFrame->getStreamBufferSet().applyRelease(getNodeId());
        MY_LOGD_IF(mLogLevel >= 1, "Dual3rdPartyNode: onDispatchFrame reqID=%d", reqID);
        onDispatchFrame(pFrame);
        MY_LOGD_IF(mLogLevel >= 1, "-");
    }
    else
    {
        MY_LOGE("Un-recognized status=%s", fnStateToName(state));
        suspendThisFrame(pFrame, pRequest, "request failed");
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
Dual3rdPartyNodeImp::
onEffectReqSucess(
    sp<IPipelineFrame> const& pFrame,
    sp<IIspPipeRequest>& pRequest
)
{
    MUINT32 reqID = pRequest->getRequestNo();
    MY_LOGD_IF(mLogLevel >= 1, "+ , reqId=%d", reqID);

    //release all input stream
    releaseAllInputStreams(pRequest, pFrame);
    // release all output stream
    releaseAllOutputStreams(pRequest, pFrame, MTRUE);

    // dispatch
    MY_LOGD_IF(mLogLevel >= 1, "Dual3rdPartyNode: applyRelease reqID=%d", reqID);
    pFrame->getStreamBufferSet().applyRelease(getNodeId());
    MY_LOGD_IF(mLogLevel >= 1, "Dual3rdPartyNode: onDispatchFrame reqID=%d", reqID);
    onDispatchFrame(pFrame);
    MY_LOGD_IF(mLogLevel >= 1, "-");

    return OK;
}

MERROR
Dual3rdPartyNodeImp::
releaseAllOutputStreams(
    sp<IIspPipeRequest>& pRequest,
    sp<IPipelineFrame> const& pFrame,
    MBOOL bIsSuccess,
    SortedVector<StreamId_T>* pExceptStreamId
)
{
    MY_LOGD("+");
    // get IOMapSet of this frame, which define all the in/out stream inside the frame
    IPipelineFrame::InfoIOMapSet IOMapSet;
    if( OK != pFrame->queryInfoIOMapSet( getNodeId(), IOMapSet ) ) {
        MY_LOGE("queryInfoIOMap failed");
        return NAME_NOT_FOUND;
    }

    // get the image stream IOMapSet
    IPipelineFrame::ImageInfoIOMap const& imageIOMap = IOMapSet.mImageInfoIOMapSet[0];

    IImageBuffer *pImgBuf;

    // iterate all the output image stream and mark release status
    for(size_t index=0; index<imageIOMap.vOut.size(); index++)
    {
        StreamId_T streamId = imageIOMap.vOut.keyAt(index);
        sp<IImageStreamInfo> streamInfo = imageIOMap.vOut.valueAt(index);
        IspPipeBufferID bufID;
        MERROR ret = mapToBufferID(streamId, bufID);

        if (ret != OK)
            return BAD_VALUE;

        if(!pRequest->getRequestImageBuffer({bufID, eBUFFER_IOTYPE_OUTPUT}, pImgBuf))
        {
            MY_LOGD_IF(mLogLevel >= 1, "Cannot find the output frameInfo, reqIdx=%d bufID=%d", pFrame->getRequestNo(), bufID);
            continue;
        }

        // check except stream id
        if(pExceptStreamId == NULL || pExceptStreamId->indexOf(streamId) < 0)
            releaseImageBuffer(pFrame, streamInfo->getStreamId(), eBUFFER_IOTYPE_OUTPUT, pImgBuf, bIsSuccess);
        else
            releaseImageBuffer(pFrame, streamInfo->getStreamId(), eBUFFER_IOTYPE_OUTPUT, pImgBuf, !bIsSuccess);
        MY_LOGD_IF(mLogLevel >= 1, "release imagebuffer, reqIdx=%d bufID=%d", pFrame->getRequestNo(), bufID);
    }

    // get the meta stream IOMapSet
    IPipelineFrame::MetaInfoIOMap const& metaIOMap = IOMapSet.mMetaInfoIOMapSet[0];

    // iterate all the output meta stream and mark release status
    for(size_t index=0; index<metaIOMap.vOut.size(); index++)
    {
        StreamId_T streamId = metaIOMap.vOut.keyAt(index);
        sp<IMetaStreamInfo> streamInfo = metaIOMap.vOut.valueAt(index);
        IspPipeBufferID bufID;
        MERROR ret = mapToBufferID(streamId, bufID);
        if (ret != OK)
            return BAD_VALUE;

        sp<EffectParameter> effParam = pRequest->vOutputFrameInfo.valueFor(bufID)->getFrameParameter();
        IMetadata* pMetaBuf;

        MBOOL bRet = pRequest->getRequestMetadata({bufID, eBUFFER_IOTYPE_OUTPUT}, pMetaBuf);
        if(!bRet)
        {
            MY_LOGE("Cannot get metadata, bufferID=%d ioType=%d", bufID, eBUFFER_IOTYPE_OUTPUT);
            return BAD_VALUE;
        }
        // check except stream id
        if(pExceptStreamId == NULL || pExceptStreamId->indexOf(streamId) < 0)
            releaseMetadata(pFrame, streamInfo->getStreamId(), eBUFFER_IOTYPE_OUTPUT, pMetaBuf, bIsSuccess);
        else
            releaseMetadata(pFrame, streamInfo->getStreamId(), eBUFFER_IOTYPE_OUTPUT, pMetaBuf, !bIsSuccess);
        MY_LOGD_IF(mLogLevel >= 1, "release metadata, reqIdx=%d bufID=%d", pFrame->getRequestNo(), bufID);
    }
    MY_LOGD("-");
    return OK;
}

MERROR
Dual3rdPartyNodeImp::
releaseAllInputStreams(
    sp<IIspPipeRequest>& pRequest,
    sp<IPipelineFrame> const& pFrame
)
{
    MY_LOGD("+");
    // get IOMapSet of this frame, which define all the in/out stream inside the frame
    IPipelineFrame::InfoIOMapSet IOMapSet;
    if( OK != pFrame->queryInfoIOMapSet( getNodeId(), IOMapSet ) ) {
        MY_LOGE("queryInfoIOMap failed");
        return NAME_NOT_FOUND;
    }

    // get the image stream IOMapSet
    IPipelineFrame::ImageInfoIOMap const& imageIOMap = IOMapSet.mImageInfoIOMapSet[0];
    MBOOL bRet;
    MY_LOGD_IF(mLogLevel >= 1, "ImageBuffer: vIn size=%d, vOut size=%d", imageIOMap.vIn.size(), imageIOMap.vOut.size());

    IImageBuffer* pImgBuf;
    // iterate all the input image stream and release
    for(size_t index=0; index<imageIOMap.vIn.size(); index++)
    {
        StreamId_T streamId = imageIOMap.vIn.keyAt(index);
        sp<IImageStreamInfo> streamInfo = imageIOMap.vIn.valueAt(index);
        IspPipeBufferID bufID;
        if (mapToBufferID(streamId, bufID) != OK)
            return BAD_VALUE;

        bRet = pRequest->getRequestImageBuffer({bufID, eBUFFER_IOTYPE_INPUT}, pImgBuf);
        if(bRet)
        {
            releaseImageBuffer(pFrame, streamInfo->getStreamId(), eBUFFER_IOTYPE_INPUT, pImgBuf, MTRUE);
            MY_LOGD_IF(mLogLevel >= 1, "release imagebuffer, reqIdx=%d bufID=%d", pFrame->getRequestNo(), bufID);
        }
        else
        {
            MY_LOGD_IF(mLogLevel >= 1, "Cannot get request imageBuffer!! bufferID=%d", bufID);
            continue;
        }
    }

    // get the meta stream IOMapSet
    IPipelineFrame::MetaInfoIOMap const& metaIOMap = IOMapSet.mMetaInfoIOMapSet[0];
    MY_LOGD_IF(mLogLevel >= 1, "Metadata: vIn size=%d, vOut size=%d", metaIOMap.vIn.size(), metaIOMap.vOut.size());

    // input meta stream
    for(size_t index=0; index<metaIOMap.vIn.size(); index++)
    {
        StreamId_T streamId = metaIOMap.vIn.keyAt(index);
        sp<IMetaStreamInfo> streamInfo = metaIOMap.vIn.valueAt(index);
        IspPipeBufferID bufID;
        if (mapToBufferID(streamId, bufID) != OK)
            return BAD_VALUE;

        IMetadata* pMetaBuf;
        bRet = pRequest->getRequestMetadata({bufID, eBUFFER_IOTYPE_INPUT}, pMetaBuf);
        if(!bRet)
        {
            MY_LOGE("Cannot get request metadata buffer!! bufferID=%d", bufID);
            return BAD_VALUE;;
        }

        releaseMetadata(pFrame, streamInfo->getStreamId(), eBUFFER_IOTYPE_INPUT, pMetaBuf, MTRUE);
        MY_LOGD_IF(mLogLevel >= 1, "release metadata, reqIdx=%d bufID=%d", pFrame->getRequestNo(), bufID);
    }
    MY_LOGD("-");
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
Dual3rdPartyNodeImp::
unlockAndMarkOutStreamError(sp<IPipelineFrame> const& pFrame, sp<IIspPipeRequest> pRequest)
{
    FUNC_START;
    // get IOMapSet of this frame, which define all the in/out stream inside the frame
    IPipelineFrame::InfoIOMapSet IOMapSet;
    if( OK != pFrame->queryInfoIOMapSet( getNodeId(), IOMapSet ) ) {
        MY_LOGE("queryInfoIOMap failed");
        return NAME_NOT_FOUND;
    }

    sp<IImageBuffer> pImageBuffer;
    IMetadata* pMetaData;

    #define GET_BUFFER_AND_UNLOCK(frameInfo) \
        frameInfo->getFrameBuffer(pImageBuffer);\
        pImageBuffer->unlockBuf(getNodeName());\
        pStreamBuffer->unlock(getNodeName(), pImageBuffer->getImageBufferHeap());

    #define GET_META_AND_UNLOCK(frameInfo) \
        pMetaData = reinterpret_cast<IMetadata*>(frameInfo->getFrameParameter()\
                                            ->getPtr(VSDOF_PARAMS_METADATA_KEY));\
        pMetaStreamBuffer->unlock(getNodeName(), pMetaData);


    IStreamBufferSet& rStreamBufferSet  = pFrame->getStreamBufferSet();
    // get the image stream IOMapSet
    IPipelineFrame::ImageInfoIOMap const& imageIOMap = IOMapSet.mImageInfoIOMapSet[0];
    // get the meta stream IOMapSet
    IPipelineFrame::MetaInfoIOMap const& metaIOMap = IOMapSet.mMetaInfoIOMapSet[0];

    // unlock input buffer
    for(size_t index=0; index<imageIOMap.vIn.size(); index++)
    {
        StreamId_T streamId = imageIOMap.vIn.keyAt(index);
        IspPipeBufferID bufId;
        MERROR ret = mapToBufferID(streamId, bufId);
        if (ret != OK)
            return BAD_VALUE;

        IImageBuffer* pImgBuf = nullptr;
        MBOOL bRet = pRequest->getRequestImageBuffer({.bufferID=bufId, .ioType=eBUFFER_IOTYPE_INPUT}, pImgBuf);
        if (bRet)
        {
            sp<IImageStreamBuffer> pStreamBuffer = rStreamBufferSet.getImageBuffer(streamId, getNodeId());
            pImgBuf->unlockBuf(getNodeName());
            rStreamBufferSet.markUserStatus(streamId, getNodeId(),
                                                IUsersManager::UserStatus::USED|IUsersManager::UserStatus::RELEASE);
            pStreamBuffer->unlock(getNodeName(), pImgBuf->getImageBufferHeap());
        }
    }

    // unlock output buffer and mark error
    for(size_t index=0; index<imageIOMap.vOut.size(); index++)
    {
        StreamId_T streamId = imageIOMap.vOut.keyAt(index);
        IspPipeBufferID bufId;
        MERROR ret = mapToBufferID(streamId, bufId);
        if (ret != OK)
            return BAD_VALUE;

        IImageBuffer* pImgBuf = nullptr;
        MBOOL bRet = pRequest->getRequestImageBuffer({.bufferID=bufId, .ioType=eBUFFER_IOTYPE_OUTPUT}, pImgBuf);
        if (bRet)
        {
            sp<IImageStreamBuffer> pStreamBuffer = rStreamBufferSet.getImageBuffer(streamId, getNodeId());
            pImgBuf->unlockBuf(getNodeName());
            pStreamBuffer->unlock(getNodeName(), pImgBuf->getImageBufferHeap());
            rStreamBufferSet.markUserStatus(streamId, getNodeId(),
                                                IUsersManager::UserStatus::USED|IUsersManager::UserStatus::RELEASE);
            //mark error
            pStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_ERROR);
        }
    }

    // unlokc input meta stream
    for(size_t index=0; index<metaIOMap.vIn.size(); index++)
    {
        StreamId_T streamId = metaIOMap.vIn.keyAt(index);
        IspPipeBufferID bufId;
        if (mapToBufferID(streamId, bufId) != OK)
            return BAD_VALUE;

        IMetadata* pMeta = nullptr;
        MBOOL bRet = pRequest->getRequestMetadata({.bufferID=bufId, .ioType=eBUFFER_IOTYPE_INPUT}, pMeta);
        if(bRet )
        {
            sp<IMetaStreamBuffer> pMetaStreamBuffer = rStreamBufferSet.getMetaBuffer(streamId, getNodeId());
            rStreamBufferSet.markUserStatus(streamId, getNodeId(), IUsersManager::UserStatus::RELEASE);
            pMetaStreamBuffer->unlock(getNodeName(), pMeta);
        }
        else
            MY_LOGW("Stream ID=%#" PRIx64 " not found, cannot unlock!", streamId);
    }

    // unlokc output meta stream and mark error
    for(size_t index=0; index<metaIOMap.vOut.size(); index++)
    {
        StreamId_T streamId = metaIOMap.vOut.keyAt(index);
        IspPipeBufferID bufId;
        if (mapToBufferID(streamId, bufId) != OK)
            return BAD_VALUE;

        IMetadata* pMeta = nullptr;
        MBOOL bRet = pRequest->getRequestMetadata({.bufferID=bufId, .ioType=eBUFFER_IOTYPE_OUTPUT}, pMeta);
        if(bRet)
        {
            sp<IMetaStreamBuffer> pMetaStreamBuffer = rStreamBufferSet.getMetaBuffer(streamId, getNodeId());
            pMetaStreamBuffer->unlock(getNodeName(), pMeta);
            rStreamBufferSet.markUserStatus(streamId, getNodeId(), IUsersManager::UserStatus::RELEASE);
            // mark error
            pMetaStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_ERROR);
        }
        else
            MY_LOGW("Stream ID=%#" PRIx64 " not found, cannot unlock!", streamId);
    }
    #undef GET_BUFFER_AND_UNLOCK
    #undef GET_META_AND_UNLOCK

    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
Dual3rdPartyNodeImp::
suspendThisFrame(
    sp<IPipelineFrame> const& pFrame,
    sp<IIspPipeRequest>& pRequest,
    const char* reason
    )
{
    MY_LOGD_IF(mLogLevel >= 1, "Discard request id=%d, reason=%s", pFrame->getRequestNo(), reason);
    //unlock the current buffer inside the EffectRequest
    unlockAndMarkOutStreamError(pFrame, pRequest);
    // flush this frame
    MERROR err = BaseNode::flush(pFrame);
    return OK;
}


MERROR
Dual3rdPartyNodeImp::
addImageBufferFrameInfo(
    const sp<IPipelineFrame>& pFrame,
    const sp<IImageStreamInfo>& pImgStreamInfo,
    const BufferSetting& setting,
    sp<IIspPipeRequest>& pEffectReq
)
{
    IspPipeBufferID bid = setting.bufferID;
    sp<IImageBuffer> pImgBuf;
    StreamId_T streamId = pImgStreamInfo->getStreamId();

    MERROR res = retrieveImageBuffer(pFrame, streamId, setting.ioType, pImgBuf);
    if(res == OK)
    {
        pEffectReq->pushRequestImageBuffer(setting, pImgBuf);
        MY_LOGD_IF(mLogLevel >= 2, "add imagebuffer into request, reqIdx=%d bufID=%d", pFrame->getRequestNo(), bid);
    }else
    {
        MBOOL bValid = validateTolerantStreamID(streamId);
        if(bValid)
        {
            res = OK;
            MY_LOGD_IF(mLogLevel >= 1, "Found missing streamId=%#" PRIx64 ", can be tolerant! BufID=%d reqIdx=%d",
                        streamId, bid, pFrame->getRequestNo());
            // mark release
            IStreamBufferSet& rStreamBufferSet  = pFrame->getStreamBufferSet();
            rStreamBufferSet.markUserStatus(streamId, getNodeId(),
                            IUsersManager::UserStatus::USED|IUsersManager::UserStatus::RELEASE);
        }
        else
        {
            MY_LOGE("Failed to get ImageBuffer, redID=%d streadmId=%#" PRIx64 " bufID=%d",
                    pFrame->getRequestNo(), streamId, bid);
        }
    }

    return res;
}


MERROR
Dual3rdPartyNodeImp::
addMetaFrameInfo(
    const sp<IPipelineFrame>& pFrame,
    const sp<IMetaStreamInfo>& pMetaStreamInfo,
    const BufferSetting& setting,
    sp<IIspPipeRequest>& pEffectReq
)
{
    BufferIOType eIOType = setting.ioType;
    IspPipeBufferID bid = setting.bufferID;
    IMetadata* pMetaBuf;
    MERROR res = retrieveMetadata(pFrame, pMetaStreamInfo->getStreamId(), eIOType, pMetaBuf);
    if(res == OK)
    {
        pEffectReq->pushRequestMetadata(setting, pMetaBuf);
        MY_LOGD_IF(mLogLevel >= 2, "add metadata into request, reqIdx=%d bufID=%d", pFrame->getRequestNo(), bid);
    }else
    {
        MY_LOGE("retrieve metadata failed!! res=%d BufID=%d reqIdx=%d", res, bid, pFrame->getRequestNo());
    }
    return res;
}

/******************************************************************************
 *
 ******************************************************************************/

/******************************************************************************
 *
 ******************************************************************************/
MERROR
Dual3rdPartyNodeImp::
queue(sp<IPipelineFrame> pFrame)
{
    if( ! pFrame.get() ) {
        MY_LOGE("Null frame");
        return BAD_VALUE;
    }
    IPipelineFrame::InfoIOMapSet IOMapSet;
    if( OK != pFrame->queryInfoIOMapSet( getNodeId(), IOMapSet ) ) {
        MY_LOGE("queryInfoIOMap failed");
        return NAME_NOT_FOUND;
    }
    // request packing
    sp<IIspPipeRequest> pEffectReq;
    MUINT32 iReqIdx = pFrame->getRequestNo();
    pEffectReq = IIspPipeRequest::createInstance(iReqIdx, onEffectRequestFinished, (void*)this);
    // start timer
    pEffectReq->startTimer();
    using std::chrono::system_clock;
    system_clock::time_point bef_enque_tp = system_clock::now();

    MY_LOGD_IF(mLogLevel >= 1, "+ , reqId=%d", iReqIdx);

    // add Image Buffer Stream to EffectRequest
    {
        // get image ioMapSet
        IPipelineFrame::ImageInfoIOMapSet& imageIOMapSet = IOMapSet.mImageInfoIOMapSet;
        // imgIOMapSet size should be only 1
        if(imageIOMapSet.size() != 1) {
            MY_LOGE("imageIOMapSet size should be 1. size=%d", imageIOMapSet.size());
            return BAD_VALUE;
        }

        MERROR ret = addEnqueImageBufferToRequest(pFrame, imageIOMapSet, pEffectReq);
        if(ret != OK)
        {
            suspendThisFrame(pFrame, pEffectReq, "failed to ensure image buffer");
            return BAD_VALUE;
        }
    }

    // add Meta Buffer Stream to EffectRequest
    {
        //get meta ioMapSet
        IPipelineFrame::MetaInfoIOMapSet& metaIOMapSet =IOMapSet.mMetaInfoIOMapSet;
        // metaIOMapSet size should be only 1
        if(metaIOMapSet.size() != 1) {
            MY_LOGE("metaIOMapSet size should be 1. size=%d", metaIOMapSet.size());
            return BAD_VALUE;
        }
        // add meta buffer into effect pRequest
        MERROR ret= addEnqueMetaBufferToRequest(pFrame, metaIOMapSet, pEffectReq);
        if(ret != OK)
        {
            suspendThisFrame(pFrame, pEffectReq, "failed to ensure meta buffer");
            return BAD_VALUE;
        }
    }
    {
        if(pEffectReq->getOutputSize() == 0)
        {
            MY_LOGD_IF(mLogLevel >= 1, "reqID=%d no output data, flush this req!", iReqIdx);
            suspendThisFrame(pFrame, pEffectReq, "no output data, flush this req");
            return OK;
        }
        MY_LOGD_IF(mLogLevel >= 1, "reqID=%d, output data size=%d!", iReqIdx, pEffectReq->getOutputSize());
        // add Frame into frame map
        {
            RWLock::AutoWLock _l(mFrameMapRWLock);
            //record the to-be-enque frame
            mvPipelineFrameMap.add(iReqIdx, pFrame);
        }

        // log enque time
        system_clock::time_point after_enque_tp = system_clock::now();
        std::chrono::duration<double> elap = system_clock::now() - bef_enque_tp;
        MY_LOGD_IF(mLogLevel >= 2, "reqID=%d  enque time=%f ms", iReqIdx, elap.count()*1000);
        // enque the effect Request
        mpIspPipe->enque(pEffectReq);
    }

    MY_LOGD_IF(mLogLevel >= 2, "- , reqId=%d", iReqIdx);
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
Dual3rdPartyNodeImp::
addEnqueImageBufferToRequest(
    sp<IPipelineFrame>& pFrame,
    IPipelineFrame::ImageInfoIOMapSet const& rIOMapSet,
    sp<IIspPipeRequest>& pEffectReq
)
{
    IPipelineFrame::ImageInfoIOMap const& imageIOMap = rIOMapSet[0];
    // Add input image stream to EffectRequest
    for(size_t index=0; index<imageIOMap.vIn.size(); index++)
    {
        StreamId_T streamId = imageIOMap.vIn.keyAt(index);
        sp<IImageStreamInfo> streamInfo = imageIOMap.vIn.valueAt(index);
        IspPipeBufferID bufID;
        if (mapToBufferID(streamId, bufID) != OK)
            return BAD_VALUE;

        MERROR ret = addImageBufferFrameInfo(pFrame, streamInfo, {bufID, eBUFFER_IOTYPE_INPUT}, pEffectReq);
        if (ret != OK)
            return BAD_VALUE;

        MY_LOGD_IF(mLogLevel >= 2, "input imagBuf stream bid=%d",bufID);
    }
    // Add output image stream to EffectRequest
    for(size_t index=0; index<imageIOMap.vOut.size(); index++)
    {
        StreamId_T streamId = imageIOMap.vOut.keyAt(index);
        sp<IImageStreamInfo> streamInfo = imageIOMap.vOut.valueAt(index);
        IspPipeBufferID bufID;
        if (mapToBufferID(streamId, bufID) != OK)
            return BAD_VALUE;

        MERROR ret = addImageBufferFrameInfo(pFrame, streamInfo, {bufID, eBUFFER_IOTYPE_OUTPUT}, pEffectReq);
        if (ret != OK)
            return BAD_VALUE;

        MY_LOGD_IF(mLogLevel >= 2, "output imagBuf stream bid=%d",bufID);
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
Dual3rdPartyNodeImp::
addEnqueMetaBufferToRequest(
    sp<IPipelineFrame>& pFrame,
    IPipelineFrame::MetaInfoIOMapSet const& rIOMapSet,
    sp<IIspPipeRequest>& pEffectReq
)
{
    IPipelineFrame::MetaInfoIOMap const& metaIOMap = rIOMapSet[0];

    IMetadata* pSplitHalMeta_Main2 = NULL;

    // Add input meta stream to EffectRequest
    for(size_t index=0; index<metaIOMap.vIn.size(); index++)
    {
        StreamId_T streamId = metaIOMap.vIn.keyAt(index);
        sp<IMetaStreamInfo> streamInfo = metaIOMap.vIn.valueAt(index);
        IspPipeBufferID bufID;
        if (mapToBufferID(streamId, bufID) != OK)
            return BAD_VALUE;

        MERROR ret = addMetaFrameInfo(pFrame, streamInfo, {bufID, eBUFFER_IOTYPE_INPUT}, pEffectReq);
        if (ret != OK)
            goto lbExit;
        MY_LOGD_IF(mLogLevel >= 2, "input MetaBuf stream bid=%d",bufID);
    }
    // Add output meta stream to EffectRequest
    for(size_t index=0; index<metaIOMap.vOut.size(); index++)
    {
        StreamId_T streamId = metaIOMap.vOut.keyAt(index);
        sp<IMetaStreamInfo> streamInfo = metaIOMap.vOut.valueAt(index);
        IspPipeBufferID bufID;
        if (mapToBufferID(streamId, bufID) != OK)
            goto lbExit;

        MERROR ret = addMetaFrameInfo(pFrame, streamInfo, {bufID, eBUFFER_IOTYPE_OUTPUT}, pEffectReq);
        if (ret != OK)
            goto lbExit;
        MY_LOGD_IF(mLogLevel >= 2, "output MetaBuf stream bid=%d",bufID);
    }

    return OK;

lbExit:
    if(pSplitHalMeta_Main2 != NULL)
        delete pSplitHalMeta_Main2;

    return BAD_VALUE;

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
Dual3rdPartyNodeImp::
mapToStreamID(IspPipeBufferID bufID, StreamId_T& rStreamId)
{
    MY_LOGD_IF(mLogLevel >= 1, "mapToStreamID: bufID=%d", bufID);

    if(mPipeBIDToStreamIDMap.indexOfKey(bufID)>=0)
    {
        rStreamId = mPipeBIDToStreamIDMap.valueFor(bufID);
        return OK;
    }
    else
    {
        MY_LOGE("mapToStreamID Failed!!! bufID=%d", bufID);
        return BAD_VALUE;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
Dual3rdPartyNodeImp::
mapToBufferID(StreamId_T streamId, IspPipeBufferID &rBufID)
{
    if(mStreamIDMapToIspBID.indexOfKey(streamId)>=0)
    {
        rBufID = mStreamIDMapToIspBID.valueFor(streamId);
        MY_LOGD_IF(mLogLevel >= 1, "mapToBufferID: streamId=%#" PRIx64 " BufID=%d", streamId, rBufID);
        return OK;
    }
    else
    {
        MY_LOGE("mapToBufferID Failed!!! streamId=%#" PRIx64 "", streamId);
        return BAD_VALUE;
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
Dual3rdPartyNodeImp::
retrieveImageBuffer(sp<IPipelineFrame> const& pFrame, StreamId_T const streamId, BufferIOType eType, sp<IImageBuffer>& rpImageBuffer)
{
    IStreamBufferSet& rStreamBufferSet  = pFrame->getStreamBufferSet();
    sp<IImageStreamBuffer>  pImageStreamBuffer = NULL;

    MERROR const err = ensureImageBufferAvailable_(
        pFrame->getFrameNo(),
        streamId,
        rStreamBufferSet,
        pImageStreamBuffer
    );

    if(err != OK)
    {
        MY_LOGW("reqID=%d streamId: %#" PRIx64 ", err = %d , failed to get pImageStreamBuffer! ",
                pFrame->getRequestNo(), streamId, err);
        return err;
    }

    MUINT groupUsage = pImageStreamBuffer->queryGroupUsage(getNodeId());
    sp<IImageBufferHeap> pImageBufferHeap = (eType == eBUFFER_IOTYPE_INPUT) ?
        pImageStreamBuffer->tryReadLock(getNodeName()) :
        pImageStreamBuffer->tryWriteLock(getNodeName());

    if(pImageBufferHeap == NULL)
    {
        MY_LOGW("[node:%#" PRIx64 "][stream buffer:%s] reqID=%d cannot get ImageBufferHeap",
                getNodeId(), pImageStreamBuffer->getName(), pFrame->getRequestNo());
        return BAD_VALUE;
    }

    rpImageBuffer = pImageBufferHeap->createImageBuffer();
    if (rpImageBuffer == NULL)
    {
        MY_LOGE("[node:%#" PRIx64 "][stream buffer:%s] reqID=%d cannot create ImageBuffer",
                getNodeId(), pImageStreamBuffer->getName(), pFrame->getRequestNo());
        return BAD_VALUE;
    }
    MBOOL bRet = rpImageBuffer->lockBuf(getNodeName(), groupUsage);
    if(!bRet)
    {
        MY_LOGE("reqID=%d LockBuf failed, stream%#" PRIx64 "!", pFrame->getRequestNo(), streamId);
        return BAD_VALUE;
    }

    MY_LOGD_IF(mLogLevel >= 1, "stream %#" PRIx64 ": buffer: %p, usage: %p",
        streamId, rpImageBuffer.get(), groupUsage);

    return OK;
}


MBOOL
Dual3rdPartyNodeImp::
validateTolerantStreamID(StreamId_T const streamId)
{
    ssize_t index = mvTolerantStreamId.indexOfKey(streamId);
    if(index<0)
        return MFALSE;
    else
        return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
Dual3rdPartyNodeImp::
releaseImageBuffer(
    sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    BufferIOType eType,
    sp<IImageBuffer> const& pImageBuffer,
    MBOOL bIsSuccess)
{
    IStreamBufferSet& rStreamBufferSet  = pFrame->getStreamBufferSet();
    sp<IImageStreamBuffer> pStreamBuffer = rStreamBufferSet.getImageBuffer(streamId, getNodeId());
    pImageBuffer->unlockBuf(getNodeName());
    pStreamBuffer->unlock(getNodeName(), pImageBuffer->getImageBufferHeap());

    if(eType==eBUFFER_IOTYPE_OUTPUT)
    {
        if(bIsSuccess)
            pStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_OK);
        else
            pStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_ERROR);
    }
    //
    //  Mark this buffer as USED by this user.
    //  Mark this buffer as RELEASE by this user.
    rStreamBufferSet.markUserStatus(
            streamId,
            getNodeId(),
            IUsersManager::UserStatus::USED|IUsersManager::UserStatus::RELEASE);

    MY_LOGD_IF(mLogLevel >= 1, "stream %#" PRIx64 ": buffer: %p", streamId, pImageBuffer.get());
    return OK;
}



/******************************************************************************
 *
 ******************************************************************************/
MERROR
Dual3rdPartyNodeImp::
retrieveMetadata(sp<IPipelineFrame> const& pFrame, StreamId_T const streamId, BufferIOType eType, IMetadata*& rpMetadata)
{
    IStreamBufferSet& rStreamBufferSet  = pFrame->getStreamBufferSet();

    sp<IMetaStreamBuffer>  pMetaStreamBuffer = NULL;

    MERROR const err = ensureMetaBufferAvailable_(
        pFrame->getFrameNo(),
        streamId,
        rStreamBufferSet,
        pMetaStreamBuffer
    );

    if(err != OK)
    {
        MY_LOGE("reqID=%d streamId: %#" PRIx64 " err=%d, failed to get pMetaStreamBuffer!", pFrame->getRequestNo(), streamId, err);
        return BAD_VALUE;
    }

    rpMetadata = (eType==eBUFFER_IOTYPE_INPUT) ?
        pMetaStreamBuffer->tryReadLock(getNodeName()) :
        pMetaStreamBuffer->tryWriteLock(getNodeName());


    if(rpMetadata == NULL)
    {
        MY_LOGE("[node:%#" PRIx64 "][stream buffer:%s] reqID=%d, Cannot get Metadata",
                getNodeId(), pMetaStreamBuffer->getName(), pFrame->getRequestNo());

    }


    return OK;

}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
Dual3rdPartyNodeImp::
releaseMetadata(
    sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    BufferIOType eType,
    IMetadata* const pMetadata,
    MBOOL bIsSuccess
)
{
    IStreamBufferSet& rStreamBufferSet  = pFrame->getStreamBufferSet();
    sp<IMetaStreamBuffer>  pMetaStreamBuffer = rStreamBufferSet.getMetaBuffer(streamId, getNodeId());

    pMetaStreamBuffer->unlock(getNodeName(), pMetadata);

    if(eType==eBUFFER_IOTYPE_OUTPUT)
    {
        if(bIsSuccess)
            pMetaStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_OK);
        else
            pMetaStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_ERROR);
    }
    rStreamBufferSet.markUserStatus(streamId, getNodeId(), IUsersManager::UserStatus::USED|IUsersManager::UserStatus::RELEASE);

    return OK;
}