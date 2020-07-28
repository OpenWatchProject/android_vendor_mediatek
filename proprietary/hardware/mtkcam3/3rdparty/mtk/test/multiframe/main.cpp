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
#define LOG_TAG "MfnrPluginTest"
//
#include <mtkcam/utils/std/Log.h>
//
#include <stdlib.h>
#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/RefBase.h>
#include <sstream>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
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
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam3/3rdparty/plugin/PipelinePlugin.h>
#include <mtkcam3/3rdparty/plugin/PipelinePluginType.h>
//
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
#define ASSERT(cond, msg)           do { if (!(cond)) { printf("Failed: %s\n", msg); return; } }while(0)


/******************************************************************************
*
******************************************************************************/
class MfnrPluginProvider : public MultiFramePlugin::IProvider
{
    typedef MultiFramePlugin::Property Property;
    typedef MultiFramePlugin::Selection Selection;
    typedef MultiFramePlugin::Request::Ptr RequestPtr;
    typedef MultiFramePlugin::RequestCallback::Ptr RequestCallbackPtr;

public:
    virtual const Property& property()
    {
        static Property prop;
        static bool inited;

        if (!inited) {
            prop.mName = "MTK MFNR";
            prop.mPriority = 1;
            inited = true;
        }
        return prop;
    };

    virtual MERROR negotiate(Selection& sel)
    {
        return OK;
    };

    virtual void init()
    {

    };

    virtual void set(MINT32 iOpenId, MINT32 iOpenId2)
    {
        MY_LOGD("set openId:%d openId2:%d", iOpenId, iOpenId2);
    }

    virtual MERROR process(RequestPtr pReq,
                           RequestCallbackPtr pCb = nullptr)
    {
        return 0;
    };

    virtual void abort(vector<RequestPtr>& pRequests)
    {

    };

    virtual void uninit()
    {

    };

    virtual ~MfnrPluginProvider()
    {

    };
};

REGISTER_PLUGIN_PROVIDER(MultiFrame, MfnrPluginProvider);

/******************************************************************************
*
******************************************************************************/
class MultiFrameInterface : public MultiFramePlugin::IInterface
{
public:
    virtual MERROR offer(MultiFramePlugin::Selection& sel)
    {
        sel.mIBufferFull
            .setRequired(true)
            .addSupportFormat(eImgFmt_YUY2)
            .addSupportFormat(eImgFmt_NV21)
            .addSupportSize(eImgSize_Full)
            .addSupportSize(eImgSize_Specified);

        sel.mIBufferSpecified
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

    virtual ~MultiFrameInterface() {};
};

REGISTER_PLUGIN_INTERFACE(MultiFrame, MultiFrameInterface);

/******************************************************************************
*
******************************************************************************/
class WarpPluginProvider : public FusionPlugin::IProvider
{
    typedef FusionPlugin::Property Property;
    typedef FusionPlugin::Selection Selection;
    typedef FusionPlugin::Request::Ptr RequestPtr;
    typedef FusionPlugin::RequestCallback::Ptr RequestCallbackPtr;

public:
    virtual const Property& property()
    {
        static Property prop;
        static bool inited;

        if (!inited) {
            prop.mName = "MTK FUSION";
            inited = true;
        }
        return prop;
    }

    virtual MERROR negotiate(Selection& sel)
    {
        sel.mIBufferFull
            .setRequired(true)
            .addAcceptedFormat(eImgFmt_YUY2)
            .addAcceptedFormat(eImgFmt_NV21)
            .addAcceptedSize(eImgSize_Full);

        sel.mIBufferFull2
            .setRequired(true)
            .addAcceptedFormat(eImgFmt_YUY2)
            .addAcceptedFormat(eImgFmt_NV21)
            .addAcceptedSize(eImgSize_Specified)
            .setSpecifiedSize(MSize(1920, 1080));
        sel.mIMetadataDynamic.setRequired(false);
        sel.mIMetadataApp.setRequired(false);
        sel.mIMetadataHal.setRequired(false);
        sel.mOMetadataApp.setRequired(false);
        sel.mOMetadataHal.setRequired(false);
        return OK;
    }

    virtual void init()
    {
        std::cout << "Warp Init" << std::endl;
    }

    virtual void set(MINT32 iOpenId, MINT32 iOpenId2)
    {
        MY_LOGD("set openId:%d openId2:%d", iOpenId, iOpenId2);
    }

    virtual MERROR process(RequestPtr pRequest,
                           RequestCallbackPtr pCallback = nullptr)
    {
        if (pRequest->mIBufferFull != nullptr) {
            IImageBuffer* pImgBuffer = pRequest->mIBufferFull->acquire();
            std::cout << "img VA: 0x" << pImgBuffer->getBufVA(0) << std::endl;

        }

        if (pRequest->mIMetadataDynamic != nullptr) {
            IMetadata *meta = pRequest->mIMetadataDynamic->acquire();
            std::cout << "meta count: " << meta->count() << std::endl;

        }

        if (pCallback != nullptr) {
            pCallback->onCompleted(pRequest, 0);
        }
        return 0;
    }

    virtual void abort(vector<RequestPtr>& pRequests)
    {

    };

    virtual void uninit()
    {

    };

    virtual ~WarpPluginProvider()
    {

    };
};

REGISTER_PLUGIN_PROVIDER(Fusion, WarpPluginProvider);

/******************************************************************************
*
******************************************************************************/
class WarpCallback : public FusionPlugin::RequestCallback
{
public:
    virtual void onAborted(FusionPlugin::Request::Ptr pRequest)
    {
        std::cout << "onAborted request: "<< pRequest.get() << " aborted" << std::endl;
    }

