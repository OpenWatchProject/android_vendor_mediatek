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

#define LOG_TAG     "NormalPipeWrapper"

#include <cinttypes>
#include <cstring>
#include <algorithm>
#include <time.h>
#include <utils/AndroidThreads.h>
#include <cutils/properties.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/std/Misc.h>
#include <iopipe/CamIO/PortMap.h>
#include <mtkcam/drv/iopipe/Event/IoPipeEvent.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/std/ULog.h>
#include "CamIoWrapperUtils.h"
#include "NormalPipeWrapper.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_DRV_NORMAL_PIPE);


#define LOG_TAG_LIFE(tag) \
    NSCam::NSIoPipe::NSCamIOPipe::Wrapper::LogLife<NSCam::NSIoPipe::NSCamIOPipe::Wrapper::NORMAL_PIPE_WRAPPER> \
    _log_(tag, NSCam::NSIoPipe::NSCamIOPipe::Wrapper::gDebugFlag)

#define LOG_FUNCTION_LIFE() LOG_TAG_LIFE(__func__)


using namespace android;
using namespace NSCam;
using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe::Wrapper;


DebugFlag NSCam::NSIoPipe::NSCamIOPipe::Wrapper::gDebugFlag = 0;


Mutex NormalPipeWrapper::sInstanceLock;
NormalPipeWrapper* NormalPipeWrapper::spInstance[EPIPE_Sensor_RSVD] = { NULL, NULL, NULL, NULL };
int NormalPipeWrapper::sNextInstanceId = 0;


INormalPipe *NormalPipeWrapper::createInstance(
    MUINT32 sensorIndex, char const* szCallerName, MUINT32 apiVersion,
    sp<IDriverFactory> driverFactory)
{
    if (szCallerName == NULL)
        szCallerName = "unknown";

    if (sensorIndex >= EPIPE_Sensor_RSVD) {
        CAM_ULOGME("NormalPipeWrapper::createInstance(%u, %s, 0x%x) invalid sensor index",
            sensorIndex, szCallerName, apiVersion);
        return NULL;
    }

    Mutex::Autolock l(sInstanceLock);

    LOG_FUNCTION_LIFE();
    CAM_TRACE_CALL();

    NormalPipeWrapper *pInstance = spInstance[sensorIndex];
    if (pInstance == NULL) {
        pInstance = new NormalPipeWrapper(sNextInstanceId, sensorIndex, driverFactory);
        sNextInstanceId++;
        spInstance[sensorIndex] = pInstance;
        gDebugFlag = property_get_int32("vendor.debug.normalpipe.wrapper", DEBUG_DETAILS);
        pInstance->mDebugFlag = gDebugFlag;
    }
    pInstance->mUsersCreated++;

    WRP_DBG("[ID=%d] %s %s; sensorIndex = %u, apiVersion = %u, mUsersCreated+1 = %d, debugFlag = 0x%x",
        pInstance->mInstanceId,
        (pInstance->mUsersCreated == 1 ? "CREATE" : "New user"), szCallerName,
        sensorIndex, apiVersion, pInstance->mUsersCreated, gDebugFlag);

    if (pInstance->mDebugFlag & DEBUG_USER) {
        CAM_ULOGMD("[%s] mUsersCreated+1 = %d, call stack:", __func__, pInstance->mUsersCreated);
        NSCam::Utils::dumpCallStack(LOG_TAG);
    }

    return pInstance;
}


MVOID NormalPipeWrapper::destroyInstance(char const* szCallerName)
{
    if (szCallerName == NULL)
        szCallerName = "unknown";

    Mutex::Autolock l(sInstanceLock);

    LOG_FUNCTION_LIFE();
    CAM_TRACE_CALL();

    mUsersCreated--;

    if (mDebugFlag & DEBUG_USER) {
        CAM_ULOGMD("[%s] mUsersCreated-1 = %d, call stack:", __func__, mUsersCreated);
        NSCam::Utils::dumpCallStack(LOG_TAG);
    }

    WRP_DBG("%s %s; mSensorIndex = %u, mUsersCreated-1 = %d",
        (mUsersCreated == 0 ? "DESTROY" : "Exit user"), szCallerName,
        mSensorIndex, mUsersCreated);

    if (mUsersCreated == 0) {
        spInstance[mSensorIndex] = NULL;
        delete this;
    }
}


NormalPipeWrapper::NormalPipeWrapper(
        int instanceId,
        MUINT32 sensorIndex,
        sp<IDriverFactory> &driverFactory) :
    mInstanceId(instanceId),
    mSensorIndex(sensorIndex),
    mUsersCreated(0),
    mUsersInited(0),
    mState(STATE_NONE),
    mIsActive(false),
    mDebugFlag(0),
    mIsDummyInfoInited(false),
    mDummyBufferImgo("Dummy_imgo", 0),
    mDummyBufferRrzo("Dummy_rrzo", 0),
    mInitMagicNum(0),
    mIsLmvEnabled(false),
    mLmvAdapter(NULL),
    mDriverFactory(driverFactory),
    mNormalPipeIsp3(NULL),
    mIspDrv(NULL),
    mIspDrvUserKey(0)
{
    for (int i = 0; i < VERI_SLOT_SIZE; i++)
        mVeriSlot[i] = NULL;
}


NormalPipeWrapper::~NormalPipeWrapper()
{
    bool invokeUninit = false;
    {
        Mutex::Autolock lock(mOpMutex);
        if (mUsersInited > 0) {
            WRP_ERR("Not uninit: %d", mUsersInited);
            mUsersInited = 1;
            invokeUninit = true;
        }
    }

    if (invokeUninit) {
        uninit();
    }

    if (mNormalPipeIsp3 != NULL) {
        mNormalPipeIsp3->destroyInstance(LOG_TAG);
        mNormalPipeIsp3 = NULL;
    }

    // Make sure again
    mVeriSlot[mInstanceId % VERI_SLOT_SIZE] = NULL;
}


inline bool NormalPipeWrapper::checkState(const char *tag, State shouldSet, State shouldClear)
{
    if ((mState & shouldSet) != shouldSet || (mState & shouldClear) != 0) {
        WRP_ERR("[%s] State not match: 0x%x, shouldSet = 0x%x, shouldClear = 0x%x", tag, mState, shouldSet, shouldClear);
        return false;
    }

    return true;
}


inline void NormalPipeWrapper::setState(const char *tag, State toSet, State toClear)
{
    State prev = mState;

    mState |= toSet;
    mState &= ~toClear;

    if (prev != mState) {
        WRP_DBG("[%s] State changed: 0x%x -> 0x%x", tag, prev, mState);
    }
}


inline void NormalPipeWrapper::logQueueEvent(const char *event, BufInfoIsp3 *imgoBufInfo, BufInfoIsp3 *rrzoBufInfo)
{
    if (mDebugFlag & Wrapper::DEBUG_QUEUE) {
        MUINT32 magicNum = 0;
        IImageBuffer *rrzoBuffer = NULL;
        if (rrzoBufInfo != NULL) {
            rrzoBuffer = rrzoBufInfo->mBuffer;
            magicNum = rrzoBufInfo->mMetaData.mMagicNum_hal;
        }
        IImageBuffer *imgoBuffer = NULL;
        MUINT32 rawType = 0;
        if (imgoBufInfo != NULL) {
            imgoBuffer = imgoBufInfo->mBuffer;
            magicNum = imgoBufInfo->mMetaData.mMagicNum_hal;
            rawType = imgoBufInfo->mMetaData.mRawType;
        }

        CAM_ULOGMD("%s: magicNum = %u, imgo = %p(raw=%u), rrzo = %p; sensor = %u",
            event, magicNum, imgoBuffer, rawType, rrzoBuffer, mSensorIndex);
    }
}


MBOOL NormalPipeWrapper::start()
{
    Mutex::Autolock lock(mOpMutex);

    LOG_FUNCTION_LIFE();
    CAM_TRACE_CALL();

    if (!checkState("start", STATE_INITED, STATE_NONE))
        return MFALSE;

    if (mStatistics.numOfUserEnqued == 0) {
        // User must enque one before start
        CAM_LOGA("P1Node did not enque before invoke start");
        return MFALSE;
    }

    if (mStatistics.numOfUserEnqued < 3) {
        // ISP3 need 3 normal enques before start
        int numOfCompensation = 3 - mStatistics.numOfUserEnqued;
        for (int i = 0; i < numOfCompensation; i++) {
            // Compensate remaining normal enques
            compensatorillyEnque(mInitMagicNum);
        }
    }

    mIsActive = true;

    // start
    if (!mNormalPipeIsp3->start()) {
        WRP_ERR("NormalPipe(ISP3) start failed, numOfUserEnqued = %d, numOfCompEnqued = %d, numOfDummysGen = %d",
            mStatistics.numOfUserEnqued, mStatistics.numOfCompEnqued, mStatistics.numOfDummysGen);
        return MFALSE;
    }

    if (mDequeThread == NULL) {
        mDequeThread = new DequeThread(*this);
        mDequeThread->run(LOG_TAG);
    }

    if (mLmvAdapter != NULL) {
        if (!mLmvAdapter->start()) {
            WRP_ERR("LMV start failed");
            mLmvAdapter = NULL;
        }
    }

    setState("start", STATE_STARTED, STATE_STOPPED);

    return MTRUE;
}


