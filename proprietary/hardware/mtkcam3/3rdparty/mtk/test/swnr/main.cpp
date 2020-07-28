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
#define LOG_TAG "NRPluginTEST"
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/std/JobQueue.h>
//

#include <stdlib.h>
#include <sys/prctl.h>
#include <utils/KeyedVector.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>
#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/RefBase.h>
#include <sstream>
#include <future>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
//
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
//
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/std/Format.h>
//
#include <mtkcam3/pipeline/hwnode/NodeId.h>

#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metastore/ITemplateRequest.h>
#include <mtkcam3/3rdparty/plugin/PipelinePlugin.h>
#include <mtkcam3/3rdparty/plugin/PipelinePluginType.h>
//
#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>

#include "../../swnr/SWNRImpl.h"

using namespace NSCam;
using namespace android;
using namespace std;
using namespace NSCam::NSPipelinePlugin;
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
//
#define FUNCTION_IN                 MY_LOGD("%s +", __FUNCTION__)
#define FUNCTION_OUT                MY_LOGD("%s -", __FUNCTION__)
#define ASSERT(cond, msg)           do { if (!(cond)) { printf("Failed: %s\n", msg); return; } }while(0)

namespace {

    //
    IHalSensor* mpSensorHalObj = NULL;
    static int gSensorId = 0;
    MRect                           mActiveArray;
    //
}; // namespace

template <class T>
inline bool
tryGetMetadata( IMetadata const *pMetadata, MUINT32 tag, T& rVal )
{
    if(pMetadata == nullptr) return MFALSE;

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if(!entry.isEmpty())
    {
        rVal = entry.itemAt(0,Type2Type<T>());
        return true;
    }
    return false;
}

/******************************************************************************
*
******************************************************************************/
class WarpCallback : public YuvPlugin::RequestCallback
{
public:
    virtual void onAborted(YuvPlugin::Request::Ptr pRequest)
    {
        std::cout << "onAborted request: "<< pRequest.get() << " aborted" << std::endl;
    }

    virtual void onCompleted(YuvPlugin::Request::Ptr pRequest, MERROR result)
    {
        std::cout << "onCompleted request: "<< pRequest.get() << " result: " << result << std::endl;
    }

    virtual ~WarpCallback() { };
};

/******************************************************************************
*
******************************************************************************/
class TestPluginProvider: public SwnrPluginProviderImp
{
    typedef YuvPlugin::Property Property;

public:
    virtual const Property& property()
    {
        static Property prop;
        static bool inited;

        if (!inited)
        {
            prop.mName = "Test Plugin";
            inited = true;
        }
        return prop;
    }

    virtual ~TestPluginProvider() {};
};

REGISTER_PLUGIN_PROVIDER(Yuv, TestPluginProvider);

class YuvInterface : public YuvPlugin::IInterface
{
public:
    virtual MERROR offer(YuvPlugin::Selection& sel)
    {
        sel.mIBufferFull
            .setRequired(true)
            .addSupportFormat(eImgFmt_YUY2)
            .addSupportFormat(eImgFmt_NV21)
            .addSupportSize(eImgSize_Full)
            .addSupportSize(eImgSize_Specified);

        sel.mOBufferFull
            .setRequired(true)
            .addSupportFormat(eImgFmt_YUY2)
            .addSupportFormat(eImgFmt_NV21)
            .addSupportSize(eImgSize_Full)
            .addSupportSize(eImgSize_Specified);

        return OK;

    };

    virtual ~YuvInterface() {};
};

REGISTER_PLUGIN_INTERFACE(Yuv, YuvInterface);

/******************************************************************************
 *
 ******************************************************************************/
class WorkingBufferHandle : public BufferHandle
{
public:
    WorkingBufferHandle(const char* name, MINT32 format, const MSize& size) :
        mName(name),
        mFormat(format),
        mSize(size),
        mpImageBuffer(NULL)
    {

    }

