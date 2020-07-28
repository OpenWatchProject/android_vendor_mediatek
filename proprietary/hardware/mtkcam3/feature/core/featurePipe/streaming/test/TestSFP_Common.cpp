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

#include <gtest/gtest.h>

#include "DebugControl.h"

#define PIPE_CLASS_TAG "TestSFP_Common"
#include <featurePipe/core/include/PipeLog.h>

#include <mtkcam3/feature/featurePipe/IStreamingFeaturePipe.h>
#include <featurePipe/streaming/StreamingFeature_Common.h>
#include <featurePipe/core/include/ImageBufferPool.h>

#include "TestSFP_Common.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

namespace NSCam{
namespace NSCamFeature{
using Feature::P2Util::P2InfoObj;
using Feature::P2Util::P2DataObj;
using Feature::P2Util::P2Pack;
namespace NSFeaturePipe{


// ***************************************************************
// Utility
// ***************************************************************
void Utility::periodicCall(std::chrono::seconds totalTime, std::function<void(std::chrono::seconds waitedTime)> func)
{
    const int waitCountForSeconds = totalTime.count();
    const std::chrono::seconds oneSecond(1);
    for(int i = 1; i <= totalTime.count(); i++)
    {
        std::this_thread::sleep_for(oneSecond);
        func(std::chrono::seconds(i));
    }
}

IImageBuffer* Utility::createImageBufferFromFile(const IImageBufferAllocator::ImgParam imgParam, const char* path, const char* name, MINT usage)
{
    IImageBufferAllocator *allocator = IImageBufferAllocator::getInstance();
    IImageBuffer* imgBuf = allocator->alloc(name, imgParam);
    EXPECT_TRUE(imgBuf->loadFromFile(path)) << "name: " << name << std::endl << "path: " << path;
    EXPECT_TRUE(imgBuf->lockBuf(name, usage)) << "name: " << name << std::endl << "usage: " << usage;
    return imgBuf;
}

IImageBuffer* Utility::createEmptyImageBuffer(const IImageBufferAllocator::ImgParam imgParam, const char* name, MINT usage)
{
    IImageBufferAllocator *allocator = IImageBufferAllocator::getInstance();
    IImageBuffer* imgBuf = allocator->alloc(name, imgParam);
    EXPECT_TRUE(imgBuf->lockBuf(name, usage)) << "name: " << name << std::endl << "usage: " << usage; ;
    return imgBuf;
}

MVOID Utility::dumpBuffer(MINT32 frameNo, IImageBuffer *buffer, const char *fmt, ...)
{
    if( buffer )
    {
        MUINT32 stride, pbpp, ibpp, width, height, size;
        MINT format = buffer->getImgFormat();
        stride = buffer->getBufStridesInBytes(0);
        pbpp = buffer->getPlaneBitsPerPixel(0);
        ibpp = buffer->getImgBitsPerPixel();
        size = buffer->getBufSizeInBytes(0);
        pbpp = pbpp ? pbpp : 8;
        width = stride * 8 / pbpp;
        width = width ? width : 1;
        ibpp = ibpp ? ibpp : 8;
        height = size / width;
        if( buffer->getPlaneCount() == 1 )
        {
          height = height * 8 / ibpp;
        }

        va_list ap;
        va_start(ap, fmt);

        char name[256] = {0};
        char path[256] = {0};
        if( 0 >= vsnprintf(name, sizeof(name), fmt, ap) )
        {
            strncpy(name, "NA", sizeof(name));
            name[sizeof(name)-1] = 0;
        }
        va_end(ap);

        snprintf(path, sizeof(path), "/sdcard/dump/%04d_%s_%dx%d_%d.%s.bin", frameNo, name, width, height, stride, Fmt2Name(format));

        buffer->saveToFile(path);
    }
}

MBOOL Utility::isBufferEmpty(IImageBuffer *buffer, void *emptyBuf)
{
    if(buffer == NULL)
    {
        MY_LOGE("Can not check Buffer Empty!! Buffer is NULL!!");
        return MTRUE;
    }
    if(emptyBuf == NULL)
    {
        MY_LOGE("Can not check Buffer Empty!! Memcmp Empty Buffer is NULL!!");
        return MTRUE;
    }

    for (size_t i = 0; i < buffer->getPlaneCount(); i++)
    {
        //MUINT8* pBuf = (MUINT8*)buffer->getBufVA(i);
        size_t  size = buffer->getBufSizeInBytes(i);
        if(memcmp( (void*)buffer->getBufVA(i), emptyBuf, sizeof(size) ) != 0)
        {
            return MFALSE;
        }
#if 0
        for(size_t index = 0; index < size; index++)
        {
            if(*(pBuf + index) != 0)
            {
                return MFALSE;
            }
        }
#endif
    }
    return MTRUE;
}
// ***************************************************************
// DCMFPipeDataMock
// ***************************************************************
void SFPDataMock::initImgBuf()
{
    static std::vector<std::vector<MUINT32>> outBufInfo;
    outBufInfo = {
        //type                w    h      fmt             trans             refInput           sfp_out_type
        { TEST_DISP_R,      1080, 1920,  eImgFmt_YUY2,  eTransform_ROT_90,   TEST_RRZO,      SFPOutput::OUT_TARGET_DISPLAY}, // display
        { TEST_REC,         1920, 1080,  eImgFmt_YV12,        0,             TEST_RRZO,      SFPOutput::OUT_TARGET_RECORD}, // record
        { TEST_FD,          640,  480,   eImgFmt_YUY2,        0,             TEST_RRZO,      SFPOutput::OUT_TARGET_FD}, // fd img2o only support YUY2
        { TEST_EXTRA,       1920, 1080,  eImgFmt_NV21,        0,             TEST_RRZO,      SFPOutput::OUT_TARGET_UNKNOWN}, // extra
        { TEST_LARGE,       5344, 4016,  eImgFmt_NV21,        0,             TEST_IMGO,      SFPOutput::OUT_TARGET_UNKNOWN}, // large
        { TEST_LARGE_2,     4032, 3016,  eImgFmt_YUY2,        0,             TEST_IMGO_2,    SFPOutput::OUT_TARGET_UNKNOWN}, // large2
        { TEST_PHYSICAL,    1920, 1080,  eImgFmt_YV12,        0,             TEST_RRZO,      SFPOutput::OUT_TARGET_PHYSICAL}, // physical
        { TEST_PHYSICAL_2,  1920, 1080,  eImgFmt_YV12,        0,             TEST_RRZO_2,    SFPOutput::OUT_TARGET_PHYSICAL} // physical2
    };
    // input
    std::vector<std::vector<MUINT32>> inBufInfo;
    inBufInfo = {
        //type            w    h        fmt                 stride
        { TEST_IMGO,    5344, 4016,  eImgFmt_BAYER10,       5600},
        { TEST_RRZO,    2560, 1440,  eImgFmt_BAYER10,       3840},
        { TEST_LCSO,    384,  384,   eImgFmt_BAYER10,       768},
        { TEST_IMGO_2,  4032, 3016,  eImgFmt_BAYER10,       4032},
        { TEST_RRZO_2,  1920, 1280,  eImgFmt_BAYER10,       2560},
        { TEST_LCSO_2,  384,  384,   eImgFmt_BAYER10,       768}
    };

    for(auto&& it : inBufInfo)
    {
        MUINT32 type = it[0];
        MUINT32 w = it[1];
        MUINT32 h = it[2];
        EImageFormat fmt = (EImageFormat)it[3];
        MUINT32 stride = it[4];

        sp<ImageBufferPool> pool = ImageBufferPool::create("testIn", w, h, fmt, ImageBufferPool::USAGE_HW );
        pool->allocate(1);
        mInBufMap[type] = SFPTestIn(type, pool);
    }

    for(auto&& it : outBufInfo)
    {
        MUINT32 type = it[0];
        MUINT32 w = it[1];
        MUINT32 h = it[2];
        EImageFormat fmt = (EImageFormat)it[3];
        MUINT32 tran = it[4];
        MUINT32 inType = it[5];
        SFPOutput::OutTargetType outType = (SFPOutput::OutTargetType)it[6];

        sp<ImageBufferPool> pool = ImageBufferPool::create("testOut", w, h, fmt, ImageBufferPool::USAGE_HW );
        pool->allocate(1);

        SFPTestOut testOut(type, pool);
        testOut.sfpOut.mTransform = tran;
        testOut.sfpOut.mTargetType = outType;
        if(outType == SFPOutput::OUT_TARGET_DISPLAY)
        {
            testOut.sfpOut.mCapability = NSCam::NSIoPipe::EPortCapbility_Disp;
        }
        else if(outType == SFPOutput::OUT_TARGET_RECORD)
        {
            testOut.sfpOut.mCapability = NSCam::NSIoPipe::EPortCapbility_Rcrd;
        }

        testOut.sfpOut.mCropRect = MRect(MPoint(0,0) , mInBufMap[inType].getImgSize());

        testOut.sfpOut.mCropDstSize = get(testOut.sfpOut.mBuffers)->getImgSize();

        mOutBufMap[type] = testOut;

        IImageBuffer *buf = mOutBufMap[type].getImgBuffer();
        for (size_t i = 0; i < buf->getPlaneCount(); i++)
        {
            size_t size = buf->getBufSizeInBytes(i);
            if(size > mMaxPlaneSize)
                mMaxPlaneSize = size;
        }
    }
}

//********************************************
// SFPRequestCreater
//********************************************
MVOID SFPRequestCreater::createNonGenIOMap(FeaturePipeParam &param, MUINT32 sID, PathType type,
                        SFPSensorInput &sensorIn, const std::vector<MUINT32> &outList,
                        MBOOL isDiffTuning)
{
    if(outList.size() == 0)
        return;

    SFPSensorTuning tuning;
    SFPIOMap ioMap;
    if(type == PATH_LARGE && get(sensorIn.mIMGO) != NULL)
    {
        tuning.addFlag(SFPSensorTuning::FLAG_IMGO_IN);
    }
    else if(type == PATH_PHYSICAL && get(sensorIn.mRRZO) != NULL)
    {
        tuning.addFlag(SFPSensorTuning::FLAG_RRZO_IN);
    }

    if(get(sensorIn.mLCSO) != NULL)
    {
        tuning.addFlag(SFPSensorTuning::FLAG_LCSO_IN);
    }

    if(isDiffTuning)
    {
        tuning.addFlag(SFPSensorTuning::FLAG_FORCE_DISABLE_3DNR);
    }

    ioMap.mPathType = type;
    ioMap.addInputTuning(sID, tuning);
    for(MUINT32 idx : outList)
    {
        ioMap.addOutput(mMock->mOutBufMap.at(idx).getOut());
    }

    if(type == PATH_LARGE)
    {
        param.mSFPIOManager.addLarge(sID, ioMap);
    }
    else if(type == PATH_PHYSICAL)
    {
        param.mSFPIOManager.addPhysical(sID, ioMap);
    }
}
MVOID SFPRequestCreater::createRequest(FeaturePipeParam &param,
                                MUINT32 sensorNum,
                                std::vector<MUINT32> &genOut,
                                std::map<MUINT32, std::vector<MUINT32>> &largeOut,
                                std::map<MUINT32, std::vector<MUINT32>> &phyOut,
                                std::map<MUINT32, MBOOL> phyDiffTuning)
{
    param.setVar<MINT32>(TEST_REQ_NO_STR, reqNo);
    reqNo++;
    // General IO ( except sensor Tuning)
    SFPIOMap genIoMap;
    genIoMap.mPathType = PATH_GENERAL;
    for(MUINT32 idx : genOut)
    {
        genIoMap.addOutput(mMock->mOutBufMap.at(idx).getOut());
    }

    for(MUINT32 sID = 0; sID < sensorNum ; sID++)
    {
        MUINT32 rrzInd, imgInd;
        if(sID == 0)
        {
            rrzInd = TEST_RRZO;
            imgInd = TEST_IMGO;
        }
        else
        {
            rrzInd = TEST_RRZO_2;
            imgInd = TEST_IMGO_2;
        }
        IMetadata *halMeta = new IMetadata();
        halInMetaList.push_back(halMeta);
        IMetadata *appMeta = new IMetadata();
        appInMetaList.push_back(appMeta);
        // ==== 1. Sensor Input ====
        SFPSensorInput sensorIn;
        sensorIn.mRRZO.push_back(mMock->getImgBuf(rrzInd));
        if(largeOut[sID].size())
        {
            sensorIn.mIMGO.push_back(mMock->getImgBuf(imgInd));
        }
        // NOTE  don't set LCSO in to prevent tuning abnormal -> driver hang
        sensorIn.mHalIn = halMeta;
        sensorIn.mAppIn = appMeta;
        param.mSFPIOManager.addInput(sID , sensorIn);
        // ==== 2. Physical ====
        createNonGenIOMap(param, sID, PATH_PHYSICAL, sensorIn, phyOut[sID], phyDiffTuning[sID]);
        // ==== 3. Large ====
        createNonGenIOMap(param, sID, PATH_LARGE, sensorIn, largeOut[sID]);
        // ==== 4. General Tuning ===
        SFPSensorTuning tuning;
        if(get(sensorIn.mRRZO) != NULL)
        {
            tuning.addFlag(SFPSensorTuning::FLAG_RRZO_IN);
        }
        if(get(sensorIn.mLCSO) != NULL)
        {
            tuning.addFlag(SFPSensorTuning::FLAG_LCSO_IN);
        }
        genIoMap.addInputTuning(sID, tuning);
    }

    param.mSFPIOManager.addGeneral(genIoMap);

    // Create P2Pack
    const MUINT32 mainID = 0;
    Feature::ILog log = Feature::makeSensorLogger("SFPCommonTest", 0,mainID);
    sp<P2InfoObj> infoObj = new P2InfoObj(log);
    infoObj->addSensorInfo(log, mainID);
    infoObj->mConfigInfo.mMainSensorID = mainID;
    sp<P2DataObj> dataObj = new P2DataObj(log);
    param.mP2Pack = P2Pack(log, infoObj, dataObj);

    // Create Slave param & VarMap
    param.setVar<MUINT32>(VAR_DUALCAM_FOV_MASTER_ID, mainID);
    for(MUINT32 sID = 1 ; sID < sensorNum ; sID++)
    {
        FeaturePipeParam p;
        param.addSlaveParam(sID, p);
        param.setVar<MUINT32>(VAR_DUALCAM_FOV_SLAVE_ID, sID);
    }
}

SFPRequestCreater::~SFPRequestCreater()
{
    for(IMetadata* meta : halInMetaList)
        delete meta;
    for(IMetadata* meta : appInMetaList)
        delete meta;
}

// **************************************************************
// DualCamMFPipeCBHandlerBase
// **************************************************************
MUINT32 SFPCBHandlerBase::waitForDesiredCount(MUINT32 desiredCount, std::chrono::milliseconds timeout)
{
    const auto startWaitTime = std::chrono::system_clock::now();
    std::chrono::milliseconds waitedTime(0);
    MUINT32 currentCount(0);
    while(true)
    {
        waitedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startWaitTime);
        currentCount = getReceivedEventCount();
        if(desiredCount == currentCount)
        {
            MY_LOGD("wait for desired count completed, count: %d (%d), waitedTime: %lld (%lld) ms.", currentCount, desiredCount, waitedTime.count(), timeout.count());
            break;
        }
        else if(waitedTime.count() < timeout.count())
        {
            MY_LOGD("wait for desired count, count: %d (%d), waitedTime: %lld (%lld) ms.", currentCount, desiredCount, waitedTime.count(), timeout.count());
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        else // wait timeout
        {
            MY_LOGD("wait for desired count timeout, count: %d (%d), waitedTime: %lld (%lld) ms.", currentCount, desiredCount, waitedTime.count(), timeout.count());
            break;
        }
    }
    return currentCount;
}

const char* SFPCBHandlerBase::toString(FeaturePipeParam::MSG_TYPE eventType)
{
    switch(eventType)
    {
        case FeaturePipeParam::MSG_FRAME_DONE:
            return "MSG_FRAME_DONE";
        case FeaturePipeParam::MSG_DISPLAY_DONE:
            return "MSG_DISPLAY_DONE";
        case FeaturePipeParam::MSG_RSSO_DONE:
            return "MSG_RSSO_DONE";
        case FeaturePipeParam::MSG_FD_DONE:
            return "MSG_FD_DONE";
    }
    return "UNKNOWN";
}

MVOID SFPCBHandlerBase::dumpBuffer(MINT32 reqNo, const SFPIOMap &ioMap)
{
    std::vector<MUINT32> ids;
    ioMap.getAllSensorIDs(ids);
    MUINT32 sID = ids[0];
    for(auto&& out : ioMap.mOutList)
    {
        Utility::dumpBuffer(reqNo, get(out.mBuffers), "%d_%s_%s",sID, SFPIOMap::pathToChar(ioMap.mPathType), SFPOutput::typeToChar(out.mTargetType));
    }
}

MVOID SFPCBHandlerBase::checkBufferContent(MINT32 reqNo, const SFPIOMap &ioMap)
{
    std::vector<MUINT32> ids;
    ioMap.getAllSensorIDs(ids);
    MUINT32 sID = ids[0];
    for(auto&& out : ioMap.mOutList)
    {
        MBOOL bufEmpty = Utility::isBufferEmpty(get(out.mBuffers), mCmpBuf);
        MY_LOGD("%s_%s, buffer empty (%d)",SFPIOMap::pathToChar(ioMap.mPathType), SFPOutput::typeToChar(out.mTargetType), bufEmpty);
        if(bufEmpty)
            printf("!!!!! %s_%s, buffer not filled!!\n",SFPIOMap::pathToChar(ioMap.mPathType), SFPOutput::typeToChar(out.mTargetType));
        EXPECT_EQ(bufEmpty, MFALSE);
    }
}
//********************************************
// StressTestEventCBHandler
//********************************************
MBOOL StressTestEventCBHandler::onEvent(FeaturePipeParam::MSG_TYPE eventType, FeaturePipeParam &param)
{
    MINT32 reqNo = param.getVar<MINT32>(TEST_REQ_NO_STR, -1);
    MY_LOGD("receive req., #: %d, type: %s", reqNo, toString(eventType));
    if(eventType == FeaturePipeParam::MSG_FRAME_DONE)
    {
        MUINT32 newCount = mCompletedCount++;
        MY_LOGD("update complete count: %d (%s)", newCount, toString(eventType));
        for(auto&& io : param.mSFPIOManager.getGeneralIOs())
        {
            if(mDumpBuffer)
            {
                dumpBuffer(reqNo, io);
            }
            if(mCheckBufferContent)
            {
                checkBufferContent(reqNo, io);
            }
        }
        for(MUINT32 sID : mSensorIds)
        {
            if(mDumpBuffer)
            {
                dumpBuffer(reqNo, param.mSFPIOManager.getPhysicalIO(sID));
                dumpBuffer(reqNo, param.mSFPIOManager.getLargeIO(sID));
            }
            if(mCheckBufferContent)
            {
                checkBufferContent(reqNo, param.mSFPIOManager.getPhysicalIO(sID));
                checkBufferContent(reqNo, param.mSFPIOManager.getLargeIO(sID));
            }
        }
    }
    return OK;
}

// **************************************************************
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