MBOOL NormalPipeWrapper::stop(MBOOL bNonblocking)
{
    mIsActive = false;

    Mutex::Autolock lock(mOpMutex);

    LOG_FUNCTION_LIFE();
    CAM_TRACE_CALL();

    if (!checkState("stop", STATE_INITED | STATE_STARTED, STATE_NONE))
        return MFALSE;

    if (bNonblocking)
        WRP_WRN("bNonblocking(MTRUE) is not supported, should be MFALSE");

    return stopInternal();
}


MBOOL NormalPipeWrapper::stopInternal()
{
    CAM_TRACE_CALL();

    WRP_DBG("stopInternal mState = 0x%x", mState);

    mIsActive = false;
    if (mLmvAdapter != NULL) {
        mLmvAdapter->requestExit();
    }
    mDequeThread->requestExit();
    abortDequeThread();
    MBOOL stopResult = MFALSE;
    {
        Mutex::Autolock lock(mDequeStopMutex);
        stopResult = mNormalPipeIsp3->stop();
    }

    if (!stopResult) {
        WRP_ERR("NormalPipe(ISP3) stop failed");
        return MFALSE;
    }

    if (mLmvAdapter != NULL) {
        mLmvAdapter->stop();
        mLmvAdapter = NULL;
    }

    if (mDequeThread != NULL) {
        {
            Mutex::Autolock lock(mDequeListMutex);
            mDequeCond.broadcast();
        }

        mDequeThread->join();
        mDequeThread = NULL;
    }

    mDequeCond.broadcast();

    setState("stop", STATE_STOPPED, STATE_STARTED);

    return MTRUE;
}


inline void NormalPipeWrapper::cloneBufInfo(const BufInfoIsp3 &src, BufInfo &dest)
{
    auto transPortId = [] (const NSCam::NSIoPipeIsp3::PortID &portId) {
        if (portId == NSCam::NSIoPipeIsp3::PORT_IMGO) return PORT_IMGO;
        else if (portId == NSCam::NSIoPipeIsp3::PORT_RRZO) return PORT_RRZO;
        WRP_AST_IF(false, "No translation of port");
        return PortID();
    };

    // Other than below fields will be copied from user enqued data
    dest.mPortID = transPortId(src.mPortID);
    dest.mBuffer = src.mBuffer;
    dest.mMetaData.mCrop_s = src.mMetaData.mCrop_s;
    dest.mMetaData.mDstSize = src.mMetaData.mDstSize;
    dest.mMetaData.mCrop_d = src.mMetaData.mCrop_d;
    dest.mMetaData.mTransform = src.mMetaData.mTransform;
    dest.mMetaData.mMagicNum_hal = src.mMetaData.mMagicNum_hal;
    dest.mMetaData.mMagicNum_tuning = src.mMetaData.mMagicNum_tuning;
    dest.mMetaData.mRawType = src.mMetaData.mRawType;
    dest.mMetaData.mTimeStamp = src.mMetaData.mTimeStamp;
    dest.mMetaData.mPrivateData = src.mMetaData.mPrivateData;
    dest.mMetaData.mPrivateDataSize = src.mMetaData.mPrivateDataSize;
    dest.mMetaData.mHighlightData = MFALSE;
    dest.mMetaData.eIQlv = eCamIQ_L;
    dest.mSize = src.mSize;
    dest.mVa = src.mVa;
    dest.mPa = src.mPa;
}


inline NSCam::NSIoPipeIsp3::PortID NormalPipeWrapper::transPortIdToIsp3(const PortID &portId)
{
    if (portId == PORT_IMGO)
        return NSCam::NSIoPipeIsp3::PORT_IMGO;
    else if (portId == PORT_RRZO)
        return NSCam::NSIoPipeIsp3::PORT_RRZO;
    else if (portId == PORT_CAMSV_IMGO)
        return NSCam::NSIoPipeIsp3::PORT_CAMSV_IMGO;
    else if (portId == PORT_CAMSV2_IMGO)
        return NSCam::NSIoPipeIsp3::PORT_CAMSV2_IMGO;

    return NSCam::NSIoPipeIsp3::PortID();
}


void NormalPipeWrapper::waitForSOFIfDropped()
{
    LOG_FUNCTION_LIFE();
    CAM_TRACE_CALL();

    int errorTolerance = 2;
    int waitTolerance = 5;

    // to ensure that status is not in drop status
    while (mIsActive.load(std::memory_order_relaxed) &&
           errorTolerance >= 0 &&
           waitTolerance >= 0)
    {
        MINT32 status = NSCam::NSIoPipeIsp3::NSCamIOPipe::_normal_status;

        if (!mNormalPipeIsp3->sendCommand(
                NSImageioIsp3::NSIspio::EPIPECmd_GET_CUR_FRM_STATUS, reinterpret_cast<MINTPTR>(&status), 0, 0))
        {
            WRP_ERR("EPIPECmd_GET_CUR_FRM_STATUS failed");
            break;
        }

        if (status == NSCam::NSIoPipeIsp3::NSCamIOPipe::_drop_frame_status ||
            status == NSCam::NSIoPipeIsp3::NSCamIOPipe::_1st_enqloop_status)
        {
            MINT32 tgIndex = mNormalPipeIsp3->getSensorTg(mSensorIndex);
            WRP_DBG("EPIPECmd_GET_CUR_FRM_STATUS = %d, tolerance = %d, tgIndex = %x", status, waitTolerance, tgIndex);

            ISP_DRV_WAIT_IRQ_STRUCT waitIrq;
            waitIrq.Clear            = ISP_DRV_IRQ_CLEAR_WAIT;
            waitIrq.UserInfo.UserKey = mIspDrvUserKey;
            waitIrq.Timeout          = 5000;
            waitIrq.bDumpReg         = 0;

            if (tgIndex == CAM_TG_1) {
                waitIrq.UserInfo.Type    = ISP_DRV_IRQ_TYPE_INT_P1_ST;
                waitIrq.UserInfo.Status  = CAM_CTL_INT_P1_STATUS_VS1_INT_ST;
            } else if (tgIndex == CAM_TG_2) {
                waitIrq.UserInfo.Type    = ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
                waitIrq.UserInfo.Status  = CAM_CTL_INT_P1_STATUS_D_VS1_INT_ST;
            } else {
                WRP_ERR("TG error: tgIndex = %d", tgIndex);
                break;
            }

            if (!mIspDrv->waitIrq(&waitIrq)) {
                WRP_ERR("Wait SOF failed: tolerance = %d", errorTolerance);
                errorTolerance--;
            }

            waitTolerance--;
        } else {
            break;
        }
    }

    if (waitTolerance == 0) {
        WRP_WRN("Wait for drop too many times!");
    }
}


MBOOL NormalPipeWrapper::enque(QBufInfo const& rQBuf)
{
    LOG_FUNCTION_LIFE();
    CAM_TRACE_CALL();

    {
        Mutex::Autolock lock(mOpMutex);

        if (!checkState("enque", STATE_INITED))
            return MFALSE;

        if (!mIsDummyInfoInited) {
            initDummyInfo(rQBuf);

            // In some sceanrio(camsv dualcam), P1Node may enque an invalid buffer for first frame
            // Driver will replace it to dummy implicitly. So we must enque dummy to driver before
            // the P1Node's enque
            for (int i = 0; i < 6; i++) {
                enqueDummy(mInitMagicNum);
            }
        }

        // There is no mutex to protect enque()/deque()
        // Deque may happen right after the driver enque()
        mEnquedBufferQueue.push_back(rQBuf);
        mStatistics.numOfUserEnqued++;
    }

    auto imgoBufInfo = findBufInfo<const BufInfo*>(rQBuf, PORT_IMGO);
    if (imgoBufInfo != NULL) {
        WRP_DBG("magicNum = %u, imgo = %p, raw = 0x%x",
            imgoBufInfo->FrameBased.mMagicNum_tuning, imgoBufInfo->mBuffer, imgoBufInfo->mRawOutFmt);
        mInitMagicNum = std::max(imgoBufInfo->FrameBased.mMagicNum_tuning, mInitMagicNum);
    } else {
        auto rrzoBufInfo = findBufInfo<const BufInfo*>(rQBuf, PORT_RRZO);
        if (rrzoBufInfo != NULL) {
            WRP_DBG("magicNum = %u, rrzo = %p", rrzoBufInfo->FrameBased.mMagicNum_tuning, rrzoBufInfo->mBuffer);
            mInitMagicNum = std::max(rrzoBufInfo->FrameBased.mMagicNum_tuning, mInitMagicNum);
        }
    }

    auto toIsp3RawFormat = [](MUINT32 rawFormat) -> MUINT32 {
        if (rawFormat == EPipe_PURE_RAW)
            return NSCam::NSIoPipeIsp3::NSCamIOPipe::EPipe_PURE_RAW;
        return NSCam::NSIoPipeIsp3::NSCamIOPipe::EPipe_PROCESSED_RAW;
    };

    QBufInfoIsp3 enQBufIsp3;
    for (const BufInfo &bufInfo : rQBuf.mvOut) {
        PortID port = bufInfo.mPortID;
        if (port == PORT_IMGO || port == PORT_RRZO || port == PORT_CAMSV_IMGO || port == PORT_CAMSV2_IMGO) {
            BufInfoIsp3 bufInfoIsp3(
                transPortIdToIsp3(port),
                bufInfo.mBuffer,
                bufInfo.FrameBased.mDstSize,
                bufInfo.FrameBased.mCropRect,
                bufInfo.FrameBased.mMagicNum_tuning,
                bufInfo.FrameBased.mSOFidx,
                toIsp3RawFormat(bufInfo.mRawOutFmt));
            enQBufIsp3.mvOut.push_back(bufInfoIsp3);
        }
    }
    enQBufIsp3.mShutterTimeNs = rQBuf.mShutterTimeNs;

    // The ports configured and enqued should be exactly the same
    // Otherwise the DequeThread can not deque in advance of the P1Node's deque
    // Driver will also report fail if not match
    if (enQBufIsp3.mvOut.size() != mConfiguredPorts.size()) {
        WRP_ERR("Ports in configPipe & enque are not match: enque = %zu, ISP3-related = %zu; configPipe ISP3 = %zu",
            rQBuf.mvOut.size(), enQBufIsp3.mvOut.size(), mConfiguredPorts.size());
    }

    if (mState & STATE_STARTED)
        waitForSOFIfDropped();

    if (!mNormalPipeIsp3->enque(enQBufIsp3)) {
        WRP_WRN("Enque failed, mvOut.size() = %zu -> %zu", rQBuf.mvOut.size(), enQBufIsp3.mvOut.size());

        // Remove the item we pushed previously
        Mutex::Autolock lock(mOpMutex);
        auto queueTail = mEnquedBufferQueue.end();
        queueTail--;
        mEnquedBufferQueue.erase(queueTail);

        return MFALSE;
    }

    return MTRUE;
}