    virtual ~WorkingBufferHandle(){
        MY_LOGD("~WorkingBufferHandle");
    }
    virtual IImageBuffer* acquire(MINT usage)
    {
        MY_LOGD("acquire buffer ");
        MUINT32 plane = NSCam::Utils::Format::queryPlaneCount(mFormat);
        size_t bufBoundaryInBytes[3] = {0, 0, 0};
        size_t bufStridesInBytes[3] = {0};

        for (MUINT32 i = 0; i < plane; i++)
        {
            bufStridesInBytes[i] = NSCam::Utils::Format::queryPlaneWidthInPixels(mFormat, i, mSize.w) *
                                   NSCam::Utils::Format::queryPlaneBitsPerPixel(mFormat, i) / 8;
        }

        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam(
                    (EImageFormat) mFormat, mSize, bufStridesInBytes, bufBoundaryInBytes, plane);

        sp<IImageBufferHeap> pHeap = IIonImageBufferHeap::create(mName, imgParam);
        if (pHeap == NULL)
        {
            std::cerr << "working buffer: create heap failed" << std::endl;
            return NULL;
        }

        mpImageBuffer = pHeap->createImageBuffer();
        if (mpImageBuffer == NULL)
        {
            std::cerr << "working buffer: create image buffer failed" << std::endl;
            return NULL;
        }

        if (!(mpImageBuffer->lockBuf(mName, usage)))
        {
            std::cerr << "working buffer: lock image buffer failed" << std::endl;
            return NULL;
        }

        return mpImageBuffer.get();

    }

    virtual void release()
    {
        if(mpImageBuffer!=NULL) {
            mpImageBuffer->unlockBuf(mName);
            mpImageBuffer.clear();
            MY_LOGD("mpImageBuffer.clear();");
        }
    }

    virtual MVOID dump(std::ostream& os) const
    {
        os << "{name: " << mName << ", format: " << mFormat
           << ", size: (" << mSize.w << "x" << mSize.h << ")}";

    }

private:
    sp<IImageBuffer>            mpImageBuffer;
    const char*                 mName;
    MINT32                      mFormat;
    MSize                       mSize;
};


class WorkingMetadataHandle : public MetadataHandle
{
public:
    WorkingMetadataHandle()
    {
    }

    virtual IMetadata* acquire()
    {
        return &mMetadata;
    }

    virtual void release()
    {
    }

    virtual MVOID dump(std::ostream& os) const
    {
        os << "{count: " << mMetadata.count() << "}";
    }

private:
    IMetadata                   mMetadata;
};

/******************************************************************************
 *
 ******************************************************************************/
void testSinglton()
{
    FUNCTION_IN;

    YuvPlugin::Ptr plFusion1 = YuvPlugin::getInstance(0);
    YuvPlugin::Ptr plFusion2 = YuvPlugin::getInstance(0);
    YuvPlugin::Ptr plFusion3 = YuvPlugin::getInstance(1);
    ASSERT(plFusion1.get() == plFusion2.get(), "should have two same pointers!");
    ASSERT(plFusion1.get() != plFusion3.get(), "should have two different pointers!");


    void* tmp = plFusion1.get();

    plFusion1 = nullptr;
    plFusion2 = nullptr;
    YuvPlugin::Ptr plFusion4 = YuvPlugin::getInstance(0);
    printf("release all references and re-create, original:%p, new:%p\n", tmp, plFusion4.get());
    ASSERT(tmp != plFusion4.get(), "should have different pointers after released all reference!");

    YuvPlugin::IInterface::Ptr desc = plFusion4->getInterface();
    YuvPlugin::IInterface::Ptr desc2 = plFusion4->getInterface();

    ASSERT(desc == desc2, "To query interface twice should get the same pointer");
    FUNCTION_OUT;
}

/******************************************************************************
 *
 ******************************************************************************/
void testGetProperty()
{
    FUNCTION_IN;

    YuvPlugin::Ptr plFusion = YuvPlugin::getInstance(0);

    auto& provs = plFusion->getProviders();

    int count = provs.size();
    ASSERT(count == 1, "there is no provider mounted");

    YuvPlugin::IProvider::Ptr prov = provs[0];
    YuvPlugin::Property prop = prov->property();
    FUNCTION_OUT;

}

/******************************************************************************
 *
 ******************************************************************************/
