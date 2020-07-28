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

#include <memory>
#include <thread>
#include <stdio.h> // snprintf
#include <cutils/properties.h>

#define LOG_TAG "ABF/AbfAdapter"
#include <mtkcam/utils/std/Trace.h>
#ifdef SWABF_USE_PERF // Defined in Android.mk
#include <vendor/mediatek/hardware/power/1.1/IPower.h>
#include <vendor/mediatek/hardware/power/1.1/types.h>
using namespace vendor::mediatek::hardware::power::V1_1;
#endif

// custom tuning
#include <mtkcam/aaa/IIspMgr.h>
#include <camera_custom_nvram.h>

#include <mtkcam/feature/abf/AbfAdapter.h>
#include "AbfAdpUtil.h"
#include "AbfAdapterUT.h"
#include <MTKABF.h> // Something bad in MTKABF.h, always put this header last


using namespace android;
using namespace NSCam;


static const char ABF_DEBUG_PROPERTY_NAME[] = "camera.abf.adp.debug";
static const char ABF_DUMP_PATH[] = "/sdcard/camera_dump";


static int abfGetCoreNumber()
{
    return static_cast<int>(std::thread::hardware_concurrency());
}


// RAII: Encapsulate the PerfService controls
// will auto disable on destructor
class PerfServiceConfigurer
{
#ifdef SWABF_USE_PERF
private:
    android::sp<IPower> mPowerSrv;
    int mHandle;

public:

    PerfServiceConfigurer() : mPowerSrv(NULL), mHandle(-1) { }

    ~PerfServiceConfigurer() {
        disable();
    }

    void enable() { // enter performance mode
        if (mHandle != -1) {
            MY_LOGE("PerfService already enabled");
            return;
        }

        mPowerSrv = IPower::getService();
        if (mPowerSrv == NULL) {
            MY_LOGE("PerfService unavailable");
            return;
        }

        mHandle = mPowerSrv->scnReg();
        if (mHandle == -1) {
            MY_LOGE("PerfService scnReg() failed");
            return;
        }

        mPowerSrv->scnConfig(mHandle, MtkPowerCmd::CMD_SET_CPU_PERF_MODE, 1, 0, 0, 0);
        mPowerSrv->scnEnable(mHandle, 1000); // timeout for 1 sec
    }

    void disable() {
        if (mPowerSrv != NULL && mHandle != -1) {
            mPowerSrv->scnDisable(mHandle);
            mHandle = -1;
        }
    }

#else // SWABF_USE_PERF not present
    void enable() { }
    void disable() { }
#endif
};


// RAII : make sure ABFReset() will be alwasys called
class ABFResetHelper
{
private:
    MTKABF *mpAbfAlgo;

public:
    ABFResetHelper(MTKABF *algo) : mpAbfAlgo(algo) { }

    ~ABFResetHelper() {
        MRESULT ret = mpAbfAlgo->ABFReset();
        if (ret != S_ABF_OK)
            MY_LOGE("ABFReset() returned %d", ret);
    }
};


sp<AbfAdapter> AbfAdapter::createInstance(MUINT32 openId, MUINT32 debugFlags)
{
    sp<AbfAdapter> instance;

    debugFlags |= static_cast<MUINT32>(property_get_int32(ABF_DEBUG_PROPERTY_NAME, 0));

    if (debugFlags & AbfAdapter::DEBUG_UT) {
        AbfAdapterUT *ut = new AbfAdapterUT(openId, debugFlags);
        ut->setMock(MTKABFMock::createInstance());
        MY_LOGD("AbfAdapter: UT mode, openId = %d, debugFlags = %d", openId, debugFlags);

        instance = ut;
    } else {
        instance = new AbfAdapter(openId, debugFlags);
    }

    return instance;
}


AbfAdapter::AbfAdapter(MUINT32 openId, unsigned int debugFlags) :
        mOpenId(openId), mDebugFlags(debugFlags)
{
    mDebugFlags |= static_cast<MUINT32>(property_get_int32(ABF_DEBUG_PROPERTY_NAME, 0));
    MY_LOGD("AbfAdapter(openId = %u), mDebugFlags = %u", openId, mDebugFlags);

    mpAbfAlgo = MTKABF::createInstance(DRV_ABF_OBJ_IMAGE);
}


AbfAdapter::~AbfAdapter()
{
    mpAbfAlgo->destroyInstance(mpAbfAlgo);

    MY_LOGD("~AbfAdapter()");
}


void* AbfAdapter::getTuningData(const ProcessParam &param)
{
    void *nvramEntry = NULL;

    MY_LOGD("iso = %d, openId = %u", param.iso, mOpenId);

    NS3Av3::IIspMgr *ispMgr = MAKE_IspMgr();
    nvramEntry = ispMgr->getAbfTuningData(mOpenId, param.iso);

    static_assert(
            sizeof(NVRAM_CAMERA_FEATURE_SWABF_STRUCT) == sizeof(ABFTuningInfo),
            "ABFTuningInfo & NVRAM structure were not sync.");

    return nvramEntry; // Will be reinterpreted to ABFTuningInfo*
}