auto NormalPipeWrapper::getEnquedBufferIter(BufInfoIsp3 *imgoBufInfoIsp3, BufInfoIsp3 *rrzoBufInfoIsp3)
{
    // Find corresponding in mEnquedBufferQueue
    auto enquedBufIt = std::find_if(mEnquedBufferQueue.begin(), mEnquedBufferQueue.end(),
        [imgoBufInfoIsp3, rrzoBufInfoIsp3] (QBufInfo &bufInfo) {
            auto imgoBufInfo = findBufInfo<BufInfo*>(bufInfo, PORT_IMGO);
            if (imgoBufInfo != NULL && imgoBufInfoIsp3 != NULL)
                if (imgoBufInfo->mBuffer != imgoBufInfoIsp3->mBuffer ||
                    imgoBufInfo->FrameBased.mMagicNum_tuning != imgoBufInfoIsp3->mMetaData.mMagicNum_hal)
                {
                    return false;
                }
            auto rrzoBufInfo = findBufInfo<BufInfo*>(bufInfo, PORT_RRZO);
            if (rrzoBufInfo != NULL && rrzoBufInfoIsp3 != NULL)
                if (rrzoBufInfo->mBuffer != rrzoBufInfoIsp3->mBuffer)
                    return false;
            return true;
        }
    );

    if (enquedBufIt == mEnquedBufferQueue.end()) {
        if (imgoBufInfoIsp3 != NULL) {
            WRP_ERR("No corresponding enqued QBufInfo for deque! magicNum = %u, imgo = %p",
                imgoBufInfoIsp3->mMetaData.mMagicNum_hal, imgoBufInfoIsp3->mBuffer);
        } else if (rrzoBufInfoIsp3 != NULL) {
            WRP_ERR("No corresponding enqued QBufInfo for deque! magicNum = %u, rrzo = %p",
                rrzoBufInfoIsp3->mMetaData.mMagicNum_hal, rrzoBufInfoIsp3->mBuffer);
        } else {
            WRP_ERR("No corresponding enqued QBufInfo for deque!");
        }
    } else if (enquedBufIt != mEnquedBufferQueue.begin()) {
        if (imgoBufInfoIsp3 != NULL) {
            if (imgoBufInfoIsp3->mBuffer != NULL) {
                WRP_WRN("Dequed from ISP3: magicNum = %u, imgo = %p(va=%" PRIxPTR ")",
                    imgoBufInfoIsp3->mMetaData.mMagicNum_hal, imgoBufInfoIsp3->mBuffer,
                    imgoBufInfoIsp3->mBuffer->getBufVA(0));
            } else {
                WRP_WRN("Dequed from ISP3: magicNum = %u", imgoBufInfoIsp3->mMetaData.mMagicNum_hal);
            }
        } else if (rrzoBufInfoIsp3 != NULL) {
            if (rrzoBufInfoIsp3->mBuffer != NULL) {
                WRP_WRN("Dequed from ISP3: magicNum = %u, rrzo = %p(va=%" PRIxPTR ")",
                    rrzoBufInfoIsp3->mMetaData.mMagicNum_hal, rrzoBufInfoIsp3->mBuffer,
                    rrzoBufInfoIsp3->mBuffer->getBufVA(0));
            } else {
                WRP_WRN("Dequed from ISP3: magicNum = %u", rrzoBufInfoIsp3->mMetaData.mMagicNum_hal);
            }
        }
        auto imgoBufInfo = findBufInfo<BufInfo*>(*mEnquedBufferQueue.begin(), PORT_IMGO);
        auto rrzoBufInfo = findBufInfo<BufInfo*>(*mEnquedBufferQueue.begin(), PORT_RRZO);
        if (imgoBufInfo != NULL && imgoBufInfo->mBuffer != NULL) {
            WRP_WRN("Frame reordered! magicNum = %u, imgo = %p(va=%" PRIxPTR ") was skipped",
                imgoBufInfo->FrameBased.mMagicNum_tuning, imgoBufInfo->mBuffer,
                imgoBufInfo->mBuffer->getBufVA(0));
        } else if (rrzoBufInfo != NULL && rrzoBufInfo->mBuffer != NULL) {
            WRP_WRN("Frame reordered! magicNum = %u, rrzo = %p(va=%" PRIxPTR ") was skipped",
                rrzoBufInfo->FrameBased.mMagicNum_tuning, rrzoBufInfo->mBuffer,
                rrzoBufInfo->mBuffer->getBufVA(0));
        } else {
            WRP_WRN("Frame reordered!");
        }

        // enquedBufIt is still valid only if mEnquedBufferQueue is a list
        mEnquedBufferQueue.erase(mEnquedBufferQueue.begin());
    }

    return enquedBufIt;
}


