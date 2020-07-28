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

#define LOG_TAG "MtkCam/ResourceContainer"
//
#include "MyUtils.h"
//
#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>

using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace android;


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

#if 1
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif

static Mutex gImgProcLock;
sp<ImagePostProcessor> IResourceContainer::mpImagePostProcessor = nullptr;
/******************************************************************************
 *
 ******************************************************************************/
class ResourceContainer : public IResourceContainer
{

public:
                                       ResourceContainer();

    virtual                            ~ResourceContainer() {MY_LOGD("dtor openid(%d)", miOpenId);}

    virtual MVOID                      setFeatureFlowControl( sp<IFeatureFlowControl> pControl );

    virtual MVOID                      setConsumer( StreamId_T id, sp<StreamBufferProvider> pProvider );

    virtual sp<StreamBufferProvider>   queryConsumer( StreamId_T id );

    virtual sp<IFeatureFlowControl>    queryFeatureFlowControl();

    virtual MVOID                      setLatestFrameInfo( sp<IFrameInfo> frameInfo );

    virtual sp<IFrameInfo>             queryLatestFrameInfo();

    virtual MVOID                      setPipelineResource( uint32_t aShotMode, sp<IPipelineResource> pipelineResource );

    virtual sp<IPipelineResource>      queryPipelineResource( uint32_t aShotMode );

    virtual MVOID                      clearPipelineResource();

    virtual MVOID                      clearBufferProviders();

    virtual MVOID                      setResourceConcurrency( ResourceConcurrencyType aType, android::sp<IResourceConcurrency> pConcurrency );

    virtual sp<IResourceConcurrency>   queryResourceConcurrency( ResourceConcurrencyType aType );

    virtual MVOID                      clearResourceConcurrency( ResourceConcurrencyType aType );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Definitions.


protected:  ////                    Data Members.
    int                                            miOpenId;
    KeyedVector<MINT32, sp<StreamBufferProvider> > mvStreamBufferProviders;   //(StreamId_T, sp<StreamBufferProvider>)
    wp<IFeatureFlowControl>                        mFeatureFlowControl;
    mutable Mutex                                  mFeatureFlowControlLock; // lock for mFeatureFlowControl, avoid data race condition
    sp<IFrameInfo>                                 mFrameInfo;
    //sp<IPipelineResource>                          mPipelineResource;
    KeyedVector<uint32_t, sp<IPipelineResource> > mvPipelineResource;   //(shotmode, sp<IPipelineResource>)
    mutable Mutex                                  mResourceSetLock;
    mutable Mutex                                  mPipelineResourceLock;
    mutable Mutex                                  mPostProcessorLock;
    KeyedVector<ResourceConcurrencyType, sp<IResourceConcurrency>> mvConcurrency;   //(ResourceConcurrencyType, sp<IResourceConcurrency>)

protected:  ////                    Operations.


public:
    virtual status_t                            onCreate(int iOpenId);


};

/******************************************************************************
 *
 ******************************************************************************/
sp<IResourceContainer>
IResourceContainer::
getInstance(int iOpenId)
{
    FUNC_START;
    static android::Mutex lock;
    Mutex::Autolock _l(lock);

    static KeyedVector<MINT32, wp<ResourceContainer> > gvResMgr;
    sp<ResourceContainer> pContainer = NULL;
    if ( gvResMgr.size() == 0 || gvResMgr.indexOfKey(iOpenId) < 0 ) {
        MY_LOGD("No Container, OpenID(%d) New ResourceContainer", iOpenId);
        pContainer = new ResourceContainer();
        pContainer->onCreate(iOpenId);
        gvResMgr.add(iOpenId,pContainer);
    }
    else
    {
        pContainer = gvResMgr.editValueFor(iOpenId).promote();
        if(pContainer == 0)
        {
            MY_LOGD("Container turn NULL, OpenID(%d) New ResourceContainer", iOpenId);
            pContainer = new ResourceContainer();
            pContainer->onCreate(iOpenId);
            gvResMgr.replaceValueFor(iOpenId,pContainer);
        }
    }
    FUNC_END;
    return pContainer;
}

