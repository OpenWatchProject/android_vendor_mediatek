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
#define LOG_TAG "MtkCam/ReqSetBud"
//
#include <utils/Mutex.h>
#include <utils/Condition.h>
#include <cutils/properties.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/IMetadataConverter.h>

using namespace android;
#include <mtkcam/middleware/v1/IParamsManager.h>
#include <mtkcam/middleware/v1/LegacyPipeline/request/RequestSettingBuilder.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>

using namespace NSCam;
using namespace std;
using namespace NSCamHW;

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

#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
/******************************************************************************
 *
 ******************************************************************************/
#define FUNCTION_IN             MY_LOGD_IF(1<=mLogLevel, "+");
#define FUNCTION_OUT            MY_LOGD_IF(1<=mLogLevel, "-");
#define PUBLIC_API_IN           MY_LOGD_IF(1<=mLogLevel, "API +");
#define PUBLIC_API_OUT          MY_LOGD_IF(1<=mLogLevel, "API -");
#define MY_LOGD1(...)           MY_LOGD_IF(1<=mLogLevel, __VA_ARGS__)
#define MY_LOGD2(...)           MY_LOGD_IF(2<=mLogLevel, __VA_ARGS__)
/******************************************************************************
 *
 ******************************************************************************/
class RequestSettingBuilderImp
    : public RequestSettingBuilder
{
public:

                                    RequestSettingBuilderImp(MINT32 cameraId, sp<IParamsManagerV3> pParamsMgrV3);

    virtual                         ~RequestSettingBuilderImp(){};

public:     ////                    Operations.

    // for flow control
    MERROR                          createDefaultRequest(int templateId, IMetadata &request) const;
    // AE
    MERROR                          triggerPrecaptureMetering(wp<IRequestCallback> pCB, MBOOL needAf);
    // AF
    MERROR                          triggerAutofocus(wp<IRequestCallback> pCB);

    MERROR                          triggerCancelAutofocus();

    MERROR                          triggerTriggerZoom(MUINT32 const& index, wp<IRequestCallback> pCB);

    MERROR                          triggerCancelZoom();

    // repeat request
    MERROR                          setStreamingRequest(IMetadata const& request/*, MUINT32 const& requestId*/);

    // for RequestProvider
    // requestId[in]
    // nextRequest[out]
    MERROR                          getRequest(MUINT32 const& requestId, IMetadata &nextAppRequest, IMetadata &nextHalRequest);

protected:
    MINT32                          mLogLevel;

private:
    MERROR                          insertTriggers(IMetadata &request);

    ITemplateRequest*               getTemplateMetaRequest(MINT32 iOpenId);

    MINT32                          mOpenId;
    sp<IParamsManagerV3>            mpParamsMgrV3;
    //MUINT32                         mFrameNumber;

    Mutex                           mInterfaceLock;
    Mutex                           mRequestLock;
    Mutex                           mRepeatingRequestLock;
    Mutex                           mInFlightRequestLock;

    //MUINT32                         mTriggerPreId;
    //MUINT32                         mTriggerAfId;
    MUINT32                         mTriggerZoomIndex;

    struct                          RequestTrigger {
                                        // Metadata tag number, e.g. MTK_CONTROL_AE_PRECAPTURE_TRIGGER
                                        MUINT32 metadataTag;
                                        // Metadata value, e.g. 'MTK_CONTROL_AE_PRECAPTURE_TRIGGER_START'
                                        MUINT8 entryValue;
                                    };

    typedef KeyedVector<MUINT32/*tag*/, MUINT8/*entryValue*/> TriggerMap;
    TriggerMap                      mTriggerMap;

    List<IMetadata>                 mRequestQueue;
    List<IMetadata>                 mRepeatingRequests;

    struct                          CallbackParam {

                                        sp<IRequestCallback> RequestCb;

                                        MINT32 frameNo;

                                        MINT32 type;

                                        MUINT32 _ext1;

                                        MUINT32 _ext2;
                                    };

    typedef CallbackParam           Callback_t;
    List<Callback_t>                mCbQueue;
    typedef KeyedVector<MUINT32/*index*/, MRect/*cropRegion*/> CropRegionMap;

    MRect                           mActiveArray;
    Condition                       mRequestQueueCond;
    Condition                       mRepeatingRequestQueueCond;

    sp<IMetadataConverter>          mpMetadataConverter;

    MUINT32                         mBurstNum;
    MUINT32                         mRequestCnt;
    MRect                           mReqCropRegion;
    MRect                           mReqSensorCropRegion;
    MRect                           mReqSensorPreviewCropRegion;
    MSize                           mActiveMargin;
    MBOOL                           mbRecordingHint;
    //
    MBOOL                           mbOldRepeatingRequest;
};

