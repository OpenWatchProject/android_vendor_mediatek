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

#define LOG_TAG "MtkCam/BokehNode"
//
#include "hwnode_utilities.h"

#include <mtkcam/pipeline/hwnode/BokehNode.h>
#include <mtkcam/pipeline/stream/IStreamInfo.h>
#include <mtkcam/pipeline/stream/IStreamBuffer.h>
#include "BaseNode.h"
//
#include <utils/RWLock.h>
#include <utils/Thread.h>

// for EffectHal
#include <mtkcam/feature/effectHalBase/IEffectHal.h>
#include <mtkcam/feature/effectHalBase/EffectHalBase.h>
#include <mtkcam/feature/effectHalBase/EffectRequest.h>
#include <mtkcam/feature/stereo/effecthal/BokehEffectHal.h>
#include <mtkcam/feature/stereo/hal/stereo_common.h>
// for metadata
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#include <mtkcam/feature/stereo/pipe/vsdof_common.h>
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>
#include <mtkcam/utils/std/Misc.h>
//
#include <string>
#include <list>
#include <algorithm>
//
#include <chrono>
// 3dnr
#include <mtkcam/feature/eis/eis_ext.h>
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils::Sync;
using namespace StereoHAL;

/******************************************************************************
 *
 ******************************************************************************/
#define BOKEHTHREAD_NAME       ("Cam@Bokeh")
#define BOKEHTHREAD_POLICY     (SCHED_OTHER)
#define BOKEHTHREAD_PRIORITY   (0)

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] [%s] " fmt, __FUNCTION__, this->getModeName(), ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] [%s] " fmt, __FUNCTION__, this->getModeName(), ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] [%s] " fmt, __FUNCTION__, this->getModeName(), ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] [%s] " fmt, __FUNCTION__, this->getModeName(), ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] [%s] " fmt, __FUNCTION__, this->getModeName(), ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] [%s] " fmt, __FUNCTION__, this->getModeName(), ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] [%s] " fmt, __FUNCTION__, this->getModeName(), ##arg)
// for static function
#define MY_LOGVS(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGDS(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGIS(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGWS(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGES(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGAS(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGFS(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
// for static function
#define MY_LOGVS_IF(cond, ...)       do { if ( (cond) ) { MY_LOGVS(__VA_ARGS__); } }while(0)
#define MY_LOGDS_IF(cond, ...)       do { if ( (cond) ) { MY_LOGDS(__VA_ARGS__); } }while(0)
#define MY_LOGIS_IF(cond, ...)       do { if ( (cond) ) { MY_LOGIS(__VA_ARGS__); } }while(0)
#define MY_LOGWS_IF(cond, ...)       do { if ( (cond) ) { MY_LOGWS(__VA_ARGS__); } }while(0)
#define MY_LOGES_IF(cond, ...)       do { if ( (cond) ) { MY_LOGES(__VA_ARGS__); } }while(0)
#define MY_LOGAS_IF(cond, ...)       do { if ( (cond) ) { MY_LOGAS(__VA_ARGS__); } }while(0)
#define MY_LOGFS_IF(cond, ...)       do { if ( (cond) ) { MY_LOGFS(__VA_ARGS__); } }while(0)

#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#define READ_USAGE eBUFFER_USAGE_SW_READ_MASK | eBUFFER_USAGE_HW_CAMERA_READ | eBUFFER_USAGE_HW_TEXTURE
/******************************************************************************
 *
 ******************************************************************************/
static inline
MBOOL
isStream(sp<IStreamInfo> pStreamInfo, StreamId_T streamId )
{
    return pStreamInfo.get() && pStreamInfo->getStreamId() == streamId;
}
/******************************************************************************
 *
 ******************************************************************************/