bool AbfAdapter::needAbf(const ProcessParam &param)
{
    void *tuningInfo = getTuningData(param);
    if (tuningInfo == NULL) {
        MY_LOGE("Tuning data is not available.");
        return false;
    }

    debugPrint(*static_cast<ABFTuningInfo*>(tuningInfo), param.iso);

    MINT32 abfSwitch;
    MRESULT ret = mpAbfAlgo->ABFFeatureCtrl(ABF_FEATURE_CHECK_ENABLE, tuningInfo, &abfSwitch);
    if (ret != S_ABF_OK) {
        MY_LOGE("[FAIL] ABF_FEATURE_CHECK_ENABLE: ret = %d", ret);
        return false;
    }

    MY_LOGD("ABF_FEATURE_CHECK_ENABLE: ret = %d", abfSwitch);

    if (mDebugFlags & DEBUG_NEVER_RUN) {
        MY_LOGD("DEBUG_NEVER_RUN was set, needAbf() return false");
        return false;
    } else if (mDebugFlags & DEBUG_ALWAYS_RUN) {
        MY_LOGD("DEBUG_ALWAYS_RUN was set, needAbf() return true");
        return true;
    }

    return (abfSwitch != 0);
}


void AbfAdapter::prepareInitInfo(const ProcessParam &param, ABFInitInfo *pInitInfo)
{
    pInitInfo->CoreNumber = abfGetCoreNumber();
    pInitInfo->pTuningInfo = reinterpret_cast<ABFTuningInfo*>(getTuningData(param));
}


bool AbfAdapter::isPaddingInLines(NSCam::IImageBuffer *pIOBuffer)
{
    MSize size = pIOBuffer->getImgSize();

    // We noly checks Y plane. Because the getPlaneBitsPerPixel() was not correctly
    // implemented. For 420 U/V plane, it also returns 8.
    const size_t plane = 0;
    size_t expectStride = size.w * pIOBuffer->getPlaneBitsPerPixel(plane) / 8;
    size_t realStride = pIOBuffer->getBufStridesInBytes(plane);
    if (realStride != expectStride) {
        MY_LOGD("Plane[%zu]: Buffer stride = %zu, expect = %zu", plane, realStride, expectStride);
        return true;
    }

    return false;
}


bool AbfAdapter::process(const ProcessParam &param, NSCam::IImageBuffer *pIOBuffer)
{
    FUNCTION_IN_OUT_LOG();
    CAM_TRACE_CALL();

    if (pIOBuffer == NULL) {
        MY_LOGE("Shall not be NULL image.");
        return false;
    }

    if (pIOBuffer->getImgFormat() != eImgFmt_I420) {
        MY_LOGE("Only accept eImgFmt_I420(%d), but %d received.", eImgFmt_I420, pIOBuffer->getImgFormat());
        return false;
    }

    if (isPaddingInLines(pIOBuffer)) { // MTKABF does not support stride
        MY_LOGE("Not support strides. Only accept no padding images.");
        return false;
    }

    debugSaveImage(pIOBuffer, DEBUG_INPUT);

    MRESULT ret;

    // initialize
    ABFInitInfo initInfo;
    prepareInitInfo(param, &initInfo);
    if (initInfo.pTuningInfo == NULL) {
        MY_LOGE("Tuning data is not available.");
        return false;
    }

    ret = mpAbfAlgo->ABFInit((void*)&initInfo);
    if (ret != S_ABF_OK) {
        MY_LOGE("[FAIL] ABFInit(): %d", ret);
        return false;
    }

    // The destructor of resetHelper can make sure ABFReset() will be always invoked
    ABFResetHelper resetHelper(mpAbfAlgo);

    // config image info
    MSize inputSize = pIOBuffer->getImgSize();
    ABFImageInfo algoImage;
    algoImage.Width = inputSize.w;
    algoImage.Height = inputSize.h;
    algoImage.ImgNum = pIOBuffer->getPlaneCount();
    algoImage.ImgFmt = ABF_IMAGE_YUV420;
    algoImage.pImg[0] = (void*)(pIOBuffer->getBufVA(0));
    algoImage.pImg[1] = (void*)(pIOBuffer->getBufVA(1));
    algoImage.pImg[2] = (void*)(pIOBuffer->getBufVA(2));

    debugPrint(algoImage);

    // Assign working buffer
    MUINT32 bufferSize;
    ret = mpAbfAlgo->ABFFeatureCtrl(ABF_FEATURE_GET_WORKBUF_SIZE, (void *)&algoImage, (void *)&bufferSize);
    MY_LOGD("Work buffer size = %u", bufferSize);
    std::unique_ptr<char[]> workBuffer(new char[bufferSize]);
    if (workBuffer.get() == nullptr) {
        MY_LOGE("[FAIL] ABF: Insufficient memory for working buffer.");
        return false;
    }

    ret = mpAbfAlgo->ABFFeatureCtrl(ABF_FEATURE_SET_WORKBUF_ADDR, (void *)workBuffer.get(), NULL);

    // pass image info
    ret = mpAbfAlgo->ABFFeatureCtrl(ABF_FEATURE_ADD_IMG, (void *)&algoImage, NULL);
    if (ret != S_ABF_OK) {
        MY_LOGE("[FAIL] ABFFeatureCtrl(ABF_FEATURE_ADD_IMG) ret = %d", ret);
        return false;
    }

    PerfServiceConfigurer perfServiceCfg;

    perfServiceCfg.enable();
    ret = mpAbfAlgo->ABFMain();
    perfServiceCfg.disable();

    if (ret != S_ABF_OK) {
        MY_LOGE("[FAIL] ABFMain() returned %d", ret);
        return false;
    }

    // get result info
    ABFResultInfo resultInfo;
    ret = mpAbfAlgo->ABFFeatureCtrl(ABF_FEATURE_GET_RESULT, NULL, (void *)&resultInfo);
    if (ret != S_ABF_OK) {
        MY_LOGE("[FAIL] ABFFeatureCtrl(ABF_FEATURE_GET_RESULT) ret = %d", ret);
        return false;
    }

    debugSaveImage(pIOBuffer, DEBUG_OUTPUT);

    pIOBuffer->syncCache(eCACHECTRL_FLUSH); // Not good here, but SwNR does so

    // mpAbfAlgo->ABFReset() will be invoked by algoResetHelper

    return true;
}