    virtual void onCompleted(FusionPlugin::Request::Ptr pRequest, MERROR result)
    {
        std::cout << "onCompleted request: "<< pRequest.get() << " result: " << result << std::endl;
    }

    virtual ~WarpCallback() { };
};

/******************************************************************************
*
******************************************************************************/
class TestPluginProvider: public WarpPluginProvider
{
    typedef FusionPlugin::Property Property;

public:
    virtual const Property& property()
    {
        static Property prop;
        static bool inited;

        if (!inited) {
            prop.mName = "Test Plugin";
            inited = true;
        }
        return prop;
    }

    virtual ~TestPluginProvider() {};
};

REGISTER_PLUGIN_PROVIDER(Fusion, TestPluginProvider);

class FusionInterface : public FusionPlugin::IInterface
{
public:
    virtual MERROR offer(FusionPlugin::Selection& sel)
    {
        sel.mIBufferFull
            .setRequired(true)
            .addSupportFormat(eImgFmt_YUY2)
            .addSupportFormat(eImgFmt_NV21)
            .addSupportSize(eImgSize_Full)
            .addSupportSize(eImgSize_Specified);

        sel.mIBufferFull2
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

    virtual ~FusionInterface() {};
};

REGISTER_PLUGIN_INTERFACE(Fusion, FusionInterface);

/******************************************************************************
 *
 ******************************************************************************/
class WorkingBufferHandle : public BufferHandle
{
public:
    WorkingBufferHandle(const char* name, MINT32 format, const MSize& size) :
        mName(name),
        mFormat(format),
        mSize(size)
    {

    }

    virtual IImageBuffer* acquire(MINT usage)
    {
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
        mpImageBuffer->unlockBuf(mName);
        mpImageBuffer.clear();
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
void testSinglton() {
    FusionPlugin::Ptr plFusion1 = FusionPlugin::getInstance(0);
    FusionPlugin::Ptr plFusion2 = FusionPlugin::getInstance(0);
    FusionPlugin::Ptr plFusion3 = FusionPlugin::getInstance(1);
    ASSERT(plFusion1.get() == plFusion2.get(), "should have two same pointers!");
    ASSERT(plFusion1.get() != plFusion3.get(), "should have two different pointers!");


    void* tmp = plFusion1.get();

    plFusion1 = nullptr;
    plFusion2 = nullptr;
    FusionPlugin::Ptr plFusion4 = FusionPlugin::getInstance(0);
    printf("release all references and re-create, original:%p, new:%p\n", tmp, plFusion4.get());
    ASSERT(tmp != plFusion4.get(), "should have different pointers after released all reference!");

    FusionPlugin::IInterface::Ptr desc = plFusion4->getInterface();
    FusionPlugin::IInterface::Ptr desc2 = plFusion4->getInterface();

    ASSERT(desc == desc2, "To query interface twice should get the same pointer");
}

/******************************************************************************
 *
 ******************************************************************************/
void testGetProperty() {
    MultiFramePlugin::Ptr plMultiFrame = MultiFramePlugin::getInstance(0);

    auto& provs = plMultiFrame->getProviders();

    int count = provs.size();
    ASSERT(count == 1, "there is no provider mounted");

    MultiFramePlugin::IProvider::Ptr prov = provs[0];
    MultiFramePlugin::Property prop = prov->property();
    printf("priority: %d\n", prop.mPriority);


    ASSERT(prop.mPriority == 1, "have wrong priority!");
}

/******************************************************************************
 *
 ******************************************************************************/
void testGetSelection() {
    FusionPlugin::Ptr plFusion = FusionPlugin::getInstance(0);

    auto& provs = plFusion->getProviders();
    int count = provs.size();
    ASSERT(count == 2, "there are wrong providers mounted");

    FusionPlugin::IProvider::Ptr prov = provs[0];

    const FusionPlugin::Selection& sel = plFusion->getSelection(prov);


    std::stringstream stream;
    stream << "Dump Fusion" << std::endl;
    plFusion->dump(stream);
    std::cout << stream.str() << std::endl;
}

/******************************************************************************
 *
 ******************************************************************************/
void testInitPlugin() {

    FusionPlugin::Ptr plFusion = FusionPlugin::getInstance(0);
    auto& provs = plFusion->getProviders();
    FusionPlugin::IProvider::Ptr prov = provs[0];

    // test to init multiple times, because the instance is shared with multiple users
    // the provider should be called to init() once. there is a layer of interception
    // to prevent from being inited multiple time.
    prov->init();
    prov->init();

    auto pCallback = make_shared<WarpCallback>();

    auto pRequest = plFusion->createRequest();
    pRequest->mIBufferFull = make_shared<WorkingBufferHandle>("Tester", eImgFmt_NV21, MSize(1920,1080));
    pRequest->mIMetadataDynamic = make_shared<WorkingMetadataHandle>();

    // Sync Call
    prov->process(pRequest);

    // ASync Call
    auto pRequest2 = plFusion->createRequest();
    pRequest2->mIBufferFull = make_shared<WorkingBufferHandle>("Tester", eImgFmt_NV21, MSize(1920,1080));
    pRequest2->mIMetadataDynamic = make_shared<WorkingMetadataHandle>();
    prov->process(pRequest2, pCallback);

    prov->uninit();
    prov->uninit();
}

/******************************************************************************
 *
 ******************************************************************************/
int main(int argc, char** argv)
{
    testSinglton();

    testGetProperty();

    testGetSelection();

    testInitPlugin();

    return 0;
}

