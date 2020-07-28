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

#ifndef _MTK_CAMERA_CAPTURE_FEATURE_PIPE_UT_COMMON_H_
#define _MTK_CAMERA_CAPTURE_FEATURE_PIPE_UT_COMMON_H_

// Standard C header file
#include <chrono>
// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/def/common.h>
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam3/feature/featurePipe/ICaptureFeaturePipe.h>
// Module header file
#include <featurePipe/core/include/ImageBufferPool.h>
// Local header file
#include "../CaptureFeature_Common.h" // TODO: remove this head, so mush dependence


/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSCapture {
namespace UnitTest {

using namespace NSCam::NSCamFeature::NSFeaturePipe::NSCapture;
/*******************************************************************************
* Global Variables.
*******************************************************************************/
namespace { // TODO: what do us need to use anonymous namespace?
char * sMain1FSFilename = {"/sdcard/test/BID_INPUT_FSRAW_1_4208x3120_5260_3.raw"};
char * sMain2FSFilename = {"/sdcard/test/BID_INPUT_FSRAW_2_4208x3120_5260_3.raw"};
char * sMain1RSFilename = {"/sdcard/test/BID_INPUT_RSRAW_1_1920x1248_3600_0.raw"};
char * sMain2RSFilename = {"/sdcard/test/BID_INPUT_RSRAW_2_1920x1248_3600_0.raw"};
char * sLCSO1Filename   = {"/sdcard/test/BID_LCSO_1_BUF_384x384_768.yuv"};
char * sLCSO2Filename   = {"/sdcard/test/BID_LCSO_2_BUF_384x384_768.yuv"};

const MSize fsRawSize(4208, 3120);
const MSize rsRawSize(1920, 1248);

const MSize yuvSize(4208, 3120);
const MSize yuvPostviewSize(1920, 1080);
const MSize thumbSize(160, 128);

const MUINT32 lcsStride = 768;
const MSize lcsSize(384, 384);

const MSize depthSize(1600, 1200);
} // anonymous namespace


/*******************************************************************************
* Class Define
********************************************************************************/
class WorkingBufferHandle final: public BufferHandle
{
public:
    WorkingBufferHandle(const char* name, MINT32 format, const MSize& size, const char* path = nullptr);

    MERROR acquire(MINT usage) override;

    IImageBuffer* native() override;

    void release() override;

    MUINT32 getTransform() override;

    void dump(std::ostream& os) const;

    ~WorkingBufferHandle();

private:
    const char*     mName;
    const char*     mPath;
    const MUINT32   mFormat;
    const MSize     mSize;

    sp<IImageBufferHeap>    mpImageBufferHeap;
    sp<IImageBuffer>        mpImageBuffer;
};
//
//
class WorkingMetadataHandle final: public MetadataHandle
{
public:
    WorkingMetadataHandle();

    MERROR acquire() override;

    IMetadata* native() override;

    void release() override;

    MVOID dump(std::ostream& os) const;

private:
    IMetadata                   mMetadata;
};
//
//
class TestCallback final: public RequestCallback
{
public:
    MUINT32 getReceivedEventCount() { return mCompletedCount; }

    MUINT32 waitForDesiredCount(MUINT32 desiredCount, const std::chrono::milliseconds timeout);

public:
    void onMetaResultAvailable(android::sp<ICaptureFeatureRequest> requestPtr, IMetadata* partialMeta) override;

    void onContinue(android::sp<ICaptureFeatureRequest> requestPtr) override;

    void onAborted(android::sp<ICaptureFeatureRequest> requestPtr) override;

    void onCompleted(android::sp<ICaptureFeatureRequest> requestPtr, MERROR error) override;

    ~TestCallback();

private:
    std::atomic<MUINT32> mCompletedCount;
};


} // UnitTest
} // NSCapture
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_CAPTURE_FEATURE_PIPE_UT_COMMON_H_