template<typename T>
inline MBOOL
tryGetMetadata(
    IMetadata *pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        MY_LOGWS("pMetadata == NULL");
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
inline MBOOL
is3DNROn(
    IMetadata* const inApp
)
{
    if (inApp == NULL) {
        return false;
    }
    MINT32 e3DnrMode = MTK_NR_FEATURE_3DNR_MODE_OFF;
    if(!tryGetMetadata<MINT32>(inApp,
        MTK_NR_FEATURE_3DNR_MODE, e3DnrMode))
    {
        MY_LOGWS_IF(1, "no MTK_NR_FEATURE_3DNR_MODE");
    }
    return e3DnrMode;
}
/******************************************************************************
 *
 ******************************************************************************/
    class process_frame
    {
        public:
            process_frame(
                        sp<IPipelineFrame> pFrame,
                        MUINT32 iFrameID,
                    MINT32 logLevel,
                    String8 modeName
                        ):
                        mpFrame(pFrame),
                        miFrameId(iFrameID),
                    mLogLevel(logLevel),
                    mModeName(modeName)
            {
            }
            ~process_frame()
            {
            }
            MBOOL                   init();
            MBOOL                   uninit(MBOOL vaild = MTRUE);
            MUINT32                 getScenario() {return scenario;}
            MUINT32                 getFrameID()  {return miFrameId;}
            MVOID                   setScenarioId(MUINT32 id)
                                    {
                                        scenario = id;
                                    }
            MVOID                   set3DNRData(
                                            MBOOL enable,
                                            MINT32 gmv_x,
                                            MINT32 gmv_y,
                                            MINT32 cmv_x,
                                            MINT32 cmv_y,
                                            MINT32 conf_x,
                                            MINT32 conf_y)
                                    {
                                        mb3DNREnable = enable;
                                        miGMV_x = gmv_x;
                                        miGMV_y = gmv_y;
                                        miCMV_x = cmv_x;
                                        miCMV_y = cmv_y;
                                        miConf_x = conf_x;
                                        miConf_y = conf_y;
                                    }
            MVOID                   setAFTrigger(MINT8 mode)
                                    {
                                        miAFTrigger = mode;
                                    }
            MVOID                   setAFRegion(MINT32 x, MINT32 y)
                                    {
                                        miAFx = x;
                                        miAFy = y;
                                    }
            MVOID                   setBokehLevel(MINT32 val)
                                    {
                                        miBokehLevel = val;
                                    }
            MVOID                   setConvOffset(MFLOAT val)
                                    {
                                        mfConvOffset = val;
                                    }
            MVOID                   setISO(MINT32 iso)
                                    {
                                        miISO = iso;
                                    }
            MVOID                   setDistance(MFLOAT dis)
                                    {
                                        mfDistance = dis;
                                    }
            MVOID                   setSensorProfile(MINT32 sensorProfile)
                                    {
                                        mSensorProfile = sensorProfile;
                                    }
            MBOOL                   is3DNREnable() {return mb3DNREnable;}
            MINT32                  getGMV_x()     {return miGMV_x;}
            MINT32                  getGMV_y()     {return miGMV_y;}
            MINT32                  getCMV_x()     {return miCMV_x;}
            MINT32                  getCMV_y()     {return miCMV_y;}
            MINT32                  getConf_x()     {return miConf_x;}
            MINT32                  getConf_y()     {return miConf_y;}
            MINT32                  getBokehLevel(){return miBokehLevel;}
            MFLOAT                  getDistance() {return mfDistance;}
            MINT32                  getAFTrigger(){return miAFTrigger;}
            MINT32                  getAFx(){return miAFx;}
            MINT32                  getAFy(){return miAFy;}
            MVOID                   setGSensor(MINT32 rot);
            MINT32                  getGSensor(){return miGSensorAngle;}
            MFLOAT                  getConvOffset(){return mfConvOffset;}
            MVOID                   setMagicNumber(MINT32 number) {miMagicNumber = number;}
            MINT32                  getMagicNumber(){return miMagicNumber;}
            MVOID                   unlockAllStream(const char* nodeName);
            const char*             getModeName() { return mModeName.string(); }
            MINT32                  getISO() { return miISO; }
            MINT32                  getSensorProfile() { return mSensorProfile; }
        private:
        public:
            sp<IPipelineFrame>          mpFrame                                 = NULL;
            DefaultKeyedVector<BokehEffectRequestBufferType, sp<IImageBuffer> >
                                        mvImgBufContainer;
            DefaultKeyedVector<BokehEffectRequestBufferType, sp<IImageStreamBuffer> >
                                        mvImgStreamBuffer;
            // Input: metadata
            IMetadata*                  mpInAppMetadata                         = NULL;
            sp<IMetaStreamBuffer>       mpInAppMetadataStreamBuffer             = NULL;
            IMetadata*                  mpInHalMetadata                         = NULL;
            sp<IMetaStreamBuffer>       mpInHalMetadataStreamBuffer             = NULL;

            IMetadata*                  mpInHalMetadata_P1                       = NULL;
            sp<IMetaStreamBuffer>       mpInHalMetadataStreamBuffer_P1           = NULL;
            // Output: metadata
            IMetadata*                  mpOutAppMetadata                         = NULL;
            sp<IMetaStreamBuffer>       mpOutAppMetadataStreamBuffer             = NULL;
            IMetadata*                  mpOutHalMetadata                         = NULL;
            sp<IMetaStreamBuffer>       mpOutHalMetadataStreamBuffer             = NULL;
            // store time to get processing time
            std::chrono::time_point<std::chrono::system_clock> start;
            std::chrono::time_point<std::chrono::system_clock> end;
            // StreamId in this pipeline frame
            DefaultKeyedVector<MUINT32, StreamId_T > mvStreamIdList;
        private:
            MUINT32                 scenario        = eSTEREO_SCENARIO_UNKNOWN;
            MBOOL                   mb3DNREnable     = MFALSE;
            MINT32                  miGMV_x         = 0;
            MINT32                  miGMV_y         = 0;
            MINT32                  miCMV_x         = 0;
            MINT32                  miCMV_y         = 0;
            MINT32                  miConf_x         = 0;
            MINT32                  miConf_y         = 0;
            MUINT32                 miFrameId       = 0;
            MINT32                  miBokehLevel    = 0;
            MINT32                  miAFx           = 0;
            MINT32                  miAFy           = 0;
            MUINT8                  miAFTrigger     = MTK_CONTROL_AF_TRIGGER_IDLE;
            MINT32                  mLogLevel       = -1;
            MINT32                  miGSensorAngle  = 0;
            MINT32                  miMagicNumber   = -1;
            MFLOAT                  mfConvOffset    = 0.0;
            String8                 mModeName = String8("ALL_HW");
            MINT32                  miISO    = 0;
            MINT32                  mSensorProfile = -1;
            MFLOAT                  mfDistance = 0;
    };
/******************************************************************************
 *
 ******************************************************************************/
class BokehNodeImp
    : public BaseNode
    , public BokehNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    //
public:     ////                    Operations.

                                    BokehNodeImp();

                                    ~BokehNodeImp();

    virtual MERROR                  config(ConfigParams const& rParams);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNode Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.

    virtual MERROR                  init(InitParams const& rParams);

    virtual MERROR                  uninit();

    virtual MERROR                  flush();

    virtual MERROR                  queue(
                                        android::sp<IPipelineFrame> pFrame
                                    );

protected:  ////                    Operations.
    MVOID                           onProcessFrame(
                                        android::sp<IPipelineFrame> const& pFrame
                                    );

    MERROR                          getImageBuffer(
                                        android::sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        sp<IImageStreamBuffer>& rpStreamBuffer,
                                        sp<IImageBuffer>& rpImageBuffer,
                                        MBOOL const isInStream
                                    );

protected:
    MBOOL                           isInMetaStream(
                                        StreamId_T const streamId
                                    ) const;
    MVOID                           releaseMetaStream(
                                                      sp<IPipelineFrame> const& pFrame,
                                                      sp<IMetaStreamBuffer> const pStreamBuffer,
                                                      MUINT32 mask)const;
    MVOID                           releaseImageStream(
                                                      sp<IPipelineFrame> const& pFrame,
                                                      sp<IImageStreamBuffer> const pStreamBuffer,
                                                      MUINT32 mask)const;
    MERROR                          suspendThisFrame(
                                                      sp<IPipelineFrame> const& pFrame,
                                                      process_frame*& data);
private:
    static MVOID                    EffectHalCb(MVOID* tag, String8 status, android::sp<NSCam::EffectRequest>& request);
    MVOID                           AddEffectRequest(process_frame *pFrameData);
    MBOOL                           queryGMVFromMetaStream(process_frame *pFrameData, IMetadata* const metaStream);
    MBOOL                           queryISOFromMetaStream(process_frame *pFrameData, IMetadata* const metaStream);
public:
    MVOID                           onProcessEffectRequestSuccess(process_frame*& frame);
    MVOID                           onProcessEffectRequestFail(process_frame*& frame);
    MVOID                           releaseBuffer(process_frame*& frame, MUINT32 mask);
    const char*                     getModeName() { return mModeName.string(); }
    MBOOL                           getAfInfo(process_frame *pProcessFrame, sp<EffectParameter>& params);

protected:  ////                    Data Members. (Request Queue)
    mutable RWLock                  mConfigRWLock;
    // image
    sp<IImageStreamInfo>            mpInHalImageMain                = NULL;
    sp<IImageStreamInfo>            mpInHalImageMain_Capture        = NULL;
    //sp<IImageStreamInfo>            mpInHalImageMY_S                = NULL;
    sp<IImageStreamInfo>            mpInHalImageDMBG                 = NULL;
    sp<IImageStreamInfo>            mpInHalImageDepth                = NULL;
    sp<IImageStreamInfo>            mpInHalImageP2Tuning             = NULL;
    // sw bokeh
    sp<IImageStreamInfo>            mpInHalDepthWrapper             = NULL;
    sp<IImageStreamInfo>            mpInHalExtraData                = NULL;
    //
    sp<IImageStreamInfo>            mpOutHalImageCleanImage        = NULL;
    sp<IImageStreamInfo>            mpOutHalImageBokehResult        = NULL;
    sp<IImageStreamInfo>            mpOutHalImageBokehThumbnail     = NULL;
    //
    sp<IImageStreamInfo>            mpOutAppImagePostView           = NULL;
    sp<IImageStreamInfo>            mpOutAppImageRecord             = NULL;
    sp<IImageStreamInfo>            mpOutAppImagePreview            = NULL;
    sp<IImageStreamInfo>            mpOutAppPrvCB                   = NULL;
    sp<IImageStreamInfo>            mpOutAppDepthMapWrapper         = NULL;
    sp<IImageStreamInfo>            mpOutAppExtraData               = NULL;
    // metadata
    sp<IMetaStreamInfo>             mpInHalMetadata                 = NULL;
    sp<IMetaStreamInfo>             mpInAppMetadata                 = NULL;
    //
    sp<IMetaStreamInfo>             mpOutAppMetadata                = NULL;
    sp<IMetaStreamInfo>             mpOutHalMetadata                 = NULL;
    //
    RWLock                          mRequestQueueLock;

private:
    //
    // EffectHal
    BokehEffectHal                 *mpBokehEffectHal                = NULL;
    //
    // debug level
    MINT32                          mLogLevel                      = -1;
    //
    DefaultKeyedVector<StreamId_T, BokehEffectRequestBufferType>
                                    mvStreamIdMap;
    MUINT32                         miEffectRequestInCount          = 0;
    MUINT32                         miEffectRequestOutCount         = 0;
    //
    //MSize                           miMYSSize;
    MBOOL                           mbFirstRun = MTRUE;
    //
    std::list<MINT32>               mvNeededStream;
    MBOOL                           mbFlushing = MFALSE;
    DefaultKeyedVector<MUINT32, sp<NSCam::EffectRequest> >
                                    mvRequestQueue;
    MUINT32                         mBokehMode = Mode::ALL_HW;
    String8                         mModeName = String8("ALL_HW");
};


/******************************************************************************
 *
 ******************************************************************************/
android::sp<BokehNode>
BokehNode::
createInstance()
{
    MY_LOGDS("createInstance");
    return new BokehNodeImp();
}


/******************************************************************************
 *
 ******************************************************************************/
