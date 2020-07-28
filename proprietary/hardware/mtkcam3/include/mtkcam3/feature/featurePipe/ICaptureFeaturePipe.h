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

#ifndef _MTK_CAMERA_FEATURE_PIPE_I_CAPTURE_FEATURE_PIPE_H_
#define _MTK_CAMERA_FEATURE_PIPE_I_CAPTURE_FEATURE_PIPE_H_

#include <mtkcam/def/common.h>
#include <utils/RefBase.h>
#include <mtkcam3/feature/featurePipe/util/VarMap.h>
#include <mtkcam3/feature/stereo/StereoCamEnum.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSCapture {

class ICaptureFeatureRequest;
class RequestCallback;

class ICaptureFeaturePipe : public virtual android::RefBase
{
public:

    enum eUsageMode
    {
        USAGE_TIME_SHARING,
        USAGE_FULL,
    };

    class UsageHint
    {
    public:
                                    UsageHint();
                                    UsageHint(eUsageMode mode);

        eUsageMode                  mMode = USAGE_FULL;
        MUINT32                     mDualMode = 0;
        MUINT64                     mSupportedScenarioFeatures = ~0;
        MINT32                      mPluginUniqueKey = 0;
        MBOOL                       mIsHidlIsp = MFALSE;
    };
public:
    // interface for PipelineNode

    static android::sp<ICaptureFeaturePipe>
                                    createInstance(
                                        const UsageHint &usageHint,
                                        const std::vector<MUINT32> sensorList
                                    );

    virtual MVOID                   init() = 0;

    virtual MVOID                   uninit() = 0;

    virtual MERROR                  enque(
                                        android::sp<ICaptureFeatureRequest>
                                    ) = 0;

    virtual MVOID                   abort(
                                        android::sp<ICaptureFeatureRequest>
                                    ) = 0;

    virtual MVOID                   setCallback(
                                        android::sp<RequestCallback>
                                    ) = 0;

    virtual MBOOL                   flush() = 0;

    virtual std::string             getStatus() =0;

    virtual android::sp<ICaptureFeatureRequest>
                                    acquireRequest() = 0;

    virtual MVOID                   releaseRequest(
                                        android::sp<ICaptureFeatureRequest>
                                    ) = 0;

    virtual                         ~ICaptureFeaturePipe() {}
protected:
    ICaptureFeaturePipe() {}
};

enum CaptureBufferID {
    BID_MAN_IN_FULL,
    BID_MAN_IN_RSZ,
    BID_MAN_IN_LCS,
    BID_MAN_IN_YUV,
    BID_MAN_OUT_YUV00,
    BID_MAN_OUT_YUV01,
    BID_MAN_OUT_YUV02,
    BID_MAN_OUT_JPEG,
    BID_MAN_OUT_POSTVIEW,
    BID_MAN_OUT_THUMBNAIL,
    BID_MAN_OUT_CLEAN,
    BID_MAN_OUT_DEPTH,
    BID_MAN_OUT_BOKEH,
    BID_MAN_OUT_RAW,
    BID_SUB_IN_FULL,
    BID_SUB_IN_RSZ,
    BID_SUB_IN_LCS,
    BID_SUB_IN_YUV,
    BID_SUB_OUT_YUV00,
    BID_SUB_OUT_YUV01,
    NUM_OF_BUFFER,
    NULL_BUFFER = 0xFF,
};

enum CaptureMetadataID {
    MID_MAN_IN_P1_DYNAMIC,
    MID_MAN_IN_APP,
    MID_MAN_IN_HAL,
    MID_MAN_OUT_APP,
    MID_MAN_OUT_HAL,
    MID_MAN_OUT_APP_PHY,
    MID_SUB_IN_P1_DYNAMIC,
    MID_SUB_IN_HAL,
    MID_SUB_OUT_APP,
    MID_SUB_OUT_HAL,
    MID_SUB_OUT_APP_PHY,
    NUM_OF_METADATA,
    NULL_METADATA = 0xFF,
};

enum CaptureFeatureFeatureID {
    FID_REMOSAIC,
    FID_ABF,
    FID_NR,
    FID_AINR,
    FID_MFNR,
    FID_FB,
    FID_HDR,
    FID_DEPTH,
    FID_BOKEH,
    FID_FUSION,
    FID_CZ,
    FID_DRE,
    FID_HFG,
    FID_DCE,
    FID_DSDN,
    FID_FB_3RD_PARTY,
    FID_MFNR_3RD_PARTY,
    FID_HDR_3RD_PARTY,
    FID_HDR2_3RD_PARTY,
    FID_DEPTH_3RD_PARTY,
    FID_BOKEH_3RD_PARTY,
    FID_FUSION_3RD_PARTY,
    FID_PUREBOKEH_3RD_PARTY,
    FID_AINR_YUV,
    FID_RELIGHTING_3RD_PARTY,
    FID_AINR_YHDR,
    FID_AIHDR,
    NUM_OF_FEATURE,
    NULL_FEATURE = 0xFF,
};