MBOOL NormalPipeWrapper::deque(QPortID& rQPort, QBufInfo& rQBuf, MUINT32 u4TimeoutMs)
{
    UNUSED(u4TimeoutMs);
    Mutex::Autolock lock(mOpMutex);

    LOG_FUNCTION_LIFE();
    CAM_TRACE_CALL();

    if (!checkState("deque", STATE_INITED | STATE_STARTED, STATE_STOPPED | STATE_UNINITED))
        return MFALSE;

    mStatistics.numOfUserDequed++;

    if (isStateSet(STATE_DEQUE_ERROR)) {
        Mutex::Autolock lock(mDequeListMutex);
        setState("deque", 0, STATE_DEQUE_ERROR);
        mDequeCond.broadcast();
    }

    bool maybeDelayed = false;
    std::unique_ptr<QBufInfoIsp3> qBufInfoIsp3;
    if (!dequeExceptCompensation(qBufInfoIsp3, maybeDelayed)) {
        if (!mIsActive.load(std::memory_order_relaxed)) {
            WRP_DBG("Deque failed due to stop.");
        } else {
            WRP_ERR("Deque failed, rQPort.mvPortId.size = %zu", rQPort.mvPortId.size());
            setState("deque", STATE_DEQUE_ERROR);
        }
        return MFALSE;
    }

    auto imgoBufInfoIsp3 = findBufInfo<BufInfoIsp3 *>(*qBufInfoIsp3, NSCam::NSIoPipeIsp3::PORT_IMGO);
    auto rrzoBufInfoIsp3 = findBufInfo<BufInfoIsp3 *>(*qBufInfoIsp3, NSCam::NSIoPipeIsp3::PORT_RRZO);
    MINT64 rrzoTimeStamp = 0;
    if (rrzoBufInfoIsp3 != NULL)
        rrzoTimeStamp = rrzoBufInfoIsp3->mMetaData.mTimeStamp;

    removeDropped();

    // Find corresponding in mEnquedBufferQueue
    auto enquedBufIt = getEnquedBufferIter(imgoBufInfoIsp3, rrzoBufInfoIsp3);

    // Driver may be unstable, we verify the address first
    if (enquedBufIt != mEnquedBufferQueue.end()) {
        mOpMutex.unlock();
        // PD requirement: this event should be broadcasted ASAP
        // So above path we have to highly optimized
        broadcastDequedEvent(imgoBufInfoIsp3, rrzoBufInfoIsp3);
        mOpMutex.lock();
    }

    // Reorder for rQPort
    rQBuf.mvOut.clear();
    // keep mvOut empty, MW would ignore this deque event
    if (mIsActive == false) {
        WRP_ERR("Deque return empty buffer due to stopping");
        return MFALSE;
    }

    for (auto &port : rQPort.mvPortId) {
        auto bufInfo = findBufInfo<BufInfo*>(rQBuf, port);
        if (bufInfo == NULL) {
            rQBuf.mvOut.emplace_back();
            bufInfo = &*(rQBuf.mvOut.end() - 1);

            if (enquedBufIt != mEnquedBufferQueue.end() &&
                !(port == PORT_IMGO || port == PORT_RRZO || port == PORT_CAMSV_IMGO || port == PORT_CAMSV2_IMGO)) {
                auto enquedBufInfo = findBufInfo<BufInfo*>(*enquedBufIt, port);
                if (enquedBufInfo != NULL) {
                    // We always return the enqued buffer to P1Node
                    // Regardless whether it is supported
                    *bufInfo = *enquedBufInfo;
                }
            }
            // Mark as invalid first
            bufInfo->mMetaData.mTimeStamp = 0;
            bufInfo->mSize = 0;
        }

        if (port == PORT_IMGO) {
            if (imgoBufInfoIsp3 != NULL) {
                cloneBufInfo(*imgoBufInfoIsp3, *bufInfo);
            } else {
                WRP_ERR("BufInfo of IMGO was not dequed from ISP3 NormalPipe");
            }
        } else if (port == PORT_RRZO) {
            if (rrzoBufInfoIsp3 != NULL) {
                cloneBufInfo(*rrzoBufInfoIsp3, *bufInfo);
            } else {
                WRP_ERR("BufInfo of RRZO was not dequed from ISP3 NormalPipe");
            }
        } else if (port == PORT_CAMSV_IMGO || port == PORT_CAMSV2_IMGO) {
            auto bufInfoIsp3 = findBufInfo<BufInfoIsp3 *>(*qBufInfoIsp3, transPortIdToIsp3(port));
            if (bufInfoIsp3 != NULL) {
                cloneBufInfo(*bufInfoIsp3, *bufInfo);
            } else {
                WRP_ERR("BufInfo of port %u was not dequed from ISP3 NormalPipe", port.index);
            }
        } else if (port == PORT_EISO) {
            static const MINT LMV_WAIT_TIMEOUT_MAX = 3; // ms

            IImageBuffer *lmvBuffer = bufInfo->mBuffer;
            MINT lmvWaitTimeout = LMV_WAIT_TIMEOUT_MAX;
            if (static_cast<MINT>(u4TimeoutMs) > 0)
                lmvWaitTimeout = std::min<MINT>(LMV_WAIT_TIMEOUT_MAX, static_cast<MINT>(u4TimeoutMs));
            if (maybeDelayed || !mIsActive.load(std::memory_order_relaxed))
                lmvWaitTimeout = 0;
            WRP_DBG("LMV: timeout = %d ms, buffer = %p", lmvWaitTimeout, lmvBuffer);

            bool lmvAvailable =
                lmvBuffer != NULL &&
                mLmvAdapter != NULL &&
                mLmvAdapter->getLmvData(reinterpret_cast<void*>(lmvBuffer->getBufVA(0)),
                    lmvBuffer->getBufSizeInBytes(0), rrzoTimeStamp, lmvWaitTimeout);

            if (lmvAvailable) {
                if (rrzoBufInfoIsp3 != NULL) {
                    bufInfo->mMetaData.mCrop_s = rrzoBufInfoIsp3->mMetaData.mCrop_s;
                    bufInfo->mMetaData.mDstSize = rrzoBufInfoIsp3->mMetaData.mDstSize;
                    bufInfo->mMetaData.mCrop_d = rrzoBufInfoIsp3->mMetaData.mCrop_d;
                    bufInfo->mMetaData.mTransform = rrzoBufInfoIsp3->mMetaData.mTransform;
                    bufInfo->mMetaData.mMagicNum_hal = rrzoBufInfoIsp3->mMetaData.mMagicNum_hal;
                    bufInfo->mMetaData.mMagicNum_tuning = rrzoBufInfoIsp3->mMetaData.mMagicNum_tuning;
                } else {
                    WRP_ERR("rrzoBufInfoIsp3 is NULL");
                }
                bufInfo->mMetaData.mTimeStamp = rrzoTimeStamp;
                bufInfo->mSize = lmvBuffer->getBufSizeInBytes(0);
            } else {
                WRP_DBG("LMV data of %" PRId64 " is missed", rrzoTimeStamp);
                // Mark as invalid(dummy LMV)
                bufInfo->mMetaData.mTimeStamp = 0;
                bufInfo->mSize = 0;
            }
        } else {
            WRP_WRN("Port %u is not supported on ISP 3", port.index);
        }
    }

    if (enquedBufIt != mEnquedBufferQueue.end())
        mEnquedBufferQueue.erase(enquedBufIt);

    return MTRUE;
}


void NormalPipeWrapper::broadcastDequedEvent(BufInfoIsp3 *imgoBufInfoIsp3, BufInfoIsp3 *rrzoBufInfoIsp3)
{
    LOG_FUNCTION_LIFE();
    CAM_TRACE_CALL();

    MUINT32 magicNum = 0;
    if (imgoBufInfoIsp3 != NULL)
        magicNum = imgoBufInfoIsp3->mMetaData.mMagicNum_hal;
    else if (rrzoBufInfoIsp3 != NULL)
        magicNum = rrzoBufInfoIsp3->mMetaData.mMagicNum_hal;

    IImageBuffer *imgoBuffer = (imgoBufInfoIsp3 != NULL) ? imgoBufInfoIsp3->mBuffer : NULL;
    IImageBuffer *rrzoBuffer = (rrzoBufInfoIsp3 != NULL) ? rrzoBufInfoIsp3->mBuffer : NULL;

    if (imgoBuffer != NULL) {
        ImageDescRawType rawType = eIMAGE_DESC_RAW_TYPE_PROCESSED;
        if (imgoBufInfoIsp3 != NULL &&
            imgoBufInfoIsp3->mMetaData.mRawType == 1)
        {
            rawType = eIMAGE_DESC_RAW_TYPE_PURE;
        }
        imgoBuffer->setImgDesc(eIMAGE_DESC_ID_RAW_TYPE, rawType, MTRUE);
    }

    NormalPipeDequedEvent dequedEvent(mSensorIndex, magicNum, imgoBuffer, rrzoBuffer);
    IoPipeEventSystem::getGlobal().sendSyncEvent(dequedEvent);
}


void NormalPipeWrapper::initDummyInfo(QBufInfo const& rQBuf)
{
    if (mIsDummyInfoInited)
        return;

    auto imgoBufInfo = findBufInfo<const BufInfo *>(rQBuf, PORT_IMGO);
    if (imgoBufInfo != NULL) {
        WRP_AST_IF(imgoBufInfo->mBuffer->getPlaneCount() != 1, "Only supports 1 plane on ISP3");
        mInitInfoImgo.isValid = true;
        mInitInfoImgo.mRawOutFmt = imgoBufInfo->mRawOutFmt;
        mInitInfoImgo.mSOFidx = imgoBufInfo->FrameBased.mSOFidx;
    } else {
        mInitInfoImgo.isValid = false;
        mInitInfoImgo.mSOFidx = 0;
    }

    auto rrzoBufInfo = findBufInfo<const BufInfo *>(rQBuf, PORT_RRZO);
    if (rrzoBufInfo != NULL) {
        mInitInfoRrzo.isValid = true;
        mInitInfoRrzo.mRawOutFmt = rrzoBufInfo->mRawOutFmt;
        mInitInfoRrzo.mSOFidx = rrzoBufInfo->FrameBased.mSOFidx;
    } else {
        mInitInfoRrzo.isValid = false;
        mInitInfoRrzo.mSOFidx = 0;
    }

    if (imgoBufInfo != NULL)
        mInitMagicNum = imgoBufInfo->FrameBased.mMagicNum_tuning;
    else if (rrzoBufInfo != NULL)
        mInitMagicNum = rrzoBufInfo->FrameBased.mMagicNum_tuning;
    else
        mInitMagicNum = 1; // Impossible scenario

    WRP_DBG("magicNum = %u; isValid = (I %d, R %d); mSOFidx = (I %d, R %d)",
        mInitMagicNum, (mInitInfoImgo.isValid ? 1 : 0), (mInitInfoRrzo.isValid ? 1 : 0),
        mInitInfoImgo.mSOFidx, mInitInfoRrzo.mSOFidx);

    mIsDummyInfoInited = true;
}


bool NormalPipeWrapper::generateDummyQBufInfo(QBufInfoIsp3 &enBuf, const char *caller, bool isCompensation, MUINT32 magicNum)
{
    const int dummyIndex = mStatistics.numOfDummysGen;

    if (!mIsDummyInfoInited)
        return false;

    MERROR err;

    // imgo
    sp<IImageBuffer> imgoBuffer;
    if (mInitInfoImgo.isValid) {
        err = mDummyBufferImgo.acquire(dummyIndex, caller, imgoBuffer);
        if (err != OK) {
            return false;
        }

        BufInfoIsp3 imgoBufInfo(
            NSCam::NSIoPipeIsp3::PORT_IMGO,
            imgoBuffer.get(),
            mInitInfoImgo.mDstSize,
            mInitInfoImgo.mCropRect,
            magicNum,
            mInitInfoImgo.mSOFidx);
        imgoBufInfo.mRawOutFmt = mInitInfoImgo.mRawOutFmt;
        enBuf.mvOut.push_back(imgoBufInfo);
    }

    // rrzo
    sp<IImageBuffer> rrzoBuffer;
    if (mInitInfoRrzo.isValid) {
        err = mDummyBufferRrzo.acquire(dummyIndex, caller, rrzoBuffer);
        if (err != OK) {
            return false;
        }

        BufInfoIsp3 rrzoBufInfo(
            NSCam::NSIoPipeIsp3::PORT_RRZO,
            rrzoBuffer.get(),
            mInitInfoRrzo.mDstSize,
            mInitInfoRrzo.mCropRect,
            magicNum,
            mInitInfoRrzo.mSOFidx);
        rrzoBufInfo.mRawOutFmt = mInitInfoRrzo.mRawOutFmt;
        enBuf.mvOut.push_back(rrzoBufInfo);
    }

    mDummyEnques.push_back(DummyEnque(isCompensation, magicNum, imgoBuffer, rrzoBuffer));
    mStatistics.numOfDummysGen++;

    return true;
}


