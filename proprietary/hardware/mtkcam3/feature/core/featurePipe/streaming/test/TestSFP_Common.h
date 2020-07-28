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

#include <vector>
#include <functional>
#include <atomic>
#include <random>
#include <thread>

#include <gtest/gtest.h>

#include <utils/String8.h>
#include <utils/Vector.h>
#include <utils/String8.h>
#include <cutils/properties.h>

#include <mtkcam/def/ImageFormat.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include "MDPWrapper.h"
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/IMetadata.h>


using namespace NSCam::NSCamFeature::NSFeaturePipe;
using NSCam::Feature::P2Util::P2IO;

namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{

#define TEST_REQ_NO_STR "test.reqno"
#define TEST_CB_STR "test.CB"
//********************************************
// Utility
//********************************************
class Utility
{
 public:
    static void periodicCall(std::chrono::seconds totalTime, std::function<void(std::chrono::seconds waitedTime)> func);

    template<typename T>
    static T getRandomNumber(T a, T b);

    static IImageBuffer* createImageBufferFromFile(const IImageBufferAllocator::ImgParam imgParam, const char* path, const char* name, MINT usage);
    static IImageBuffer* createEmptyImageBuffer(const IImageBufferAllocator::ImgParam imgParam, const char* name, MINT usage);
    static MVOID dumpBuffer(MINT32 frameNo, IImageBuffer *buffer, const char *fmt, ...);
    static MBOOL isBufferEmpty(IImageBuffer *buffer, void *emptyBuf);
};

template<typename T>
T Utility::getRandomNumber(T a, T b)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::uniform_int_distribution<T> dis(a, b);
    return dis(gen);
}
//********************************************
// SFPDataMock
//********************************************
enum SFPTestBufType
{
    // in
    TEST_IMGO,
    TEST_RRZO,
    TEST_LCSO,
    TEST_IMGO_2,
    TEST_RRZO_2,
    TEST_LCSO_2,
    TEST_TestIn_COUNT,
    // out
    TEST_FD,
    TEST_DISP,
    TEST_DISP_R,
    TEST_REC,
    TEST_REC_R,
    TEST_EXTRA,
    TEST_EXTRA_1_R,
    TEST_EXTRA_2,
    TEST_LARGE,
    TEST_LARGE_2,
    TEST_PHYSICAL,
    TEST_PHYSICAL_2,
    TEST_TestOut_COUNT
};

class SFPTestIn
{
public:
    SFPTestIn(){}
    SFPTestIn(MUINT32 _type, sp<ImageBufferPool> _pool)
    : type(_type)
    , pool(_pool)
    {
        buffer = pool->requestIIBuffer();
    }
    ~SFPTestIn()
    {
        buffer = NULL;
    }

    sp<IIBuffer> getBuffer() const
    {
        return buffer;
    }

    IImageBuffer* getImgBuffer() const
    {
        if(buffer != NULL)
            return buffer->getImageBufferPtr();
        return NULL;
    }

    MSize getImgSize() const
    {
        if(pool != NULL)
            return pool->getImageSize();
        return MSize(0,0);
    }

    MUINT32 type;
    sp<ImageBufferPool> pool;
    sp<IIBuffer> buffer;
};

class SFPTestOut
{
public:
    SFPTestOut(){}

    SFPTestOut(MUINT32 _type, sp<ImageBufferPool> _pool)
    : type(_type)
    , pqParam()
    , dppq()
    , pool(_pool)
    {
        buffer = pool->requestIIBuffer();
        sfpOut.mBuffers.push_back(buffer->getImageBufferPtr());
    }

    ~SFPTestOut()
    {
        buffer = NULL;
    }

    const SFPOutput& getOut() const
    {
        return sfpOut;
    }

    IImageBuffer* getImgBuffer() const
    {
        if(buffer != NULL)
            return buffer->getImageBufferPtr();
        return NULL;
    }

    MUINT32 type;
    NSIoPipe::PQParam pqParam;
    DpPqParam dppq;
    SFPOutput sfpOut;
    sp<ImageBufferPool> pool;
    sp<IIBuffer> buffer;
};
class SFPDataMock : public virtual android::RefBase
{
public:
    SFPDataMock()
    {
        initImgBuf();
    };


public:
    SFPDataMock(SFPDataMock const&)   = delete;
    void operator=(SFPDataMock const&)     = delete;

public:
    IImageBuffer* getImgBuf(MUINT32 type)
    {
        if(type < TEST_TestIn_COUNT)
            return mInBufMap.at(type).getImgBuffer();
        else
            return get(mOutBufMap.at(type).sfpOut.mBuffers);
    }
    // in