enum CaptureFeatureParameterID {
    PID_REQUEST_NUM,
    PID_FRAME_NUM,
    PID_FRAME_INDEX,
    PID_FRAME_COUNT,
    PID_MAIN_FRAME,
    PID_ENABLE_MFB,
    PID_ENABLE_HDR,
    PID_ENABLE_NEXT_CAPTURE,
    PID_ENABLE_PRERELEASE,
    PID_CSHOT_REQUEST,
    PID_VSDOF_MULTICAM,
    NUM_OF_PARAMETER,
};

typedef MUINT8 BufferID_T;
typedef MUINT8 MetadataID_T;
typedef MUINT8 FeatureID_T;
typedef MUINT8 ParameterID_T;

class MetadataHandle : public virtual android::RefBase
{
public:
    /*
     * Acquire the pointer of locked metadata
     *
     * @return the pointer of metadata
     */
    virtual MERROR                  acquire() = 0;

    virtual NSCam::IMetadata*       native() = 0;
    /*
     * Release the metadata to the caller
     */
    virtual void                    release() = 0;

    virtual                         ~MetadataHandle() { };
};

class BufferHandle : public virtual android::RefBase
{
public:

    /*
     * Acquire the pointer of locked image buffer
     *
     * @param[in] usage: the buffer usage
     * @return the pointer of image buffer
     */
    virtual MERROR                  acquire(
                                        MINT usage = eBUFFER_USAGE_HW_CAMERA_READWRITE |
                                                     eBUFFER_USAGE_SW_READ_OFTEN
                                    ) = 0;

    virtual NSCam::IImageBuffer*    native() = 0;
    /*
     * Release the metadata to the caller
     */
    virtual void                    release() = 0;

    virtual MUINT32                 getTransform() = 0;

    virtual                         ~BufferHandle() { };
};

class ICaptureFeatureRequest : public android::RefBase
{
public:

    /*
     * Add a buffer handle into request
     *
     * @param[in] buffer Id: the named buffer
     * @param[in] buffer handle: the buffer handle
     */
    virtual MVOID                   addBuffer(BufferID_T, android::sp<BufferHandle>) = 0;

    virtual android::sp<BufferHandle>
                                    getBuffer(BufferID_T) = 0;


    virtual MVOID                   addParameter(ParameterID_T, MINT32) = 0;

    virtual MINT32                  getParameter(ParameterID_T) = 0;

    /*
     * Add a metadata handle into request
     *
     * @param[in] metadata Id: the named metadata
     * @param[in] metadata handle: the metadata handle
     */
    virtual MVOID                   addMetadata(MetadataID_T, android::sp<MetadataHandle>) = 0;

    virtual android::sp<MetadataHandle>
                                    getMetadata(MetadataID_T) = 0;
    /*
     * Apply a feature into output result
     *
     * @param[in] feature Id: the named feature
     */
    virtual MVOID                   addFeature(FeatureID_T);
    virtual MVOID                   setFeatures(MUINT64);

    virtual MVOID                   setSensorIndex(MINT32, MINT32) = 0;
    virtual MVOID                   setIsPhysicalStream(MBOOL) = 0;

    virtual MINT32                  getRequestNo() = 0;
    virtual MINT32                  getFrameNo() = 0;
    virtual MBOOL                   detachResourceItems() = 0;
    virtual MVOID                   waitInference() =0;
    virtual std::string             getStatus(std::string& strDispatch) =0;


private:
};

class RequestCallback : public virtual android::RefBase
{
public:

    /*
     * ready to accept the next request to process
     *
     * @param[in] req: the request pointer
     * @param[in] paritalMeta: early callback partial result metadata
     */
    virtual void                    onMetaResultAvailable(android::sp<ICaptureFeatureRequest>, IMetadata*) = 0;

    /*
     * ready to accept the next request to process
     *
     * @param[in] req: the request pointer
     */
    virtual void                    onContinue(android::sp<ICaptureFeatureRequest>) = 0;

    /*
     * Cancel a request which have sent to plugin successfully
     * Should be notified of aborted or restored while abort a request
     *
     * @param[in] req: the request pointer
     */
    virtual void                    onAborted(android::sp<ICaptureFeatureRequest>) = 0;

    /*
     * Restore a cancelled request which has call abort() to plugin
     * Should be notified of aborted or restored while abort a request.
     * It represents the aborted request could be finished later, and
     * must call onCompleted() afther the restored request is finished.
     *
     * @param[in] req: the request pointer
     */
    virtual void                    onRestored(android::sp<ICaptureFeatureRequest>) = 0;

    /*
     * Notify a completed result and request result
     *
     * @param[in] req: the request pointer
     * @param[in] err: result status
     */
    virtual void                    onCompleted(android::sp<ICaptureFeatureRequest>, MERROR) = 0;

    virtual                         ~RequestCallback() { };
};


}; // NSCapture
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam

#undef MAKE_FEATURE_MASK_FUNC

#endif // _MTK_CAMERA_FEATURE_PIPE_I_CAPTURE_FEATURE_PIPE_H_
