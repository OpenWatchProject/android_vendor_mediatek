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

#define LOG_TAG "MtkCam/StreamBufferProviderFactory"
//
#include "MyUtils.h"
//
#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/utils/streambuf/StreamBuffers.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>
#include "StreamBufferProviderImp.h"
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/BufferPoolImp.h>
#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>

using namespace android;
using namespace NSCam;
using namespace NSCam::v1;

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

/******************************************************************************
 *
 ******************************************************************************/

class StreamBufferProviderFactoryImp
    : public StreamBufferProviderFactory
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StreamBufferProviderFactory Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    virtual MERROR                      setImageStreamInfo(
                                            sp<IImageStreamInfo> pStreamInfo
                                        );

    virtual MERROR                      setUsersPool(
                                            sp< IBufferPool > pPool
                                        );

    virtual MERROR                      setPairRule(
                                            sp< PairMetadata > pPairMeta,
                                            MINT32             numberToPair
                                        );

    virtual sp<StreamBufferProvider>    create( MBOOL NeedTimeStamp );

    virtual sp<StreamBufferProvider>    create( MINT32 rOpenId, MBOOL rStoreInContainer, MBOOL NeedTimeStamp);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
                                        StreamBufferProviderFactoryImp();

                                        ~StreamBufferProviderFactoryImp() {};

protected:
    sp< PStreamBufferProviderImp >      createProducer();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    sp< PStreamBufferProviderImp >      mpProducer;
    sp< IBufferPool >                   mpUserPool;
};



/******************************************************************************
 *
 ******************************************************************************/
sp<StreamBufferProviderFactory>
StreamBufferProviderFactory::
createInstance()
{
    return new StreamBufferProviderFactoryImp();
}

/******************************************************************************
 *
 ******************************************************************************/
StreamBufferProviderFactoryImp::
StreamBufferProviderFactoryImp()
{
}


/******************************************************************************
 *
 ******************************************************************************/
sp< PStreamBufferProviderImp >
StreamBufferProviderFactoryImp::
createProducer()
{
    if ( mpProducer == NULL ) {
        mpProducer = new PStreamBufferProviderImp();
    }
    return mpProducer;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
StreamBufferProviderFactoryImp::
setImageStreamInfo(
    sp<IImageStreamInfo> pStreamInfo
)
{
    sp< PStreamBufferProviderImp > pProducer = createProducer();
    pProducer->setImageStreamInfo(pStreamInfo);

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
StreamBufferProviderFactoryImp::
setUsersPool(
    sp< IBufferPool > pPool
)
{
    FUNC_START;

    mpUserPool = pPool;

    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
StreamBufferProviderFactoryImp::
setPairRule(
    sp< PairMetadata > pPairMeta,
    MINT32             numberToPair
)
{
    sp< PStreamBufferProviderImp > pProducer = createProducer();
    pPairMeta->setNumberOfMetadata(numberToPair);
    pProducer->setPairRule(pPairMeta);

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<StreamBufferProvider>
StreamBufferProviderFactoryImp::
create(
    MBOOL NeedTimeStamp
)
{
    if ( mpProducer == NULL ) {
        mpProducer = createProducer();
        if ( mpProducer == NULL ) {
            MY_LOGE("no StreamBufferProvider created.");
            return NULL;
        }
    }
    //
    sp<IImageStreamInfo> pStreamInfo = mpProducer->queryImageStreamInfo();

    sp< BufferPoolImp > pPool;
    if( mpUserPool.get() ) {
        mpProducer->setBufferPool(mpUserPool);
    } else {
        pPool = new BufferPoolImp(pStreamInfo);
        pPool->allocateBuffer(pStreamInfo->getStreamName(), pStreamInfo->getMaxBufNum(), pStreamInfo->getMinInitBufNum());
        mpProducer->setBufferPool(pPool);
    }
    mpProducer->sendCommand(
            ProviderMsg_T::TIMESTAMP_INFO,
            NeedTimeStamp,
            0
        );
    sp<StreamBufferProvider> pProvider = mpProducer;
    mpProducer = NULL;
    mpUserPool = NULL;

    return pProvider;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<StreamBufferProvider>
StreamBufferProviderFactoryImp::
create(
    MINT32                   rOpenId,
    MBOOL                    rStoreInContainer,
    MBOOL                    NeedTimeStamp
)
{
    if ( mpProducer == NULL ) {
        mpProducer = createProducer();
        if ( mpProducer == NULL ) {
            MY_LOGE("no StreamBufferProvider created.");
            return NULL;
        }
    }
    //
    sp<IImageStreamInfo> pStreamInfo = mpProducer->queryImageStreamInfo();

    sp< BufferPoolImp > pPool;
    if( mpUserPool.get() ) {
        mpProducer->setBufferPool(mpUserPool);
    } else {
        pPool = new BufferPoolImp(pStreamInfo);
        pPool->allocateBuffer(pStreamInfo->getStreamName(), pStreamInfo->getMaxBufNum(), pStreamInfo->getMinInitBufNum());
        mpProducer->setBufferPool(pPool);
    }

    if ( rStoreInContainer ) {
        IResourceContainer::getInstance(rOpenId)->setConsumer(pStreamInfo->getStreamId(), mpProducer);
    }
    mpProducer->sendCommand(
        ProviderMsg_T::TIMESTAMP_INFO,
        NeedTimeStamp,
        0
    );

    sp<StreamBufferProvider> pProvider = mpProducer;
    mpProducer = NULL;
    mpUserPool = NULL;

    return pProvider;
}
