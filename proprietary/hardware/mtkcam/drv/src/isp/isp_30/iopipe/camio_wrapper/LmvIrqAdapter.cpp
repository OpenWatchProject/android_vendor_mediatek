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


#define LOG_TAG     "LmvIrqAdapter"

#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#include <drv/isp_drv.h>
#include <imageio/ispio_stddef.h>
#include "CamIoWrapperUtils.h"
#include "LmvIrqAdapter.h"


#define LOG_FUNCTION_LIFE() \
    NSCam::NSIoPipe::NSCamIOPipe::Wrapper::LogLife<NSCam::NSIoPipe::NSCamIOPipe::Wrapper::LMV_IRQ_ADAPTER> \
    _log_(__func__, NSCam::NSIoPipe::NSCamIOPipe::Wrapper::gDebugFlag)


using namespace android;
using namespace NSCam;


namespace NSCam {
namespace NSIoPipe {
namespace NSCamIOPipe {
namespace Wrapper {


static const char *MODULE_NAME = "LmvIrqAdapter";


// RAII to manage EISO handle
class EisoHandle
{
public:
    EisoHandle(INormalPipeIsp3 *normalPipeIsp3) :
        mNormalPipeIsp3(normalPipeIsp3), mHandle(0)
    {
    }

    ~EisoHandle() {
        if (mHandle != 0) {
            if (!mNormalPipeIsp3->sendCommand(
                    NSImageioIsp3::NSIspio::EPIPECmd_RELEASE_MODULE_HANDLE,
                    mHandle, reinterpret_cast<MINTPTR>(MODULE_NAME), -1))
            {
                WRP_ERR("EPIPECmd_RELEASE_MODULE_HANDLE failed");
            }
        }
    }

    MINTPTR get() {
        if (mHandle == 0) {
            if (mNormalPipeIsp3->sendCommand(
                    NSImageioIsp3::NSIspio::EPIPECmd_GET_MODULE_HANDLE, NSImageioIsp3::NSIspio::EModule_EISO,
                    reinterpret_cast<MINTPTR>(&mHandle), reinterpret_cast<MINTPTR>(MODULE_NAME)))
            {
                WRP_DBG("mHandle = 0x%" PRIxPTR, mHandle);
            } else {
                WRP_ERR("EPIPECmd_GET_MODULE_HANDLE(EModule_EISO) failed");
                mHandle = 0;
            }
        }
        return mHandle;
    }

    bool done() {
        return (mNormalPipeIsp3->sendCommand(
            NSImageioIsp3::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, mHandle, -1, 1) != MFALSE);
    }

private:
    INormalPipeIsp3 *mNormalPipeIsp3;
    MINTPTR mHandle;
};


// RAII to manage SGG2 handle
class Sgg2Handle
{
public:
    Sgg2Handle(INormalPipeIsp3 *normalPipeIsp3) :
        mNormalPipeIsp3(normalPipeIsp3), mHandle(0)
    {
    }

    ~Sgg2Handle() {
        if (mHandle != 0) {
            if (!mNormalPipeIsp3->sendCommand(
                    NSImageioIsp3::NSIspio::EPIPECmd_RELEASE_MODULE_HANDLE,
                    mHandle, reinterpret_cast<MINTPTR>(MODULE_NAME), -1))
            {
                WRP_ERR("EPIPECmd_RELEASE_MODULE_HANDLE failed");
            }
        }
    }

    MINTPTR get() {
        if (mHandle == 0) {
            if (mNormalPipeIsp3->sendCommand(
                    NSImageioIsp3::NSIspio::EPIPECmd_GET_MODULE_HANDLE, NSImageioIsp3::NSIspio::EModule_SGG2,
                    reinterpret_cast<MINTPTR>(&mHandle), reinterpret_cast<MINTPTR>(MODULE_NAME)))
            {
                WRP_DBG("mHandle = 0x%" PRIxPTR, mHandle);

            } else {
                WRP_ERR("EPIPECmd_GET_MODULE_HANDLE(EModule_SGG2) failed");
                mHandle = 0;
            }
        }
        return mHandle;
    }