/******************************************************************************
 *
 ******************************************************************************/
ResourceContainer::
ResourceContainer()
    : miOpenId(0)
    , mFeatureFlowControl(nullptr)
    , mFrameInfo(nullptr)
{
    MY_LOGD("ctor");
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
IResourceContainer::
setImagePostProcessor(
    sp<ImagePostProcessor>& processor
)
{
    Mutex::Autolock _g(gImgProcLock);
    MY_LOGD("set post image processor %p", processor.get());
    mpImagePostProcessor = processor;
}
/******************************************************************************
 *
 ******************************************************************************/
sp<ImagePostProcessor>
IResourceContainer::
queryImagePostProcessor()
{
    Mutex::Autolock _g(gImgProcLock);
    return mpImagePostProcessor;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
IResourceContainer::
clearImagePostProcessor()
{
    Mutex::Autolock _g(gImgProcLock);
    FUNC_START;
    if (mpImagePostProcessor != nullptr)
    {
        mpImagePostProcessor->waitUntilDrain();
        mpImagePostProcessor = nullptr;
    }
    FUNC_END;
}

status_t
ResourceContainer::
onCreate(int iOpenId)
{
    Mutex::Autolock _l(mResourceSetLock);
    miOpenId = iOpenId;
    MY_LOGD("(%d)", iOpenId);
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
ResourceContainer::
setFeatureFlowControl( android::sp<IFeatureFlowControl> pControl )
{
    FUNC_START;
    if ( pControl == 0 )
    {
        MY_LOGW("OpenID(%d) set NULL pControl", miOpenId);
        FUNC_END;
        return;
    }
    //
    {
        Mutex::Autolock _l(mFeatureFlowControlLock);
        mFeatureFlowControl = pControl;
    }
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
ResourceContainer::
setConsumer( StreamId_T id, android::sp<StreamBufferProvider> pProvider )
{
    FUNC_START;
    if ( pProvider == 0 )
    {
        MY_LOGW("OpenID(%d) set NULL pProvider", miOpenId);
        FUNC_END;
        return;
    }
    //
    {
        Mutex::Autolock _l(mResourceSetLock);
        ssize_t index = mvStreamBufferProviders.indexOfKey(id);
        if ( index < 0 ) {
            MY_LOGD("OpenID(%d) set StreamBufferProvider for streamId %#" PRIx64 , miOpenId, id);
            mvStreamBufferProviders.add( id, pProvider );
        }
        else
        {
            mvStreamBufferProviders.editValueAt(index) = pProvider;
        }
    }
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<StreamBufferProvider>
ResourceContainer::
queryConsumer( StreamId_T id )
{
    FUNC_START;
    Mutex::Autolock _l(mResourceSetLock);
    sp<StreamBufferProvider> pProvider = NULL;
    ssize_t index = mvStreamBufferProviders.indexOfKey(id);
    if( index < 0 )
    {
        MY_LOGW("OpenID(%d) queryConsumer streamId(%#" PRIx64 ") found nothing", miOpenId,id);
    }
    else
    {
        pProvider = mvStreamBufferProviders.editValueFor(id);
    }
    FUNC_END;
    return pProvider;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<IFeatureFlowControl>
ResourceContainer::
queryFeatureFlowControl()
{
    FUNC_START;
    Mutex::Autolock _l(mFeatureFlowControlLock);
    sp<IFeatureFlowControl> pControl = mFeatureFlowControl.promote();
    if( pControl == NULL )
    {
        MY_LOGW("queryFeatureFlowControl found nothing");
    }
    FUNC_END;
    return pControl;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
ResourceContainer::
setLatestFrameInfo( sp<IFrameInfo> frameInfo)
{
    Mutex::Autolock _l(mResourceSetLock);
    mFrameInfo = frameInfo;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<IFrameInfo>
ResourceContainer::
queryLatestFrameInfo()
{
    Mutex::Autolock _l(mResourceSetLock);
    return mFrameInfo;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
ResourceContainer::
setPipelineResource( uint32_t aShotMode, sp<IPipelineResource> pipelineResource)
{
    FUNC_START;
    Mutex::Autolock _l(mPipelineResourceLock);
    mvPipelineResource.add(aShotMode, pipelineResource);
    /*if( mPipelineResource != nullptr )
    {
        mPipelineResource->clearShot();
        mPipelineResource = nullptr;
    }
    mPipelineResource = pipelineResource;*/
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<IPipelineResource>
ResourceContainer::
queryPipelineResource( uint32_t aShotMode )
{
    //FUNC_START;
    Mutex::Autolock _l(mPipelineResourceLock);
    if ( mvPipelineResource.indexOfKey(aShotMode) >= 0) {
        return mvPipelineResource.valueFor(aShotMode);
    }

    return nullptr;
    //FUNC_END;
    //return mPipelineResource;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
ResourceContainer::
clearPipelineResource()
{
    FUNC_START;
    Mutex::Autolock _l(mPipelineResourceLock);
    for ( size_t i = 0; i < mvPipelineResource.size(); ++i ) {
        mvPipelineResource.valueAt(i)->clearShot();
    }
    mvPipelineResource.clear();
    /*if( mPipelineResource != nullptr )
    {
        mPipelineResource->clearShot();
        mPipelineResource = nullptr;
    }*/

    FUNC_END;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
ResourceContainer::
clearBufferProviders()
{
    FUNC_START;
    Mutex::Autolock _l(mResourceSetLock);
    MY_LOGD("OpenId(%d) clear all buffer providers(%d)", miOpenId, mvStreamBufferProviders.size());
    mvStreamBufferProviders.clear();
    FUNC_END;
}
/******************************************************************************
 *
 ******************************************************************************/
FrameInfo::
FrameInfo()
{

}

/******************************************************************************
 *
 ******************************************************************************/
FrameInfo::
~FrameInfo()
{
    Mutex::Autolock _l(mResultSetLock);
    mvResultMeta.clear();
}

/******************************************************************************
 *
 ******************************************************************************/
void
FrameInfo::
onResultReceived(
    MUINT32         const requestNo,
    StreamId_T      const streamId,
    MBOOL           const errorResult,
    IMetadata       const result
)
{
    Mutex::Autolock _l(mResultSetLock);
    if (errorResult) {
        MY_LOGW("[%d]error result %#" PRIx64 , requestNo, streamId);
        return;
    }
    //
    for(int i=mvResultMeta.size()-1; i>=0; i--)
    {
        //MY_LOGD("requestNo %d / %d",mvResultMeta[i].requestNo, requestNo);
        if(mvResultMeta[i].requestNo == requestNo)
        {
            ssize_t index = mvResultMeta[i].kvMeta.indexOfKey(streamId);
            if (index < 0)
            {
                MY_LOGV("[%d] first received streamId 0x%x", requestNo, streamId);
                mvResultMeta.editItemAt(i).kvMeta.add(streamId, result);
            }
            else
            {
                MY_LOGV("[%d] replace existing streamId 0x%x", requestNo, streamId);
                mvResultMeta.editItemAt(i).kvMeta.replaceValueAt(index, result);
            }
            return;
        }
    }
    //
    RESULT_META_T resultMeta;
    resultMeta.requestNo = requestNo;
    resultMeta.kvMeta.add(streamId, result);
    mvResultMeta.push_back(resultMeta);
    MY_LOGV("[%d] first received requestNo. size %d", requestNo, mvResultMeta.size());
    //
    if(mvResultMeta.size() > MAX_RESULT_META_NUM)
    {
        //MY_LOGD("Remove 0");
        mvResultMeta.removeAt(0);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
String8
FrameInfo::
getUserName()
{
    return String8(LOG_TAG);
}

/******************************************************************************
 *
 ******************************************************************************/
void
FrameInfo::
getFrameMetadata(StreamId_T const streamId, IMetadata &rResultMeta)
{
    Mutex::Autolock _l(mResultSetLock);

    if(mvResultMeta.isEmpty())
    {
        MY_LOGW("mvResultMeta is empty");
        return;
    }

    ssize_t index = 0;
    for(int i=mvResultMeta.size()-1; i>=0; i--)
    {
        index = mvResultMeta[i].kvMeta.indexOfKey(streamId);
        if (index >= 0)
        {
            rResultMeta = mvResultMeta.editItemAt(i).kvMeta.editValueAt(index);
            return;
        }
    }

    MY_LOGW("invalid streamId %#" PRIx64 , streamId);
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
FrameInfo::
getFrameMetadata(
    MUINT32 const       requestNo,
    StreamId_T const    streamId,
    IMetadata&          rResultMeta)

{
    Mutex::Autolock _l(mResultSetLock);

    if(mvResultMeta.isEmpty())
    {
        MY_LOGW("mvResultMeta is empty");
        return MFALSE;
    }

    for(int i=mvResultMeta.size()-1; i>=0; i--)
    {
        if(mvResultMeta[i].requestNo == requestNo)
        {
            ssize_t index = mvResultMeta[i].kvMeta.indexOfKey(streamId);
            if (index < 0)
            {
                MY_LOGW("[%d] invalid streamId 0x%x", requestNo, streamId);
                return MFALSE;
            }

            rResultMeta = mvResultMeta.editItemAt(i).kvMeta.editValueAt(index);
            //MY_LOGD("%d / %d",i+1, mvResultMeta.size());
            return MTRUE;
        }
    }
    return MFALSE;
}


/*******************************************************************************
*
********************************************************************************/
PipelineResource::
PipelineResource()
{

}

/*******************************************************************************
*
********************************************************************************/
PipelineResource::
~PipelineResource()
{

}

/*******************************************************************************
*
********************************************************************************/
MVOID
PipelineResource::
setShot(sp<IShot> pShot)
{
    FUNC_START;
    Mutex::Autolock _l(mResourceSetLock);
    mpShot = pShot;
    FUNC_END;
}

/*******************************************************************************
*
********************************************************************************/
sp<IShot>
PipelineResource::
getShot()
{
    Mutex::Autolock _l(mResourceSetLock);
    return mpShot;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
PipelineResource::
clearShot()
{
    Mutex::Autolock _l(mResourceSetLock);
    if( mpShot != NULL )
    {
        mpShot->sendCommand(eCmd_cancel);
    }
    mpShot = NULL;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
ResourceContainer::
setResourceConcurrency( ResourceConcurrencyType aType, android::sp<IResourceConcurrency> pConcurrency )
{
    FUNC_START;
    Mutex::Autolock _l(mResourceSetLock);
    mvConcurrency.add(aType, pConcurrency);
    MY_LOGD("setResourceConcurrency(%d) : %p",aType,pConcurrency.get());
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<IResourceConcurrency>
ResourceContainer::
queryResourceConcurrency( ResourceConcurrencyType aType )
{
    FUNC_START;
    Mutex::Autolock _l(mResourceSetLock);
    sp<IResourceConcurrency> pConcurrency = NULL;
    ssize_t index = mvConcurrency.indexOfKey(aType);
    if( index < 0 )
    {
        MY_LOGW("aType(%d) queryResourceConcurrency found nothing", aType);
    }
    else
    {
        pConcurrency = mvConcurrency.editValueFor(aType);
    }
    MY_LOGD("queryResourceConcurrency(%d) : %p",aType,pConcurrency.get());
    FUNC_END;
    return pConcurrency;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
ResourceContainer::
clearResourceConcurrency( ResourceConcurrencyType aType )
{
    FUNC_START;
    Mutex::Autolock _l(mResourceSetLock);
    int index = mvConcurrency.indexOfKey(aType);
    if( index < 0 )
    {
        MY_LOGW("aType(%d) clearResourceConcurrency found nothing", aType);
    }
    else
    {
        index = mvConcurrency.removeItemsAt(index);
    }
    MY_LOGD("clearResourceConcurrency(%d)",aType);
    FUNC_END;
}