void NormalPipeWrapper::releaseDummy(DummyEnque &dummyEnque)
{
    dummyEnque.magicNum = 0;
    if (dummyEnque.imgoBuffer != NULL)
        mDummyBufferImgo.release(LOG_TAG, dummyEnque.imgoBuffer);
    if (dummyEnque.rrzoBuffer != NULL)
        mDummyBufferRrzo.release(LOG_TAG, dummyEnque.rrzoBuffer);
}


void NormalPipeWrapper::compensatorillyEnque(MUINT32 magicNum)
{
    QBufInfoIsp3 enBuf;

    if (generateDummyQBufInfo(enBuf, LOG_TAG, true, magicNum)) {
        mNormalPipeIsp3->enque(enBuf);
        mStatistics.numOfCompEnqued++;
    }
}


void NormalPipeWrapper::enqueDummy(MUINT32 magicNum)
{
    QBufInfoIsp3 enBuf;

    if (generateDummyQBufInfo(enBuf, LOG_TAG, false, magicNum)) {
        mNormalPipeIsp3->DummyFrame(enBuf);
    }
}


bool NormalPipeWrapper::isCompensationAndDiscarded(BufInfoIsp3 *imgoBufInfo, BufInfoIsp3 *rrzoBufInfo)
{
    if (mDummyEnques.size() == 6) {
        // (Fast check optimization) We assume driver is always correct
        // No more compensation
        return false;
    }

    auto dummyEnque = std::find_if(mDummyEnques.begin(), mDummyEnques.end(),
        [&imgoBufInfo, &rrzoBufInfo] (DummyEnque &dummyEnque) {
            return (
                (imgoBufInfo != NULL && imgoBufInfo->mBuffer == dummyEnque.imgoBuffer.get()) ||
                (rrzoBufInfo != NULL && rrzoBufInfo->mBuffer == dummyEnque.rrzoBuffer.get()));
        }
    );

    if (dummyEnque == mDummyEnques.end()) // not found in dummy addresses
        return false; // not dummy

    MUINT32 magicNum = dummyEnque->magicNum;
    bool isCompensation = dummyEnque->isCompensation;
    if (isCompensation) {
        releaseDummy(*dummyEnque);
        mDummyEnques.erase(dummyEnque);
        mStatistics.numOfCompDequed++;
    } else {
        IImageBuffer *imgoBuffer = NULL;
        if (imgoBufInfo != NULL)
            imgoBuffer = imgoBufInfo->mBuffer;
        IImageBuffer *rrzoBuffer = NULL;
        if (rrzoBufInfo != NULL)
            rrzoBuffer = rrzoBufInfo->mBuffer;

        WRP_WRN("Dummy detected. But driver did not mark the frame to dummy: imgo = %p, rrzo = %p", imgoBuffer, rrzoBuffer);
        mStatistics.numOfDummyDequed++;
    }

    WRP_DBG("Dummy %u dequed, isCompensation = %d. numOfDummyDequed = %d, numOfCompDequed = %d",
        magicNum, (isCompensation ? 1 : 0), mStatistics.numOfDummyDequed, mStatistics.numOfCompDequed);

    return true;
}


bool NormalPipeWrapper::dequeExceptCompensation(std::unique_ptr<QBufInfoIsp3> &deBuf, bool &maybeDelayed)
{
    LOG_FUNCTION_LIFE();
    CAM_TRACE_CALL();

    auto isActive = [this] { return mIsActive.load(std::memory_order_relaxed); };

    while (true) {
        if (!isActive())
            return false;

        // Allow enque here, otherwise we will deque dummy infinitely
        mOpMutex.unlock();

        {
            Mutex::Autolock lock(mDequeListMutex);
            while (isActive() && mDequedList.empty()) {
                mDequeCond.wait(mDequeListMutex);
            }

            deBuf.reset(nullptr);
            if (!mDequedList.empty()) {
                deBuf = std::move(mDequedList.front());
                mDequedList.pop_front();
                // Still something in the queue
                maybeDelayed = !mDequedList.empty();
            }
        }

        mOpMutex.lock();

        if (deBuf == nullptr) // deque failed
            return false;

        auto imgoBufInfo = findBufInfo<BufInfoIsp3*>(*deBuf, NSCam::NSIoPipeIsp3::PORT_IMGO);
        auto rrzoBufInfo = findBufInfo<BufInfoIsp3*>(*deBuf, NSCam::NSIoPipeIsp3::PORT_RRZO);
        if (!isCompensationAndDiscarded(imgoBufInfo, rrzoBufInfo)) {
            logQueueEvent("Dequed from internal", imgoBufInfo, rrzoBufInfo);
            break;
        }
    }

    return true;
}


MBOOL NormalPipeWrapper::abort()
{
    // Let P1 stop don't need to wait for 1 frame
    ISP_DRV_WAIT_IRQ_STRUCT waitIrq;
    waitIrq.UserInfo.Type       = ISP_DRV_IRQ_TYPE_INT_P1_ST;
    waitIrq.UserInfo.Status     = CAM_CTL_INT_P1_STATUS_PASS1_DON_ST;
    waitIrq.UserInfo.UserKey    = 0;
    waitIrq.Timeout             = 1000;

    //TODO: add abort state?
    //re-use stop state in current stage
    if (!checkState("stop", STATE_INITED | STATE_STARTED, STATE_NONE))
        return MFALSE;

    WRP_DBG("abort: + mState = 0x%x", mState);

    mIsActive = false;
    mDequeThread->requestExit();

    //TODO: 1)consider to call stopInternal directly or
    //      2)add NormalPipe_FrmB::abort()
    abortDequeThread();

    MBOOL stopResult = MFALSE;
    {
        Mutex::Autolock lock(mDequeStopMutex);
        stopResult = mNormalPipeIsp3->abort();
    }

    if (!stopResult) {
        WRP_ERR("NormalPipe(ISP3) stop failed");
        return MFALSE;
    }

    if (mLmvAdapter != NULL) {
        mLmvAdapter->stop();
        mLmvAdapter = NULL;
    }

    if (mDequeThread != NULL) {
        {
            Mutex::Autolock lock(mDequeListMutex);
            mDequeCond.broadcast();
        }

        mDequeThread->join();
        mDequeThread = NULL;
    }

    mDequeCond.broadcast();
    setState("stop", STATE_STOPPED, STATE_STARTED);
    WRP_DBG("abort: -");
    return MTRUE;
}


MBOOL NormalPipeWrapper::init(MBOOL EnableSec)
{
    Mutex::Autolock lock(mOpMutex);
    LOG_FUNCTION_LIFE();
    CAM_TRACE_CALL();

    if (mDebugFlag & DEBUG_USER) {
        CAM_ULOGMD("[%s] mUsersInited = %d -> %d, call stack:", __func__, mUsersInited, mUsersInited + 1);
        NSCam::Utils::dumpCallStack(LOG_TAG);
    }

    WRP_DBG("mUsersInited+1 = %d", mUsersInited + 1);

    if (mUsersInited > 0) {
        mUsersInited++; // Allow multiple users call init()
        return MTRUE;
    }

    if (!checkState("init", STATE_NONE, STATE_INITED | STATE_STARTED))
        return MFALSE;

    mUsersInited++;

    if (mNormalPipeIsp3 == NULL) {
        INormalPipeIsp3 *instance = mDriverFactory->createNormalPipeIsp3(mSensorIndex, LOG_TAG, 1);
        std::atomic_thread_fence(std::memory_order_release);
        mNormalPipeIsp3 = instance;
    }

    WRP_AST_IF(mNormalPipeIsp3 == NULL, "createNormalPipeIsp3() returned NULL");
    if (!mNormalPipeIsp3->init()) {
        WRP_ERR("NormalPipe(ISP3) init failed");
        return MFALSE;
    }

    mIsDummyInfoInited = false;
    mStatistics.reset();
    mIsLmvEnabled = false;

    mIspDrv = mDriverFactory->createIspDrvIsp3();
    WRP_AST_IF(mIspDrv == NULL, "createIspDrvIsp3() returned NULL");
    mIspDrv->init(LOG_TAG);
    mIspDrvUserKey = mIspDrv->registerIrq(LOG_TAG);

    setState("init", STATE_INITED, STATE_UNINITED);

    if (!mEnquedBufferQueue.empty()) {
        WRP_ERR("Something in the enque buffer before init. Previous session was not destroyed correctly");
        mEnquedBufferQueue.clear();
    }

    return MTRUE;
}