/******************************************************************************
 *
 ******************************************************************************/
sp<RequestSettingBuilder>
RequestSettingBuilder::
createInstance(MINT32 cameraId, sp<IParamsManagerV3> pParamsMgrV3)
{
    return new RequestSettingBuilderImp(cameraId, pParamsMgrV3);
}


/******************************************************************************
 *
 ******************************************************************************/
RequestSettingBuilderImp::
RequestSettingBuilderImp(MINT32 cameraId, sp<IParamsManagerV3> pParamsMgrV3)
    : mOpenId(cameraId)
    , mpParamsMgrV3(pParamsMgrV3)
    , mBurstNum(1)
    , mRequestCnt(0)
    , mReqCropRegion(MRect(MPoint(0,0),MSize(0,0)))
    , mReqSensorCropRegion(MRect(MPoint(0,0),MSize(0,0)))
    , mReqSensorPreviewCropRegion(MRect(MPoint(0,0),MSize(0,0)))
    , mActiveMargin(MSize(0, 0))
{
    // keep this sp
    //mInFlightRequest =  InFlightRequest::createInstance();
    mLogLevel = ::property_get_int32("debug.camera.log", 0);
    if ( mLogLevel == 0 ) {
        mLogLevel = ::property_get_int32("debug.camera.log.RequestBuilder", 0);
    }

    sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(cameraId);
    if( ! pMetadataProvider.get() ) {
        MY_LOGE("%d pMetadataProvider.get() is NULL", cameraId);
        pMetadataProvider = IMetadataProvider::create(cameraId);
        NSMetadataProviderManager::add(cameraId, pMetadataProvider.get());
    }
    IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();
    IMetadata::IEntry activeA = static_meta.entryFor(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION);
    if( !activeA.isEmpty() ) {
        mActiveArray = activeA.itemAt(0, Type2Type<MRect>());
    }

    mpMetadataConverter = IMetadataConverter::createInstance(IDefaultMetadataTagSet::singleton()->getTagSet());

    #define BURST_NUM_BASE  30
    if(mpParamsMgrV3->getPreviewMaxFps() > BURST_NUM_BASE)
    {
        mBurstNum = mpParamsMgrV3->getPreviewMaxFps()/BURST_NUM_BASE;
    }
    MY_LOGD("mBurstNum %d",mBurstNum);

    mbRecordingHint = mpParamsMgrV3->getParamsMgr()->getRecordingHint();
    MY_LOGD("mbRecordingHint(%d)", mbRecordingHint);
}


/******************************************************************************
 *
 ******************************************************************************/
/*virtual RequestSettingBuilderImp::
~RequestSettingBuilderImp()
{
}
*/

/******************************************************************************
 *
 ******************************************************************************/
#if 0
android::sp<InFlightRequest>
RequestSettingBuilder::getInFlightRequest()
{
    return mInFlightRequest;
}
#endif