BokehNodeImp::
BokehNodeImp()
    : BaseNode()
    , BokehNode()
    , mConfigRWLock()
    , mRequestQueueLock()
{
    char cLogLevel[PROPERTY_VALUE_MAX];
    ::property_get("vendor.debug.camera.log", cLogLevel, "0");
    mLogLevel = atoi(cLogLevel);
    if ( mLogLevel == 0 ) {
        ::property_get("vendor.debug.camera.log.bokeh", cLogLevel, "0");
        mLogLevel = atoi(cLogLevel);
    }
    if ( 0 == mLogLevel ) {
        ::property_get("vendor.debug.camera.log.basenode", cLogLevel, "0");
        mLogLevel = ::atoi(cLogLevel);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
BokehNodeImp::
~BokehNodeImp()
{

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
BokehNodeImp::
init(InitParams const& rParams)
{
    FUNC_START;
    //
    mOpenId = rParams.openId;
    mNodeId = rParams.nodeId;
    mNodeName = rParams.nodeName;
    //
    MY_LOGDS("OpenId %d, mNodeId %d, mNodeName %s",
            getOpenId(),
            getNodeId(),
            getNodeName());
    //
    mvNeededStream.clear();
    mvRequestQueue.clear();
    //
    MY_LOGDS("mLogLevel(%d)", mLogLevel);
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
BokehNodeImp::
config(ConfigParams const& rParams)
{
    FUNC_START;
    //create effecthal
    if(rParams.bokehMode == ALL_HW)
    {
        mBokehMode = GENERAL;
        mModeName = "ALL_HW";
    }
    else if(rParams.bokehMode == HW_BOKEH_NODE)
    {
        mBokehMode = HW_BOKEH;
        mModeName = "HW";
    }
    else if(rParams.bokehMode == SW_BOKEH_NODE)
    {
        mBokehMode = SW_BOKEH;
        mModeName = "SW";
    }
    else if(rParams.bokehMode == VENDOR_BOKEH_NODE)
    {
        mBokehMode = VENDOR_BOKEH;
        mModeName = "VENDOR";
    }
    else
    {
        MY_LOGE("Should not happened");
        return UNKNOWN_ERROR;
    }
    // create bokeh EffectHal
    mpBokehEffectHal = new BokehEffectHal();
    mpBokehEffectHal->init();
    sp<EffectParameter> effectParams = new EffectParameter();
    effectParams->set(BOKEH_EFFECT_REQUEST_OPEN_ID, getOpenId());
    effectParams->set(BOKEH_EFFECT_REQUEST_RUN_PATH, mBokehMode);
    effectParams->set(VSDOF_FEAME_STOP_MODE, FLUSH);//VSDOF_STOP_MODE::FLUSH);
    mpBokehEffectHal->setParameters(effectParams);
    mpBokehEffectHal->configure();
    mpBokehEffectHal->prepare();
    //
    if(mvStreamIdMap.size()!=0)
        mvStreamIdMap.clear();
    //
    {
        // metadata: input
        mpInAppMetadata = rParams.pInAppMeta;
        mpInHalMetadata = rParams.pInHalMeta;
        // metadata: output
        mpOutAppMetadata = rParams.pOutAppMeta;
        mpOutHalMetadata = rParams.pOutHalMeta;
        // imagebuffer stream: input
        mpInHalImageMain = rParams.pInHalImageMainImage;
        mpInHalImageMain_Capture = rParams.pInHalImageMainImage_Capture;
        //mpInHalImageMY_S = rParams.pInHalImageMYS;
        mpInHalImageDMBG = rParams.pInHalImageDMBG;
        mpInHalImageDepth = rParams.pInHalImageDepth;
        mpInHalImageP2Tuning = rParams.pInHalImageP2Tuning;
        // imagebuffer stream: output
        mpOutAppImagePostView = rParams.pOutAppImagePostView;
        mpOutHalImageCleanImage = rParams.pOutHalImageCleanImage;
        mpOutAppImageRecord = rParams.pOutAppImageRecord;
        mpOutAppImagePreview = rParams.pOutAppImagePreview;
        mpOutHalImageBokehResult = rParams.pOutHalImageBokehResult;
        mpOutHalImageBokehThumbnail = rParams.pOutHalImageBokehThumbnail;
        mpOutAppPrvCB = rParams.pOutAppPrvCB;
        // sw bokeh
        mpInHalDepthWrapper = rParams.pInHalDepthWrapper;
        mpInHalExtraData = rParams.pInHalExtraData;
        mpOutAppDepthMapWrapper = rParams.pOutAppDepthMapWrapper;
        mpOutAppExtraData = rParams.pOutAppExtraData;
        // build StreamId map for effect request
#define BUILD_STREAMID_AND_REQUESTID_MAP(STREAMINFO, TYPE)\
        do{\
            if(STREAMINFO!=nullptr)\
            {\
                mvStreamIdMap.add(\
                        STREAMINFO->getStreamId(), TYPE);\
                MY_LOGD("key "#STREAMINFO" (%#" PRIx64 ") value "#TYPE" %x", STREAMINFO->getStreamId(), TYPE);\
            }\
            else\
            {\
                MY_LOGD(#STREAMINFO " is null");\
            }\
        }while(0);
        BUILD_STREAMID_AND_REQUESTID_MAP(mpInHalImageMain, BOKEH_ER_BUF_MAIN1);
        BUILD_STREAMID_AND_REQUESTID_MAP(mpInHalImageMain_Capture, BOKEH_ER_BUF_MAIN1);
        BUILD_STREAMID_AND_REQUESTID_MAP(mpInHalImageDMBG, BOKEH_ER_BUF_DMBG);
        BUILD_STREAMID_AND_REQUESTID_MAP(mpInHalImageDepth, BOKEH_ER_OUTPUT_DEPTHMAP);
        BUILD_STREAMID_AND_REQUESTID_MAP(mpInHalImageP2Tuning, BOKEH_ER_BUF_P2_TUNING);
        // sw bokeh
        BUILD_STREAMID_AND_REQUESTID_MAP(mpInHalDepthWrapper, BOKEH_ER_BUF_HAL_DEPTHWRAPPER);
        BUILD_STREAMID_AND_REQUESTID_MAP(mpInHalExtraData, BOKEH_ER_BUF_EXTRADATA);
        //
        BUILD_STREAMID_AND_REQUESTID_MAP(mpOutHalImageCleanImage, BOKEH_ER_BUF_CLEAN_IMG);
        BUILD_STREAMID_AND_REQUESTID_MAP(mpOutAppImageRecord, BOKEH_ER_BUF_RECORD);
        BUILD_STREAMID_AND_REQUESTID_MAP(mpOutAppImagePreview, BOKEH_ER_BUF_DISPLAY);
        BUILD_STREAMID_AND_REQUESTID_MAP(mpOutHalImageBokehResult, BOKEH_ER_BUF_VSDOF_IMG);
        BUILD_STREAMID_AND_REQUESTID_MAP(mpOutHalImageBokehThumbnail, BOKEH_ER_BUF_THUMBNAIL);
        BUILD_STREAMID_AND_REQUESTID_MAP(mpOutAppPrvCB, BOKEH_ER_BUF_PRVCB);
        // sw bokeh
        BUILD_STREAMID_AND_REQUESTID_MAP(mpOutAppDepthMapWrapper, BOKEH_ER_BUF_DEPTHMAPWRAPPER);
        BUILD_STREAMID_AND_REQUESTID_MAP(mpOutAppExtraData, BOKEH_ER_BUF_APP_EXTRA_DATA);
#undef BUILD_STREAMID_AND_REQUESTID_MAP
    }

/******************************************************************************
 *
 ******************************************************************************/
auto dumpStreamInfo = [](const sp<IImageStreamInfo>& info)
    {
    if(info == nullptr)
    {
        return;
    }
    MY_LOGDS("streamInfo(%#08x) name(%s) format(%#08x) size(%dx%d)",
            info->getStreamId(), info->getStreamName(),
            info->getImgFormat(), info->getImgSize().w, info->getImgSize().h);
};
    //
    dumpStreamInfo(mpInHalImageMain);
    //dumpStreamInfo(mpInHalImageMY_S);
    dumpStreamInfo(mpInHalImageDMBG);
    dumpStreamInfo(mpInHalImageDepth);
    dumpStreamInfo(mpInHalImageP2Tuning);
    // sw bokeh
    dumpStreamInfo(mpInHalDepthWrapper);
    dumpStreamInfo(mpInHalExtraData);
    dumpStreamInfo(mpOutAppImagePostView);
    dumpStreamInfo(mpOutHalImageCleanImage);
    dumpStreamInfo(mpOutAppImageRecord);
    dumpStreamInfo(mpOutAppImagePreview);
    dumpStreamInfo(mpOutHalImageBokehResult);
    dumpStreamInfo(mpInHalImageMain_Capture);
    dumpStreamInfo(mpOutHalImageBokehThumbnail);
    dumpStreamInfo(mpOutAppPrvCB);
    dumpStreamInfo(mpOutAppDepthMapWrapper);
    dumpStreamInfo(mpOutAppExtraData);
    //
    mpBokehEffectHal->start();
    //
    {
        auto addToNeedStream =[this](sp<IImageStreamInfo> imgBuffer)
        {
            if(imgBuffer!=0)
            {
                mvNeededStream.push_back(imgBuffer->getStreamId());
            }
            return;
        };
        if(mBokehMode == HW_BOKEH||
           mBokehMode == GENERAL)
        {
            MY_LOGD("HW and ALL_HW flow");
            addToNeedStream(mpInHalImageMain);
            addToNeedStream(mpInHalImageP2Tuning);
            addToNeedStream(mpOutAppImagePostView);
            addToNeedStream(mpOutAppImagePreview);
        }
        if(mBokehMode == SW_BOKEH||
           mBokehMode == GENERAL)
        {
            MY_LOGD("SW and ALL_HW flow");
            addToNeedStream(mpInHalImageMain_Capture);
            addToNeedStream(mpInHalDepthWrapper);
            addToNeedStream(mpInHalExtraData);
            addToNeedStream(mpOutHalImageCleanImage);
            addToNeedStream(mpOutHalImageBokehResult);
            addToNeedStream(mpOutHalImageBokehThumbnail);
            addToNeedStream(mpOutAppDepthMapWrapper);
            addToNeedStream(mpOutAppExtraData);
        }
        if(mpInHalImageDMBG != nullptr)
            addToNeedStream(mpInHalImageDMBG);
        if(mpInHalImageDepth != nullptr)
            addToNeedStream(mpInHalImageDepth);
        //addToNeedStream(mpOutAppImageRecord);
    }
    mbFlushing = MFALSE;
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
BokehNodeImp::
uninit()
{
    FUNC_START;
    //
    if ( OK != flush() )
        MY_LOGE("flush failed");
    // release effectHal
    if(NULL != mpBokehEffectHal)
    {
        mpBokehEffectHal->abort();
        mpBokehEffectHal->release();
        mpBokehEffectHal->unconfigure();
        mpBokehEffectHal->uninit();
        delete mpBokehEffectHal;
        mpBokehEffectHal = NULL;
        MY_LOGD("Destroy bokeh effectHal.");
    }
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
BokehNodeImp::
flush()
{
    FUNC_START;
    {
        RWLock::AutoWLock _l(mRequestQueueLock);
        mbFlushing = MTRUE;
    }
    // clear effect hal request
    if(mpBokehEffectHal != nullptr)
    {
        mpBokehEffectHal->flush();
    }
    {
        RWLock::AutoWLock _l(mRequestQueueLock);
    //
        sp<EffectRequest> request = nullptr;
        for(MUINT32 i=0;i<mvRequestQueue.size();++i)
        {
            request = mvRequestQueue.valueAt(i);
            if(request!=nullptr)
            {
                const sp<EffectParameter> params = request->getRequestParameter();
                process_frame *frame = reinterpret_cast<process_frame*>(params->getPtr(VSDOF_FRAME_USER_DATA));

                MUINT32 frameId = frame->getFrameID();
                MY_LOGD_IF(mLogLevel>0, "+ status(FLUSH) requestId(%d)", frameId);
                //
                frame->unlockAllStream(getNodeName());
                // release image buffer/ metadata buffer
                releaseBuffer(frame, STREAM_BUFFER_STATUS::WRITE_ERROR);
                // release
                sp<IPipelineFrame> const pFrame = frame->mpFrame;
                if(frame->mpFrame == 0)
                {
                    MY_LOGE("mpFrame is null");
                }
                IStreamBufferSet& streamBufferSet = pFrame->getStreamBufferSet();
                streamBufferSet.applyRelease(getNodeId());
                //
                frame->uninit();
                delete frame;
                frame = nullptr;
                onDispatchFrame(pFrame);
            }
        }
        mvRequestQueue.clear();
        mbFlushing = MFALSE;
    }
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
BokehNodeImp::
queue(android::sp<IPipelineFrame> pFrame)
{
    //FUNC_START;
    //
    if( ! pFrame.get() ) {
        MY_LOGE("Null frame");
        return BAD_VALUE;
    }

    MBOOL bFlushing;
    {
        RWLock::AutoRLock _l(mRequestQueueLock);
        MBOOL bFlushing = mbFlushing;
        }
    if(!bFlushing)
    {
        onProcessFrame(pFrame);
    }
    else
{
        BaseNode::flush(pFrame);
        }
    //
    //FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
BokehNodeImp::
onProcessFrame(
    android::sp<IPipelineFrame> const& pFrame
)
{
    //FUNC_START;
    //
    IPipelineFrame::InfoIOMapSet IOMapSet;

    IStreamBufferSet& streamBufferSet = pFrame->getStreamBufferSet();
    //
    process_frame* pProcessFrame = NULL;

    if(OK != pFrame->queryInfoIOMapSet( getNodeId(), IOMapSet )) {
        MY_LOGE("Get IOMapSet failed.");
        return;
    }
    android::sp<IStreamInfoSet const> rIn, rOut;
    pFrame->queryIOStreamInfoSet(getNodeId(), rIn, rOut);
    //
    pProcessFrame = new process_frame(pFrame, pFrame->getRequestNo(), mLogLevel, mModeName);
    pProcessFrame->init();
    // Process InHalMeta and InHalMeta.
    {
        // Get GMV data and AF info
        // 1. Check if NR3D is on.
        // 2. Get GMV data from Hal metadata.
        // 3. Get AF data from Hal metadata.
        MBOOL b3DNR = MFALSE;
        // Step 1. -----------------------------------------------------------------------------------
        if(OK==ensureMetaBufferAvailable_(
                                        pFrame->getFrameNo(),
                                        mpInAppMetadata->getStreamId(),
                                        streamBufferSet,
                                        pProcessFrame->mpInAppMetadataStreamBuffer))
        {
            pProcessFrame->mpInAppMetadata = pProcessFrame->mpInAppMetadataStreamBuffer->tryReadLock(getNodeName());
            // get bokeh level
            MINT32 tempBokehLevel = 0;
            if(!tryGetMetadata<MINT32>(pProcessFrame->mpInAppMetadata, MTK_STEREO_FEATURE_DOF_LEVEL, tempBokehLevel))
            {
                MY_LOGE("Get bokeh level fail.");
            }
            pProcessFrame->setBokehLevel(tempBokehLevel);
            //
            b3DNR = is3DNROn(pProcessFrame->mpInAppMetadata);
        }
        else
        {
            MY_LOGE("(InAppMetadata) ensureMetaBufferAvailable_");
            suspendThisFrame(pFrame, pProcessFrame);
            return;
        }
        if(OK==ensureMetaBufferAvailable_(pFrame->getFrameNo(),
                                          mpInHalMetadata->getStreamId(),
                                          streamBufferSet,
                                          pProcessFrame->mpInHalMetadataStreamBuffer))
        {
            pProcessFrame->mpInHalMetadata = isInMetaStream(mpInHalMetadata->getStreamId()) ?
                                      pProcessFrame->mpInHalMetadataStreamBuffer->tryReadLock(getNodeName()) :
                                      pProcessFrame->mpInHalMetadataStreamBuffer->tryWriteLock(getNodeName());
            MFLOAT distance = 0;
            if (!tryGetMetadata<MFLOAT>(pProcessFrame->mpInHalMetadata,
                                        MTK_STEREO_FEATURE_RESULT_DISTANCE, distance))
            {
                MY_LOGW("Failed to get MTK_STEREO_FEATURE_RESULT_DISTANCE, reqID=%d",
                                                                pFrame->getRequestNo());
            }
            else
            {
                pProcessFrame->setDistance(distance);
            }
        }
        else
        {
            MY_LOGE("(InHalMetadata) ensureMetaBufferAvailable_");
            suspendThisFrame(pFrame, pProcessFrame);
            return;
        }
        // Step 2. -----------------------------------------------------------------------------------
        if(b3DNR)
        {
            if(mBokehMode != VENDOR_BOKEH)
            {
                // 3rd flow doesn't have n3rd.
                queryGMVFromMetaStream(pProcessFrame, pProcessFrame->mpInHalMetadata);
            }
            queryISOFromMetaStream(pProcessFrame, pProcessFrame->mpInHalMetadata);
        }
        // get sensor profile
        {
            MINT32 sensorProfile = -1;
            if(!tryGetMetadata<MINT32>(pProcessFrame->mpInHalMetadata, MTK_STEREO_FEATURE_SENSOR_PROFILE, sensorProfile))
            {
//                MY_LOGE("Get sensor profile fail.");
            }
            pProcessFrame->setSensorProfile(sensorProfile);
        }
        // get magic number
        MINT32 tempMagicNumber = 0;
        if(!tryGetMetadata<MINT32>(pProcessFrame->mpInHalMetadata, MTK_P1NODE_PROCESSOR_MAGICNUM, tempMagicNumber))
        {
            MY_LOGE("Get magic number fail.");
        }
        pProcessFrame->setMagicNumber(tempMagicNumber);
        // -------------------------------------------------------------------------------------------
    }
    // Handle output metadata
    {
        if(OK != ensureMetaBufferAvailable_(
                                    pFrame->getFrameNo(),
                                    mpOutAppMetadata->getStreamId(),
                                    streamBufferSet,
                                    pProcessFrame->mpOutAppMetadataStreamBuffer))
        {
            MY_LOGE("(OutAppMetadata) ensureMetaBufferAvailable_");
            suspendThisFrame(pFrame, pProcessFrame);
            return;
        }
        pProcessFrame->mpOutAppMetadata = pProcessFrame->mpOutAppMetadataStreamBuffer->tryWriteLock(getNodeName());
        if(OK != ensureMetaBufferAvailable_(
                                    pFrame->getFrameNo(),
                                    mpOutHalMetadata->getStreamId(),
                                    streamBufferSet,
                                    pProcessFrame->mpOutHalMetadataStreamBuffer))
        {
            MY_LOGE("(OutHalMetadata) ensureMetaBufferAvailable_");
            suspendThisFrame(pFrame, pProcessFrame);
            return;
        }
        pProcessFrame->mpOutHalMetadata = pProcessFrame->mpOutHalMetadataStreamBuffer->tryWriteLock(getNodeName());
    }
    //
    IPipelineFrame::ImageInfoIOMap const& imageIOMap = IOMapSet.mImageInfoIOMapSet[0];
    {
        StreamId_T streamId = 0;
        MINT8 scenario = eSTEREO_SCENARIO_UNKNOWN;
        sp<IImageStreamBuffer> pStreamBuffer = nullptr;
        sp<IImageBuffer> pImgBuffer = nullptr;
        auto checkIsNeededStream=[this](MINT32 streamid)
        {
            return (std::find(mvNeededStream.begin(), mvNeededStream.end(), streamid) != mvNeededStream.end());
        };

        for( size_t i=0; i<imageIOMap.vOut.size() ; ++i )
        {
            streamId = imageIOMap.vOut.keyAt(i);
            if(isStream(this->mpOutHalImageCleanImage, streamId))
            {
                MY_LOGD("capture frame(%d)", pFrame->getRequestNo());
                scenario = eSTEREO_SCENARIO_CAPTURE;
                // get g sensor
                MINT32 tempGsensor = 0;
                if(!tryGetMetadata<MINT32>(pProcessFrame->mpInAppMetadata, MTK_JPEG_ORIENTATION, tempGsensor))
                {
                    MY_LOGE("Get g sensor fail.");
                }
                pProcessFrame->setGSensor(tempGsensor);
            }
            else if(isStream(this->mpOutAppImageRecord, streamId))
            {
                scenario = eSTEREO_SCENARIO_RECORD;
            }
            // get output buffer
            auto index = mvStreamIdMap.indexOfKey(streamId);
            auto bufIndex = mvStreamIdMap.valueAt(index);
            MY_LOGD_IF(mLogLevel>0, "reqID=%d, out streamid(%#" PRIx64 ") bufIndex(%x)",
                                     pFrame->getRequestNo(), streamId, bufIndex);
            if(index >= 0 && bufIndex >= 0)
            {
                MERROR const err1 = getImageBuffer(
                            pFrame,
                            streamId,
                            pStreamBuffer,//pProcessFrame->mvImgStreamBuffer[it->second],
                            pImgBuffer, //pProcessFrame->mvImgBufContainer[it->second],
                            MFALSE
                            );
                MY_LOGD_IF(mLogLevel>0, "err(%d) streamid(%#" PRIx64 ") need(%d)",
                                    err1 != OK, streamId, checkIsNeededStream(streamId));
                if( err1 == OK)
                {
                    //  Query the group usage.
                    MUINT const groupUsage = pStreamBuffer->queryGroupUsage(getNodeId());
                    pImgBuffer->lockBuf(getNodeName(), groupUsage);

                    MY_LOGD_IF(mLogLevel>0, "reqID=%d, out streamid(%#" PRIx64 ") bufIndex(%x),"
                                            "ensure buffer success, buffer=%x",
                            pFrame->getRequestNo(), streamId, bufIndex, pImgBuffer.get());
                }
                else if( err1 != OK && checkIsNeededStream(streamId)) {
                    MY_LOGE("getImageBuffer err = %d StreamId(%#" PRIx64 ")", err1, streamId);
                    suspendThisFrame(pFrame, pProcessFrame);
                    return;
                }
                auto bufKey = pProcessFrame->mvImgStreamBuffer.indexOfKey(bufIndex);
                pProcessFrame->mvImgStreamBuffer.replaceValueAt(bufKey, pStreamBuffer);
                pProcessFrame->mvImgBufContainer.replaceValueAt(bufKey, pImgBuffer);
                if(mLogLevel>0)
                {
                    MY_LOGD("reqID=%d, Lock out buffer streamId(%#" PRIx64 ") imgBuf(0x%x) streamBuf(0x%x)",
                            pFrame->getRequestNo(),
                            streamId,
                            pProcessFrame->mvImgStreamBuffer.valueAt(bufKey).get(),
                            pProcessFrame->mvImgBufContainer.valueAt(bufKey).get());
                }
            }
            else
            {
                MY_LOGE("cannot find stream in StreamIdMap(%#" PRIx64 ")", streamId);
            }
        }
        //
        if(scenario == eSTEREO_SCENARIO_UNKNOWN)
        {
            scenario = eSTEREO_SCENARIO_PREVIEW;
        }
        pProcessFrame->setScenarioId(scenario);
        //
        for( size_t i=0; i<imageIOMap.vIn.size() ; ++i )
        {
            streamId = imageIOMap.vIn.keyAt(i);
            auto index = mvStreamIdMap.indexOfKey(streamId);
            auto bufIndex = mvStreamIdMap.valueAt(index);
            MY_LOGD_IF(mLogLevel>0, "in streamid(%#" PRIx64 ") bufIndex(%x)", streamId, bufIndex);
            if(index >= 0 && bufIndex >= 0)
            {
                MERROR const err1 = getImageBuffer(
                            pFrame,
                            streamId,
                            pStreamBuffer,//pProcessFrame->mvImgStreamBuffer[it->second],
                            pImgBuffer, //pProcessFrame->mvImgBufContainer[it->second],
                            MTRUE
                            );

                if( err1 == OK)
                {
                    pImgBuffer->lockBuf(getNodeName(), READ_USAGE);
                }
                else if( err1 != OK && checkIsNeededStream(streamId)) {
                    MY_LOGE("getImageBuffer err = %d StreamId(%#" PRIx64 ")", err1, streamId);
                    suspendThisFrame(pFrame, pProcessFrame);
                    return;
                }
                auto bufKey = pProcessFrame->mvImgStreamBuffer.indexOfKey(bufIndex);
                pProcessFrame->mvImgStreamBuffer.replaceValueAt(bufKey, pStreamBuffer);
                pProcessFrame->mvImgBufContainer.replaceValueAt(bufKey, pImgBuffer);
                if(mLogLevel>0)
                {
                    MY_LOGD("Lock in buffer streamId(%#" PRIx64 ") imgBuf(0x%x) streamBuf(0x%x)",
                        streamId,
                        pProcessFrame->mvImgStreamBuffer.valueAt(bufKey).get(),
                        pProcessFrame->mvImgBufContainer.valueAt(bufKey).get());
                }
            }
            else
            {
                MY_LOGE("cannot find stream in StreamIdMap(%#" PRIx64 ")", streamId);
            }
        }
        if(eSTEREO_SCENARIO_CAPTURE==scenario)
        {
            // replace APP out metadata
            if(pProcessFrame->mpInAppMetadata!=nullptr &&
                pProcessFrame->mpOutAppMetadata!=nullptr)
            {
                *pProcessFrame->mpOutAppMetadata = *pProcessFrame->mpInAppMetadata;
            }
        }
    }
    AddEffectRequest(pProcessFrame);

    //FUNC_END;
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
BokehNodeImp::
getImageBuffer(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    sp<IImageStreamBuffer>& rpStreamBuffer,
    sp<IImageBuffer>& rpImageBuffer,
    MBOOL const isInStream
)
{
    IStreamBufferSet&      rStreamBufferSet = pFrame->getStreamBufferSet();
    sp<IImageBufferHeap>   pImageBufferHeap = NULL;
    MY_LOGD_IF(mLogLevel>0, "RequestNo(%d) StreamId(%#" PRIx64 ")", pFrame->getRequestNo(), streamId);
    if(streamId == 0)
    {
        MY_LOGE("StreamId is 0");
        return BAD_VALUE;
    }
    MERROR const err = ensureImageBufferAvailable_(
            pFrame->getFrameNo(),
            streamId,
            rStreamBufferSet,
            rpStreamBuffer
            );

    if( err != OK )
    {
        MY_LOGD_IF(mLogLevel>0, "ensureImageBufferAvailable_ fail");
        return err;
    }

    if(isInStream && !isStream(mpInHalImageP2Tuning, streamId)){
        pImageBufferHeap = rpStreamBuffer->tryReadLock(getNodeName());
    }
    else{
        pImageBufferHeap = rpStreamBuffer->tryWriteLock(getNodeName());
    }

    if (pImageBufferHeap == NULL) {
        MY_LOGE("pImageBufferHeap == NULL");
        return BAD_VALUE;
    }
    MY_LOGD_IF(mLogLevel>0, "@pImageBufferHeap->getBufSizeInBytes(0) = %d", pImageBufferHeap->getBufSizeInBytes(0));
    rpImageBuffer = pImageBufferHeap->createImageBuffer();

    if (rpImageBuffer == NULL) {
        MY_LOGE("rpImageBuffer == NULL");
        return BAD_VALUE;
    }

    MY_LOGD_IF(mLogLevel>0, "stream buffer: (%p) %p, heap: %p, buffer: %p",
        streamId, rpStreamBuffer.get(), pImageBufferHeap.get(), rpImageBuffer.get());

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
BokehNodeImp::
EffectHalCb(MVOID* tag, String8 status, sp<NSCam::EffectRequest>& request)
{
    BokehNodeImp *_BokehNode = reinterpret_cast<BokehNodeImp*>(tag);
    //
    const sp<EffectParameter> params = request->getRequestParameter();
    process_frame *frame = reinterpret_cast<process_frame*>(params->getPtr(VSDOF_FRAME_USER_DATA));
    //
    if(status == String8("Done"))
    {
        _BokehNode->onProcessEffectRequestSuccess(frame);
    }
    else
    {
        MY_LOGDS("status(%s)", status.string());
        _BokehNode->onProcessEffectRequestFail(frame);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
BokehNodeImp::
onProcessEffectRequestFail(process_frame*& frame)
{
    if(frame!=nullptr)
    {
        MUINT32 frameId = frame->getFrameID();
        MY_LOGD_IF(mLogLevel>0, "+ status(fail) requestId(%d)", frameId);
        //
        frame->unlockAllStream(getNodeName());
        // release image buffer/ metadata buffer
        releaseBuffer(frame, STREAM_BUFFER_STATUS::WRITE_ERROR);
        // release
        sp<IPipelineFrame> const pFrame = frame->mpFrame;
        if(frame->mpFrame == 0)
        {
            MY_LOGE("mpFrame is null");
        }
        IStreamBufferSet& streamBufferSet = pFrame->getStreamBufferSet();
        streamBufferSet.applyRelease(getNodeId());
        //
        frame->uninit();
        delete frame;
        frame = nullptr;
        onDispatchFrame(pFrame);
        {
            RWLock::AutoWLock _l(mRequestQueueLock);
            ssize_t index = mvRequestQueue.indexOfKey(frameId);
            if(index>=0)
            {
                mvRequestQueue.removeItemsAt(index);
            }
        miEffectRequestOutCount++;
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
BokehNodeImp::
onProcessEffectRequestSuccess(process_frame*& frame)
{
    if(mLogLevel==28285)
    {
        MY_LOGD("Dump image +");
        NSCam::Utils::makePath("/sdcard/bokeh/",0660);
        std::string filename = "/sdcard/bokeh/bokeh_"+std::to_string(frame->getFrameID())+".yuv";
        sp<IImageBuffer> pBuf = nullptr;
        for(ssize_t i=0;i<frame->mvImgBufContainer.size();++i)
        {
            pBuf = frame->mvImgBufContainer.valueAt(i);
            if(pBuf!=nullptr)
            {
                pBuf->saveToFile(filename.c_str());
            }
        }
        MY_LOGD("Dump image -");
    }
    // get process_frame
    if(frame!=nullptr)
    {
        MUINT32 frameId = frame->getFrameID();
        MY_LOGD_IF(mLogLevel>0, "+ status(done) requestId(%d)", frameId);
        //
        frame->unlockAllStream(getNodeName());
        // release image buffer/ metadata buffer
        releaseBuffer(frame, STREAM_BUFFER_STATUS::WRITE_OK);
        // release
        sp<IPipelineFrame> const pFrame = frame->mpFrame;
        if(frame->mpFrame == 0)
        {
            MY_LOGE("mpFrame is null");
        }
        IStreamBufferSet& streamBufferSet = pFrame->getStreamBufferSet();
        streamBufferSet.applyRelease(getNodeId());
        //
        frame->uninit();
        delete frame;
        frame = nullptr;
        onDispatchFrame(pFrame);
        {
            RWLock::AutoWLock _l(mRequestQueueLock);
            ssize_t index = mvRequestQueue.indexOfKey(frameId);
            if(index>=0)
            {
                mvRequestQueue.removeItemsAt(index);
            }
        miEffectRequestOutCount++;
        }
    }
    else
    {
        MY_LOGE_IF(frame==nullptr, "frame is NULL.");
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
BokehNodeImp::
releaseBuffer(
    process_frame*& frame,
    MUINT32 mask
)
{
        // release metadata
        if(frame->mpInHalMetadataStreamBuffer!=nullptr)
        {
            releaseMetaStream(
                                frame->mpFrame,
                                frame->mpInHalMetadataStreamBuffer,
                                mask
                        );
        }
        //
        if(frame->mpInAppMetadataStreamBuffer!=nullptr)
        {
            releaseMetaStream(
                                frame->mpFrame,
                                frame->mpInAppMetadataStreamBuffer,
                                mask
                        );
        }
        //
        if(frame->mpOutHalMetadataStreamBuffer!=nullptr)
        {
            releaseMetaStream(
                                frame->mpFrame,
                                frame->mpOutHalMetadataStreamBuffer,
                                mask
                        );
        }
        //
        if(frame->mpOutAppMetadataStreamBuffer!=nullptr)
        {
            releaseMetaStream(
                                frame->mpFrame,
                                frame->mpOutAppMetadataStreamBuffer,
                                mask
                        );
        }
        // release image buffer
        if(frame->mvImgBufContainer.size() !=
           frame->mvImgStreamBuffer.size())
        {
            MY_LOGE("mvImgBufContainer.size()!=mvImgStreamBuffer.size()");
            return;
        }
        MINT32 index = -1;
        sp<IImageBuffer> imgBuffer;
        sp<IImageStreamBuffer> imgStreamBuffer;
        for(ssize_t i=0; i<frame->mvImgBufContainer.size();++i)
        {
            imgBuffer = frame->mvImgBufContainer.editValueAt(i);
            imgStreamBuffer = frame->mvImgStreamBuffer.editValueAt(i);
            if(imgBuffer != nullptr && imgStreamBuffer != nullptr)
            {
                if(frame->mpFrame == 0)
                {
                    MY_LOGE("mpFrame is null");
                }
                //releaseImageBuffer(imgStreamBuffer,
                //                   imgBuffer);
                releaseImageStream(frame->mpFrame,
                                   imgStreamBuffer,
                                   mask);
                MY_LOGD_IF(mLogLevel>0, "Release buf(0x%x)", frame->mvImgStreamBuffer.keyAt(i));
            }
        }
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BokehNodeImp::
isInMetaStream(
    StreamId_T const streamId
) const
{
    RWLock::AutoRLock _l(mConfigRWLock);
    return isStream(mpInHalMetadata, streamId) ||
           isStream(mpInAppMetadata, streamId);
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
BokehNodeImp::
releaseMetaStream(
                  sp<IPipelineFrame> const& pFrame,
                  sp<IMetaStreamBuffer> const pStreamBuffer,
                  MUINT32 mask) const
{
    IStreamBufferSet& rStreamBufferSet = pFrame->getStreamBufferSet();
    StreamId_T const streamId = pStreamBuffer->getStreamInfo()->getStreamId();
    //
    if( pStreamBuffer.get() == NULL ) {
        MY_LOGES("ReqId(%d) StreamId (%#" PRIx64 "): pStreamBuffer == NULL",
                pFrame->getRequestNo(),
                streamId);
        return;
    }
    //
    if( !isInMetaStream(streamId) ) {
        pStreamBuffer->markStatus(mask);
    }
    //
    //  Mark this buffer as USED by this user.
    //  Mark this buffer as RELEASE by this user.
    rStreamBufferSet.markUserStatus(
            streamId, getNodeId(),
            IUsersManager::UserStatus::USED |
            IUsersManager::UserStatus::RELEASE
            );
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
BokehNodeImp::
releaseImageStream(
                  sp<IPipelineFrame> const& pFrame,
                  sp<IImageStreamBuffer> const pStreamBuffer,
                  MUINT32 mask) const
{
    IStreamBufferSet& rStreamBufferSet = pFrame->getStreamBufferSet();
    StreamId_T const streamId = pStreamBuffer->getStreamInfo()->getStreamId();
    //
    if( pStreamBuffer.get() == NULL ) {
        MY_LOGES("ReqId(%d) StreamId (%#" PRIx64 "): pStreamBuffer == NULL",
                pFrame->getRequestNo(),
                streamId);
        return;
    }
    //
    if( !isInMetaStream(streamId) ) {
        pStreamBuffer->markStatus(mask);
    }
    //
    //  Mark this buffer as USED by this user.
    //  Mark this buffer as RELEASE by this user.
    rStreamBufferSet.markUserStatus(
            streamId, getNodeId(),
            IUsersManager::UserStatus::USED |
            IUsersManager::UserStatus::RELEASE
            );
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
BokehNodeImp::
suspendThisFrame(
    sp<IPipelineFrame> const& pFrame,
    process_frame*& data)
{
    MY_LOGE("Discard request id=%d", pFrame->getRequestNo());

    data->unlockAllStream(getNodeName());
    IStreamBufferSet& rStreamBufferSet = pFrame->getStreamBufferSet();

    auto markStreamBufferStatusToError = [&rStreamBufferSet](MUINT32 streamId, MUINT32 nodeId)
    {
        sp<IImageStreamBuffer> pStreamBuffer = rStreamBufferSet.getImageBuffer(streamId, nodeId);
        if(pStreamBuffer!=nullptr)
        {
            pStreamBuffer->markStatus(STREAM_BUFFER_STATUS::ERROR);
        }
    };
    // mark preview or record to ERROR to avoid empty frame(green screen)
    if(mpOutAppImagePreview!=nullptr)
    {
        markStreamBufferStatusToError(mpOutAppImagePreview->getStreamId(), getNodeId());
    }
    if(mpOutAppImageRecord!=nullptr)
    {
        markStreamBufferStatusToError(mpOutAppImageRecord->getStreamId(), getNodeId());
    }
    data->uninit(MFALSE);
    delete data;
    data = nullptr;

    // mark input/output buffer to release
    MERROR err = BaseNode::flush(pFrame);
    return err;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
BokehNodeImp::
AddEffectRequest(process_frame *pFrameData)
{
    if(pFrameData == nullptr)
    {
        MY_LOGE("pFrameData is NULL.");
        return;
    }
    android::sp<NSCam::EffectRequest> request = new NSCam::EffectRequest(pFrameData->getFrameID(), EffectHalCb, this);
    //
    MUINT32 scenarioId = pFrameData->getScenario();
    // set request params.
    sp<EffectParameter> reqst_para = new EffectParameter();
    reqst_para->set(VSDOF_FRAME_SCENARIO, scenarioId);
    reqst_para->set(VSDOF_FRAME_ISO, pFrameData->getISO());
    reqst_para->setPtr(VSDOF_FRAME_USER_DATA, pFrameData);
    reqst_para->set(VSDOF_FRAME_BOKEH_LEVEL, pFrameData->getBokehLevel());
    reqst_para->set(VSDOF_FRAME_AF_TRIGGER_FLAG, pFrameData->getAFTrigger());
    reqst_para->set(VSDOF_FRAME_AF_POINT_X, pFrameData->getAFx());
    reqst_para->set(VSDOF_FRAME_AF_POINT_Y, pFrameData->getAFy());
    reqst_para->set(VSDOF_FEAME_MAGIC_NUMBER, pFrameData->getMagicNumber());
    reqst_para->set(VSDOF_FRAME_SENSOR_PROFILE, pFrameData->getSensorProfile());
    
    if(mBokehMode == VENDOR_BOKEH)
    {
        // only 3rd flow needs af info.
        getAfInfo(pFrameData, reqst_para);
        // distance
        reqst_para->set(VSDOF_FRAME_DISTANCE, pFrameData->getDistance());
    }
    else
    {
        // for tk only
        // to do: if 3dnr is off, does not set value.
        reqst_para->set(VSDOF_FRAME_3DNR_FLAG, pFrameData->is3DNREnable());
        reqst_para->set(VSDOF_FRAME_GMV_X, pFrameData->getGMV_x());
        reqst_para->set(VSDOF_FRAME_GMV_Y, pFrameData->getGMV_y());
        reqst_para->set(VSDOF_FRAME_CMV_X, pFrameData->getCMV_x());
        reqst_para->set(VSDOF_FRAME_CMV_Y, pFrameData->getCMV_y());
        reqst_para->set(VSDOF_FRAME_CONF_X, pFrameData->getConf_x());
        reqst_para->set(VSDOF_FRAME_CONF_Y, pFrameData->getConf_y());
        reqst_para->setFloat(VSDOF_FRAME_CONV_OFFSET, pFrameData->getConvOffset());
    }
    if(scenarioId == eSTEREO_SCENARIO_CAPTURE)
    {
        reqst_para->set(VSDOF_FRAME_G_SENSOR_ORIENTATION, pFrameData->getGSensor());
        MY_LOGD("G sensor(%d)", pFrameData->getGSensor());
    }
    else
    {
        reqst_para->set(VSDOF_FRAME_G_SENSOR_ORIENTATION, 0);
    }
    // add metadata
    //
    request->setRequestParameter(reqst_para);
    //
    sp<EffectFrameInfo>  frame = nullptr;
    sp<EffectParameter>  param = nullptr;
    // set input frame
    // main1 image buffer
#define SET_EFFECT_FRAME(FRAMEID, TYPE)\
    do{\
        MINT32 frameIndex = pFrameData->mvImgBufContainer.indexOfKey(FRAMEID);\
        if(frameIndex>=0)\
        {\
            sp<IImageBuffer> imgBuf = pFrameData->mvImgBufContainer.valueAt(frameIndex);\
            if(imgBuf!=0)\
            {\
                frame = new EffectFrameInfo(pFrameData->getFrameID(), FRAMEID, nullptr, this);\
                frame->setFrameBuffer(pFrameData->mvImgBufContainer.valueAt(frameIndex));\
                request->v##TYPE##FrameInfo.add(FRAMEID, frame);\
                if(mLogLevel>0) MY_LOGD("reqID=%d add "#TYPE" "#FRAMEID, request->getRequestNo());\
            }\
        }\
    }while(0);
    SET_EFFECT_FRAME(BOKEH_ER_BUF_MAIN1, Input);
    SET_EFFECT_FRAME(BOKEH_ER_BUF_DMBG, Input);
    SET_EFFECT_FRAME(BOKEH_ER_BUF_P2_TUNING, Input);
    SET_EFFECT_FRAME(BOKEH_ER_OUTPUT_DEPTHMAP, Input);
    // sw bokeh
    SET_EFFECT_FRAME(BOKEH_ER_BUF_HAL_DEPTHWRAPPER, Input);
    SET_EFFECT_FRAME(BOKEH_ER_BUF_EXTRADATA, Input);
    //
    //SET_EFFECT_FRAME(BOKEH_ER_BUF_MYS, Input);
    SET_EFFECT_FRAME(BOKEH_ER_BUF_DISPLAY, Output);
    SET_EFFECT_FRAME(BOKEH_ER_BUF_CLEAN_IMG, Output);
    SET_EFFECT_FRAME(BOKEH_ER_BUF_RECORD, Output);
    SET_EFFECT_FRAME(BOKEH_ER_BUF_VSDOF_IMG, Output);
    SET_EFFECT_FRAME(BOKEH_ER_BUF_THUMBNAIL, Output);
    SET_EFFECT_FRAME(BOKEH_ER_BUF_DEPTHMAPWRAPPER, Output);
    SET_EFFECT_FRAME(BOKEH_ER_BUF_APP_EXTRA_DATA, Output);
#undef SET_EFFECT_FRAME

#define SET_METADATA_EFFECT_FRAME(BUFFERID, METADATA, TYPE)\
    frame = new EffectFrameInfo(pFrameData->getFrameID(), BUFFERID, nullptr, this);\
    param = new EffectParameter();\
    param->setPtr(BOKEH_META_KEY_STRING, (void*)METADATA);\
    frame->setFrameParameter(param);\
    request->v##TYPE##FrameInfo.add(BUFFERID, frame);\
    if(mLogLevel>0) MY_LOGD("reqID=%d add "#TYPE" metadta "#BUFFERID" ", request->getRequestNo());
    // Add Metadata
    SET_METADATA_EFFECT_FRAME(BOKEH_ER_IN_APP_META, pFrameData->mpInAppMetadata, Input);
    SET_METADATA_EFFECT_FRAME(BOKEH_ER_IN_HAL_META_MAIN1, pFrameData->mpInHalMetadata, Input);
    SET_METADATA_EFFECT_FRAME(BOKEH_ER_OUT_APP_META, pFrameData->mpOutAppMetadata, Output);
    SET_METADATA_EFFECT_FRAME(BOKEH_ER_OUT_HAL_META, pFrameData->mpOutHalMetadata, Output);
    //
    // Dump request debug info
    if(mLogLevel>0)
    {
        MY_LOGD("Dump BokehRequest +");
        MY_LOGD("FrameNo(%d)", pFrameData->getFrameID());
        MY_LOGD("ScenarioId(%d)", scenarioId);
        MY_LOGD("Bokeh level(%d)", pFrameData->getBokehLevel());
        MY_LOGD("3dnr(%d)", pFrameData->is3DNREnable());
        MY_LOGD("gmv_x(%d)", pFrameData->getGMV_x());
        MY_LOGD("gmv_y(%d)", pFrameData->getGMV_y());
        MY_LOGD("cmv_x(%d)", pFrameData->getCMV_x());
        MY_LOGD("cmv_y(%d)", pFrameData->getCMV_y());
        MY_LOGD("conf_x(%d)", pFrameData->getConf_x());
        MY_LOGD("conf_y(%d)", pFrameData->getConf_y());
        MY_LOGD("ISO(%d)", pFrameData->getISO());
        MY_LOGD("userdata(0x%x)", pFrameData);
        MY_LOGD("AF trigger(%d)", pFrameData->getAFTrigger());
        MY_LOGD("AFx(%d)", pFrameData->getAFx());
        MY_LOGD("AFy(%d)", pFrameData->getAFy());
        MY_LOGD("In buffer size(%d)", request->vInputFrameInfo.size());
        MY_LOGD("Out buffer size(%d)", request->vOutputFrameInfo.size());
        MY_LOGD("G sensor(%d)", pFrameData->getGSensor());
        MY_LOGD("Magic number(%d)", pFrameData->getMagicNumber());
        MY_LOGD("Dump BokehRequest -");
    }
    //
    {
        RWLock::AutoWLock _l(mRequestQueueLock);
        mpBokehEffectHal->updateEffectRequest(request);
        mvRequestQueue.add(pFrameData->getFrameID(), request);
        miEffectRequestInCount++;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BokehNodeImp::
queryGMVFromMetaStream(process_frame *pFrameData, IMetadata* const metaStream)
{
    // EIS information needs sync with pass1 hal metadata.
    // For vsdof, EIS just use GMV
    IMetadata::IEntry entry = metaStream->entryFor(MTK_EIS_REGION);
    MINT32 gmv_x = 0, gmv_y = 0, cmv_x = 0, cmv_y = 0, conf_x = 0, conf_y = 0;
    if(entry.count() != 0)
    {
        gmv_x = entry.itemAt(EIS_REGION_INDEX_GMVX, Type2Type<MINT32>());
        gmv_y = entry.itemAt(EIS_REGION_INDEX_GMVY, Type2Type<MINT32>());
        cmv_x = entry.itemAt(EIS_REGION_INDEX_XINT, Type2Type<MINT32>());
        cmv_y = entry.itemAt(EIS_REGION_INDEX_YINT, Type2Type<MINT32>());
        conf_x = entry.itemAt(EIS_REGION_INDEX_CONFX, Type2Type<MINT32>());
        conf_y = entry.itemAt(EIS_REGION_INDEX_CONFY, Type2Type<MINT32>());
    }
    //
    pFrameData->set3DNRData(MTRUE, gmv_x, gmv_y, cmv_x, cmv_y, conf_x, conf_y);
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BokehNodeImp::
queryISOFromMetaStream(process_frame *pFrameData, IMetadata* const metaStream)
{
    // EIS information needs sync with pass1 hal metadata.
    // For vsdof, EIS just use GMV
    IMetadata::IEntry entry = metaStream->entryFor(MTK_VSDOF_P1_MAIN1_ISO);
    MINT32 iso = 0;
    if(entry.count() != 0)
    {
        iso = entry.itemAt(0, Type2Type<MINT32>());
    }
    else
        MY_LOGE("can not get iso");
    //
    pFrameData->setISO(iso);
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
BokehNodeImp::
getAfInfo(process_frame *pProcessFrame, sp<EffectParameter>& params)
{
    if(pProcessFrame->mpInAppMetadata == nullptr)
    {
        MY_LOGE("get app metadata fail");
        return MFALSE;
    }
    IMetadata::IEntry entry = pProcessFrame->mpInAppMetadata->entryFor(MTK_3A_FEATURE_AF_ROI);
    MINT32 af_head1 = 0;
    MINT32 afroinum = 0;
    MINT32 afTopLeftX     = 0;
    MINT32 afTopLeftY     = 0;
    MINT32 afBottomRightX = 0;
    MINT32 afBottomRightY = 0;
    MINT32 aftype = 0;
    af_head1         = entry.itemAt(0, Type2Type<MINT32>());
    afroinum         = entry.itemAt(1, Type2Type<MINT32>());
    afTopLeftX       = entry.itemAt(2, Type2Type<MINT32>());
    afTopLeftY       = entry.itemAt(3, Type2Type<MINT32>());
    afBottomRightX   = entry.itemAt(4, Type2Type<MINT32>());
    afBottomRightY   = entry.itemAt(5, Type2Type<MINT32>());
    aftype           = entry.itemAt(6, Type2Type<MINT32>());
    //
    params->set(VSDOF_FRAME_BOKEH_AF_STATE, aftype);
    params->set(VSDOF_FRAME_BOKEH_AF_ROI_T_L_X, afTopLeftX);
    params->set(VSDOF_FRAME_BOKEH_AF_ROI_T_L_Y, afTopLeftY);
    params->set(VSDOF_FRAME_BOKEH_AF_ROI_B_R_X, afBottomRightX);
    params->set(VSDOF_FRAME_BOKEH_AF_ROI_B_R_Y, afBottomRightY);
    return OK;
}
/******************************************************************************
 * process_frame
 ******************************************************************************/
MBOOL
process_frame::
init()
{
    start = std::chrono::system_clock::now();
    //mvImgBufContainer.add(BOKEH_ER_BUF_MYS, nullptr);
    mvImgBufContainer.add(BOKEH_ER_BUF_DMBG, nullptr);
    mvImgBufContainer.add(BOKEH_ER_BUF_P2_TUNING, nullptr);
    mvImgBufContainer.add(BOKEH_ER_BUF_MAIN1, nullptr);
    mvImgBufContainer.add(BOKEH_ER_BUF_RECORD, nullptr);
    mvImgBufContainer.add(BOKEH_ER_BUF_DISPLAY, nullptr);
    mvImgBufContainer.add(BOKEH_ER_BUF_VSDOF_IMG, nullptr);
    mvImgBufContainer.add(BOKEH_ER_BUF_CLEAN_IMG, nullptr);
    mvImgBufContainer.add(BOKEH_ER_BUF_THUMBNAIL, nullptr);
    mvImgBufContainer.add(BOKEH_ER_BUF_PRVCB, nullptr);
    // sw bokeh
    mvImgBufContainer.add(BOKEH_ER_BUF_HAL_DEPTHWRAPPER, nullptr);
    mvImgBufContainer.add(BOKEH_ER_BUF_EXTRADATA, nullptr);
    mvImgBufContainer.add(BOKEH_ER_BUF_DEPTHMAPWRAPPER, nullptr);
    mvImgBufContainer.add(BOKEH_ER_BUF_APP_EXTRA_DATA, nullptr);
    // vendor flow
    mvImgBufContainer.add(BOKEH_ER_OUTPUT_DEPTHMAP, nullptr);
    //
    //mvImgStreamBuffer.add(BOKEH_ER_BUF_MYS, nullptr);
    mvImgStreamBuffer.add(BOKEH_ER_BUF_DMBG, nullptr);
    mvImgStreamBuffer.add(BOKEH_ER_BUF_P2_TUNING, nullptr);
    mvImgStreamBuffer.add(BOKEH_ER_BUF_MAIN1, nullptr);
    mvImgStreamBuffer.add(BOKEH_ER_BUF_RECORD, nullptr);
    mvImgStreamBuffer.add(BOKEH_ER_BUF_DISPLAY, nullptr);
    mvImgStreamBuffer.add(BOKEH_ER_BUF_VSDOF_IMG, nullptr);
    mvImgStreamBuffer.add(BOKEH_ER_BUF_CLEAN_IMG, nullptr);
    mvImgStreamBuffer.add(BOKEH_ER_BUF_THUMBNAIL, nullptr);
    mvImgStreamBuffer.add(BOKEH_ER_BUF_PRVCB, nullptr);
    // sw bokeh
    mvImgStreamBuffer.add(BOKEH_ER_BUF_HAL_DEPTHWRAPPER, nullptr);
    mvImgStreamBuffer.add(BOKEH_ER_BUF_EXTRADATA, nullptr);
    mvImgStreamBuffer.add(BOKEH_ER_BUF_DEPTHMAPWRAPPER, nullptr);
    mvImgStreamBuffer.add(BOKEH_ER_BUF_APP_EXTRA_DATA, nullptr);
    // vendor flow
    mvImgStreamBuffer.add(BOKEH_ER_OUTPUT_DEPTHMAP, nullptr);
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
process_frame::
uninit(MBOOL vaild)
{
    MINT32 reqID = mpFrame->getRequestNo();
    // relase stream
    mpFrame = nullptr;
    mpInAppMetadata = nullptr;
    mpInAppMetadataStreamBuffer = nullptr;
    mpInHalMetadata = nullptr;
    mpInHalMetadataStreamBuffer = nullptr;
    mpOutAppMetadata = nullptr;
    mpOutAppMetadataStreamBuffer = nullptr;
    mpOutHalMetadata = nullptr;
    mpOutHalMetadataStreamBuffer = nullptr;
    mvImgBufContainer.clear();
    mvImgStreamBuffer.clear();
    if(vaild == MTRUE && mLogLevel > 0)
    {
        end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end-start;
        MY_LOGE("VSDOF_Profile: processing time(%lf ms) reqID=%d BokehLevel(%d)",
                                    elapsed_seconds.count() *1000,
                                    reqID,
                                    getBokehLevel());
    }
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
process_frame::
unlockAllStream(const char* nodeName)
{
    // unlock metadata
    {
        if(mpInHalMetadataStreamBuffer!=nullptr)
        {
            mpInHalMetadataStreamBuffer->unlock(nodeName, mpInHalMetadata);
        }
        //
        if(mpInAppMetadataStreamBuffer!=nullptr)
        {
            mpInAppMetadataStreamBuffer->unlock(nodeName, mpInAppMetadata);
        }
        //
        if(mpOutHalMetadataStreamBuffer!=nullptr)
        {
            mpOutHalMetadataStreamBuffer->unlock(nodeName, mpOutHalMetadata);
        }
        //
        if(mpOutAppMetadataStreamBuffer!=nullptr)
        {
            mpOutAppMetadataStreamBuffer->unlock(nodeName, mpOutAppMetadata);
        }
    }
    // unlock image stream
    {
        if(mvImgBufContainer.size() !=
           mvImgStreamBuffer.size())
        {
            MY_LOGE("mvImgBufContainer.size()!=mvImgStreamBuffer.size()");
            return;
        }
        MINT32 index = -1;
        sp<IImageBuffer> imgBuffer;
        sp<IImageStreamBuffer> imgStreamBuffer;
        for(ssize_t i=0; i<mvImgBufContainer.size();++i)
        {
            imgBuffer = mvImgBufContainer.editValueAt(i);
            imgStreamBuffer = mvImgStreamBuffer.editValueAt(i);
            if(imgBuffer != nullptr && imgStreamBuffer != nullptr)
            {
                imgBuffer->unlockBuf(nodeName);
                imgStreamBuffer->unlock(nodeName, imgBuffer->getImageBufferHeap());
            }
        }
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
process_frame::
setGSensor(MINT32 rot)
{
    miGSensorAngle = rot;
}