MBOOL NormalPipeWrapper::uninit()
{
    Mutex::Autolock lock(mOpMutex);
    LOG_FUNCTION_LIFE();
    CAM_TRACE_CALL();

    if (mDebugFlag & DEBUG_USER) {
        CAM_ULOGMD("[%s] mUsersInited = %d -> %d, call stack:", __func__, mUsersInited, mUsersInited - 1);
        NSCam::Utils::dumpCallStack(LOG_TAG);
    }

    WRP_DBG("mUsersInited-1 = %d", mUsersInited - 1);

    if (mUsersInited > 1) {
        mUsersInited--;
        return MTRUE;
    }

    if (!checkState("uninit", STATE_INITED, STATE_UNINITED))
        return MFALSE;

    if (isStateSet(STATE_STARTED)) {
        WRP_ERR("Not stopped!");
        if (!stopInternal())
            return MFALSE;
    }

    mUsersInited = 0;

    if (mNormalPipeIsp3 != NULL) {
        mNormalPipeIsp3->sendCommand(NSImageioIsp3::NSIspio::EPIPECmd_SET_EIS_CBFP, NULL, -1, -1);
        mNormalPipeIsp3->sendCommand(NSImageioIsp3::NSIspio::EPIPECmd_SET_LCS_CBFP, NULL, -1, -1);
        mNormalPipeIsp3->uninit();
    }

    if (mIspDrv != NULL) {
        mIspDrv->uninit(LOG_TAG);
        mIspDrv->destroyInstance();
        mIspDrv = NULL;
    }

    for (auto &dummyEnque : mDummyEnques)
        releaseDummy(dummyEnque);
    mDummyEnques.clear();

    if (mIsDummyInfoInited) {
        mDummyBufferImgo.uninit();
        mDummyBufferRrzo.uninit();
        mIsDummyInfoInited = false;
    }

    mVeriSlot[mInstanceId % VERI_SLOT_SIZE] = NULL;

    setState("uninit", STATE_UNINITED, STATE_INITED);

    return MTRUE;
}


MBOOL NormalPipeWrapper::reset()
{
    // Reset() is used to reset the NormalPipe when deque fail,
    // The full control is already included in the wrapper deque
    WRP_ERR("reset() is not directly supported. Will be done by wrapper");
    return MFALSE;
}


MBOOL NormalPipeWrapper::configPipe(QInitParam const& vInPorts, MINT32 burstQnum)
{
    Mutex::Autolock lock(mOpMutex);

    if (!checkState("configPipe", STATE_INITED))
        return MFALSE;

    if (burstQnum != 1) {
        WRP_ERR("burstQnum = %d but only support 1 on ISP 3", burstQnum);
    }

    std::vector<NSCam::NSIoPipeIsp3::NSCamIOPipe::portInfo> emptyPortInfo;
    NSCam::NSIoPipeIsp3::NSCamIOPipe::QInitParam initParamIsp3(
        vInPorts.mRawType,
        vInPorts.mBitdepth,
        vInPorts.mSensorCfg,
        emptyPortInfo, // directly push into QInitParam intead of deep-copy for speed up
        vInPorts.m_DynamicRawType);

    mDropCallback.m_DropCB = vInPorts.m_DropCB;
    mDropCallback.m_returnCookie = vInPorts.m_returnCookie;
    NormalPipeWrapper **slotPos = &mVeriSlot[mInstanceId % VERI_SLOT_SIZE];
    *slotPos = this;
    initParamIsp3.m_returnCookie = static_cast<void*>(slotPos);
    initParamIsp3.m_DropCB = &NormalPipeWrapper::onDrvDropped;
    initParamIsp3.m_bN3D = vInPorts.m_bN3D;

    mConfiguredPorts.clear();

    size_t imgoBufSize = 0, rrzoBufSize = 0;
    const NSCam::NSIoPipeIsp3::PortID NullPortIdIsp3;
    for (auto &portInfoRef : vInPorts.mPortInfo) {
        if (portInfoRef.mPortID == PORT_IMGO) {
            imgoBufSize = portInfoRef.mStride[0] * portInfoRef.mDstSize.h;
            mInitInfoImgo.mDstSize = portInfoRef.mDstSize;
            mInitInfoImgo.mCropRect = portInfoRef.mCropRect;
        } else if (portInfoRef.mPortID == PORT_RRZO) {
            rrzoBufSize = portInfoRef.mStride[0] * portInfoRef.mDstSize.h;
            mInitInfoRrzo.mDstSize = portInfoRef.mDstSize;
            mInitInfoRrzo.mCropRect = portInfoRef.mCropRect;
        }
        if (portInfoRef.mPortID == PORT_EISO) {
            mIsLmvEnabled = true;
        } else {
            NSCam::NSIoPipeIsp3::PortID portId = transPortIdToIsp3(portInfoRef.mPortID);
            if (portId == NullPortIdIsp3) { // not supported
                WRP_WRN("Port %d is not supported", portInfoRef.mPortID.index);
            } else {
                initParamIsp3.mPortInfo.emplace_back(
                    portId,
                    portInfoRef.mFmt,
                    portInfoRef.mDstSize,
                    portInfoRef.mCropRect,
                    portInfoRef.mStride[0],
                    portInfoRef.mStride[1],
                    portInfoRef.mStride[2],
                    portInfoRef.mPureRaw,
                    portInfoRef.mPureRawPak
                );

                mConfiguredPorts.push_back(portId);
            }
        }
    }

    static const int NUM_OF_DUMMY_FRAMES = 8;
    if (imgoBufSize > 0) {
        if( mDummyBufferImgo.init(imgoBufSize, NUM_OF_DUMMY_FRAMES) != OK) {
            WRP_ERR("imgo DummyBuffer init failed, configPipe Fail.");
            return MFALSE;
        }
    }
    if (rrzoBufSize > 0) {
        if( mDummyBufferRrzo.init(rrzoBufSize, NUM_OF_DUMMY_FRAMES) != OK) {
            WRP_ERR("rrzo DummyBuffer init failed, configPipe Fail.");
            return MFALSE;
        }
    }

    WRP_DBG("PortInfo.size() = %zu, BufSize = %zu,%zu , mIsLmvEnabled = %d",
        vInPorts.mPortInfo.size(), imgoBufSize, rrzoBufSize, mIsLmvEnabled ? 1 : 0);

    MBOOL result = mNormalPipeIsp3->configPipe(initParamIsp3);

    if (result && mIsLmvEnabled) {
        mLmvAdapter = new LmvIrqAdapter;
        if (!mLmvAdapter->init(mSensorIndex, mNormalPipeIsp3, mIspDrv)) {
            WRP_ERR("LMV init failed");
            mLmvAdapter = NULL;
        }
    }

    return result;
}


void NormalPipeWrapper::onDrvDropped(MUINT32 magic, void* cookie)
{
    LOG_FUNCTION_LIFE();
    CAM_TRACE_CALL();

    // NOTE:
    // This callback may be called from the enque() directly or
    // from another thread. We have to be careful of both!

    auto getInstance = [] (void* cookie) -> NormalPipeWrapper* {
        if (cookie == NULL)
            return NULL;

        NormalPipeWrapper **slotPos = static_cast<NormalPipeWrapper**>(cookie);
        if (*slotPos == NULL)
            return NULL;

        NormalPipeWrapper *inst = *slotPos;
        bool isSlotInRange =
            reinterpret_cast<intptr_t>(&(inst->mVeriSlot[0])) <= reinterpret_cast<intptr_t>(slotPos) &&
            reinterpret_cast<intptr_t>(slotPos) <= reinterpret_cast<intptr_t>(&(inst->mVeriSlot[VERI_SLOT_SIZE - 1]));
        if (!isSlotInRange)
            return NULL;

        return inst;
    };

    NormalPipeWrapper *inst = getInstance(cookie);
    if (inst == NULL) {
        WRP_ERR("Drop callback after uninit: magicNum = %u, cookie = %p", magic, cookie);
        return;
    }

    {
        Mutex::Autolock lock(inst->mDroppedMutex);
        inst->mDroppedMagicNums.push_back(magic);
    }

    int dropped = inst->mStatistics.numOfDropped.fetch_add(1, std::memory_order_relaxed) + 1;
    WRP_DBG("Frame dropped: magicNum = %u, cookie = %p, [sensor %d]numOfDropped = %d; enq = %d, deq = %d",
        magic, cookie, inst->mSensorIndex, dropped,
        inst->mStatistics.numOfUserEnqued, inst->mStatistics.numOfUserDequed);

    DropCallback &dropCallback = inst->mDropCallback;
    if (dropCallback.m_DropCB != NULL) {
        (*dropCallback.m_DropCB)(magic, dropCallback.m_returnCookie);
    }
}


void NormalPipeWrapper::removeDropped()
{
    // mOpMutex must be locked before
    Mutex::Autolock lock(mDroppedMutex);

    if (mDroppedMagicNums.size() == 0)
        return;

    for (MUINT32 magic : mDroppedMagicNums) {
        auto enquedBufIt = std::find_if(mEnquedBufferQueue.begin(), mEnquedBufferQueue.end(),
            [magic] (QBufInfo &bufInfo) -> bool {
                auto imgoBufInfo = findBufInfo<BufInfo*>(bufInfo, PORT_IMGO);
                if (imgoBufInfo != NULL && imgoBufInfo->FrameBased.mMagicNum_tuning == magic)
                    return true;
                auto rrzoBufInfo = findBufInfo<BufInfo*>(bufInfo, PORT_RRZO);
                if (rrzoBufInfo != NULL && rrzoBufInfo->FrameBased.mMagicNum_tuning == magic)
                    return true;
                return false;
            }
        );

        if (enquedBufIt != mEnquedBufferQueue.end())
            mEnquedBufferQueue.erase(enquedBufIt);
    }

    mDroppedMagicNums.clear();
}

