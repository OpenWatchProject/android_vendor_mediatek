/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef _HDR_H_
#define _HDR_H_

#include "../IHDR.h"

#include <hal/IHDRHAL.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/aaa/IHal3A.h>
#include <future>
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h> // tuning file naming



using namespace android;
using namespace NSCam;
using namespace HDRCommon;
using namespace HDR2;
using namespace NSCam::TuningUtils;


// ---------------------------------------------------------------------------

typedef MBOOL (*HDRProcCompleteCallback_t)(MVOID* user, const sp<IImageBuffer>& hdrResult, MBOOL ret);

// ---------------------------------------------------------------------------

namespace NS3Av3 {
    struct CaptureParam_T;
};

// ---------------------------------------------------------------------------

namespace NSCam {

class VendorHDR final : public IHDR
{
public:
    // IHDR interface
    VendorHDR(char const * const pszShotName,
            uint32_t const u4ShotMode, int32_t const i4OpenId);
    virtual ~VendorHDR();

    // init the instance
    virtual MBOOL init();

    // uninit the instance
    virtual MBOOL uninit();

    // Update resolution information...make HDR directory
    virtual MBOOL updateInfo();

    virtual MBOOL setParam(MUINT32 paramId, MUINTPTR iArg1, MUINTPTR iArg2);

    virtual MBOOL getParam(MUINT32 paramId, MUINT32& rArg1, MUINT32& rArg2);

    virtual MBOOL setShotParam(void const* pParam);

    // prepare() is used to init HDR HAL and
    // allocate processing memory asynchronously
    virtual MBOOL prepare();

    // addInputFrame() set full YUV and grayscale images as HDR's input buffers.
    //
    // NOTE: the frame index ordering should be
    // {0, 2, 4, ...} for YUV and {1, 3, 5, ...} for grayscale frames
    virtual MBOOL addInputFrame(
            MINT32 frameIndex, const sp<IImageBuffer>& inBuffer);

    // process() is an asynchronous call and do the following tasks:
    // 1. creates a detached worker thread that processes the post-processing task
    // 2. waits for all HDR input buffers are received
    // 3. notifies the caller once the post-processing task is done
    virtual MBOOL process();

    // release() releases all internal working buffers
    virtual MBOOL release();

    // getHDRCapInfo() is used to
    // 1. get exposure settings
    // 2. record the number of HDR input frames to mu4OutputFrameNum
    // @deprecated, valid until HDR 2.2
    virtual MBOOL getCaptureInfo(Vector<NS3Av3::CaptureParam_T>& vCapExpParam, MINT32& hdrFrameNum);

    // getHDRCapInfo() is used to
    // 1. get exposure settings
    // 2. ISP settings
    //    a. LTM configuration per HDR input frame;
    //       used to determine whether to apply LTM of each HDR input frame
    //
    // NOTE: the number of HDR input frames equals vCapParam's size
    // valid since HDR 2.3
    virtual MBOOL getCaptureInfo(std::vector<HDR2::HDRCaptureParam>& vCapParam);

    // setCompleteCallback() registers a deferred callback, which notifies
    // the caller that the post-processing task is done.
    virtual MVOID setCompleteCallback(
            HDRProcCompleteCallback_t completeCB, MVOID* user);

    //query HDR algo type raw/yuv
    MINT32 queryHDRAlgoType(void);

private:
    // number of sensor output frames (i.e. HDR input frames)
    MINT32                      mInputFramesNum;

    // YUV buffer size
    MUINT32                     mSourceW;
    MUINT32                     mSourceH;

    // camera device ID
    MINT32                      mOpenID;

    // shot mode
    MUINT32                     mHDRShotMode;

    // sensor full size
    MSize                       mSensorSize;

    // sensor type
    MUINT32                     mSensorType;

    // HDR input buffers
    mutable Mutex               mSourceImgBufLock[HDR_MAX_INPUT_FRAME];
    sem_t                       mSourceImgBufSem[HDR_MAX_INPUT_FRAME];
    sp<IImageBuffer>            mpSourceImgBuf[HDR_MAX_INPUT_FRAME];
    //raw16
    mutable Mutex               mSourceImgBufRaw16Lock[HDR_MAX_INPUT_FRAME];
    sem_t                       mSourceImgBufRaw16Sem[HDR_MAX_INPUT_FRAME];
    sp<IImageBuffer>            mpSourceImgBufRaw16[HDR_MAX_INPUT_FRAME];

    // HDR working buffers
    // mutable Mutex            mXXXImgBufLock;
    // sem_t                    mXXXImgBufSem;
    // sp<IImageBuffer>         mpXXXImgBuf;

    // HDR result buffer
    sp<IImageBuffer>            mHdrResult;

    // HDRProc parameters
    HDRProc_ShotParam           mShotParam;

    // complete callback
    mutable Mutex               mCompleteCBLock;
    HDRProcCompleteCallback_t   mCompleteCB;
    MVOID*                      mpCompleteCBUser;

    // capture policy & priority
    int                         mCapturePolicy;
    int                         mCapturePriority;

    // indicates whether a cancelling capture event has been sent
    MBOOL                       mforceBreak;

    // option of debug mode
    MINT32                      mDebugMode;

    // Hal3A
    std::unique_ptr< NS3Av3::IHal3A, std::function<void(NS3Av3::IHal3A*)> >
        mHal3A;

    // HDR arguments for arcsoft
    // Lux Index
    double                      mLuxIndex;
    MINT32                      mScene;
    float                       mAWB[3]; // 0:R, 1:G, 2:B
    MINT32                      mBlackLevel[4];
    /**
     * Bounds of the face [left, top, right, bottom]. (-1000, -1000) represents
     * the top-left of the camera field of view, and (1000, 1000) represents the
     * bottom-right of the field of view. The width and height cannot be 0 or
     * negative. This is supported by both hardware and software face detection.
     *
     * The direction is relative to the sensor orientation, that is, what the
     * sensor sees. The direction is not affected by the rotation or mirroring
     * of CAMERA_CMD_SET_DISPLAY_ORIENTATION.
     */
    MINT32                      mRect[4]; //Face detection
    FILE_DUMP_NAMING_HINT       m_dumpNamingHint;

 private:
    // Put vendor algo API here
    // allocateProcessMemory() is used to allocate processing memory asynchronously
    MBOOL allocateProcessMemory();

    // the start routine of HDR post-processing
    static  MVOID hdrProcess(MVOID* arg);

    // the start routine of allocating HDR working buffer
    static  MVOID allocateProcessMemoryTask(MVOID* arg);

    // wait for input frames
    MVOID waitInputFrame();

    // Notify HDR process done
    MVOID notify(MBOOL ret) const;

    // Write HDR result
    MBOOL writeHDROutputFrame();
private:
    //We put threads member in the end of HDR instance
    //HDR process thread
    std::future<void>           mProcessThread;
    //HDR allocate memory thread
    std::future<void>           mMemAllocThread;
#ifdef FAKEHDR_ENABLE
    MBOOL transformToUnpackraw16(int index);
#endif
};

} // namespace NSCam

#endif // _HDR_H_
