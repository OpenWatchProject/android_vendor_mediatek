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

#ifndef LMV_IRQ_ADAPTER_H
#define LMV_IRQ_ADAPTER_H


#include <utils/Thread.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>
#include <utils/List.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <iopipe/CamIO/CamIoWrapperPublic.h>


namespace NSCam {
namespace NSIoPipe {
namespace NSCamIOPipe {
namespace Wrapper {

class LmvIrqAdapter : public android::Thread
{
public:
    static constexpr MINTPTR UT_MODULE_HANDLE = static_cast<MINTPTR>(0xabcd02812969abcd);

    LmvIrqAdapter();
    virtual ~LmvIrqAdapter();
    virtual android::status_t readyToRun() override;
    bool init(MUINT32 sensorIndex, INormalPipeIsp3 *normalPipeIsp3, IspDrv *ispDrv);
    bool start();
    void stop();
    bool getLmvData(void *bufVa, MINT32 bufSize, MINT64 timeStamp, MUINT timeoutInMs);

private:
    static const int LMV_DATA_SIZE;
    static const int LMV_BUFFER_NUM;
    static const int SGG2_PGN;
    static const int SGG2_GMRC1;
    static const int SGG2_GMRC2;

    struct EisoData {
        MUINTPTR va = 0;
        MUINTPTR pa = 0;
        MINT64 timeStamp = 0;

        void clear() {
            va = 0;
            pa = 0;
            timeStamp = 0;
        }

        bool isValid() {
            return (va != 0 && pa != 0);
        }

    };

    INormalPipeIsp3 *mNormalPipeIsp3;
    IspDrv *mIspDrv;
    MINT32 mIspDrvUserKey;
    android::Mutex mBufferLock;
    android::sp<NSCam::IIonImageBufferHeap> mDataBufferHeap;
    android::sp<NSCam::IImageBuffer> mDataBufferSpace;

    android::List<EisoData> mEmptyBufferPool;
    android::List<EisoData> mLmvDataList;
    android::Condition mDataCond;
    std::atomic_bool mThreadIsRunning;
    android::Condition mThreadStopCond;

    bool initDataBuffers();
    void uninitDataBuffers();
    bool acquireBuffer(EisoData &buf);
    void releaseBuffer(EisoData &buf);
    void pushToDataList(EisoData &buf);
    virtual bool threadLoop() override;
    bool initEisoReg();
    bool initSgg2Reg();
    bool initIrq();
    bool interruptWaiting();
    bool waitForP1Done(MINT64 &timeStamp);
    bool clearP1Done();
    bool setBufferAddressToReg(const EisoData &buf);
    bool tryToGetLmvData(void *bufVa, MINT32 bufSize, MINT64 timeStamp);

    void fillWaitIrq(
        ISP_DRV_WAIT_IRQ_STRUCT &waitIrq,
        ISP_DRV_IRQ_CLEAR_ENUM clear,
        ISP_DRV_IRQ_TYPE_ENUM type,
        MUINT32 status,
        MUINT32 timeout = 503);

};

}
}
}
}

#endif