void NormalPipeWrapper::abortDequeThread() {

    ISP_DRV_WAIT_IRQ_STRUCT waitIrq;
    waitIrq.UserInfo.UserKey    = 0;
    waitIrq.Timeout             = 1000;

    MINT32 tgIndex = mNormalPipeIsp3->getSensorTg(mSensorIndex);

    if (tgIndex == CAM_TG_1) {
        waitIrq.UserInfo.Type    = ISP_DRV_IRQ_TYPE_INT_P1_ST;
        waitIrq.UserInfo.Status  = CAM_CTL_INT_P1_STATUS_PASS1_DON_ST;
    } else if (tgIndex == CAM_TG_2) {
        waitIrq.UserInfo.Type    = ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
        waitIrq.UserInfo.Status  = CAM_CTL_INT_P1_STATUS_D_PASS1_DON_ST;
    } else {
        WRP_ERR("TG error: tgIndex = %d", tgIndex);
    }

    mNormalPipeIsp3->abortDeque();
    if (!mIspDrv->flushIrq(waitIrq)) {
        CAM_ULOGME("flushIrq(INT_P1_ST, PASS1_DON_ST) failed");
    }

}

MBOOL NormalPipeWrapper::sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    WRP_DBG("0x%04x: 0x%" PRIxPTR "(%" PRIdPTR "), 0x%" PRIxPTR ", 0x%" PRIxPTR, cmd, arg1, arg1, arg2, arg3);

    std::atomic_thread_fence(std::memory_order_acquire);
    if (mNormalPipeIsp3 == NULL) {
        WRP_ERR("Not initialized yet");
        return MFALSE;
    }

    auto writeInt32 = [] (MINTPTR target, MINT32 value) {
        *reinterpret_cast<MINT32*>(target) = value;
    };


    switch (cmd) {
    case ENPipeCmd_GET_BURSTQNUM:
        writeInt32(arg1, 1);
        return MTRUE;

    case ENPipeCmd_GET_HW_PATH_CFG:
        // An arbitrary value to initialize
        *reinterpret_cast<NSCam::NSIoPipe::NSCamIOPipe::E_CamHwPathCfg*>(arg1) = eCamHwPathCfg_One_TG;
        // go through
    case ENPipeCmd_SET_HW_PATH_CFG:
        WRP_ERR("SET_HW_PATH_CFG/ENPipeCmd_GET_HW_PATH_CFG is not supported on ISP 3");
        return MFALSE;

    case ENPipeCmd_GET_EIS_INFO:
        {
            NSCam::NSIoPipeIsp3::NSCamIOPipe::NormalPipe_EIS_Info eisInfoIsp3;
            if (!mNormalPipeIsp3->sendCommand(
                    NSImageioIsp3::NSIspio::EPIPECmd_GET_EIS_INFO,
                    arg1, reinterpret_cast<MINTPTR>(&eisInfoIsp3), arg3))
            {
                WRP_ERR("EPIPECmd_GET_EIS_INFO failed");
                return MFALSE;
            }

            NormalPipe_EIS_Info *eisInfo = reinterpret_cast<NormalPipe_EIS_Info*>(arg2);
            eisInfo->mSupported = eisInfoIsp3.mSupported;
            return MTRUE;
        }

    case ENPipeCmd_GET_BIN_INFO:
        {
            MINT32 w = 0, h = 0;
            MBOOL success = mNormalPipeIsp3->sendCommand(
                    NSImageioIsp3::NSIspio::EPIPECmd_GET_TG_OUT_SIZE,
                    mSensorIndex, reinterpret_cast<MINTPTR>(&w), reinterpret_cast<MINTPTR>(&h));
            writeInt32(arg1, w);
            writeInt32(arg2, h);
            return success;
        }

    case ENPipeCmd_GET_IMGO_INFO:
        {
            MINT32 w = 0, h = 0;
            MBOOL success = mNormalPipeIsp3->sendCommand(
                    NSImageioIsp3::NSIspio::EPIPECmd_GET_TG_OUT_SIZE,
                    mSensorIndex, reinterpret_cast<MINTPTR>(&w), reinterpret_cast<MINTPTR>(&h));
            MSize *rawSizes = reinterpret_cast<MSize*>(arg1);
            // Processed RAW
            rawSizes[0].w = w;
            rawSizes[0].h = h;
            // Pure RAW
            rawSizes[1].w = w;
            rawSizes[1].h = h;

            return success;
        }

    case ENPipeCmd_SET_EIS_CBFP:
        return mNormalPipeIsp3->sendCommand(
                NSImageioIsp3::NSIspio::EPIPECmd_SET_EIS_CBFP,
                arg1, arg2, arg3);

    case ENPipeCmd_GET_TG_OUT_SIZE:
        return mNormalPipeIsp3->sendCommand(
                NSImageioIsp3::NSIspio::EPIPECmd_GET_TG_OUT_SIZE,
                mSensorIndex, arg1, arg2);

    case ENPipeCmd_GET_UNI_SWITCH_STATE:
        writeInt32(arg1, 0);
        return MFALSE;

    case ENPipeCmd_GET_DTwin_INFO:
        *reinterpret_cast<MBOOL*>(arg1) = MFALSE; // off
        return MTRUE;

    case ENPipeCmd_GET_HBIN_INFO:
        {
            NSCam::NSIoPipeIsp3::NSCamIOPipe::NormalPipe_HBIN_Info hBinInfo;
            MBOOL success = mNormalPipeIsp3->sendCommand(
                    NSImageioIsp3::NSIspio::EPIPECmd_GET_HBIN_INFO,
                    mSensorIndex, reinterpret_cast<MINTPTR>(&hBinInfo), 0);
            if (success) {
                writeInt32(arg1, hBinInfo.size.w);
                writeInt32(arg2, hBinInfo.size.h);
            }
            return success;
        }

    case ENPipeCmd_GET_HBIN1_INFO:
        {
            NSCam::NSIoPipeIsp3::NSCamIOPipe::NormalPipe_HBIN_Info hBinInfo;
            MBOOL success = mNormalPipeIsp3->sendCommand(
                    NSImageioIsp3::NSIspio::EPIPECmd_GET_HBIN1_INFO,
                    mSensorIndex, reinterpret_cast<MINTPTR>(&hBinInfo), 0);
            if (success) {
                writeInt32(arg1, hBinInfo.size.w);
                writeInt32(arg2, hBinInfo.size.h);
            }
            return success;
        }

    case ENPipeCmd_GET_CUR_FRM_STATUS:
        return mNormalPipeIsp3->sendCommand(
                NSImageioIsp3::NSIspio::EPIPECmd_GET_CUR_FRM_STATUS,
                arg1, arg2, arg3);

    case ENPipeCmd_GET_CUR_SOF_IDX:
        return mNormalPipeIsp3->sendCommand(
                NSImageioIsp3::NSIspio::EPIPECmd_GET_CUR_SOF_IDX,
                arg1, arg2, arg3);

    case ENPipeCmd_SET_MODULE_EN:
    case ENPipeCmd_SET_MODULE_SEL:
    case ENPipeCmd_SET_MODULE_CFG:
    case ENPipeCmd_GET_MODULE_HANDLE:
    case ENPipeCmd_SET_MODULE_CFG_DONE:
    case ENPipeCmd_RELEASE_MODULE_HANDLE:
    case ENPipeCmd_SET_MODULE_DBG_DUMP:
        static_assert(MINT32{ENPipeCmd_SET_MODULE_EN} == MINT32{NSImageioIsp3::NSIspio::EPIPECmd_SET_MODULE_EN},
            "ENPipeCmd_SET_MODULE_EN: the value is different from ISP3");
        static_assert(MINT32{ENPipeCmd_SET_MODULE_DBG_DUMP} == MINT32{NSImageioIsp3::NSIspio::EPIPECmd_SET_MODULE_DBG_DUMP},
            "ENPipeCmd_SET_MODULE_DBG_DUMP: the value is different from ISP3");
    default:
        return mNormalPipeIsp3->sendCommand(cmd, arg1, arg2, arg3);
    }

    return MFALSE;
}


MINT32 NormalPipeWrapper::attach(const char* UserName)
{
    WRP_ERR("attach() is not supported on ISP 3, UserName = %s", UserName);

    if (mDebugFlag & DEBUG_USER)
        NSCam::Utils::dumpCallStack(LOG_TAG);

    return 0;
}


MBOOL NormalPipeWrapper::wait(EPipeSignal eSignal, EPipeSignalClearType eClear, const MINT32 mUserKey,
                        MUINT32 TimeoutMsF, SignalTimeInfo *pTime)
{
    UNUSED(eClear);
    UNUSED(mUserKey);
    UNUSED(TimeoutMsF);
    UNUSED(pTime);
    WRP_ERR("wait() is not supported on ISP 3, eSignal = 0x%x", static_cast<unsigned int>(eSignal));

    if (mDebugFlag & DEBUG_USER)
        NSCam::Utils::dumpCallStack(LOG_TAG);

    return MFALSE;
}