/*******************************************************************************
*
********************************************************************************/
ITemplateRequest*
RequestSettingBuilderImp::
getTemplateMetaRequest(MINT32 iOpenId)
{
    ITemplateRequest* obj = NSTemplateRequestManager::valueFor(iOpenId);
    if(obj == NULL) {
        obj = ITemplateRequest::getInstance(iOpenId);
        NSTemplateRequestManager::add(iOpenId, obj);
    }
    return obj;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestSettingBuilderImp::
createDefaultRequest(
        int templateId,
        IMetadata &request /*out*/
)const
{
    //ITemplateRequest* obj = getTemplateMetaRequest(0/*openId*/);
    //request = obj->getMtkData(templateId);

    ITemplateRequest* obj = NSTemplateRequestManager::valueFor(mOpenId);
    if(obj == NULL) {
        obj = ITemplateRequest::getInstance(mOpenId);
        NSTemplateRequestManager::add(mOpenId, obj);
    }
    request = obj->getMtkData(templateId);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/

MERROR
RequestSettingBuilderImp::
triggerPrecaptureMetering(wp<IRequestCallback> pCB, MBOOL needAf)
{
    Mutex::Autolock il(mInterfaceLock);
    sp<IRequestCallback> cb = pCB.promote();
    if (cb != NULL) {
        Callback_t callback;
        callback.RequestCb = cb;
        if( needAf ) {
            callback.type = IRequestCallback::MSG_START_PRECAPTURE_WITH_AF;
        } else {
            callback.type = IRequestCallback::MSG_START_PRECAPTURE;
        }
        mCbQueue.push_back (callback);
    }
    //mTriggerPreId = requestId;
    //MUINT8
    RequestTrigger trigger =
    {
        MTK_CONTROL_AE_PRECAPTURE_TRIGGER,/*metadataTag*/
        MTK_CONTROL_AE_PRECAPTURE_TRIGGER_START/*entryValue*/
    };

    MUINT32 metadataTag = trigger.metadataTag;
    ssize_t index = mTriggerMap.indexOfKey(metadataTag);

    if (index != NAME_NOT_FOUND) {
        mTriggerMap.editValueAt(index) = trigger.entryValue;
    } else {
        mTriggerMap.add(metadataTag, trigger.entryValue);
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestSettingBuilderImp::
triggerAutofocus(wp<IRequestCallback> pCB)
{
    FUNC_START;
    Mutex::Autolock il(mInterfaceLock);

    sp<IRequestCallback> cb = pCB.promote();
    if (cb != NULL) {
        Callback_t callback;
        callback.RequestCb = cb;
        callback.type = IRequestCallback::MSG_START_AUTOFOCUS;
        mCbQueue.push_back (callback);
    }
    //mTriggerAfId = id;
    //MUINT8
    RequestTrigger trigger =
    {
        MTK_CONTROL_AF_TRIGGER,
        MTK_CONTROL_AF_TRIGGER_START
    };

    MUINT32 metadataTag = trigger.metadataTag;
    ssize_t index = mTriggerMap.indexOfKey(metadataTag);

    if (index != NAME_NOT_FOUND) {
        mTriggerMap.editValueAt(index) = trigger.entryValue;
    } else {
        mTriggerMap.add(metadataTag, trigger.entryValue);
    }

    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestSettingBuilderImp::
triggerCancelAutofocus()
{
    Mutex::Autolock il(mInterfaceLock);

    // when trigger cancel zoom, mCbQueue contain tag MSG_START_AUTOFOCUS will be removed from queue
    List<Callback_t>::iterator item = mCbQueue.begin();
    while ( item != mCbQueue.end() ) {
        switch (item->type) {
            case IRequestCallback::MSG_START_AUTOFOCUS :
                {
                    item = mCbQueue.erase(item);
                    break;
                }
            default :
                item++;
                break;
        }
    }
    Callback_t callback;
    callback.type = IRequestCallback::MSG_CANCEL_AUTOFOCUS;
    mCbQueue.push_back (callback);

    //mTriggerAfId = id;
    //MUINT8
    RequestTrigger trigger =
    {
        MTK_CONTROL_AF_TRIGGER,
        MTK_CONTROL_AF_TRIGGER_CANCEL
    };

    MUINT32 metadataTag = trigger.metadataTag;
    ssize_t index = mTriggerMap.indexOfKey(metadataTag);

    if (index != NAME_NOT_FOUND) {
        mTriggerMap.editValueAt(index) = trigger.entryValue;
    } else {
        mTriggerMap.add(metadataTag, trigger.entryValue);
    }

    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestSettingBuilderImp::
triggerTriggerZoom(MUINT32 const& index, wp<IRequestCallback> pCB)
{
    Mutex::Autolock il(mInterfaceLock);
    //mTriggerZoomIndex = index;

    sp<IRequestCallback> cb = pCB.promote();
    if (cb != NULL) {
        Callback_t callback;
        callback.RequestCb = cb;
        callback.type = IRequestCallback::MSG_START_ZOOM;
        callback._ext1 = index;
        mCbQueue.push_back (callback);
    }
    MY_LOGD1("triggerTriggerZoom index = %d", index);

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestSettingBuilderImp::
triggerCancelZoom()
{
    Mutex::Autolock il(mInterfaceLock);

    // when trigger cancel zoom, mCbQueue contain tag MSG_START_ZOOM will be removed from queue
    List<Callback_t>::iterator item = mCbQueue.begin();
    while ( item != mCbQueue.end() ) {
        switch (item->type) {
            case IRequestCallback::MSG_START_ZOOM :
                {
                    item = mCbQueue.erase(item);
                    break;
                }
            default :
                item++;
                break;
        }
    }

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestSettingBuilderImp::
setStreamingRequest(IMetadata const& request/*, MUINT32 const& requestId*/)
{
    FUNC_START;
    Mutex::Autolock l(mRequestLock);

    mbOldRepeatingRequest = false;
    IMetadata requestL = request;

    List<IMetadata> requests;
    requests.push_back(requestL);


    mRepeatingRequests.clear();
    const List<IMetadata> cRequests = requests;
    mRepeatingRequests.insert(mRepeatingRequests.begin(),
            cRequests.begin(), cRequests.end());

    FUNC_END;

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
#if 0
MUINT32
RequestSettingBuilder::
getCaptureRequest(IMetadata &nextRequest)
{
    MERROR err = OK;
    Mutex::Autolock l(mRequestLock);

    while(mRequestQueue.empty())
    {
        status_t status = mRequestQueueCond.wait(mRequestLock);
        if  ( OK != status ) {
            MY_LOGW(
                "wait status:%d:%s, mRequestQueue.size:%zu",
                status, ::strerror(-status), mRequestQueue.size()
            );
        }
    }
    List< IMetadata >::iterator firstRequest = mRequestQueue.begin();
    nextRequest = *firstRequest;
    mRequestQueue.erase(firstRequest);

    // insert trigger from triggerMap
    MY_LOGD_IF(0,"insert trigger from triggerMap");
    err  = insertTriggers(nextRequest);

    // register in flight
    /*{
        //Mutex::Autolock l(mInFlightRequestLock);
        mFrameNumber++;
        InFlightRequest::FlightRequest flightRequest;
        flightRequest.precaptureTriggerId = mTriggerPreId;
        flightRequest.afTriggerId = mTriggerAfId;

        mInFlightRequest->enque(mFrameNumber, flightRequest);
    }*/
    return mFrameNumber;
}
#endif

/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestSettingBuilderImp::
getRequest(MUINT32 const& requestId, IMetadata &nextAppRequest, IMetadata &nextHalRequest)
{
    CAM_TRACE_NAME("RSB:getRequest");
    MERROR err = OK;
    //IMetadata* nextRequest;
    //get a request and remove it

    Mutex::Autolock l(mRequestLock);

    MBOOL isRepeating = true;
    // TODO:
    while (mRequestQueue.empty()) {
        if (!mRepeatingRequests.empty()) {
            const List< IMetadata > &requests = mRepeatingRequests;
            List< IMetadata >::const_iterator firstRequest = requests.begin();
            mRequestQueue.insert(mRequestQueue.end(),
                    firstRequest,
                    requests.end());

        }
        break;
    }

    List< IMetadata >::iterator firstRequest = mRequestQueue.begin();
    nextAppRequest = *firstRequest;
    mRequestQueue.erase(firstRequest);

    {
        //MY_LOGD("mRequestCnt %d/%d",mRequestCnt,mBurstNum);
        MRect tempReqCropRegion;
        MRect reqPreviewCropRegion;
        MSize sensorSize;

        mpParamsMgrV3->getCropRegion(sensorSize,
                                        tempReqCropRegion,
                                        mReqSensorCropRegion,
                                        reqPreviewCropRegion,
                                        mReqSensorPreviewCropRegion,
                                        mActiveMargin);
        if(mRequestCnt == 0)
        {
            if ( tempReqCropRegion != mReqCropRegion ) {
                mReqCropRegion = tempReqCropRegion;
                isRepeating = false;
            }
        }
        {
            IMetadata::IEntry entry1(MTK_SCALER_CROP_REGION);
            entry1.push_back(reqPreviewCropRegion, Type2Type<MRect>());
            nextAppRequest.update(MTK_SCALER_CROP_REGION, entry1);
        }
        {
            IMetadata::IEntry entry1(MTK_P1NODE_SENSOR_CROP_REGION);
            entry1.push_back(mReqSensorPreviewCropRegion, Type2Type<MRect>());
            nextHalRequest.update(MTK_P1NODE_SENSOR_CROP_REGION, entry1);
        }
        {
            IMetadata::IEntry entry1(MTK_DUALZOOM_FOV_MARGIN_PIXEL);
            entry1.push_back(mActiveMargin, Type2Type<MSize>());
            nextHalRequest.update(MTK_DUALZOOM_FOV_MARGIN_PIXEL, entry1);
        }
        //
        mRequestCnt++;
        if(mRequestCnt == mBurstNum)
        {
            mRequestCnt = 0;
        }
    }

    /*Mutex::Autolock l(mRepeatingRequestLock);

    while(mRepeatingRequests.empty())
    {
        status_t status = mRepeatingRequestQueueCond.wait(mRepeatingRequestLock);
        if  ( OK != status ) {
            MY_LOGW(
                "wait status:%d:%s, mRepeatingRequests.size:%zu",
                status, ::strerror(-status), mRepeatingRequests.size()
            );
        }
    }
    List< IMetadata >::iterator firstRequest = mRepeatingRequests.begin();
    nextRequest = *firstRequest;
    mRepeatingRequests.erase(firstRequest);
*/
    // insert trigger from triggerMap
    //err = insertTriggers(nextRequest);

    //mFrameNumber++;
    {
    Mutex::Autolock l(mInterfaceLock);
    bool hasZoom = false;// zoom once at get a request
    //bool hasCancelZoom = false;
    // mCbQueue
    //for (List<Callback_t> ::iterator i = mCbQueue.begin(); i != mCbQueue.end(); i++)

    List<Callback_t>::iterator item = mCbQueue.begin();
    // if MSG_CANCEL_ZOOM, erase queue after
    while ( item != mCbQueue.end() ) {
        switch (item->type) {
            case IRequestCallback::MSG_START_ZOOM :
                {
                    //if (hasCancelZoom) break;
                    if (hasZoom)
                    {
                        item++;
                        break;
                    }
                    //
                    /*mpParamsMgrV3->getParamsMgr()->set(
                        CameraParameters::KEY_ZOOM,
                        item->_ext1
                    );*/
                    mpParamsMgrV3->calculateCropRegion();
                    //
                    MRect reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion;
                    MSize sensorSize;
                    mpParamsMgrV3->getCropRegion(sensorSize, reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion);
                    //
                    {
                        IMetadata::IEntry entry(MTK_SCALER_CROP_REGION);
                        entry.push_back(reqPreviewCropRegion, Type2Type<MRect>());
                        nextAppRequest.update(MTK_SCALER_CROP_REGION, entry);
                        List<IMetadata>::iterator iter = mRepeatingRequests.begin();
                        while( iter != mRepeatingRequests.end() ) {
                            iter->update(MTK_SCALER_CROP_REGION, entry);
                            iter++;
                        }
                    }
                    //
                    {
                        IMetadata::IEntry entry(MTK_3A_PRV_CROP_REGION);
                        entry.push_back(reqPreviewCropRegion, Type2Type<MRect>());
                        nextHalRequest.update(MTK_3A_PRV_CROP_REGION, entry);
                        List<IMetadata>::iterator iter = mRepeatingRequests.begin();
                        while( iter != mRepeatingRequests.end() ) {
                            iter->update(MTK_3A_PRV_CROP_REGION, entry);
                            iter++;
                        }
                    }
                    //
                    isRepeating = false;
                    hasZoom = true;
                    item->RequestCb->RequestCallback(requestId, item->type, item->_ext1);
                    item = mCbQueue.erase(item);
                    break;
                }
            /*case IRequestCallback::MSG_CANCEL_ZOOM :
                {
                    hasCancelZoom = true;
                }*/
            case IRequestCallback::MSG_START_AUTOFOCUS :
                {
                    {
                        IMetadata::IEntry entry(MTK_CONTROL_AF_TRIGGER);
                        entry.push_back(MTK_CONTROL_AF_TRIGGER_START, Type2Type< MUINT8 >());
                        nextAppRequest.update(entry.tag(), entry);
                        mpParamsMgrV3->setAfTriggered(MTRUE);
                        mpParamsMgrV3->setCancelAF(MFALSE);
                    }
                    // 3A region
                    {
                        MRect reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion;
                        MSize sensorSize;
                        mpParamsMgrV3->getCropRegion(sensorSize, reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion);
                        // af region
                        {
                            int max = mpParamsMgrV3->getParamsMgr()->getInt(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS);
                            String8 const s = mpParamsMgrV3->getParamsMgr()->getStr(CameraParameters::KEY_FOCUS_AREAS);
                            List<camera_area_t> areas;
                            if ( OK == mpParamsMgrV3->getParamsMgr()->parseCamAreas( s.string(), areas, max ) ) {
                                IMetadata::IEntry entry(MTK_CONTROL_AF_REGIONS);
                                List<camera_area_t>::iterator it = areas.begin();
                                while ( it != areas.end() ) {
                                    int xmin = ((*it).left   + 1000)*reqPreviewCropRegion.s.w/2000 + reqPreviewCropRegion.p.x;
                                    int ymin = ((*it).top    + 1000)*reqPreviewCropRegion.s.h/2000 + reqPreviewCropRegion.p.y;
                                    int xmax = ((*it).right  + 1000)*reqPreviewCropRegion.s.w/2000 + reqPreviewCropRegion.p.x;
                                    int ymax = ((*it).bottom + 1000)*reqPreviewCropRegion.s.h/2000 + reqPreviewCropRegion.p.y;
                                    //
                                    entry.push_back( xmin, Type2Type< MINT32 >());
                                    entry.push_back( ymin, Type2Type< MINT32 >());
                                    entry.push_back( xmax, Type2Type< MINT32 >());
                                    entry.push_back( ymax, Type2Type< MINT32 >());
                                    entry.push_back( (*it).weight, Type2Type< MINT32 >());
                                    //
                                    it++;
                                }
                                nextAppRequest.update(entry.tag(), entry);
                            }
                        }
                        // ae / awb region
                        {
                            int max = mpParamsMgrV3->getParamsMgr()->getInt(CameraParameters::KEY_MAX_NUM_METERING_AREAS);
                            String8 const s = mpParamsMgrV3->getParamsMgr()->getStr(CameraParameters::KEY_METERING_AREAS);
                            List<camera_area_t> areas;
                            if ( OK == mpParamsMgrV3->getParamsMgr()->parseCamAreas( s.string(), areas, max ) ) {
                                IMetadata::IEntry entry(MTK_CONTROL_AE_REGIONS);
                                IMetadata::IEntry entry1(MTK_CONTROL_AWB_REGIONS);
                                //
                                List<camera_area_t>::iterator it = areas.begin();
                                while ( it != areas.end() ) {
                                    int xmin = ((*it).left   + 1000)*reqPreviewCropRegion.s.w/2000 + reqPreviewCropRegion.p.x;
                                    int ymin = ((*it).top    + 1000)*reqPreviewCropRegion.s.h/2000 + reqPreviewCropRegion.p.y;
                                    int xmax = ((*it).right  + 1000)*reqPreviewCropRegion.s.w/2000 + reqPreviewCropRegion.p.x;
                                    int ymax = ((*it).bottom + 1000)*reqPreviewCropRegion.s.h/2000 + reqPreviewCropRegion.p.y;
                                    //
                                    entry.push_back( xmin, Type2Type< MINT32 >());
                                    entry.push_back( ymin, Type2Type< MINT32 >());
                                    entry.push_back( xmax, Type2Type< MINT32 >());
                                    entry.push_back( ymax, Type2Type< MINT32 >());
                                    entry.push_back( (*it).weight, Type2Type< MINT32 >());
                                    //
                                    entry1.push_back( xmin, Type2Type< MINT32 >());
                                    entry1.push_back( ymin, Type2Type< MINT32 >());
                                    entry1.push_back( xmax, Type2Type< MINT32 >());
                                    entry1.push_back( ymax, Type2Type< MINT32 >());
                                    entry1.push_back( (*it).weight, Type2Type< MINT32 >());
                                    //
                                    it++;
                                }
                                nextAppRequest.update(entry.tag(), entry);
                                nextAppRequest.update(entry1.tag(), entry1);
                            }
                        }
                    }
                    isRepeating = false;
                    item->RequestCb->RequestCallback(requestId, item->type);
                    item = mCbQueue.erase(item);
                    MY_LOGD("Do auto focus: %d", requestId);
                    break;
                }
            case IRequestCallback::MSG_CANCEL_AUTOFOCUS :
                {
                    IMetadata::IEntry entry(MTK_CONTROL_AF_TRIGGER);
                    entry.push_back(MTK_CONTROL_AF_TRIGGER_CANCEL, Type2Type< MUINT8 >());
                    nextAppRequest.update(entry.tag(), entry);
                    mpParamsMgrV3->setAfTriggered(MFALSE);
                    item = mCbQueue.erase(item);
                    isRepeating = false;
                    MY_LOGD("Cancel auto focus: %d", requestId);
                    break;
                }
            case IRequestCallback::MSG_START_PRECAPTURE :
                {
                    // trigger AE only
                    MY_LOGD("No AF trigger precapture.");
                    IMetadata::IEntry entry(MTK_CONTROL_AE_PRECAPTURE_TRIGGER);
                    entry.push_back(MTK_CONTROL_AE_PRECAPTURE_TRIGGER_START, Type2Type< MUINT8 >());
                    nextAppRequest.update(entry.tag(), entry);
                    //
                    isRepeating = false;
                    item->RequestCb->RequestCallback(requestId, item->type);
                    item = mCbQueue.erase(item);
                    break;
                }
            case IRequestCallback::MSG_START_PRECAPTURE_WITH_AF :
                {
                    // trigger AE & AF at the same time
                    MY_LOGD("Has AF trigger precapture.");
                    IMetadata::IEntry entry(MTK_CONTROL_AE_PRECAPTURE_TRIGGER);
                    IMetadata::IEntry entryAF(MTK_CONTROL_AF_TRIGGER);
                    entry.push_back(MTK_CONTROL_AE_PRECAPTURE_TRIGGER_START, Type2Type< MUINT8 >());
                    entryAF.push_back(MTK_CONTROL_AF_TRIGGER_START, Type2Type< MUINT8 >());
                    nextAppRequest.update(entry.tag(), entry);
                    nextAppRequest.update(entryAF.tag(), entryAF);
                    //
                    isRepeating = false;
                    item->RequestCb->RequestCallback(requestId, item->type);
                    item = mCbQueue.erase(item);
                    break;
                }
            default :
                {
                    MY_LOGE("no available type : %d", item->type);
                    item++;
                    break;
                }
        }
    }
    }//end mInterfaceLock

    MY_LOGD_IF( mLogLevel >= 2, "repeat result %d", isRepeating & mbOldRepeatingRequest);
    IMetadata::IEntry entry(MTK_HAL_REQUEST_REPEAT);
    entry.push_back(isRepeating & mbOldRepeatingRequest, Type2Type< MUINT8 >());
    nextHalRequest.update(entry.tag(), entry);
    //
    mbOldRepeatingRequest = isRepeating;

    if ( mpMetadataConverter != 0 )
    {
         if ( mLogLevel >= 2 ) mpMetadataConverter->dumpAll(nextAppRequest);
         else if ( mLogLevel >= 1 )mpMetadataConverter->dump(nextAppRequest);
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
RequestSettingBuilderImp::
insertTriggers(IMetadata &request)
{
    Mutex::Autolock il(mInterfaceLock);

    size_t count = mTriggerMap.size();

    for (size_t i = 0; i < count; ++i) {
        MUINT32 tag          = mTriggerMap.keyAt(i);
        MUINT8 entryValue    = mTriggerMap.valueAt(i);

        IMetadata::IEntry entry(tag);
        entry.push_back(entryValue, Type2Type< MUINT8 >());
        request.update(entry.tag(), entry);
        MY_LOGD1("insert trigger tag %d, value %d\n", tag, entryValue);
    }

    mTriggerMap.clear();

    return OK;
}