    bool done() {
        return (mNormalPipeIsp3->sendCommand(
            NSImageioIsp3::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, mHandle, -1, 1) != MFALSE);
    }

private:
    INormalPipeIsp3 *mNormalPipeIsp3;
    MINTPTR mHandle;
};

const int LmvIrqAdapter::LMV_DATA_SIZE = 256; // bytes
const int LmvIrqAdapter::LMV_BUFFER_NUM = 32;
const int LmvIrqAdapter::SGG2_PGN   = 0x10;
const int LmvIrqAdapter::SGG2_GMRC1 = 0x63493527;
const int LmvIrqAdapter::SGG2_GMRC2 = 0x00FFBB88;

LmvIrqAdapter::LmvIrqAdapter() :
    mNormalPipeIsp3(NULL),
    mIspDrv(NULL),
    mIspDrvUserKey(0),
    mDataBufferHeap(NULL),
    mDataBufferSpace(NULL),
    mThreadIsRunning(false)
{
}


LmvIrqAdapter::~LmvIrqAdapter()
{
    if (isRunning())
        stop();
    uninitDataBuffers();
}


bool LmvIrqAdapter::init(MUINT32 sensorIndex, INormalPipeIsp3 *normalPipeIsp3, IspDrv *ispDrv)
{
    WRP_DBG("SensorIndex = %d; %p, %p", sensorIndex, normalPipeIsp3, ispDrv);

    if (normalPipeIsp3 == NULL || ispDrv == NULL)
        return false;

    mNormalPipeIsp3 = normalPipeIsp3;
    mIspDrv = ispDrv;
    mIspDrvUserKey = mIspDrv->registerIrq(LOG_TAG);

    if (!initDataBuffers())
        return false;

    // Must set before first enque
    if (!initEisoReg())
        return false;

    if (!initSgg2Reg())
        return false;

    return true;
}


bool LmvIrqAdapter::start()
{
    return (run(LOG_TAG) == OK);
}


void LmvIrqAdapter::stop()
{
    LOG_FUNCTION_LIFE();

    requestExit();

    // We can not use mutex to protect waitIrq & flushIrq, because
    // the only way to interrupt waitIrq is flushIrq.
    // Thus we can not guarantee the order of waitIrq and the flushIrq
    // due to stop. If waitIrq happens first, it's good. But if not,
    // waitIrq will enter a long wait. We must resend flushIrq periodically
    // to let waitIrq can be exit more rapidly.
    Mutex::Autolock lock(mBufferLock);
    while (mThreadIsRunning.load(std::memory_order_relaxed)) {
        interruptWaiting();
        mThreadStopCond.waitRelative(mBufferLock, 1000000LL);
    }

    join();
}


status_t LmvIrqAdapter::readyToRun()
{
    return OK;
}


bool LmvIrqAdapter::initDataBuffers()
{
    // To create LMV data buffers and push them into mEmptyBufferPool

    if (mDataBufferSpace != NULL) {
        WRP_WRN("mDataBufferSpace already created");
        return true;
    }

    size_t totalSize = static_cast<size_t>(LMV_DATA_SIZE * LMV_BUFFER_NUM);
    IImageBufferAllocator::ImgParam imgParam(totalSize, 0);

    mDataBufferHeap = IIonImageBufferHeap::create(LOG_TAG, imgParam);
    if (mDataBufferHeap == NULL) {
        WRP_ERR("mDataBufferHeap create failed");
        return false;
    }

    mDataBufferSpace = mDataBufferHeap->createImageBuffer();
    if (mDataBufferSpace == NULL) {
        WRP_ERR("mDataBufferSpace create failed");
        mDataBufferHeap = NULL;
        return false;
    }

    MUINT const usage = (GRALLOC_USAGE_SW_READ_OFTEN |
                        GRALLOC_USAGE_HW_CAMERA_READ |
                        GRALLOC_USAGE_HW_CAMERA_WRITE);
    if (!(mDataBufferSpace->lockBuf(LOG_TAG, usage))) {
        WRP_ERR("mDataBufferSpace lock failed");
        mDataBufferSpace = NULL;
        mDataBufferHeap = NULL;
        return false;
    }

    MUINTPTR va = mDataBufferSpace->getBufVA(0);
    MUINTPTR pa = mDataBufferSpace->getBufPA(0);
    WRP_DBG("mDataBufferSpace created on (va,pa) = (%p,%p)", reinterpret_cast<void*>(va), reinterpret_cast<void*>(pa));

    Mutex::Autolock lock(mBufferLock);
    EisoData buffer;
    for (int i = 0; i < LMV_BUFFER_NUM; i++) {
        buffer.va = va;
        buffer.pa = pa;
        buffer.timeStamp = 0;

        mEmptyBufferPool.push_back(buffer);

        va += LMV_DATA_SIZE;
        pa += LMV_DATA_SIZE;
    }

    return true;
}


void LmvIrqAdapter::uninitDataBuffers()
{
    Mutex::Autolock lock(mBufferLock);

    mEmptyBufferPool.clear();
    mLmvDataList.clear();
    if (mDataBufferSpace != NULL)
        mDataBufferSpace->unlockBuf(LOG_TAG);
    mDataBufferSpace = NULL;
    mDataBufferHeap= NULL;
}


bool LmvIrqAdapter::acquireBuffer(EisoData &buf)
{
    Mutex::Autolock lock(mBufferLock);

    if (!mEmptyBufferPool.empty()) {
        buf = *mEmptyBufferPool.begin();
        mEmptyBufferPool.erase(mEmptyBufferPool.begin());
        return true;
    } else if (!mLmvDataList.empty()) {
        // Get oldest
        buf = *mLmvDataList.begin();
        mLmvDataList.erase(mLmvDataList.begin());
        return true;
    }

    WRP_ERR("acquireOneBuffer() fail: buffer leaked!");

    return false;
}


void LmvIrqAdapter::releaseBuffer(EisoData &buf)
{
    buf.timeStamp = 0;

    Mutex::Autolock lock(mBufferLock);
    mEmptyBufferPool.push_back(buf);
    buf.clear();
}


void LmvIrqAdapter::pushToDataList(EisoData &buf)
{
    WRP_DBG("EisoData: pa = 0x%" PRIxPTR ", va = 0x%" PRIxPTR "; timeStamp = %" PRId64,
        buf.pa, buf.va, buf.timeStamp);

    Mutex::Autolock lock(mBufferLock);
    mLmvDataList.push_back(buf);
    buf.clear();
    mDataCond.broadcast();
}


bool LmvIrqAdapter::threadLoop()
{
    mThreadIsRunning.store(true, std::memory_order_release);

    initIrq();

    // This thread is not real time. Current design may miss data if system busy
    // We ignore the limitation here as old design(legacy) don't care as well
    EisoData buf;
    if (acquireBuffer(buf)) {
        if (!clearP1Done() ||
            !setBufferAddressToReg(buf) ||
            !waitForP1Done(buf.timeStamp) )
        {
            WRP_ERR("Set LMVO address failed");
        }
        else
        {
            // HW will write to last buffer in current frame
            // Hence we extract last buffer and marked as current timestamp
            EisoData lastBuf;
            while (true) {
                lastBuf = buf;
                buf.clear();

                if (!acquireBuffer(buf))
                    break;

                if (!setBufferAddressToReg(buf))
                    break;

                if (exitPending())
                    break;

                // Marked as current timestamp
                if (!waitForP1Done(lastBuf.timeStamp))
                    break;

                pushToDataList(lastBuf);
                lastBuf.clear();
            }

            if (lastBuf.isValid()) {
                releaseBuffer(lastBuf);
            }
        }

        if (buf.isValid()) {
            releaseBuffer(buf);
        }
    }

    Mutex::Autolock lock(mBufferLock);
    mDataCond.broadcast();
    mThreadIsRunning.store(false, std::memory_order_relaxed);
    mThreadStopCond.broadcast();

    return false; // stop thread
}


inline void LmvIrqAdapter::fillWaitIrq(
    ISP_DRV_WAIT_IRQ_STRUCT &waitIrq,
    ISP_DRV_IRQ_CLEAR_ENUM clear,
    ISP_DRV_IRQ_TYPE_ENUM type,
    MUINT32 status,
    MUINT32 timeout)
{
    waitIrq.Clear      = clear;
    waitIrq.Type       = type;
    waitIrq.Status     = status;
    waitIrq.Timeout    = timeout;
    waitIrq.UserInfo.Type    = type;
    waitIrq.UserInfo.Status  = status;
    waitIrq.UserInfo.UserKey = mIspDrvUserKey;
    waitIrq.SpecUser   = ISP_DRV_WAITIRQ_SPEUSER_EIS;
}


bool LmvIrqAdapter::initIrq()
{
    LOG_FUNCTION_LIFE();

    // Clear P1Done
    ISP_DRV_WAIT_IRQ_STRUCT waitIrq;
    fillWaitIrq(
        waitIrq,
        ISP_DRV_IRQ_CLEAR_STATUS,
        ISP_DRV_IRQ_TYPE_INT_P1_ST,
        CAM_CTL_INT_P1_STATUS_PASS1_DON_ST,
        401);

    if (!mIspDrv->waitIrq(&waitIrq)) {
        return false;
    }

    return true;
}



bool LmvIrqAdapter::interruptWaiting()
{
    LOG_FUNCTION_LIFE();

    bool result = true;

    ISP_DRV_WAIT_IRQ_STRUCT waitIrq;

    fillWaitIrq(
        waitIrq,
        ISP_DRV_IRQ_CLEAR_STATUS,
        ISP_DRV_IRQ_TYPE_INT_P1_ST,
        CAM_CTL_INT_P1_STATUS_VS1_INT_ST);

    if (!mIspDrv->flushIrq(waitIrq)) {
        result = false;
    }

    fillWaitIrq(
        waitIrq,
        ISP_DRV_IRQ_CLEAR_STATUS,
        ISP_DRV_IRQ_TYPE_INT_P1_ST,
        CAM_CTL_INT_P1_STATUS_PASS1_DON_ST);

    if (!mIspDrv->flushIrq(waitIrq)) {
        result= false;
    }

    return result;
}


bool LmvIrqAdapter::initEisoReg()
{
    LOG_FUNCTION_LIFE();

    EisoHandle eisoHandle(mNormalPipeIsp3);
    auto handle = eisoHandle.get();

    if (handle == 0) {
        WRP_ERR("EISO handle is not available");
        return false;
    }

    if (handle != UT_MODULE_HANDLE) {
        IOPIPE_SET_MODUL_REG(handle, CAM_EISO_BASE_ADDR, mDataBufferSpace->getBufPA(0));
        IOPIPE_SET_MODUL_REG(handle, CAM_EISO_XSIZE, LMV_DATA_SIZE - 1);
    }

    if (!eisoHandle.done()) {
        WRP_ERR("EISO config failed");
        return false;
    }

    if (!mNormalPipeIsp3->sendCommand(
            NSImageioIsp3::NSIspio::EPIPECmd_SET_MODULE_EN, NSImageioIsp3::NSIspio::EModule_EISO, MTRUE, -1))
    {
        WRP_ERR("Enable EISO failed");
        return false;
    }

    return true;
}


bool LmvIrqAdapter::initSgg2Reg()
{
    LOG_FUNCTION_LIFE();

    Sgg2Handle sgg2Handle(mNormalPipeIsp3);
    auto handle = sgg2Handle.get();

    if (handle == 0) {
        WRP_ERR("SGG2 handle is not available");
        return false;
    }

    if (handle != UT_MODULE_HANDLE) {
        IOPIPE_SET_MODUL_REG(handle, CAM_SGG2_PGN, SGG2_PGN);
        IOPIPE_SET_MODUL_REG(handle, CAM_SGG2_GMRC_1, SGG2_GMRC1);
        IOPIPE_SET_MODUL_REG(handle, CAM_SGG2_GMRC_2, SGG2_GMRC2);
    }

    if (!sgg2Handle.done()) {
        WRP_ERR("SGG2 config failed");
        return false;
    }

    if (!mNormalPipeIsp3->sendCommand(
            NSImageioIsp3::NSIspio::EPIPECmd_SET_MODULE_EN, NSImageioIsp3::NSIspio::EModule_SGG2, MTRUE, -1))
    {
        WRP_ERR("Enable SGG2 failed");
        return false;
    }

    return true;
}


bool LmvIrqAdapter::setBufferAddressToReg(const EisoData &buf)
{
    LOG_FUNCTION_LIFE();

    // NOTE: This works on next p1Done, not current frame
    EisoHandle eisoHandle(mNormalPipeIsp3);

    bool ret = false;
    int numOfFailed = 0;
    while (!exitPending() &&
            numOfFailed < 3)
    {
        auto handle = eisoHandle.get();

        if (handle == 0) {
            WRP_ERR("EISO handle is not available");
            numOfFailed++;
            continue;
        }

        if (handle != UT_MODULE_HANDLE) {
            IOPIPE_SET_MODUL_REG(handle, CAM_EISO_BASE_ADDR, buf.pa);
        }

        if (!eisoHandle.done()) {
            WRP_ERR("EISO config failed");
            numOfFailed++;
            continue;
        }

        ret = true;
        break;
    }

    return ret;
}


bool LmvIrqAdapter::waitForP1Done(MINT64 &timeStamp)
{
    LOG_FUNCTION_LIFE();
    // P1 done may happen before our wait, so we use CLEAR_NONE here
    ISP_DRV_WAIT_IRQ_STRUCT waitIrq;
    fillWaitIrq(
        waitIrq,
        ISP_DRV_IRQ_CLEAR_NONE,
        ISP_DRV_IRQ_TYPE_INT_P1_ST,
        CAM_CTL_INT_P1_STATUS_PASS1_DON_ST,
        501); // Magic! Copied from legacy code

    int numOfFailed = 0;
    bool ret = false;
    while (!exitPending() &&
            numOfFailed < 3)
    {
        if (mIspDrv->waitIrq(&waitIrq)) {
            timeStamp =
                (waitIrq.EisMeta.tLastSOF2P1done_sec * 1000000000LL + waitIrq.EisMeta.tLastSOF2P1done_usec * 1000LL);

            ret = true;
            break;
        } else {
            timeStamp = 0;
            numOfFailed++;
        }
    }

    if (!clearP1Done()) {
        return false;
    }

    return ret;
}


bool LmvIrqAdapter::clearP1Done()
{
    // Clear next p1Done, make sure the next p1Done is waiting for corresponding next frame
    ISP_DRV_WAIT_IRQ_STRUCT waitIrq;
    fillWaitIrq(
        waitIrq,
        ISP_DRV_IRQ_CLEAR_STATUS,
        ISP_DRV_IRQ_TYPE_INT_P1_ST,
        CAM_CTL_INT_P1_STATUS_PASS1_DON_ST,
        10);

    bool ret = false;
    int numOfFailed = 0;
    while (!exitPending() &&
            numOfFailed < 3)
    {
        if (mIspDrv->waitIrq(&waitIrq)) {
            return true;
        } else {
            numOfFailed++;
        }
    }

    return false;
}


bool LmvIrqAdapter::tryToGetLmvData(void *bufVa, MINT32 bufSize, MINT64 timeStamp)
{
    // NOTE: mBufferLock should be locked

    while (!mLmvDataList.empty()) {
        auto frontItem = mLmvDataList.begin();

        if (frontItem->timeStamp == timeStamp) {
            memcpy(bufVa, reinterpret_cast<void*>(frontItem->va), bufSize);
            mEmptyBufferPool.push_back(*frontItem);
            mLmvDataList.erase(frontItem);
            WRP_DBG("%p at %" PRId64 " is available", bufVa, timeStamp);
            return true;
        } else if (frontItem->timeStamp < timeStamp) {
            // discard
            mEmptyBufferPool.push_back(*frontItem);
            mLmvDataList.erase(frontItem);
        } else {
            break;
        }
    }

    WRP_DBG("%p at %" PRId64 " is UNAVAILABLE", bufVa, timeStamp);
    return false;
}


bool LmvIrqAdapter::getLmvData(void *bufVa, MINT32 bufSize, MINT64 timeStamp, MUINT timeoutInMs)
{
    LOG_FUNCTION_LIFE();
    CAM_TRACE_CALL();

    if (bufVa == NULL)
        return false;

    if (bufSize < LMV_DATA_SIZE) {
        return false;
    } else if (bufSize > LMV_DATA_SIZE) {
        bufSize = LMV_DATA_SIZE;
    }

    Mutex::Autolock lock(mBufferLock);

    if (tryToGetLmvData(bufVa, bufSize, timeStamp))
        return true;

    // Data is not available yet, wait for given time
    if (timeoutInMs > 0) {
        mDataCond.waitRelative(mBufferLock, static_cast<nsecs_t>(timeoutInMs) * 1000000LL);

        // Try again
        if (tryToGetLmvData(bufVa, bufSize, timeStamp))
            return true;
    }

    return false;
}


};
};
};
};