MBOOL NormalPipeWrapper::signal(EPipeSignal eType, const MINT32 mUserKey)
{
    UNUSED(mUserKey);
    WRP_ERR("signal() is not supported on ISP 3, eSignal = 0x%x", static_cast<unsigned int>(eType));

    if (mDebugFlag & DEBUG_USER)
        NSCam::Utils::dumpCallStack(LOG_TAG);

    return MFALSE;
}


MBOOL NormalPipeWrapper::abortDma(PortID port, char const* szCallerName)
{
    UNUSED(port);
    WRP_ERR("abortDma() is not supported on ISP 3, szCallerName = %s", szCallerName);

    if (mDebugFlag & DEBUG_USER)
        NSCam::Utils::dumpCallStack(LOG_TAG);

    return MFALSE;
}


MUINT32 NormalPipeWrapper::getIspReg(MUINT32 RegAddr, MUINT32 RegCount, MUINT32 RegData[], MBOOL bPhysical)
{
    UNUSED(RegAddr);
    UNUSED(bPhysical);
    memset(RegData, 0, sizeof(RegData[0]) * RegCount);
    WRP_ERR("getIspReg(0x%x) is not supported on ISP 3", RegAddr);

    if (mDebugFlag & DEBUG_USER)
        NSCam::Utils::dumpCallStack(LOG_TAG);

    return MFALSE; // Return value is MBOOL by reference implementation(ISP5)
}


MUINT32 NormalPipeWrapper::getIspReg(IspP1RegIo *pRegs, MUINT32 RegCount, MBOOL bPhysical)
{
    UNUSED(bPhysical);
    memset(pRegs, 0, sizeof(pRegs[0]) * RegCount);
    WRP_ERR("getIspReg() is not supported on ISP 3");

    if (mDebugFlag & DEBUG_USER)
        NSCam::Utils::dumpCallStack(LOG_TAG);

    return MFALSE; // Return value is MBOOL by reference implementation(ISP5)
}


MUINT32 NormalPipeWrapper::getIspUniReg(IspP1RegIo *pRegs, MUINT32 RegCount, MBOOL bPhysical)
{
    UNUSED(bPhysical);
    memset(pRegs, 0, sizeof(pRegs[0]) * RegCount);
    WRP_ERR("getIspReg() is not supported on ISP 3");

    if (mDebugFlag & DEBUG_USER)
        NSCam::Utils::dumpCallStack(LOG_TAG);

    return MFALSE; // Return value is MBOOL by reference implementation(ISP5)
}


NormalPipeWrapper::DequeThread::DequeThread(NormalPipeWrapper &wrapper) : mWrapper(wrapper)
{
}


bool NormalPipeWrapper::DequeThread::threadLoop()
{
    androidSetThreadName("NormalPipe:Deque");
    // This thread should be an almost real-time thread
    // Otherwise frame drop may cause capture fail.
    androidSetThreadPriority(getTid(), android::PRIORITY_HIGHEST);

    static constexpr size_t SHOW_TOO_MANY_THRESHOLD = 7;
    size_t showTooMany = SHOW_TOO_MANY_THRESHOLD;

    while (!exitPending()) {
        std::unique_ptr<QBufInfoIsp3> deBuf = std::make_unique<QBufInfoIsp3>();

        static constexpr MUINT32 DEFAULT_TIMEOUT = 1000; // But driver may not support
        size_t queueSize = 0;
        if (dequeExceptDummy(*deBuf, DEFAULT_TIMEOUT)) {
            Mutex::Autolock lock(mWrapper.mDequeListMutex);
            mWrapper.mDequedList.emplace_back(std::move(deBuf));
            mWrapper.mDequeCond.broadcast();

            if (mWrapper.mDequedList.size() >= showTooMany) {
                WRP_DBG("Too many deque pending: %zu", mWrapper.mDequedList.size());
                showTooMany = mWrapper.mDequedList.size();
            } else if (mWrapper.mDequedList.size() < SHOW_TOO_MANY_THRESHOLD) {
                showTooMany = SHOW_TOO_MANY_THRESHOLD;
            }
            queueSize = mWrapper.mDequedList.size();
        } else if (!exitPending()) {
            // Use nullptr to annotate error
            deBuf.release();
            Mutex::Autolock lock(mWrapper.mDequeListMutex);
            mWrapper.mDequedList.emplace_back(nullptr);
            mWrapper.mDequeCond.broadcast();
            // Wait for stop or resume
            WRP_WRN("Deque thread suspended due to too many deque fail.");
            mWrapper.mDequeCond.wait(mWrapper.mDequeListMutex);
            WRP_DBG("Deque thread waked up!");
            queueSize = mWrapper.mDequedList.size();
        }

        if (mWrapper.mDebugFlag & Wrapper::DEBUG_QUEUE) {
            // We rely on the acquire of mDequeListMutex to sync
            // However, we don't care the accuracy
            CAM_ULOGMD("User enque = %d, deque = %d, dropped = %d; dummy = %d; sensor = %u, queueSize = %zu",
                mWrapper.mStatistics.numOfUserEnqued, mWrapper.mStatistics.numOfUserDequed,
                mWrapper.mStatistics.numOfDropped.load(std::memory_order_relaxed),
                mWrapper.mStatistics.numOfDummyDequed,
                mWrapper.mSensorIndex, queueSize);
        }
    }

    mWrapper.mDequeCond.broadcast();

    return false;
}


bool NormalPipeWrapper::DequeThread::dequeExceptDummy(QBufInfoIsp3 &deBuf, MUINT32 timeoutInMs)
{
    LOG_FUNCTION_LIFE();
    CAM_TRACE_CALL();

    auto isDummy = [] (BufInfoIsp3 *imgoBufInfo, BufInfoIsp3 *rrzoBufInfo) -> bool {
        return (imgoBufInfo != NULL && imgoBufInfo->mMetaData.m_bDummyFrame) ||
               (rrzoBufInfo != NULL && rrzoBufInfo->mMetaData.m_bDummyFrame);
    };

    deBuf.mvOut.clear();
    deBuf.mvOut.reserve(2);
    for (const NSIoPipeIsp3::PortID &isp3PortId : mWrapper.mConfiguredPorts) {
        deBuf.mvOut.emplace_back();
        BufInfoIsp3 *lastMvOut = &*(deBuf.mvOut.end() - 1);
        lastMvOut->mPortID = isp3PortId;
    }

    BufInfoIsp3 *imgoBufInfo = NULL, *rrzoBufInfo = NULL;
    imgoBufInfo = findBufInfo<BufInfoIsp3*>(deBuf, NSCam::NSIoPipeIsp3::PORT_IMGO);
    rrzoBufInfo = findBufInfo<BufInfoIsp3*>(deBuf, NSCam::NSIoPipeIsp3::PORT_RRZO);

    int numOfFails = 0;
    while (numOfFails < 3 && !exitPending()) {
        MBOOL dequeResult = MFALSE;
        {
            if (imgoBufInfo != NULL)
                imgoBufInfo->mMetaData.m_bDummyFrame = MFALSE;
            if (rrzoBufInfo != NULL)
                rrzoBufInfo->mMetaData.m_bDummyFrame = MFALSE;

            // On ISP3 driver, if we call stop() during deque(), the deque() will return only after timeout(2s)
            // We use a mutex to control the worst case:
            // stop() ahead of deque(): deque() will only block one frame
            Mutex::Autolock lock(mWrapper.mDequeStopMutex);
            LOG_TAG_LIFE("Driver deque");
            dequeResult = mWrapper.mNormalPipeIsp3->deque(deBuf, timeoutInMs);
        }

        // Driver might operate the vector, we find the new address
        imgoBufInfo = findBufInfo<BufInfoIsp3*>(deBuf, NSCam::NSIoPipeIsp3::PORT_IMGO);
        rrzoBufInfo = findBufInfo<BufInfoIsp3*>(deBuf, NSCam::NSIoPipeIsp3::PORT_RRZO);

        // Deque will return MFALSE for dummy, but it is a successful deque
        if (dequeResult || isDummy(imgoBufInfo, rrzoBufInfo)) {
            if (isDummy(imgoBufInfo, rrzoBufInfo)) {
                mWrapper.mStatistics.numOfDummyDequed++;
                if (mWrapper.mDebugFlag & Wrapper::DEBUG_QUEUE) {
                    CAM_ULOGMD("Dummy was dequed. Total = %d; user enque = %d, deque = %d",
                        mWrapper.mStatistics.numOfDummyDequed,
                        mWrapper.mStatistics.numOfUserEnqued, mWrapper.mStatistics.numOfUserDequed);
                }
            } else {
                mWrapper.logQueueEvent("Dequed from driver", imgoBufInfo, rrzoBufInfo);
                return true;
            }
        } else if (!exitPending()) {
            WRP_WRN("Deque failed: reset. timeoutInMs = %d, numOfUserEnqued = %d, numOfUserDequed = %d",
                timeoutInMs, mWrapper.mStatistics.numOfUserEnqued, mWrapper.mStatistics.numOfUserDequed);
            mWrapper.mNormalPipeIsp3->Reset();
            numOfFails++;
        }
    }

    return false;
}



INormalPipe *NSCam::NSIoPipe::NSCamIOPipe::Wrapper::NormalPipeWrapper_createInstance(
    MUINT32 sensorIndex, char const* szCallerName, MUINT32 apiVersion,
    android::sp<IDriverFactory> driverFactory)
{
    return NormalPipeWrapper::createInstance(sensorIndex, szCallerName, apiVersion, driverFactory);
}