int AbfAdapter::printInt32Array(char *buffer, int bufferSize, const MINT32 *array, int len)
{
    int totalWritten = 0;

    while (len > 0 && bufferSize > 0) {
        int oneWritten = 0;

        if (len >= 4) {
            oneWritten = snprintf(buffer, bufferSize, " %d %d %d %d", array[0], array[1], array[2], array[3]);
            array += 4;
            len -= 4;
        } else if (len >= 2) {
            oneWritten = snprintf(buffer, bufferSize, " %d %d", array[0], array[1]);
            array += 2;
            len -= 2;
        } else if (len >= 1) {
            oneWritten = snprintf(buffer, bufferSize, " %d", array[0]);
            array += 1;
            len -= 1;
        }

        if (oneWritten <= 0) {
            buffer[0] = '\0';
            break;
        }

        buffer += oneWritten;
        bufferSize -= oneWritten;
        totalWritten += oneWritten;
    }

    return totalWritten;
}


void AbfAdapter::debugPrint(const ABFTuningInfo &tuningInfo, int iso)
{
    if (!(mDebugFlags & DEBUG_INTERMEDIATE))
        return;

    CAM_LOGD("TuningInfo: openId = %u, iso = %d", mOpenId, iso);

    static const int PER_PRINT = 10;
    static const int BUFFER_SIZE = PER_PRINT * 16 + 1;
    std::unique_ptr<char[]> buffer(new char[BUFFER_SIZE]);

    const MINT32 *fieldPtr = reinterpret_cast<const MINT32*>(&tuningInfo);
    int len = sizeof(ABFTuningInfo) / sizeof(MINT32);
    for (int i = 0; len > 0; i += PER_PRINT) {
        int onePrintLen = (PER_PRINT < len) ? PER_PRINT : len;
        printInt32Array(buffer.get(), BUFFER_SIZE, fieldPtr, onePrintLen);
        CAM_LOGD("Tuning[%2d]%s", i, buffer.get());
        fieldPtr += onePrintLen;
        len -= onePrintLen;
    }
}


void AbfAdapter::debugPrint(const ABFImageInfo &imageInfo)
{
    if (!(mDebugFlags & DEBUG_INTERMEDIATE))
        return;

    CAM_LOGD("ImageInfo: [0] = %p, [1] = %p, [2] = %p",
            imageInfo.pImg[0], imageInfo.pImg[1], imageInfo.pImg[2]);
    CAM_LOGD("ImageInfo: ImgNum = %d, ImgFmt = %d, Size = (%d, %d)",
            imageInfo.ImgNum, imageInfo.ImgFmt, imageInfo.Width, imageInfo.Height);
}


void AbfAdapter::debugSaveImage(NSCam::IImageBuffer *pBuffer, unsigned int source)
{
    if (!(mDebugFlags & source))
        return;

    const char *tag = "temp";
    if (source == DEBUG_INPUT)
        tag = "in";
    else if (source == DEBUG_OUTPUT)
        tag = "out";

    static const int ABF_DUMP_PATH_MAX = 256;
    std::unique_ptr<char[]> path(new char[ABF_DUMP_PATH_MAX]);

    MSize size = pBuffer->getImgSize();
    int n = snprintf(path.get(), ABF_DUMP_PATH_MAX, "%s/abf_%u_%s_%dx%d.yuv",
            ABF_DUMP_PATH, mOpenId, tag, size.w, size.h);

    if (n > 0) {
        if (!NSCam::Utils::makePath(ABF_DUMP_PATH, 0660)) {
            MY_LOGW("makePath(%s) failed", ABF_DUMP_PATH);
            return;
        }

        pBuffer->saveToFile(path.get());
    }
}