    //sp<IImageBuffer> mpInRSMain1ImgBuf  = nullptr;
    //sp<IImageBuffer> mpInLCSO1ImgBuf    = nullptr;
    std::map<MUINT32, SFPTestIn> mInBufMap;
    // out
    std::map<MUINT32, SFPTestOut> mOutBufMap;
    MUINT32 mMaxPlaneSize = 0;
private:


    void initImgBuf();

    const char * sMain1FSFilename = "/sdcard/SFP/test/BID_INPUT_FSRAW_1_4208x3120_5260_3.raw";
    const char * sMain2FSFilename = "/sdcard/SFP/test/BID_INPUT_FSRAW_2_4208x3120_5260_3.raw";
    const char * sMain1RSFilename = "/sdcard/SFP/test/BID_INPUT_RSRAW_1_1920x1248_3600_0.raw";
    const char * sMain2RSFilename = "/sdcard/SFP/test/BID_INPUT_RSRAW_2_1920x1248_3600_0.raw";
    const char * sLCSO1Filename   = "/sdcard/SFP/test/BID_LCSO_1_BUF_384x384_768.yuv";
    const char * sLCSO2Filename   = "/sdcard/SFP/test/BID_LCSO_2_BUF_384x384_768.yuv";
};

//********************************************
// SFPRequestCreater
//********************************************
class SFPRequestCreater : public virtual android::RefBase
{
public:
    SFPRequestCreater(sp<SFPDataMock>& mock) : mMock(mock){};
    virtual MVOID createRequest(FeaturePipeParam &param,
                                MUINT32 sensorNum,
                                std::vector<MUINT32> &genOut,
                                std::map<MUINT32, std::vector<MUINT32>> &largeOut,
                                std::map<MUINT32, std::vector<MUINT32>> &phyOut,
                                std::map<MUINT32, MBOOL> phyDiffTuning);
    virtual ~SFPRequestCreater();

    MUINT32 reqNo = 0;
    std::vector<IMetadata*> halInMetaList;
    std::vector<IMetadata*> appInMetaList;
    MUINT32 getMaxPlaneSize() {return (mMock != NULL) ? mMock->mMaxPlaneSize : 0;}

private:
    sp<SFPDataMock> mMock;
    MVOID createNonGenIOMap(FeaturePipeParam &param, MUINT32 sID, PathType type,
                        SFPSensorInput &sensorIn, const std::vector<MUINT32> &outList,
                        MBOOL isDiffTuning = MFALSE);
};

//********************************************
// DualCamMFPipeCBHandlerBase
//********************************************
class SFPCBHandlerBase : public virtual android::RefBase
{
public:
    SFPCBHandlerBase(MUINT32 maxPlaneSize) : mCompletedCount(0) {
        mCmpBuf = malloc(maxPlaneSize);
        memset(mCmpBuf , 0, maxPlaneSize);
    }

    virtual ~SFPCBHandlerBase(){
        if(mCmpBuf != NULL)
            free(mCmpBuf);
    }

    MUINT32 getReceivedEventCount() { return mCompletedCount; }
    MUINT32 waitForDesiredCount(MUINT32 desiredCount, std::chrono::milliseconds timeout);
    virtual MBOOL onEvent(FeaturePipeParam::MSG_TYPE msg, FeaturePipeParam &param) = 0;

    MBOOL mDumpBuffer = MFALSE;
    MBOOL mCheckBufferContent = MTRUE;
    std::vector<MUINT32> mSensorIds;
protected:
    const char* toString(FeaturePipeParam::MSG_TYPE eventType);
    MVOID dumpBuffer(MINT32 reqNo, const SFPIOMap &ioMap);
    MVOID checkBufferContent(MINT32 reqNo, const SFPIOMap &ioMap);
    void* mCmpBuf = NULL;


protected:
    std::atomic<MUINT32> mCompletedCount;
};
//********************************************
// StressTestEventCBHandler
//********************************************
class StressTestEventCBHandler : public SFPCBHandlerBase
{
public:
    StressTestEventCBHandler(MUINT32 maxPlaneSize) : SFPCBHandlerBase(maxPlaneSize) {
    }
    virtual MBOOL onEvent(FeaturePipeParam::MSG_TYPE msg, FeaturePipeParam &param) override;
};
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
//********************************************