void testGetSelection()
{
    FUNCTION_IN;
    YuvPlugin::Ptr plFusion = YuvPlugin::getInstance(0);

    auto& provs = plFusion->getProviders();
    int count = provs.size();
    ASSERT(count == 2, "there are wrong providers mounted");

    YuvPlugin::IProvider::Ptr prov = provs[0];

    const YuvPlugin::Selection& sel = plFusion->getSelection(prov);


    std::stringstream stream;
    stream << "Dump Yuv" << std::endl;
    plFusion->dump(stream);
    std::cout << stream.str() << std::endl;
    FUNCTION_OUT;
}

/******************************************************************************
 *
 ******************************************************************************/
void testInitPlugin()
{
    FUNCTION_IN;
    YuvPlugin::Ptr plFusion = YuvPlugin::getInstance(0);
    auto& provs = plFusion->getProviders();
    YuvPlugin::IProvider::Ptr prov = provs[0];

    // test to init multiple times, because the instance is shared with multiple users
    // the provider should be called to init() once. there is a layer of interception
    // to prevent from being inited multiple time.
    prov->init();
    usleep(30000); //30ms
    //prov->init();

    auto pCallback = make_shared<WarpCallback>();

    //auto pRequest = plFusion->createRequest();
    //pRequest->mIBuffer_MainFull = make_shared<WorkingBufferHandle>("Tester", eImgFmt_I420, MSize(1920,1080));
    //pRequest->mIMetadata_P1 = make_shared<WorkingMetadataHandle>();

    // Sync Call
    //prov->process(pRequest);

    // ASync Call
    auto pRequest2 = plFusion->createRequest();
    pRequest2->mIBufferFull = make_shared<WorkingBufferHandle>("Tester", eImgFmt_I420, mActiveArray.s);
    //pRequest2->mOpenId = 0;
    //pRequest2->mIMetadataDynamic = make_shared<WorkingMetadataHandle>();
    prov->process(pRequest2, pCallback);
    usleep(30000); //30ms
    prov->uninit();
    pRequest2->mIBufferFull->release();
    //prov->uninit();
    FUNCTION_OUT;
}


/******************************************************************************
 *
 ******************************************************************************/
void prepareSensor(int openId)
{
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    mpSensorHalObj = pHalSensorList->createSensor("tester", openId);
    if( ! mpSensorHalObj ) {
        MY_LOGE("create sensor failed");
        exit(1);
        return;
    }
    MUINT32    sensorArray[1] = {static_cast<MUINT32>(openId)};
    mpSensorHalObj->powerOn("tester", 1, &sensorArray[0]);
}

/******************************************************************************
 *
 ******************************************************************************/
void closeSensor(int openId)
{
    MUINT32    sensorArray[1] = {static_cast<MUINT32>(openId)};
    mpSensorHalObj->powerOff("tester", 1, &sensorArray[0]);
}

/******************************************************************************
 *
 ******************************************************************************/
void initMetadata(int openId)
{

    sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(openId);
    IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();

    if( tryGetMetadata<MRect>(&static_meta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, mActiveArray) ) {
        MY_LOGD_IF(1,"active array(%d, %d, %dx%d)",
             mActiveArray.p.x, mActiveArray.p.y, mActiveArray.s.w, mActiveArray.s.h);
    } else {
        MY_LOGE("no static info: MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION");
    }

}


/******************************************************************************
 *
 ******************************************************************************/
void clearMetadata()
{
    NSMetadataProviderManager::clear();
}


/******************************************************************************
 *
 ******************************************************************************/
int main(int argc, char** argv)
{
    FUNCTION_IN;

    IHalLogicalDeviceList* const HalLogicalDeviceList = MAKE_HalLogicalDeviceList();
    MY_LOGD("search sensor ...");
    std::cout << "search sensor ..." << std::endl;
    MUINT num = HalLogicalDeviceList->searchDevices();
    std::cout << "search sensor end" << std::endl;

    prepareSensor(gSensorId);

    sp<IMetadataProvider> pMetadataProvider = IMetadataProvider::create(gSensorId);
    NSMetadataProviderManager::add(gSensorId, pMetadataProvider.get());
    initMetadata(gSensorId);

    testSinglton();

    testGetProperty();

    testGetSelection();

    testInitPlugin();

    usleep(100000);

    clearMetadata();

    FUNCTION_OUT;
    return 0;
}

